#include "core/core.h"
#include "zmodem.hpp"
#include "crc.h"
#include "dbg/print.h"
#include "ipi3_os_server.hpp"

extern "C" uint8_t readNextByte();
extern "C" void writeByte(uint8_t c);

// Send ZMODEM HEX header hdr of type type
void ZModem::zshhdr(FrameType type, uint8_t *hdr) {
	uint16_t crc;

	//		vfile("zshhdr: %s %lx", frametypes[type+FTOFFSET], rclhdr(hdr));
	writeByte(ZMODEM_PAD);
	writeByte(ZMODEM_PAD);
	writeByte(ZMODEM_IDLE);
	writeByte((char) HeaderType::ZHEX);
	zputhex((uint8_t) type);
	Crc32t = false;

	crc = updcrc((uint8_t) type, 0);
	for (int n = 4; --n >= 0; ++hdr) {
		zputhex(*hdr);
		crc = updcrc((0377 & *hdr), crc);
	}
	crc = updcrc(0, updcrc(0, crc));
	zputhex((char) (crc >> 8));
	zputhex((char) crc);

	/* Make it printable on remote machine */
	writeByte(015);
	writeByte(012);
	/*
	 * Uncork the remote in case a fake XOFF has stopped data flow
	 */
	if (type != FrameType::ZFIN && type != FrameType::ZACK) {
		writeByte(021);
	}
	//		flushmo();
}

/*
 * Receive a file with ZMODEM protocol
 *  Assumes file name frame is in secbuf
 */
ZModemResult ZModem::rzfile() {
	int n = 20;
	uint32_t rxbytes = 0;

	for (;;) {
		stohdr(rxbytes);
		zshhdr(FrameType::ZRPOS, Txhdr);

		nxthdr:
		switch (zgethdr(Rxhdr)) {
			default:
				return ZModemResult::ERROR;
			case FrameType::ZNAK:
				if ( --n < 0) {
					return ZModemResult::ERROR;
				}
			case FrameType::ZFILE:
				zrdata(sectorBuffer, KSIZE);
				continue;
			case FrameType::ZEOF:
				if (rclhdr(Rxhdr) != rxbytes) {
					goto nxthdr;
				}
//				if (closeit()) {
//					tryzhdrtype = FrameType::ZFERR;
//					return ZModemResult::ERROR;
//				}
				//vfile("rzfile: normal EOF");
				return ZModemResult::EOF;
			case FrameType::ZDATA:
				if (rclhdr(Rxhdr) != rxbytes) {
					if (--n < 0) {
						return ZModemResult::ERROR;
					}
					zmputs(Attn);
					continue;
				}
moredata:
				switch (zrdata(sectorBuffer, KSIZE)) {
					case ZModemResult::GOTCAN:
						return ZModemResult::ERROR;
					case ZModemResult::GOTCRCW:
						n = 20;
//						PutSizedData(Rxcount, sectorBuffer);
						rxbytes += Rxcount;
						stohdr(rxbytes);
						zshhdr(FrameType::ZACK, Txhdr);
						writeByte(MODEM_XON);
						goto nxthdr;
					case ZModemResult::GOTCRCQ:
						n = 20;
//						PutSizedData(Rxcount, sectorBuffer);
						rxbytes += Rxcount;
						stohdr(rxbytes);
						zshhdr(FrameType::ZACK, Txhdr);
						goto moredata;
					case ZModemResult::GOTCRCG:
						n = 20;
//						PutSizedData(Rxcount, sectorBuffer);
						rxbytes += Rxcount;
						goto moredata;
					case ZModemResult::GOTCRCE: n = 20;
//						PutSizedData(Rxcount, sectorBuffer);
						rxbytes += Rxcount;
						goto nxthdr;
					case ZModemResult::ERROR:
						if (--n < 0) {
							return ZModemResult::ERROR;
						}
						continue;
					default:
						break;
				}
		}
	}
}
/*
 * Receive 1 or more files with ZMODEM protocol
 */
ZModemResult ZModem::rzfiles() {
	ZModemResult c;
	for (;;) {
		switch (c = rzfile()) {
			case ZModemResult::EOF:
			case ZModemResult::SKIP:
				switch (tryz()) {
					case FrameType::ZCOMPL: return ZModemResult::SUCCESS;
					case FrameType::ZFILE: break;
					default:
						return ZModemResult::ERROR;
				}
				continue;
			case ZModemResult::ERROR:
				return ZModemResult::ERROR;
			default:
				return c;
		}
	}
}

