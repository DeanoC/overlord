#include "core/core.h"
#include "zmodem.hpp"
#include "crc.h"
#include "os_heap.hpp"
#include "host_interface.hpp"
#include "os/ipi3_os_server.hpp"
#include "dbg/ansi_escapes.h"
#include "zynqps8/dma/lpddma.hpp"

static const int MaxHeaderPerQuantum = 10;

void ZModem::WriteByte(uint8_t c) {
	IPI3_OsServer::PutByte(c);
}

uint8_t ZModem::ReadNextByte() {
	reget:
	if (this->tmpBufferSize != 0) {
		auto const startOfBuffer = (uint8_t *) this->tmpBufferAddr;
		uint8_t b = startOfBuffer[this->tmpBufferIndex++];
		if (this->tmpBufferIndex == this->tmpBufferSize) {
			osHeap->tmpOsBufferAllocator.Free(this->tmpBufferAddr, this->tmpBufferSize/64);
			this->tmpBufferSize = 0;
			this->tmpBufferAddr = 0;
			this->tmpBufferIndex = 0;
		}
		return b;
	}
	HostInterface::TmpBufferRefill(this->tmpBufferAddr, this->tmpBufferSize);
	goto reget;
}

// 1024+1 bytes buffer needed, rounded up to nearest 64 byte chunk
#define KSIZE (1024+64)

void ZModem::Init() {
	this->sectorBuffer = (uint8_t *) osHeap->tmpOsBufferAllocator.Alloc((KSIZE + 1) / 64);
	ReInit();
}

void ZModem::ReInit() {
	this->state = State::INITIAL_HEADER;
	this->tryCount = 20;
	this->fileBytesRecv = 0;
	this->Rxframeind = HeaderType::ZHEX;
	if (this->tmpBufferSize != 0) {
		osHeap->tmpOsBufferAllocator.Free(this->tmpBufferAddr, this->tmpBufferSize / 64);
		this->tmpBufferSize = 0;
		this->tmpBufferAddr = 0;
		this->tmpBufferIndex = 0;
	}
#ifdef CANBREAK
	Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO|CANBRK;
#else
	Txhdr[ZF0] = CANFC32 | CANFDX | CANOVIO;
#endif
	if (Zctlesc) {
		Txhdr[ZF0] |= TESCCTL;
	}
	this->destinationAddress = 0x8'0000'0000;
}

void ZModem::Fini() {
	osHeap->tmpOsBufferAllocator.Free((uintptr_t) this->sectorBuffer, (KSIZE + 1) / 64);
}

ZModem::Result ZModem::Receive() {
	for (int i = 0; i < MaxHeaderPerQuantum; ++i) {
		switch (this->state) {
			case State::INITIAL_HEADER:
				if (this->tryCount-- == 0) {
					return Result::FAIL;
				}
				if (Try() != FrameType::ZFILE) {
					continue;
				}
				this->state = State::RECEIVE_FILE;
				[[fallthrough]];
			case State::RECEIVE_FILE: stohdr(this->fileBytesRecv);
				zshhdr(FrameType::ZRPOS, Txhdr);
				this->state = State::NEXT_HEADER;
				[[fallthrough]];
			case State::NEXT_HEADER: {
				auto result = NextHeader();
				if (result != Result::CONTINUE) {
					return result;
				}
				continue;
			}
			case State::MORE_DATA: {
				auto result = MoreData();
				switch(result) {
					case Result::CONTINUE:
						continue;
					case Result::FAIL:
						return result;
					case Result::SUCCESS:
						return result;
				}
			}
			case State::NEXT_FILE: return NextFile();
			default: return Result::CONTINUE;
		}
	}
	return Result::CONTINUE;
}

ZModem::Result ZModem::NextFile() {
	if (this->tryCount-- == 0) {
		return Result::FAIL;
	}
	switch (Try()) {
		case FrameType::ZCOMPL: return Result::SUCCESS;
		case FrameType::ZFILE: this->state = State::RECEIVE_FILE;
			return Result::CONTINUE;
		default: return Result::CONTINUE;
	}
}

