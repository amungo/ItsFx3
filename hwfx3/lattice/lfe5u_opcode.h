#ifndef __LFE5U_OPCODE_H__
#define __LFE5U_OPCODE_H__

/************************************************************************
*
* SSPI Opcode Table
*
* Version 0.1.1
*		- add Opcode HCOMMENT 0x
*************************************************************************/


/* transmission related opcode def */
/* having CSTOGGLE, TRANSOUT, TRANSIN, RUNCLOCK, and ENDTRAN without STARTTRAN will result fail */
#define	STARTTRAN		0x10
#define	CSTOGGLE		0x11
#define	TRANSOUT		0x12
#define	TRANSIN			0x13
#define	RUNCLOCK		0x14
#define TRSTTOGGLE		0x15;
#define ENDTRAN			0x1F


/* data related opcode definition */

#define	MASK			0x21
#define	ALGODATA		0x22	/* DATA directly in algo */
#define PROGDATA		0x25	/* DATA from memory */
#define RESETDATA		0x26
#define PROGDATAEH		0x27
/* process related opcode definition */
#define	WAIT			0x40
#define	REPEAT			0x41
#define	ENDREPEAT		0x42
#define	LOOP			0x43
#define	ENDLOOP			0x44


/* following opcode is reserved for channels: */
/* 0x50 - 0x5F for CH0 - CH15, respectively */
/*
#define	CH0				0x50
#define	CH1				0x51
#define	CH2				0x52
#define	CH3				0x53
#define	CH4				0x54
#define	CH5				0x55
#define	CH6				0x56
#define	CH7				0x57
#define	CH8				0x58
#define	CH9				0x59
#define	CHA				0x5A
#define	CHB				0x5B
#define	CHC				0x5C
#define	CHD				0x5D
#define	CHE				0x5E
#define	CHF				0x5F
*/

/************************************************************************
* if your SPI system does not allow you to drive free clock,
* a workaround may be pulsing CS of another "unused" channel
* so the current channel still see the clock while CS is high.
* However, that should be handled in hardware layer.
************************************************************************/

#define	STARTOFALGO		0x60
#define	ENDOFALGO		0x61


#define HCOMMENT		0xA0
#define HENDCOMMENT		0xA1
#define ALGOID			0xA2
#define VERSION			0xA3	/* version */
#define BUFFERREQ		0xA4	/* buffer required, 2 bytes attached to it */
#define STACKREQ		0xA5	/* stack required */
#define MASKBUFREQ		0xA6	/* mask buffer required */
#define HCHANNEL		0xA7
#define HEADERCRC		0xA8	/* header CRC check */
#define COMPRESSION		0xA9
#define HDATASET_NUM	0xAA
#define HTOC			0xAB

/* process definition */
#define PROC_COMPLETE	0
#define PROC_FAIL		1
#define PROC_OVER		2

#endif // __LFE5U_OPCODE_H__
