#ifndef AD9361_DEFINES_H
#define AD9361_DEFINES_H


#include <algorithm>


////////////////////////////////////////////////////////////
// the following macros evaluate to a compile time constant
// macros By Tom Torfs - donated to the public domain

/* turn a numeric literal into a hex constant
(avoids problems with leading zeroes)
8-bit constants max value 0x11111111, always fits in unsigned long
*/
#define HEX__(n) 0x##n##LU

/* 8-bit conversion function */
#define B8__(x) ((x&0x0000000FLU)?1:0) \
+((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)

/* for upto 8-bit binary constants */
#define B8(d) ((unsigned char)B8__(HEX__(d)))
////////////////////////////////////////////////////////////



#endif // AD9361_DEFINES_H

