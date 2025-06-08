#pragma once
#include <defines.h>

#define SH1107_ADDRESS              0x3D    //  Or 0x3C
#define SH1107_WIDTH                128
#define SH1107_HEIGHT               128

#define SH1107_BLACK                0       //  Draw "off" pixels
#define SH1107_WHITE                1       //  Draw "on"  pixels
#define SH1107_INVERSE              2       //  Invert pixels

#define SH1107_MEMORYMODE           0x20    //  Draw screen: 0x20 - left to right ; 0x21 - top to bottom
#define SH1107_COLUMNADDR           0x21    //  "Set Column Address" command
#define SH1107_PAGEADDR             0x22    //  "Set Page Address" command
#define SH1107_CONTRAST             0x81    //  "Set Contrast" command
#define SH1107_CHARGEPUMP           0x8D    //  Followed by: 0x14 - enable ; 0x10 - disable ; generates power for OLED
#define SH1107_SEGREMAP             0xA0    //  Followed by: 0xA0 - normal display ; 0xA1 - horizontal flip
#define SH1107_DISPLAYON_RESUME     0xA4    //  Resume display for "all pixels on" state; restores content from GDDM internal memory
#define SH1107_DISPLAYALLON         0xA5    //  Draw ALL pixels "on"; ignores GDDM internal memory
#define SH1107_NORMALDISPLAY        0xA6    //  Display in normal mode: "1" in GDDM internal memory means the pixel is "on"
#define SH1107_INVERTDISPLAY        0xA7    //  Display in inverse mode: "1" in GDDM internal memoery means the pixel is "off"
#define SH1107_MULTIPLEX            0xA8    //  Number of rows that are scanned; "127" for a 128x128 display
#define SH1107_DCDC                 0xAD    //  Followed by: 0x8B - turn on ; 0x8A - turn off internal voltage converter
#define SH1107_DISPLAYOFF           0xAE    //  Puts display in sleep mode ("off")
#define SH1107_DISPLAYON            0xAF    //  Restores display from sleep mode into normal mode ("on")
#define SH1107_SETPAGEADDR          0xB0    //  0xB0 to 0xBG - single byte command, pages from 0 to 15
#define SH1107_COMSCANINC           0xC0    //  Displays content from top to bottom in the standard mode
#define SH1107_COMSCANDEC           0xC8    //  Displays content from bottom to top in the standard mode
#define SH1107_DISPLAYOFFSET        0xD3    //  Followed by a data byte, shifts display verticaly
#define SH1107_DISPLAYCLOCKDIV      0xD5    //  Display refresh rate - followed by 0x80 should be enough most of the time
#define SH1107_PRECHARGE            0xD9    //  Controls brightness and contrast - followed by 0xF1 should be enough most of the time
#define SH1107_COMPINS              0xDA    //  Read datasheet, it's to much for a simple comment - followed by 0x34 should be enough for 128x64 or 128x128
#define SH1107_VCOMDETECT           0xDB    //  Reference voltage for the pixel driving circuit
#define SH1107_DISPLAYSTARTLINE     0xDC    //  Display start RAM address - allows scrolling without redrawing the whole screen 0x00 - 0x00 to 0x7F for 128x128
#define SH1107_LOWCOLUMN            0x00    //  Datasheet
#define SH1107_HIGHCOLUMN           0x10    //  Datasheet
#define SH1107_STARTLINE            0x40    //  Datasheet