FrameType ZModem::Try() {
	this->fileBytesRecv = 0;
	// Set buffer length (0) and capability flags */
	stohdr(0L);
	zshhdr(FrameType::ZRINIT, Txhdr);
	again:
	FrameType const headerType = zgethdr(Rxhdr);
	//	osHeap->console.console.Printf("Try Frame Type %d %lu\n", (int) headerType, this->fileBytesRecv);
	switch (headerType) {
		case FrameType::ZRQINIT: return headerType;
		case FrameType::ZEOF: return headerType;
		case FrameType::ZFILE:
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
		case FrameType::ZCOMMAND: return FrameType::ZCOMPL;
		case FrameType::ZCOMPL: goto again;
		default: return headerType;
		case FrameType::ZFIN: ackbibi();
			return FrameType::ZCOMPL;
		case FrameType::ZCAN: return FrameType::ZCAN;
	}
}

ZModem::Result ZModem::NextHeader() {
	FrameType const headerType = zgethdr(Rxhdr);
//	osHeap->console.console.Printf("RF Frame Type %d %lu\n", (int) headerType, this->fileBytesRecv);
	switch (headerType) {
		default: return Result::FAIL;
		case FrameType::ZNAK:
			if (--this->tryCount < 0) {
				return Result::FAIL;
			}
		[[fallthrough]];
		case FrameType::ZFILE: zrdata(sectorBuffer, KSIZE);
			return Result::CONTINUE;
		case FrameType::ZEOF:
			if (rclhdr(Rxhdr) != this->fileBytesRecv) {
				return Result::CONTINUE;
			}
			osHeap->console.console.Print(ANSI_YELLOW_PEN "EOF" ANSI_RESET_ATTRIBUTES);
			this->state = State::NEXT_FILE;
			return Result::CONTINUE;
		case FrameType::ZSKIP: this->state = State::NEXT_FILE;
			return Result::CONTINUE;
		case FrameType::ZFERR:
			if (--this->tryCount < 0) {
				return Result::FAIL;
			}
			zmputs(Attn);
			return Result::CONTINUE;
		case FrameType::ZDATA:
			if (rclhdr(Rxhdr) != this->fileBytesRecv) {
				if (--this->tryCount < 0) {
					return Result::FAIL;
				}
				zmputs(Attn);
				return Result::CONTINUE;
			}
			this->state = State::MORE_DATA;
	}
	return Result::CONTINUE;
}
void ZModem::MoveReceivedData(uint32_t size) {
	//	osHeap->console.console.PutChar('#');
	Dma::LpdDma::SimpleDmaCopy(Dma::LpdDma::Channels::ChannelSevern,
														 (uintptr_all_t)sectorBuffer,
														 (uintptr_all_t)this->destinationAddress + this->fileBytesRecv,
														 size);
	this->fileBytesRecv += size;
	Dma::LpdDma::Stall(Dma::LpdDma::Channels::ChannelSevern);
}

ZModem::Result ZModem::MoreData() {
	ZModemResult const dataResult = zrdata(sectorBuffer, KSIZE);
	switch (dataResult) {
		case ZModemResult::GOTCAN: return Result::FAIL;
		case ZModemResult::GOTCRCW: this->tryCount = 20;
			this->MoveReceivedData(Rxcount);
			stohdr(this->fileBytesRecv);
			zshhdr(FrameType::ZACK, Txhdr);
			WriteByte(MODEM_XON);
			this->state = State::NEXT_HEADER;
			osHeap->console.console.PutChar('W');
			return Result::CONTINUE;
		case ZModemResult::GOTCRCQ: this->tryCount = 20;
			osHeap->console.console.PutChar('Q');
			this->MoveReceivedData(Rxcount);
			stohdr(this->fileBytesRecv);
			zshhdr(FrameType::ZACK, Txhdr);
			return Result::CONTINUE;
		case ZModemResult::GOTCRCG: this->tryCount = 20;
			osHeap->console.console.PutChar('G');
			this->MoveReceivedData(Rxcount);
			return Result::CONTINUE;
		case ZModemResult::GOTCRCE: this->tryCount = 20;
			osHeap->console.console.PutChar('E');
			this->MoveReceivedData(Rxcount);
			this->state = State::NEXT_HEADER;
			return Result::CONTINUE;
		case ZModemResult::ERROR:
			if (--this->tryCount < 0) {
				return Result::FAIL;
			}
			osHeap->console.console.PrintLn("ZModemResult::ERROR");
			this->state = State::RECEIVE_FILE;
			return Result::CONTINUE;
		default: return Result::CONTINUE;
	}
}

