/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * GNU Unifont 16.0.04 (https://unifoundry.com/pub/unifont/unifont-16.0.04/font-builds/),
 * OTF SHA-256 0e3981ab552231b5a2a870f2b61741903a4bf25c23ef5aeb05fdced1b3c7af4d, unaltered glyphs.
 * Copyright © 1998-2025 Roman Czyborra, Paul Hardy, Qianqian Fang, Andrew Miller, Johnnie Weaver,
 * David Corbett, Nils Moskopp, Rebecca Bettencourt, Ho-Seok Ee, et al. SIL Open Font License 1.1.
 * Converter: lvgl/lv_font_conv 1.5.3 at 2107ab79b615a6ff7051796fa9f7ae52f042437b.
 * Opts: --font unifont-16.0.04.otf --size 16 --bpp 1 --format lvgl --no-compress --lv-font-name pet_font_pixel_16 -r 0x20-0x7E -r 0xA0-0xFF -r 0x2013-0x2014 -r 0x2018-0x2019 -r 0x201C-0x201D -r 0x2026 -r 0x20AC -o pet_font_pixel_16.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

#ifndef PET_FONT_PIXEL_16
#define PET_FONT_PIXEL_16 1
#endif

#if PET_FONT_PIXEL_16



/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x00,

    /* U+0021 "!" */
    0xfe, 0x40,

    /* U+0022 "\"" */
    0x8c, 0x63, 0x10,

    /* U+0023 "#" */
    0x24, 0x92, 0x7f, 0x49, 0x2f, 0xe4, 0x92, 0x40,

    /* U+0024 "$" */
    0x10, 0xfa, 0x48, 0x87, 0x03, 0x84, 0xca, 0x7c,
    0x20,

    /* U+0025 "%" */
    0x63, 0x2a, 0x63, 0x41, 0x06, 0x0b, 0x29, 0x91,
    0x18,

    /* U+0026 "&" */
    0x38, 0x89, 0x01, 0x45, 0x0a, 0x63, 0x42, 0x8c,
    0xe4,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0x2a, 0x49, 0x24, 0x91, 0x10,

    /* U+0029 ")" */
    0x89, 0x12, 0x49, 0x29, 0x40,

    /* U+002A "*" */
    0x11, 0x25, 0x51, 0xc5, 0x52, 0x44, 0x00,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x02, 0x04, 0x00,

    /* U+002C "," */
    0xd6,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x04, 0x20, 0x84, 0x20, 0x84, 0x10, 0x82, 0x00,

    /* U+0030 "0" */
    0x31, 0x28, 0x63, 0x96, 0x9c, 0x61, 0x48, 0xc0,

    /* U+0031 "1" */
    0x23, 0x28, 0x42, 0x10, 0x84, 0x27, 0xc0,

    /* U+0032 "2" */
    0x7a, 0x10, 0x41, 0x18, 0x84, 0x20, 0x83, 0xf0,

    /* U+0033 "3" */
    0x7a, 0x10, 0x41, 0x38, 0x10, 0x61, 0x89, 0xe0,

    /* U+0034 "4" */
    0x08, 0x62, 0x92, 0x8a, 0x2f, 0xc2, 0x08, 0x20,

    /* U+0035 "5" */
    0xfe, 0x08, 0x20, 0xf8, 0x10, 0x41, 0x85, 0xe0,

    /* U+0036 "6" */
    0x39, 0x08, 0x20, 0xfa, 0x18, 0x61, 0x85, 0xe0,

    /* U+0037 "7" */
    0xfc, 0x10, 0x42, 0x08, 0x21, 0x04, 0x10, 0x40,

    /* U+0038 "8" */
    0x7a, 0x18, 0x61, 0x7a, 0x18, 0x61, 0x85, 0xe0,

    /* U+0039 "9" */
    0x7a, 0x18, 0x61, 0x7c, 0x10, 0x41, 0x09, 0xc0,

    /* U+003A ":" */
    0xc0, 0xc0,

    /* U+003B ";" */
    0xc0, 0x35, 0x80,

    /* U+003C "<" */
    0x08, 0x88, 0x88, 0x20, 0x82, 0x08,

    /* U+003D "=" */
    0xfc, 0x00, 0x00, 0xfc,

    /* U+003E ">" */
    0x82, 0x08, 0x20, 0x88, 0x88, 0x80,

    /* U+003F "?" */
    0x7a, 0x10, 0x41, 0x08, 0x40, 0x00, 0x00, 0x40,

    /* U+0040 "@" */
    0x39, 0x19, 0x6b, 0xa6, 0x9a, 0x67, 0x40, 0xf0,

    /* U+0041 "A" */
    0x31, 0x28, 0xa1, 0x87, 0xf8, 0x61, 0x86, 0x10,

    /* U+0042 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x18, 0x61, 0x87, 0xe0,

    /* U+0043 "C" */
    0x7a, 0x18, 0x20, 0x82, 0x08, 0x21, 0x89, 0xe0,

    /* U+0044 "D" */
    0xf2, 0x28, 0x61, 0x86, 0x18, 0x61, 0x8b, 0xc0,

    /* U+0045 "E" */
    0xfe, 0x08, 0x20, 0xfa, 0x08, 0x20, 0x83, 0xf0,

    /* U+0046 "F" */
    0xfe, 0x08, 0x20, 0xfa, 0x08, 0x20, 0x82, 0x00,

    /* U+0047 "G" */
    0x7a, 0x18, 0x20, 0x82, 0x78, 0x61, 0x8d, 0xd0,

    /* U+0048 "H" */
    0x86, 0x18, 0x61, 0xfe, 0x18, 0x61, 0x86, 0x10,

    /* U+0049 "I" */
    0xf9, 0x08, 0x42, 0x10, 0x84, 0x27, 0xc0,

    /* U+004A "J" */
    0x3e, 0x10, 0x20, 0x40, 0x81, 0x02, 0x44, 0x90,
    0xe0,

    /* U+004B "K" */
    0x86, 0x29, 0x28, 0xc3, 0x0a, 0x24, 0x8a, 0x10,

    /* U+004C "L" */
    0x82, 0x08, 0x20, 0x82, 0x08, 0x20, 0x83, 0xf0,

    /* U+004D "M" */
    0x86, 0x1c, 0xf5, 0xb6, 0x18, 0x61, 0x86, 0x10,

    /* U+004E "N" */
    0x87, 0x1c, 0x69, 0xa6, 0x59, 0x63, 0x86, 0x10,

    /* U+004F "O" */
    0x7a, 0x18, 0x61, 0x86, 0x18, 0x61, 0x85, 0xe0,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x08, 0x20, 0x82, 0x00,

    /* U+0051 "Q" */
    0x79, 0x0a, 0x14, 0x28, 0x50, 0xa1, 0x5a, 0xcc,
    0xf0, 0x18,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x48, 0xa2, 0x8a, 0x10,

    /* U+0053 "S" */
    0x7a, 0x18, 0x20, 0x60, 0x60, 0x61, 0x89, 0xe0,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x02, 0x04, 0x08, 0x10,
    0x20,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x61, 0x85, 0xe0,

    /* U+0056 "V" */
    0x83, 0x06, 0x0a, 0x24, 0x48, 0x8a, 0x18, 0x30,
    0x20,

    /* U+0057 "W" */
    0x86, 0x18, 0x61, 0xb7, 0x5c, 0xe1, 0x86, 0x10,

    /* U+0058 "X" */
    0x86, 0x24, 0x94, 0x31, 0x44, 0xa2, 0x8a, 0x10,

    /* U+0059 "Y" */
    0x83, 0x09, 0x12, 0x42, 0x82, 0x04, 0x08, 0x10,
    0x20,

    /* U+005A "Z" */
    0xfc, 0x10, 0x42, 0x10, 0x84, 0x20, 0x83, 0xf0,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x70,

    /* U+005C "\\" */
    0x82, 0x04, 0x08, 0x20, 0x41, 0x02, 0x08, 0x10,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0xf0,

    /* U+005E "^" */
    0x31, 0x28, 0x40,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0x88, 0x80,

    /* U+0061 "a" */
    0x7a, 0x10, 0x5f, 0x86, 0x18, 0xdd,

    /* U+0062 "b" */
    0x82, 0x08, 0x2e, 0xc6, 0x18, 0x61, 0x87, 0x1b,
    0x80,

    /* U+0063 "c" */
    0x7a, 0x18, 0x20, 0x82, 0x08, 0x5e,

    /* U+0064 "d" */
    0x04, 0x10, 0x5d, 0x8e, 0x18, 0x61, 0x86, 0x37,
    0x40,

    /* U+0065 "e" */
    0x7a, 0x18, 0x7f, 0x82, 0x08, 0x5e,

    /* U+0066 "f" */
    0x19, 0x08, 0x4f, 0x90, 0x84, 0x21, 0x08,

    /* U+0067 "g" */
    0x05, 0xe8, 0xa2, 0x89, 0xc4, 0x1e, 0x86, 0x07,
    0x80,

    /* U+0068 "h" */
    0x82, 0x08, 0x2e, 0xc6, 0x18, 0x61, 0x86, 0x18,
    0x40,

    /* U+0069 "i" */
    0x20, 0x00, 0xc2, 0x10, 0x84, 0x21, 0x3e,

    /* U+006A "j" */
    0x08, 0x00, 0x30, 0x84, 0x21, 0x08, 0x43, 0x26,
    0x00,

    /* U+006B "k" */
    0x82, 0x08, 0x22, 0x92, 0x8c, 0x28, 0x92, 0x28,
    0x40,

    /* U+006C "l" */
    0x61, 0x08, 0x42, 0x10, 0x84, 0x21, 0x3e,

    /* U+006D "m" */
    0xed, 0x26, 0x4c, 0x99, 0x32, 0x64, 0xc9,

    /* U+006E "n" */
    0xbb, 0x18, 0x61, 0x86, 0x18, 0x61,

    /* U+006F "o" */
    0x7a, 0x18, 0x61, 0x86, 0x18, 0x5e,

    /* U+0070 "p" */
    0xbb, 0x18, 0x61, 0x86, 0x1c, 0x6e, 0x82, 0x00,

    /* U+0071 "q" */
    0x76, 0x38, 0x61, 0x86, 0x18, 0xdd, 0x04, 0x10,

    /* U+0072 "r" */
    0xbb, 0x18, 0x20, 0x82, 0x08, 0x20,

    /* U+0073 "s" */
    0x7a, 0x18, 0x18, 0x18, 0x18, 0x9e,

    /* U+0074 "t" */
    0x21, 0x09, 0xf2, 0x10, 0x84, 0x20, 0xc0,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0xdd,

    /* U+0076 "v" */
    0x86, 0x18, 0x52, 0x49, 0x25, 0x0c,

    /* U+0077 "w" */
    0x83, 0x26, 0x4c, 0x99, 0x32, 0x64, 0xb6,

    /* U+0078 "x" */
    0x86, 0x24, 0x8c, 0x51, 0x28, 0xa1,

    /* U+0079 "y" */
    0x86, 0x18, 0x61, 0x85, 0x33, 0x41, 0x05, 0xe0,

    /* U+007A "z" */
    0xfc, 0x10, 0x84, 0x21, 0x08, 0x3f,

    /* U+007B "{" */
    0x34, 0x42, 0x44, 0x84, 0x24, 0x44, 0x30,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0xc2, 0x44, 0x42, 0x12, 0x44, 0x24, 0xc0,

    /* U+007E "~" */
    0x63, 0x2a, 0x30,

    /* U+00A0 " " */
    0x00,

    /* U+00A1 "¡" */
    0x9f, 0xc0,

    /* U+00A2 "¢" */
    0x10, 0x21, 0xf4, 0x99, 0x12, 0x24, 0xbe, 0x10,
    0x20,

    /* U+00A3 "£" */
    0x1c, 0x40, 0x81, 0x0f, 0x84, 0x08, 0x10, 0x7d,
    0x84,

    /* U+00A4 "¤" */
    0x85, 0xe4, 0xa1, 0x81, 0x27, 0xa1,

    /* U+00A5 "¥" */
    0x82, 0x88, 0xa0, 0x8f, 0xe2, 0x3f, 0x88, 0x10,
    0x20,

    /* U+00A6 "¦" */
    0xf3, 0xc0,

    /* U+00A7 "§" */
    0x7a, 0x18, 0x1e, 0x86, 0x07, 0x81, 0x89, 0xe0,

    /* U+00A8 "¨" */
    0x98,

    /* U+00A9 "©" */
    0x3c, 0x42, 0x99, 0xa5, 0xa1, 0xa1, 0xa5, 0x99,
    0x42, 0x3c,

    /* U+00AA "ª" */
    0x70, 0x5f, 0x17, 0x83, 0xe0,

    /* U+00AB "«" */
    0x25, 0x24, 0xa4, 0x91, 0x24, 0x92, 0x24,

    /* U+00AC "¬" */
    0xfc, 0x10, 0x41,

    /* U+00AD "­" */
    0xaa, 0xaa, 0x00, 0x01, 0x80, 0x00, 0x3a, 0x63,
    0xc2, 0x54, 0x33, 0xc9, 0x8a, 0x48, 0x72, 0x49,
    0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x03, 0xc1,
    0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x55, 0x55,

    /* U+00AE "®" */
    0x3c, 0x42, 0xb9, 0xa5, 0xa9, 0xb9, 0xa9, 0xa5,
    0x42, 0x3c,

    /* U+00AF "¯" */
    0xfc,

    /* U+00B0 "°" */
    0x69, 0x86,

    /* U+00B1 "±" */
    0x10, 0x20, 0x47, 0xf1, 0x02, 0x04, 0x00, 0xfe,

    /* U+00B2 "²" */
    0x74, 0x42, 0x64, 0x43, 0xe0,

    /* U+00B3 "³" */
    0x74, 0x42, 0xe0, 0xc5, 0xc0,

    /* U+00B4 "´" */
    0x2a, 0x00,

    /* U+00B5 "µ" */
    0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x66, 0x59,
    0x40, 0x80,

    /* U+00B6 "¶" */
    0x7f, 0xeb, 0xd7, 0xa7, 0x42, 0x85, 0x0a, 0x14,
    0x28, 0x50,

    /* U+00B7 "·" */
    0xc0,

    /* U+00B8 "¸" */
    0x38,

    /* U+00B9 "¹" */
    0x23, 0x28, 0x42, 0x13, 0xe0,

    /* U+00BA "º" */
    0x74, 0x63, 0x17, 0x03, 0xe0,

    /* U+00BB "»" */
    0x92, 0x44, 0x92, 0x25, 0x29, 0x24, 0x90,

    /* U+00BC "¼" */
    0x47, 0x24, 0x94, 0x60, 0x96, 0xd5, 0x9e, 0x10,

    /* U+00BD "½" */
    0x47, 0x24, 0x94, 0x60, 0xa3, 0x52, 0x8a, 0x70,

    /* U+00BE "¾" */
    0xc4, 0xa4, 0x8c, 0xd0, 0x96, 0xd5, 0x9e, 0x10,

    /* U+00BF "¿" */
    0x20, 0x00, 0x08, 0x41, 0x08, 0x21, 0x89, 0xe0,

    /* U+00C0 "À" */
    0x60, 0x60, 0x00, 0x31, 0x28, 0xa1, 0x87, 0xf8,
    0x61, 0x86, 0x10,

    /* U+00C1 "Á" */
    0x19, 0x80, 0x00, 0x31, 0x28, 0xa1, 0x87, 0xf8,
    0x61, 0x86, 0x10,

    /* U+00C2 "Â" */
    0x31, 0x20, 0x00, 0x31, 0x28, 0xa1, 0x87, 0xf8,
    0x61, 0x86, 0x10,

    /* U+00C3 "Ã" */
    0x66, 0x60, 0x00, 0x31, 0x28, 0xa1, 0x87, 0xf8,
    0x61, 0x86, 0x10,

    /* U+00C4 "Ä" */
    0x48, 0x00, 0x00, 0x31, 0x28, 0xa1, 0x87, 0xf8,
    0x61, 0x86, 0x10,

    /* U+00C5 "Å" */
    0x31, 0x23, 0x00, 0x31, 0x28, 0xa1, 0x87, 0xf8,
    0x61, 0x86, 0x10,

    /* U+00C6 "Æ" */
    0x3e, 0xa2, 0x44, 0x8f, 0xf2, 0x24, 0x48, 0x91,
    0x3c,

    /* U+00C7 "Ç" */
    0x7a, 0x18, 0x20, 0x82, 0x08, 0x21, 0x89, 0xe1,
    0x18,

    /* U+00C8 "È" */
    0x60, 0x60, 0x00, 0xfe, 0x08, 0x20, 0xfa, 0x08,
    0x20, 0x83, 0xf0,

    /* U+00C9 "É" */
    0x19, 0x80, 0x00, 0xfe, 0x08, 0x20, 0xfa, 0x08,
    0x20, 0x83, 0xf0,

    /* U+00CA "Ê" */
    0x31, 0x20, 0x00, 0xfe, 0x08, 0x20, 0xfa, 0x08,
    0x20, 0x83, 0xf0,

    /* U+00CB "Ë" */
    0x48, 0x00, 0x00, 0xfe, 0x08, 0x20, 0xfa, 0x08,
    0x20, 0x83, 0xf0,

    /* U+00CC "Ì" */
    0x60, 0xc0, 0x0f, 0x90, 0x84, 0x21, 0x08, 0x42,
    0x7c,

    /* U+00CD "Í" */
    0x36, 0x00, 0x0f, 0x90, 0x84, 0x21, 0x08, 0x42,
    0x7c,

    /* U+00CE "Î" */
    0x64, 0x80, 0x0f, 0x90, 0x84, 0x21, 0x08, 0x42,
    0x7c,

    /* U+00CF "Ï" */
    0x90, 0x00, 0x0f, 0x90, 0x84, 0x21, 0x08, 0x42,
    0x7c,

    /* U+00D0 "Ð" */
    0x78, 0x89, 0x0a, 0x1f, 0x28, 0x50, 0xa1, 0x44,
    0xf0,

    /* U+00D1 "Ñ" */
    0x66, 0x60, 0x00, 0x87, 0x1c, 0x69, 0xa6, 0x59,
    0x63, 0x86, 0x10,

    /* U+00D2 "Ò" */
    0x60, 0x60, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x85, 0xe0,

    /* U+00D3 "Ó" */
    0x19, 0x80, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x85, 0xe0,

    /* U+00D4 "Ô" */
    0x31, 0x20, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x85, 0xe0,

    /* U+00D5 "Õ" */
    0x66, 0x60, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x85, 0xe0,

    /* U+00D6 "Ö" */
    0x48, 0x00, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x85, 0xe0,

    /* U+00D7 "×" */
    0x85, 0x23, 0x12, 0x84,

    /* U+00D8 "Ø" */
    0x05, 0xe8, 0xa5, 0x96, 0x1a, 0x71, 0xc6, 0x1b,
    0x80,

    /* U+00D9 "Ù" */
    0x60, 0x60, 0x00, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x85, 0xe0,

    /* U+00DA "Ú" */
    0x19, 0x80, 0x00, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x85, 0xe0,

    /* U+00DB "Û" */
    0x31, 0x20, 0x00, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x85, 0xe0,

    /* U+00DC "Ü" */
    0x48, 0x00, 0x00, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x61, 0x85, 0xe0,

    /* U+00DD "Ý" */
    0x18, 0xc0, 0x00, 0x08, 0x30, 0x91, 0x24, 0x28,
    0x20, 0x40, 0x81, 0x02, 0x00,

    /* U+00DE "Þ" */
    0x82, 0x0f, 0x22, 0x86, 0x28, 0xbc, 0x82, 0x08,
    0x00,

    /* U+00DF "ß" */
    0x72, 0x29, 0x24, 0xb2, 0x28, 0x61, 0xa6, 0x60,

    /* U+00E0 "à" */
    0x60, 0x60, 0x00, 0x7a, 0x10, 0x5f, 0x86, 0x18,
    0xdd,

    /* U+00E1 "á" */
    0x19, 0x80, 0x00, 0x7a, 0x10, 0x5f, 0x86, 0x18,
    0xdd,

    /* U+00E2 "â" */
    0x31, 0x20, 0x00, 0x7a, 0x10, 0x5f, 0x86, 0x18,
    0xdd,

    /* U+00E3 "ã" */
    0x66, 0x60, 0x00, 0x7a, 0x10, 0x5f, 0x86, 0x18,
    0xdd,

    /* U+00E4 "ä" */
    0x48, 0x00, 0x00, 0x7a, 0x10, 0x5f, 0x86, 0x18,
    0xdd,

    /* U+00E5 "å" */
    0x31, 0x23, 0x00, 0x01, 0xe8, 0x41, 0x7e, 0x18,
    0x63, 0x74,

    /* U+00E6 "æ" */
    0x7d, 0x24, 0x4b, 0xf9, 0x12, 0x24, 0xbe,

    /* U+00E7 "ç" */
    0x7a, 0x18, 0x20, 0x82, 0x08, 0x5e, 0x11, 0x80,

    /* U+00E8 "è" */
    0x60, 0x60, 0x00, 0x7a, 0x18, 0x7f, 0x82, 0x08,
    0x5e,

    /* U+00E9 "é" */
    0x19, 0x80, 0x00, 0x7a, 0x18, 0x7f, 0x82, 0x08,
    0x5e,

    /* U+00EA "ê" */
    0x31, 0x20, 0x00, 0x7a, 0x18, 0x7f, 0x82, 0x08,
    0x5e,

    /* U+00EB "ë" */
    0x48, 0x00, 0x00, 0x7a, 0x18, 0x7f, 0x82, 0x08,
    0x5e,

    /* U+00EC "ì" */
    0xc1, 0x80, 0x06, 0x10, 0x84, 0x21, 0x09, 0xf0,

    /* U+00ED "í" */
    0x36, 0x00, 0x06, 0x10, 0x84, 0x21, 0x09, 0xf0,

    /* U+00EE "î" */
    0x64, 0x80, 0x06, 0x10, 0x84, 0x21, 0x09, 0xf0,

    /* U+00EF "ï" */
    0x90, 0x00, 0x06, 0x10, 0x84, 0x21, 0x09, 0xf0,

    /* U+00F0 "ð" */
    0x64, 0x62, 0x91, 0x05, 0xf8, 0x61, 0x86, 0x18,
    0x5e,

    /* U+00F1 "ñ" */
    0x66, 0x60, 0x00, 0xbb, 0x18, 0x61, 0x86, 0x18,
    0x61,

    /* U+00F2 "ò" */
    0x60, 0x60, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x5e,

    /* U+00F3 "ó" */
    0x19, 0x80, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x5e,

    /* U+00F4 "ô" */
    0x31, 0x20, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x5e,

    /* U+00F5 "õ" */
    0x66, 0x60, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x5e,

    /* U+00F6 "ö" */
    0x48, 0x00, 0x00, 0x7a, 0x18, 0x61, 0x86, 0x18,
    0x5e,

    /* U+00F7 "÷" */
    0x30, 0x00, 0x3f, 0x00, 0x03, 0x00,

    /* U+00F8 "ø" */
    0x05, 0xe8, 0xe5, 0xa6, 0x9c, 0x71, 0x7a, 0x00,

    /* U+00F9 "ù" */
    0x60, 0x60, 0x00, 0x86, 0x18, 0x61, 0x86, 0x18,
    0xdd,

    /* U+00FA "ú" */
    0x19, 0x80, 0x00, 0x86, 0x18, 0x61, 0x86, 0x18,
    0xdd,

    /* U+00FB "û" */
    0x31, 0x20, 0x00, 0x86, 0x18, 0x61, 0x86, 0x18,
    0xdd,

    /* U+00FC "ü" */
    0x48, 0x00, 0x00, 0x86, 0x18, 0x61, 0x86, 0x18,
    0xdd,

    /* U+00FD "ý" */
    0x19, 0x80, 0x00, 0x86, 0x18, 0x61, 0x85, 0x33,
    0x41, 0x05, 0xe0,

    /* U+00FE "þ" */
    0x82, 0x08, 0x2e, 0xc6, 0x18, 0x61, 0x87, 0x1b,
    0xa0, 0x80,

    /* U+00FF "ÿ" */
    0x48, 0x00, 0x00, 0x86, 0x18, 0x61, 0x85, 0x33,
    0x41, 0x05, 0xe0,

    /* U+2013 "–" */
    0xfc,

    /* U+2014 "—" */
    0xff,

    /* U+2018 "‘" */
    0x6b,

    /* U+2019 "’" */
    0xd6,

    /* U+201C "“" */
    0x46, 0x28, 0xb3,

    /* U+201D "”" */
    0xcd, 0x14, 0x62,

    /* U+2026 "…" */
    0x92,

    /* U+20AC "€" */
    0x18, 0x94, 0x3e, 0x43, 0xe4, 0x10, 0x24, 0x60
};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 128, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 128, .box_w = 1, .box_h = 10, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 128, .box_w = 5, .box_h = 4, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 6, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 14, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 32, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 128, .box_w = 1, .box_h = 4, .ofs_x = 4, .ofs_y = 8},
    {.bitmap_index = 42, .adv_w = 128, .box_w = 3, .box_h = 12, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 47, .adv_w = 128, .box_w = 3, .box_h = 12, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 52, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 59, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 66, .adv_w = 128, .box_w = 2, .box_h = 4, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 67, .adv_w = 128, .box_w = 4, .box_h = 1, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 68, .adv_w = 128, .box_w = 2, .box_h = 1, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 128, .box_w = 5, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 128, .box_w = 2, .box_h = 5, .ofs_x = 3, .ofs_y = 3},
    {.bitmap_index = 158, .adv_w = 128, .box_w = 2, .box_h = 9, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 161, .adv_w = 128, .box_w = 5, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 128, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 171, .adv_w = 128, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 209, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 128, .box_w = 5, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 313, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 331, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 128, .box_w = 3, .box_h = 12, .ofs_x = 4, .ofs_y = -1},
    {.bitmap_index = 411, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 419, .adv_w = 128, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 424, .adv_w = 128, .box_w = 6, .box_h = 3, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 427, .adv_w = 128, .box_w = 7, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 428, .adv_w = 128, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = 10},
    {.bitmap_index = 430, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 445, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 451, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 460, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 466, .adv_w = 128, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 482, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 491, .adv_w = 128, .box_w = 5, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 498, .adv_w = 128, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 507, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 516, .adv_w = 128, .box_w = 5, .box_h = 11, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 523, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 530, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 536, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 542, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 550, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 558, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 564, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 570, .adv_w = 128, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 577, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 583, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 589, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 596, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 602, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 610, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 616, .adv_w = 128, .box_w = 4, .box_h = 13, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 623, .adv_w = 128, .box_w = 1, .box_h = 14, .ofs_x = 4, .ofs_y = -2},
    {.bitmap_index = 625, .adv_w = 128, .box_w = 4, .box_h = 13, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 632, .adv_w = 128, .box_w = 7, .box_h = 3, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 635, .adv_w = 128, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 636, .adv_w = 128, .box_w = 1, .box_h = 10, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 638, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 647, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 656, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 662, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 671, .adv_w = 128, .box_w = 1, .box_h = 10, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 673, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 681, .adv_w = 128, .box_w = 4, .box_h = 2, .ofs_x = 2, .ofs_y = 13},
    {.bitmap_index = 682, .adv_w = 128, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 692, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 697, .adv_w = 128, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 704, .adv_w = 128, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 707, .adv_w = 256, .box_w = 16, .box_h = 16, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 739, .adv_w = 128, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 128, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = 11},
    {.bitmap_index = 750, .adv_w = 128, .box_w = 4, .box_h = 4, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 752, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 760, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 765, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 770, .adv_w = 128, .box_w = 3, .box_h = 3, .ofs_x = 3, .ofs_y = 10},
    {.bitmap_index = 772, .adv_w = 128, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 782, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 792, .adv_w = 128, .box_w = 2, .box_h = 1, .ofs_x = 3, .ofs_y = 5},
    {.bitmap_index = 793, .adv_w = 128, .box_w = 3, .box_h = 2, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 794, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 799, .adv_w = 128, .box_w = 5, .box_h = 7, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 804, .adv_w = 128, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 811, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 819, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 827, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 835, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 843, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 854, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 865, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 876, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 887, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 898, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 909, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 918, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 927, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 938, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 949, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 960, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 971, .adv_w = 128, .box_w = 5, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 980, .adv_w = 128, .box_w = 5, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 989, .adv_w = 128, .box_w = 5, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 998, .adv_w = 128, .box_w = 5, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1007, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1016, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1027, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1038, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1049, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1060, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1071, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1082, .adv_w = 128, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 1086, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1095, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1106, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1117, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1128, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1139, .adv_w = 128, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1152, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1161, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1169, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1178, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1187, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1196, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1205, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1214, .adv_w = 128, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1224, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1231, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1239, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1248, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1257, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1266, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1275, .adv_w = 128, .box_w = 5, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1283, .adv_w = 128, .box_w = 5, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1291, .adv_w = 128, .box_w = 5, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1299, .adv_w = 128, .box_w = 5, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1307, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1316, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1325, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1334, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1343, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1352, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1361, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1370, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 1376, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1384, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1393, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1402, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1411, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1420, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1431, .adv_w = 128, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1441, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1452, .adv_w = 128, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 1453, .adv_w = 128, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1454, .adv_w = 128, .box_w = 2, .box_h = 4, .ofs_x = 3, .ofs_y = 8},
    {.bitmap_index = 1455, .adv_w = 128, .box_w = 2, .box_h = 4, .ofs_x = 3, .ofs_y = 8},
    {.bitmap_index = 1456, .adv_w = 128, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 1459, .adv_w = 128, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 1462, .adv_w = 128, .box_w = 7, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1463, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_2[] = {
    0x00, 0x01, 0x05, 0x06, 0x09, 0x0a, 0x13, 0x99
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 160, .range_length = 96, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 8211, .range_length = 154, .glyph_id_start = 192,
        .unicode_list = unicode_list_2, .glyph_id_ofs_list = NULL, .list_length = 8, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 3,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t pet_font_pixel_16 = {
#else
lv_font_t pet_font_pixel_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if LV_VERSION_CHECK(9, 6, 0) || LVGL_VERSION_MAJOR >= 10
    .cap_height = 10,           /*Cap height of the font*/
    .x_height = 8,               /*x-height of the font*/
#endif
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 0,
    .underline_thickness = 0,
#endif

#if LV_VERSION_CHECK(9, 3, 0) || LVGL_VERSION_MAJOR >= 10
    .static_bitmap = 1,    /*Bitmaps are stored as const so they are always static if not compressed */
#endif

    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if PET_FONT_PIXEL_16*/
