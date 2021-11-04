#pragma once
//
// Z M O D E M . H     Manifest constants for ZMODEM
// application to application file transfer protocol
// 05-23-87  Chuck Forsberg Omen Technology Inc
//
// Adapted 1/11/2021 by DeanoC

enum class HeaderType : uint8_t {
	ZBIN = 'A',      // Binary frame indicator
	ZHEX = 'B',      // HEX frame indicator
	ZBIN32 = 'C',    // Binary frame with 32 bit FCS
};

/* Frame types (see array "frametypes" in zm.c) */
enum class FrameType : uint8_t {
	ZRQINIT = 0,    // Request receive init
	ZRINIT = 1,    // Receive init
	ZSINIT = 2,      // Send init sequence (optional)
	ZACK = 3,        // ACK to above
	ZFILE = 4,      // File name from sender
	ZSKIP = 5,      // To sender: skip this file
	ZNAK = 6,        // Last packet was garbled
	ZABORT = 7,      // Abort batch transfers
	ZFIN = 8,        // Finish session
	ZRPOS = 9,      // Resume data trans at this position
	ZDATA = 10,      // Data packet(s) follow
	ZEOF = 11,      // End of file
	ZFERR = 12,      // File Error Detected
	ZCRC = 13,      // Request for file CRC and response
	ZCHALLENGE = 14,// Receiver's Challenge
	ZCOMPL = 15,    // Request is complete
	ZCAN = 16,      // Other end canned session with CAN*5
	ZFREECNT = 17,  // Request for free bytes on filesystem
	ZCOMMAND = 18,  // Command from sending program
	ZSTDERR = 19,    // Output to standard error, data follows
};

enum ZModemResult : uint16_t {
	// ZDLE sequences
	ZCRCE = 'h',  // CRC next, frame ends, header packet follows
	ZCRCG = 'i',  // CRC next, frame continues nonstop
	ZCRCQ = 'j',  // CRC next, frame continues, ZACK expected
	ZCRCW = 'k',  // CRC next, ZACK expected, end of frame
	ZRUB0 = 'l',  // Translate to rubout 0177
	ZRUB1 = 'm',  // Translate to rubout 0377

	// zdlread return values (internal)
	GOTOR =  0x100, // 0400
	GOTCRCE =  (ZCRCE|GOTOR),  	// ZDLE-ZCRCE received
	GOTCRCG =  (ZCRCG|GOTOR),  	// ZDLE-ZCRCG received
	GOTCRCQ =  (ZCRCQ|GOTOR),  	// ZDLE-ZCRCQ received
	GOTCRCW =  (ZCRCW|GOTOR),  	// ZDLE-ZCRCW received
	GOTCAN =   (GOTOR|030),  		// CAN*5 seen

	EOF = 0x8000,
	SKIP = 0x8001,

	SUCCESS = 0,
	ERROR = 0xFFFF,
	CRC_ERROR = 0xFFFE,

};

/* Byte positions within header array */
#define ZF0  3  /* First flags byte */
#define ZF1  2
#define ZF2  1
#define ZF3  0
#define ZP0  0  /* Low order 8 bits of position */
#define ZP1  1
#define ZP2  2
#define ZP3  3  /* High order 8 bits of file position */

/* Bit Masks for ZRINIT flags byte ZF0 */
#define CANFDX  01  /* Rx can send and receive true FDX */
#define CANOVIO  02  /* Rx can receive data during disk I/O */
#define CANBRK  04  /* Rx can send a break signal */
#define CANCRY  010  /* Receiver can decrypt */
#define CANLZW  020  /* Receiver can uncompress */
#define CANFC32  040  /* Receiver can use 32 bit Frame Check */
#define ESCCTL 0100  /* Receiver expects ctl chars to be escaped */
#define ESC8   0200  /* Receiver expects 8th bit to be escaped */

/* Parameters for ZSINIT frame */
#define ZATTNLEN 32  /* Max length of attention string */
/* Bit Masks for ZSINIT flags byte ZF0 */
#define TESCCTL 0100  /* Transmitter expects ctl chars to be escaped */
#define TESC8   0200  /* Transmitter expects 8th bit to be escaped */

