#pragma once
#include <defines.h>

#define SSD1327_RESET_PIN               10

#define SSD1327_ADDRESS                 0x3C    //  Or 0x3D
#define SSD1327_WIDTH                   128
#define SSD1327_HEIGHT                  128

#define SSD1327_SET_COMMAND_LOCK        0xFD
#define SSD1327_DISPLAY_OFF             0xAE
#define SSD1327_DISPLAY_ON              0xAF
#define SSD1327_SET_CLOCK_DIV           0xB3
#define SSD1327_SET_MUX_RATIO           0xA8
#define SSD1327_SET_DISPLAY_OFFSET      0xA2
#define SSD1327_SET_START_LINE          0xA1
#define SSD1327_SET_REMAP               0xA0
#define SSD1327_SET_FUNC_SELECT_A       0xAB
#define SSD1327_SET_CONTRAST_CURRENT    0x81 // Or 0xC1 depending on board
#define SSD1327_SET_PHASE_LENGTH        0xB1
#define SSD1327_SET_PRECHARGE           0xBC
#define SSD1327_SET_VCOMH               0xBE
#define SSD1327_SET_FUNC_SELECT_B       0xD5
#define SSD1327_SET_SECOND_PRECHARGE    0xB6
#define SSD1327_SET_NORMAL_DISPLAY      0xA6
#define SSD1327_SET_COL_ADDR            0x15
#define SSD1327_SET_ROW_ADDR            0x75
#define SSD1327_SET_MASTER_CONTRAST     0xC1
#define SSD1327_SET_MASTER_CURRENT      0xC7
#define SSD1327_SET_GRAYSCALE_TABLE     0xB8