FrameType ZModem::tryz() {
	for (int n = 15; --n >= 0;) {
		/* Set buffer length (0) and capability flags */
		stohdr(0L);
#ifdef CANBREAK
		Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO|CANBRK;
#else
		Txhdr[ZF0] = CANFC32 | CANFDX | CANOVIO;
#endif
		if (Zctlesc) {
			Txhdr[ZF0] |= TESCCTL;
		}
		zshhdr(tryzhdrtype, Txhdr);
		if (tryzhdrtype == FrameType::ZSKIP) {  /* Don't skip too far */
			tryzhdrtype = FrameType::ZRINIT;
		}  /* CAF 8-21-87 */
again:
		switch (zgethdr(Rxhdr)) {
			case FrameType::ZRQINIT: continue;
			case FrameType::ZEOF: continue;
			case FrameType::ZFILE:
				tryzhdrtype = FrameType::ZRINIT;
				if (zrdata(sectorBuffer, KSIZE) == GOTCRCW) {
					return FrameType::ZFILE;
				}
				zshhdr(FrameType::ZNAK, Txhdr);
				goto again;
			case FrameType::ZSINIT:
				Zctlesc = TESCCTL & Rxhdr[ZF0];
				if (zrdata(Attn, ZATTNLEN) == GOTCRCW) {
					zshhdr(FrameType::ZACK, Txhdr);
					goto again;
				}
				zshhdr(FrameType::ZNAK, Txhdr);
				goto again;
			case FrameType::ZFREECNT:
				stohdr(0xFFFFFFFF);
				zshhdr(FrameType::ZNAK, Txhdr);
				goto again;
			case FrameType::ZCOMMAND:
				return FrameType::ZCOMPL;
			case FrameType::ZCOMPL: goto again;
			default: continue;
			case FrameType::ZFIN:
				ackbibi();
				return FrameType::ZCOMPL;
			case FrameType::ZCAN:
				return FrameType::ZCAN;
		}
	}
	return FrameType::ZRQINIT;
}

/*
 * Ack a ZFIN packet, let byegones be byegones
 */
void ZModem::ackbibi() {
	int n;

	//	vfile("ackbibi:");
	stohdr(0L);
	for (n = 3; --n >= 0;) {
		//		purgeline();
		zshhdr(FrameType::ZFIN, Txhdr);
		switch (readNextByte()) {
			case 'O': readNextByte();  /* Discard 2nd 'O' */
				return;
		}
	}
}

FrameType ZModem::zgethdr(uint8_t *hdr) {
	int n, cancount;
	FrameType hdrType = FrameType::ZRQINIT;
	n = Zrwindow;// + Baudrate;  // Max bytes before start of frame
	Rxframeind = HeaderType::ZHEX;
	Rxtype = FrameType::ZRQINIT;

startover:
	cancount = 5;
again:
	// Return immediate ERROR if ZCRCW sequence seen
	uint8_t c = readNextByte();
	if( c != (ZMODEM_PAD | 0200) && c != ZMODEM_PAD) {
		if(c == ASCII_CAN) {
gotcan:
			if (--cancount <= 0) { return FrameType::ZCAN; }
			switch (readNextByte()) {
				case ASCII_CAN:
					if (--cancount <= 0) { return FrameType::ZCAN; }
					goto again;
				case ZCRCW:
					return FrameType::ZCAN;
				default: break;
			}
		}
		// skip garbage
		if (--n == 0) { return FrameType::ZFERR; }
		goto startover;
	}

	cancount = 5;
splat:
	switch (noxrd7()) {
		case ZMODEM_PAD: goto splat;
		default:
			if (--n == 0) { return FrameType::ZFERR; }
			goto startover;
		case ZMODEM_IDLE:    // This is what we want.
			break;
	}


	switch (noxrd7()) {
		case (char) HeaderType::ZBIN:
			Rxframeind = HeaderType::ZBIN;
			Crc32 = false;
			hdrType = zrbhdr(hdr);
			break;
		case (char) HeaderType::ZBIN32:
			Crc32 = true;
			Rxframeind = HeaderType::ZBIN32;
			hdrType = zrbhdr32(hdr);
			break;
		case (char) HeaderType::ZHEX:
			Rxframeind = HeaderType::ZHEX;
			Crc32 = false;
			hdrType = zrhhdr(hdr);
			break;
		case ASCII_CAN:
			goto gotcan;
		default:
			// skip garbage
			if (--n == 0) { return FrameType::ZFERR; }
			goto startover;
	}

	Rxpos = hdr[ZP3] & 0377;
	Rxpos = (Rxpos << 8) + (hdr[ZP2] & 0377);
	Rxpos = (Rxpos << 8) + (hdr[ZP1] & 0377);
	Rxpos = (Rxpos << 8) + (hdr[ZP0] & 0377);
	return hdrType;
}