// Send ZMODEM HEX header hdr of type type
void ZModem::zshhdr(FrameType type, uint8_t const *hdr) {
	uint16_t crc;
	WriteByte(ZMODEM_PAD);
	WriteByte(ZMODEM_PAD);
	WriteByte(ZMODEM_IDLE);
	WriteByte((char) HeaderType::ZHEX);
	PutHex((uint8_t) type);

	crc = updcrc((uint8_t) type, 0);
	for (int32_t n = 4; --n >= 0; ++hdr) {
		PutHex(*hdr);
		crc = updcrc((0377 & *hdr), crc);
	}
	crc = updcrc(0, updcrc(0, crc));
	PutHex((char) (crc >> 8));
	PutHex((char) crc);

	/* Make it printable on remote machine */
	WriteByte(015);
	WriteByte(0212);
	/*
	 * Uncork the remote in case a fake XOFF has stopped data flow
	 */
	if (type != FrameType::ZFIN && type != FrameType::ZACK) {
		WriteByte(021);
	}
	//		flushmo();
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
		switch (ReadNextByte()) {
			case 'O': ReadNextByte();  /* Discard 2nd 'O' */
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
	uint8_t const c = ReadNextByte();
	switch (c) {
		case ASCII_CAN: {
			gotcan:
			if (--cancount <= 0) {
				return FrameType::ZCAN;
			}
			switch (ReadNextByte()) {
				case ASCII_CAN:
					if (--cancount <= 0) {
						return FrameType::ZCAN;
					}
					goto again;
				case ZCRCW: return FrameType::ZFERR;
			}
		}
			[[fallthrough]];
		default:
			// skip garbage
			if (--n == 0) {
				return FrameType::ZFERR;
			}
			goto startover;
		case ZMODEM_PAD:
		case ZMODEM_PAD | 0200: break;
	}

	cancount = 5;
	splat:
	switch (noxrd7()) {
		case ZMODEM_PAD: goto splat;
		default:
			if (--n == 0) {
				return FrameType::ZFERR;
			}
			goto startover;
		case ZMODEM_IDLE:    // This is what we want.
			break;
	}

	uint8_t const type = noxrd7();
	switch (type) {
		case (char) HeaderType::ZBIN: Rxframeind = HeaderType::ZBIN;
			hdrType = zrbhdr(hdr);
			break;
		case (char) HeaderType::ZBIN32: Rxframeind = HeaderType::ZBIN32;
			hdrType = zrbhdr32(hdr);
			break;
		case (char) HeaderType::ZHEX: Rxframeind = HeaderType::ZHEX;
			hdrType = zrhhdr(hdr);
			break;
		case ASCII_CAN: goto gotcan;
		default:
			// skip garbage
			if (--n == 0) {
				return FrameType::ZFERR;
			}
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
	ZModemResult c = zdlread();
	if(c & ~0xFF) return FrameType::ZFERR;
	Rxtype = (FrameType) ((uint16_t) c & 0xFF);
	uint16_t crc = updcrc(c, 0);

	for (int n = 4; --n >= 0; ++hdr) {
		c = zdlread();
		if(c & ~0xFF) return FrameType::ZFERR;
		crc = updcrc(c, crc);
		*hdr = c;
	}
	c = zdlread();
	if(c & ~0xFF) return FrameType::ZFERR;
	crc = updcrc(c, crc);

	c = zdlread();
	if(c & ~0xFF) return FrameType::ZFERR;
	crc = updcrc(c, crc);
	if (crc & 0xFFFF) {
		osHeap->console.console.PrintLn(ANSI_RED_PEN "Bad Header CRC" ANSI_RESET_ATTRIBUTES);
		return FrameType::ZFERR;
	}
	return Rxtype;
}

/* Receive a binary style header (type and position) with 32 bit FCS */
FrameType ZModem::zrbhdr32(uint8_t *hdr) {
	uint8_t c;
	uint32_t crc;

	c = zdlread();
	if(c & ~0xFF) return FrameType::ZFERR;
	Rxtype = (FrameType) c;
	crc = 0xFFFFFFFFL;
	crc = UPDC32(c, crc);

	for (int n = 4; --n >= 0; ++hdr) {
		c = zdlread();
		if(c & ~0xFF) return FrameType::ZFERR;
		crc = UPDC32(c, crc);
		*hdr = c;
	}
	for (int n = 4; --n >= 0;) {
		c = zdlread();
		if(c & ~0xFF) return FrameType::ZFERR;
		crc = UPDC32(c, crc);
	}
	if (crc != 0xDEBB20E3) {
		osHeap->console.console.Print(ANSI_RED_PAPER ANSI_BRIGHT_ON "Bad Header CRC\n" ANSI_RESET_ATTRIBUTES);
		return FrameType::ZFERR;
	}
	return Rxtype;
}

// Receive a hex style header (type and position)
FrameType ZModem::zrhhdr(uint8_t *hdr) {
	uint8_t c = zgethex();
	if(c & ~0xFF) return FrameType::ZFERR;
	Rxtype = (FrameType) c;
	uint16_t crc = updcrc(c, 0);

	for (int n = 4; --n >= 0; ++hdr) {
		c = zgethex();
		if(c & ~0xFF) return FrameType::ZFERR;
		crc = updcrc(c, crc);
		*hdr = c;
	}
	c = zgethex();
	if(c & ~0xFF) return FrameType::ZFERR;
	crc = updcrc(c, crc);
	c = zgethex();
	if(c & ~0xFF) return FrameType::ZFERR;
	crc = updcrc(c, crc);
	if (crc & 0xFFFF) {
		osHeap->console.console.Print(ANSI_RED_PAPER ANSI_BRIGHT_ON "Bad Header CRC\n" ANSI_RESET_ATTRIBUTES);
		return FrameType::ZFERR;
	}

	if (ReadNextByte() == '\r') {  /* Throw away possible cr/lf */
		ReadNextByte();
	}
	return Rxtype;
}

/*
 * Receive array buf of max length with ending ZDLE sequence
 *  and CRC.  Returns the ending character or error code.
 *  NB: On errors may store length+1 bytes!
 */
ZModemResult ZModem::zrdata(uint8_t *buf, uint32_t length) {
	if (Rxframeind == HeaderType::ZBIN32) {
		return zrdat32(buf, length);
	}
	ZModemResult c;
	ZModemResult d;
	uint16_t crc = 0;
	Rxcount = 0;
	uint8_t *const end = buf + length;
	while (buf <= end) {
		if ((c = zdlread()) & ~0xFF) {
crcfoo:
			switch (c) {
				case GOTCRCE:
				case GOTCRCG:
				case GOTCRCQ:
				case GOTCRCW: crc = updcrc((d = c) & 0377, crc);
					if ((c = zdlread()) & ~0377) {
						goto crcfoo;
					}
					crc = updcrc(c, crc);
					if ((c = zdlread()) & ~0377) {
						goto crcfoo;
					}
					crc = updcrc(c, crc);
					if (crc & 0xFFFF) {
						osHeap->console.console.Printf(ANSI_RED_PAPER ANSI_BRIGHT_ON "CRC ERROR %#06x\n" ANSI_RESET_ATTRIBUTES, crc);
						return ZModemResult::CRC_ERROR;
					}
					Rxcount = length - (end - buf);
					return d;
				case GOTCAN:
					osHeap->console.console.PrintLn(ANSI_RED_PAPER ANSI_BRIGHT_ON "Send Cancelled" ANSI_RESET_ATTRIBUTES);
					return ZModemResult::GOTCAN;
				default:
					osHeap->console.console.PrintLn(ANSI_RED_PAPER ANSI_BRIGHT_ON "Bad data subpacket" ANSI_RESET_ATTRIBUTES);
					return c;
			}
		}
		*buf++ = c;
		crc = updcrc(c, crc);
	}
	osHeap->console.console.Print(ANSI_RED_PAPER ANSI_BRIGHT_ON "Data subpacket too long\n" ANSI_RESET_ATTRIBUTES);
	return ZModemResult::ERROR;
}

ZModemResult ZModem::zrdat32(uint8_t *buf, uint32_t length) {
	ZModemResult c;
	ZModemResult d;
	uint8_t b;

	uint32_t crc = 0xFFFFFFFFL;
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
						osHeap->console.console.Printf(ANSI_RED_PAPER ANSI_BRIGHT_ON "CRC ERROR %#010lx\n" ANSI_RESET_ATTRIBUTES, crc);
						return ZModemResult::CRC_ERROR;
					}
					Rxcount = length - (end - buf);
					return d;
				case GOTCAN:
					osHeap->console.console.PrintLn(ANSI_RED_PAPER ANSI_BRIGHT_ON "Send Cancelled" ANSI_RESET_ATTRIBUTES);
					return ZModemResult::GOTCAN;
				default:
					osHeap->console.console.PrintLn(ANSI_RED_PAPER ANSI_BRIGHT_ON "Bad data subpacket" ANSI_RESET_ATTRIBUTES);
					return c;
			}
		}
		*buf++ = c;
		crc = UPDC32(c, crc);
	}
	osHeap->console.console.PrintLn("Data subpacket too long");
	return GOTCAN;
}

