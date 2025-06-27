#pragma once
#include <defines.h>
#include <string.h>

#include <lwip/raw.h>
#include <lwip/icmp.h>
#include <lwip/inet_chksum.h>
#include <lwip/timeouts.h>

#include <FreeRTOS.h>
#include <event_groups.h>

#include <comm/network/wifi.h>
#include <comm/network/ping.h>

static struct raw_pcb* pingPCB;
static ip4_addr_t      pingTarget;
static u16             pingSeqNum = 0;


extern EventGroupHandle_t xSystemStateEventGroup;
static void pingPrepareEcho(struct icmp_echo_hdr* iecho, u16 len) {
    ICMPH_TYPE_SET(iecho, ICMP_ECHO);
    ICMPH_CODE_SET(iecho, 0);
    iecho->chksum = 0;
    iecho->id     = lwip_htons(0xAFAF);
    iecho->seqno  = lwip_htons(pingSeqNum++);
    memset((u8*)iecho + sizeof(*iecho), 0xA5, len - sizeof(*iecho));
    iecho->chksum = inet_chksum(iecho, len);
}

static u8 pingRecvCallback(void* arg, struct raw_pcb* pcb, struct pbuf* p, const ip_addr_t* addr) {
    struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr*)p->payload;
    if (iecho->id == lwip_htons(0xAFAF) && iecho->seqno == lwip_htons(pingSeqNum-1)) {
        // Heartbeat succeeded
        xEventGroupSetBits(xSystemStateEventGroup, WIFI_CONNECTED_BIT);
    }

    pbuf_free(p);
    return 1;
}

static void pingSendTimer(void* arg) {
    const u16 pingSize = sizeof(struct icmp_echo_hdr) + 32;
    struct pbuf* p = pbuf_alloc(PBUF_IP, pingSize, PBUF_RAM);
    if(p) {
        pingPrepareEcho((struct icmp_echo_hdr*)p->payload, pingSize);
        raw_sendto(pingPCB, p, (const ip_addr_t*)&pingTarget);
        pbuf_free(p);
    }
    
    sys_timeout(5000, pingSendTimer, NULL);
}

static void initHeartbeat(const ip4_addr_t* gw) {
    pingTarget = *gw;
    pingPCB    = raw_new(IP_PROTO_ICMP);
    raw_recv(pingPCB, pingRecvCallback, NULL);
    raw_bind(pingPCB, IP4_ADDR_ANY);

    pingSendTimer(NULL);
}