/* Parameters for ZFILE frame */
/* Conversion options one of these in ZF0 */
#define ZCBIN  1  /* Binary transfer - inhibit conversion */
#define ZCNL  2  /* Convert NL to local end of line convention */
#define ZCRESUM  3  /* Resume interrupted file transfer */
/* Management include options, one of these ored in ZF1 */
#define ZMSKNOLOC  0200  /* Skip file if not present at rx */
/* Management options, one of these ored in ZF1 */
#define ZMMASK  037  /* Mask for the choices below */
#define ZMNEWL  1  /* Transfer if source newer or longer */
#define ZMCRC  2  /* Transfer if different file CRC or length */
#define ZMAPND  3  /* Append contents to existing file (if any) */
#define ZMCLOB  4  /* Replace existing file */
#define ZMNEW  5  /* Transfer if source newer */
/* Number 5 is alive ... */
#define ZMDIFF  6  /* Transfer if dates or lengths different */
#define ZMPROT  7  /* Protect destination file */
/* Transport options, one of these in ZF2 */
#define ZTLZW  1  /* Lempel-Ziv compression */
#define ZTCRYPT  2  /* Encryption */
#define ZTRLE  3  /* Run Length encoding */
/* Extended options for ZF3, bit encoded */
#define ZXSPARS  64  /* Encoding for sparse file operations */

/* Parameters for ZCOMMAND frame ZF0 (otherwise 0) */
#define ZCACK1  1  /* Acknowledge, then do command */

#define ASCII_EOT 3
#define ASCII_BACKSPACE 8
#define ASCII_LF 10
#define ASCII_CR 13
#define ASCII_CAN 24

#define MODEM_XOFF 19
#define MODEM_XON 17
#define ZMODEM_PAD 42
#define ZMODEM_IDLE ASCII_CAN


#ifndef HOWMANY
#define HOWMANY 133
#endif
#define KSIZE 1024

#define zperr
//debug_printf

struct ZModem {
	int Topipe = 0;
	uint8_t sectorBuffer[KSIZE + 1];
	int Zctlesc;    /* Encode control characters */
	int Zrwindow = 1400;  /* RX window size (controls garbage count) */

	/* Globals used by ZMODEM functions */
	uint32_t Rxcount;    // Count of data bytes received
	uint8_t Rxhdr[4];  // Received header
	uint8_t Txhdr[4];  // Transmitted header
	uint32_t Rxpos;    	// Received file position
	bool Crc32t; 		// 32 bit CRC being transmitted
	bool Crc32;    	// Display flag indicating 32 bit CRC being received
	int Znulls;    	// Number of nulls to send at beginning of ZDATA hdr
	uint8_t Attn[ZATTNLEN + 1];  // Attention string rx sends to tx on err

	FrameType Rxtype;    // Type of header received
	FrameType tryzhdrtype = FrameType::ZRINIT;  // Header type to send corresponding to Last rx close
	HeaderType Rxframeind;    // ZBIN ZBIN32, or ZHEX type of frame received

	FrameType tryz();
	ZModemResult zrdata(uint8_t *buf, uint32_t length);
	ZModemResult zrdat32(uint8_t *buf, uint32_t length);

	uint8_t noxrd7() const;
	uint8_t zgeth1() const;
	uint8_t zgethex() const;

	ZModemResult zdlread() const;

	static void zputhex(uint8_t bin);
	FrameType zrhhdr(uint8_t *hdr);
	FrameType zrbhdr(uint8_t *hdr);
	FrameType zrbhdr32(uint8_t *hdr);
	FrameType zgethdr(uint8_t *hdr);

	void ackbibi();

	// Store long integer pos in Txhdr
	void stohdr(uint32_t pos) {
		Txhdr[ZP0] = pos;
		Txhdr[ZP1] = pos >> 8;
		Txhdr[ZP2] = pos >> 16;
		Txhdr[ZP3] = pos >> 24;
	}
	/* Recover a long integer from a header */
	uint32_t rclhdr(uint8_t const* hdr)
	{
		uint32_t l;

		l = (hdr[ZP3] & 0377);
		l = (l << 8) | (hdr[ZP2] & 0377);
		l = (l << 8) | (hdr[ZP1] & 0377);
		l = (l << 8) | (hdr[ZP0] & 0377);
		return l;
	}
	void zmputs(uint8_t const* s);

	void zWriteByte(uint8_t c) const;
	void zshhdr(FrameType type, uint8_t *hdr);

	ZModemResult rzfile();
	ZModemResult rzfiles();

};