// Send a byte as two hex digits
void ZModem::PutHex(uint8_t bin) {
	static char digits[] = "0123456789abcdef";

	WriteByte(digits[(bin & 0xF0) >> 4]);
	WriteByte(digits[(bin) & 0xF]);
}

/*
 * Read a byte, checking for ZMODEM escape encoding
 *  including CAN*5 which represents a quick abort
 */
ZModemResult ZModem::zdlread() {
	uint16_t c;
	again:
	switch (c = ReadNextByte()) {
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
	c = ReadNextByte();

	// if canceled read a few more to make sure
	if (c == ASCII_CAN) {
		c = ReadNextByte();
	}
	if (c == ASCII_CAN) {
		c = ReadNextByte();
	}
	if (c == ASCII_CAN) {
		c = ReadNextByte();
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
	osHeap->console.console.Printf("Bad escape sequence %#x\n", c);
	return GOTCAN;
}

/*
 * Read a character from the modem line with timeout.
 *  Eat parity, XON and XOFF characters.
 */
uint8_t ZModem::noxrd7() {
	uint8_t c;
	for (;;) {
		c = ReadNextByte();

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
uint8_t ZModem::zgethex() {
	uint8_t const c0 = noxrd7();
	uint8_t const c1 = noxrd7();

	uint8_t n0 = c0 - '0';
	uint8_t n1 = c1 - '0';
	if (n0 > 9) {
		n0 -= ('a' - ':');
	}
	if (n1 > 9) {
		n1 -= ('a' - ':');
	}
	if (n0 & ~0xF || n1 & ~0xF) {
		return 0;
	} // ERROR
	uint8_t const result = n1 + (n0 << 4);
	return result;
}

void ZModem::zmputs(uint8_t const *s) {
	uint8_t c;

	while (*s) {
		switch (c = *s++) {
			case (uint8_t)'\336':
				//				Utils_BusySecondSleep(1);
				continue;
			case (uint8_t)'\335': continue;
			default: WriteByte(c);
		}
	}
}