/* Receive a binary style header (type and position) */
FrameType ZModem::zrbhdr(uint8_t *hdr) {
	uint8_t c;
	uint16_t crc;

	c = zdlread();
	Rxtype = (FrameType) c;
	crc = updcrc(c, 0);

	for (int n = 4; --n >= 0; ++hdr) {
		c = zdlread();
		crc = updcrc(c, crc);
		*hdr = c;
	}
	c = zdlread();
	crc = updcrc(c, crc);

	c = zdlread();
	crc = updcrc(c, crc);
	if (crc & 0xFFFF) {
		return FrameType::ZFERR;
	}


	return Rxtype;
}

/* Receive a binary style header (type and position) with 32 bit FCS */
FrameType ZModem::zrbhdr32(uint8_t *hdr) {
	uint8_t c;
	uint32_t crc;

	c = zdlread();
	Rxtype = (FrameType) c;
	crc = 0xFFFFFFFFL;
	crc = UPDC32(c, crc);

	for (int n = 4; --n >= 0; ++hdr) {
		c = zdlread();
		crc = UPDC32(c, crc);
		*hdr = c;
	}
	for (int n = 4; --n >= 0;) {
		c = zdlread();
		crc = UPDC32(c, crc);
	}
	if (crc != 0xDEBB20E3) {
		zperr("Bad Header CRC");
		return FrameType::ZFERR;
	}
	return Rxtype;
}

/* Receive a hex style header (type and position) */
FrameType ZModem::zrhhdr(uint8_t *hdr) {
	uint8_t c;
	unsigned short crc;
	int n;

	c = zgethex();
	Rxtype = (FrameType) c;
	crc = updcrc(c, 0);

	for (n = 4; --n >= 0; ++hdr) {
		c = zgethex();
		crc = updcrc(c, crc);
		*hdr = c;
	}
	c = zgethex();
	crc = updcrc(c, crc);
	c = zgethex();
	crc = updcrc(c, crc);
	if (crc & 0xFFFF) {
		return FrameType::ZFERR;
	}

	if (readNextByte() == '\r') {  /* Throw away possible cr/lf */
		readNextByte();
	}
	return Rxtype;
}

/*
 * Receive array buf of max length with ending ZDLE sequence
 *  and CRC.  Returns the ending character or error code.
 *  NB: On errors may store length+1 bytes!
 */
ZModemResult ZModem::zrdata(uint8_t *buf, uint32_t length) {
	ZModemResult c;
	uint16_t crc;
	ZModemResult d;
	uint8_t b;

	if (Rxframeind == HeaderType::ZBIN32) {
		return zrdat32(buf, length);
	}

	crc = 0;
	Rxcount = 0;
	uint8_t *const end = buf + length;
	while (buf <= end) {
		if ((c = zdlread()) & ~0xFF) {
crcfoo:
			switch (c) {
				case GOTCRCE:
				case GOTCRCG:
				case GOTCRCQ:
				case GOTCRCW: d = c;
					b = c & 0xFF;
					crc = updcrc(b, crc);
					if ((c = zdlread()) & ~0xFF) {
						goto crcfoo;
					}
					b = c & 0xFF;
					crc = updcrc(b, crc);
					if ((c = zdlread()) & ~0xFF) {
						goto crcfoo;
					}
					b = c & 0xFF;
					crc = updcrc(b, crc);
					if ((c = zdlread()) & ~0xFF) {
						goto crcfoo;
					}
					b = c & 0xFF;
					crc = updcrc(b, crc);
					if ((c = zdlread()) & ~0xFF) {
						goto crcfoo;
					}
					b = c & 0xFF;
					crc = updcrc(b, crc);
					if (crc != 0xFFFF) {
						return ZModemResult::CRC_ERROR;
					}
					Rxcount = length - (end - buf);
					return d;
				default:
					return c;
			}
		}
		*buf++ = c;
		crc = updcrc(c, crc);
	}
	zperr("Data subpacket too long");
	return GOTCAN;
}

ZModemResult ZModem::zrdat32(uint8_t *buf, uint32_t length) {
	ZModemResult c;
	uint32_t crc;
	ZModemResult d;
	uint8_t b;

	crc = 0xFFFFFFFFL;
	Rxcount = 0;
	uint8_t *const end = buf + length;
	while (buf <= end) {
		if ((c = zdlread()) & ~0xFF) {
			crcfoo:
			switch (c) {
				case GOTCRCE:
				case GOTCRCG:
				case GOTCRCQ:
				case GOTCRCW: d = c;
					b = c & 0xFF;
					crc = UPDC32(b, crc);
					if ((c = zdlread()) & ~0xFF) {
						goto crcfoo;
					}
					b = c & 0xFF;
					crc = UPDC32(b, crc);
					if ((c = zdlread()) & ~0xFF) {
						goto crcfoo;
					}
					b = c & 0xFF;
					crc = UPDC32(b, crc);
					if ((c = zdlread()) & ~0xFF) {
						goto crcfoo;
					}
					b = c & 0xFF;
					crc = UPDC32(b, crc);
					if ((c = zdlread()) & ~0xFF) {
						goto crcfoo;
					}
					b = c & 0xFF;
					crc = UPDC32(b, crc);
					if (crc != 0xDEBB20E3) {
						return ZModemResult::CRC_ERROR;
					}
					Rxcount = length - (end - buf);
					return d;
				default:
					return c;
			}
		}
		*buf++ = c;
		crc = UPDC32(c, crc);
	}
	zperr("Data subpacket too long");
	return GOTCAN;
}

// Send a byte as two hex digits
void ZModem::zputhex(uint8_t bin) {
	static char digits[] = "0123456789abcdef";

	writeByte(digits[(bin & 0xF0) >> 4]);
	writeByte(digits[(bin) & 0xF]);
}

/*
 * Read a byte, checking for ZMODEM escape encoding
 *  including CAN*5 which represents a quick abort
 */
ZModemResult ZModem::zdlread() const {
	uint8_t c;

	again:
	switch (c = readNextByte()) {
		case ZMODEM_IDLE: break;
		case 023:
		case 0223:
		case 021:
		case 0221: goto again;
		default:
			if (Zctlesc && !(c & 0140)) {
				goto again;
			}
			return (ZModemResult) c;
	}

	again2:
	c = readNextByte();

	// if canceled read a few more to make sure
	if (c == ASCII_CAN) {
		c = readNextByte();
	}
	if (c == ASCII_CAN) {
		c = readNextByte();
	}
	if (c == ASCII_CAN) {
		c = readNextByte();
	}

	switch (c) {
		case ASCII_CAN: return GOTCAN;
		case ZCRCE:
		case ZCRCG:
		case ZCRCQ:
		case ZCRCW: return (ZModemResult) (c | GOTOR);
		case ZRUB0: return (ZModemResult) 0177;
		case ZRUB1: return (ZModemResult) 0377;
		case 023:
		case 0223:
		case 021:
		case 0221: goto again2;
		default:
			if (Zctlesc && !(c & 0140)) {
				goto again2;
			}
			if ((c & 0140) == 0100) {
				return (ZModemResult) (c ^ 0100);
			}
			break;
	}
	zperr("Bad escape sequence %x", c);
	return GOTCAN;
}

/*
 * Read a character from the modem line with timeout.
 *  Eat parity, XON and XOFF characters.
 */
uint8_t ZModem::noxrd7() const {
	uint8_t c;

	for (;;) {
		if ((c = readNextByte()) < 0) {
			return c;
		}
		switch (c &= 0177) {
			case MODEM_XON:
			case MODEM_XOFF: continue;
			default:
				if (Zctlesc && !(c & 0140)) {
					continue;
				}
			case '\r':
			case '\n':
			case ZMODEM_IDLE: return c;
		}
	}
}
uint8_t ZModem::zgeth1() const {
	uint8_t c0, c1;

	c0 = noxrd7();
	c1 = noxrd7();
	uint8_t c = c0;
	for (int i = 0; i < 2; i++) {
		uint8_t n = c - '0';
		if (n > 9) {
			n -= ('a' - ':');
		}
		if (n & ~0xF) {
			return 0; // ERROR
		}
		if (i == 1) {
			return c0 + (n << 4);
		} else {
			c0 = n;
			c = c1;
		}
	}
	return 0; // never reached
}

uint8_t ZModem::zgethex() const {
	uint8_t c;

	c = zgeth1();
	return c;
}

void ZModem::zmputs(uint8_t const * s)
{
	char c;

	while (*s) {
		switch (c = *s++) {
			case '\336':
//				Utils_BusySecondSleep(1);
				continue;
			case '\335':
				continue;
			default:
				writeByte(c);
		}
	}
}
