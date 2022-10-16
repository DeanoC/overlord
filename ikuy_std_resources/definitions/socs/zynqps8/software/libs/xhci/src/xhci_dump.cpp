
#include "core/core.h"
#include "zynqps8/xhci/xhci.hpp"
#include "platform/registers/usb3_xhci.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"

namespace XHCI {

void DumpGSTSBits( Controller * device_ ) {
	uint32_t const gsts = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GSTS );
	debug_printf( XHCI_LOG_ANSI("gsts: cbelt %i ssic_ip %i otg_ip %i bc_ip %i adp_ip %i host_ip %i device_ip %i csrtimeout %i buserr %i curmod %i"),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, CBELT, gsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, SSIC_IP, gsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, OTG_IP, gsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, BC_IP, gsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, ADP_IP, gsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, HOST_IP, gsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, DEVICE_IP, gsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, CSRTIMEOUT, gsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, BUSERRADDRVLD, gsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, GSTS, CURMOD, gsts ));

}


void DumpCRCRBits( Controller * device_ ) {
	uint32_t const crcrLo = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, CRCR_LO );
	debug_printf( XHCI_LOG_ANSI("CRCR: rcs %i cs %i ca %i crr %i"),
	              HW_REG_DECODE_BIT( USB3_XHCI, CRCR_LO, RCS, crcrLo ),
	              HW_REG_DECODE_BIT( USB3_XHCI, CRCR_LO, CS, crcrLo ),
	              HW_REG_DECODE_BIT( USB3_XHCI, CRCR_LO, CA, crcrLo ),
	              HW_REG_DECODE_BIT( USB3_XHCI, CRCR_LO, CRR, crcrLo ));

}

void DumpUSBCMDBits( Controller * device_ ) {
	uint32_t const usbcmd = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBCMD );
	debug_printf( XHCI_LOG_ANSI("USBCMD: r_s %i hcrst %i inte %i hsee %i lhcrst %i css %i crs %i ewe %i eu3s %i cme %i"),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, R_S, usbcmd ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, HCRST, usbcmd ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, INTE, usbcmd ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, HSEE, usbcmd ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, LHCRST, usbcmd ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, CSS, usbcmd ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, CRS, usbcmd ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, EWE, usbcmd ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, EU3S, usbcmd ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBCMD, CME, usbcmd ));

}

void DumpIMan0Bits( Controller * device_ ) {
	uint32_t const iman0 = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, IMAN_0 );
	debug_printf( XHCI_LOG_ANSI("IMAN0: ie %i ip %i"),
	              HW_REG_DECODE_BIT( USB3_XHCI, IMAN_0, IE, iman0 ),
	              HW_REG_DECODE_BIT( USB3_XHCI, IMAN_0, IP, iman0 ));
}

void DumpUSBSTSBits( Controller * device_ ) {
	uint32_t const usbsts = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBSTS );
	debug_printf( XHCI_LOG_ANSI("USBSTS: hce %i cnr %i sre %i rss %i sss %i pcd %i eint %i hse %i hch %i"),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, HCE, usbsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, CNR, usbsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, SRE, usbsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, RSS, usbsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, SSS, usbsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, PCD, usbsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, EINT, usbsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, HSE, usbsts ),
	              HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, HCH, usbsts ));

}

void DumpRootPort( uintptr_t portaddr_ ) {
	uint32_t const portsc = *((uint32_t * )( USB30_XHCI_BASE_ADDR + portaddr_ ));
	if((uintptr_t) portaddr_ == (USB3_XHCI_PORTSC_20_OFFSET)) {
		debug_print( XHCI_LOG_ANSI("USB 2.0: ") );
		debug_printf( XHCI_LOG_ANSI("pec %i csc %i lws %i pic %i portspeed %i pp %i pls %i pr %i oca %i ped %i ccs %i "),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PEC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, CSC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, LWS, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PIC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PORTSPEED, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PP, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PLS, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PR, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, OCA, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PED, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, CCS, portsc ));
		debug_printf( XHCI_LOG_ANSI("dr %i woe %i wde %i wce %i cas %i plc %i prc %i occ %i"),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, DR, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, WOE, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, WDE, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, WCE, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, CAS, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PLC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PRC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, OCC, portsc ));
	} else {
		debug_print( XHCI_LOG_ANSI("USB 3.0: ") );
		debug_printf( XHCI_LOG_ANSI("pec %i csc %i lws %i pic %i portspeed %i pp %i pls %i pr %i oca %i ped %i ccs %i "),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, PEC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, CSC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, LWS, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, PIC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, PORTSPEED, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, PP, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, PLS, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, PR, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, OCA, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, PED, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, CCS, portsc ));
		debug_printf( XHCI_LOG_ANSI("wpr %i dr %i woe %i wde %i wce %i cas %i cec %i plc %i prc %i occ %i wrc %i"),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, WPR, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, DR, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, WOE, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, WDE, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, WCE, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, CAS, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, CEC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, PLC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, PRC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, OCC, portsc ),
		              HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_30, WRC, portsc ));
		debug_printf( XHCI_LOG_ANSI("USB 3.0: Link Errors %i"), HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTLI_30, LINK_ERROR_COUNT ));
	}
}

void DumpCaps() {
	{
		uint32_t const hciVersion = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, CAPLENGTH, HCIVERSION );
		debug_printf( XHCI_LOG_ANSI("XHCI: - version %i.%i"), hciVersion >> 8, hciVersion & 0xFF );
		uint32_t const capLength = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, CAPLENGTH, CAPLENGTH );
		assert( capLength == USB3_XHCI_USBCMD_OFFSET );
	}

	{
		uint32_t const maxPorts = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS1, MAXPORTS );
		uint32_t const maxInterrupters = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS1, MAXINTRS );
		uint32_t const maxSlots = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS1, MAXSLOTS );
		debug_printf( XHCI_LOG_ANSI("XHCI: maxPorts %i maxInterupters %i maxSlots %i"), maxPorts, maxInterrupters, maxSlots );
	}
	{
		uint32_t const maxScratchPadBuffs = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS2, MAXSCRATCHPADBUFS ) |
		                                    (HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS2, MAXSCRATCHPADBUFS_HI ) << 5);
		uint32_t const scratchPadResource = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS2, SPR );
		uint32_t const erstMax = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS2, ERSTMAX );
		uint32_t const ist = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS2, IST );
		debug_printf( XHCI_LOG_ANSI("XHCI: maxScratchPadBuffs %i scratchPadResource %i erstMax %i ist %i"),
		              maxScratchPadBuffs,
		              scratchPadResource,
		              erstMax,
		              ist );
	}
	{
		uint32_t const xecpDWORDS = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, XECP );
		uint32_t const maxPSASize = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, MAXPSASIZE );
		uint32_t const cfc = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, CFC );
		uint32_t const sec = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, SEC );
		uint32_t const spc = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, SPC );
		uint32_t const pae = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, PAE );
		uint32_t const nss = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, NSS );
		uint32_t const ltc = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, LTC );
		uint32_t const lhrc = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, LHRC );
		uint32_t const pind = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, PIND );
		uint32_t const ppc = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, PPC );
		uint32_t const csz = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, CSZ );
		uint32_t const bnc = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, BNC );
		uint32_t const ac64 = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, AC64 );
		debug_printf( XHCI_LOG_ANSI("XHCI: | XECP  | MAXPSASIZE | CFC | SEC | SPC | PAE |") );
		debug_printf( XHCI_LOG_ANSI("XHCI: |  %x  |    %i      |  %i  |  %i  |  %i  |  %i  |"),
		              xecpDWORDS, maxPSASize, cfc, sec, spc, pae );
		debug_printf( XHCI_LOG_ANSI("XHCI: | NSS | LTC | LHRC | PIND | PPC | CSZ | BNC | AC64 |") );
		debug_printf( XHCI_LOG_ANSI("XHCI: |  %i  |  %i  |   %i  |   %i  |  %i  |  %i  |  %i  |  %i   |"),
		              nss, ltc, lhrc, pind, ppc, csz, bnc, ac64 );
	}
	{
		uint32_t const dboffInBytes = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DBOFF );
		debug_printf( XHCI_LOG_ANSI("XHCI: Doorbell array offset %0x"), dboffInBytes );
		uint32_t const rtsoffInBytes = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, RTSOFF );
		debug_printf( XHCI_LOG_ANSI("XHCI: Runtime Reg Space offset %0x"), rtsoffInBytes );
	}
	{
		uint32_t const u3c = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS2, U3C );
		uint32_t const cmc = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS2, CMC );
		uint32_t const fsc = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS2, FSC );
		uint32_t const ctc = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS2, CTC );
		uint32_t const lec = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS2, LEC );
		uint32_t const cic = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS2, CIC );
		debug_printf( XHCI_LOG_ANSI("XHCI: | U3C  | CMC | FSC | CTC | LEC | CIC |") );
		debug_printf( XHCI_LOG_ANSI("XHCI: |  %i   |  %i  |  %i  |  %i  |  %i  |  %i  |"),
		              u3c, cmc, fsc, ctc, lec, cic );
	}
	{
		uint32_t curOffset = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCCPARAMS1, XECP ) * 4;
		uint8_t nextOffset = 0;
		do {
			uint32_t extCapEntry = hw_RegRead( USB30_XHCI_BASE_ADDR, curOffset );
			auto const id = (ExtensionID)( extCapEntry & 0xFF );
			switch(id) {
				case ExtensionID::LegacyUSB: debug_print( XHCI_LOG_ANSI("XHCI: Legacy USB Extensions") );
					if(extCapEntry & USB3_XHCI_USBLEGSUP_HC_BIOS_OWNED)
						debug_print( XHCI_LOG_ANSI("    BIOS Owned") );
					if(extCapEntry & USB3_XHCI_USBLEGSUP_HC_OS_OWNED)
						debug_print( XHCI_LOG_ANSI("    OS Owned") );
					break;
				case ExtensionID::SupportedProtocol: {
					uint32_t dword1 = hw_RegRead( USB30_XHCI_BASE_ADDR, curOffset + 4 );
					uint32_t dword2 = hw_RegRead( USB30_XHCI_BASE_ADDR, curOffset + 8 );
					uint32_t dword3 = hw_RegRead( USB30_XHCI_BASE_ADDR, curOffset + 12 );
					char usbString[5];
					memcpy( usbString, &dword1, 4 );
					usbString[4] = 0;
					debug_printf( XHCI_LOG_ANSI("XHCI: Supported Protocol %s%i.%i"),
					              usbString,
					              (extCapEntry & 0xFF000000) >> 24,
					              (extCapEntry & 0xFF0000) >> 16 );
					debug_printf( XHCI_LOG_ANSI("    Port Offset %i Port Count %i"), dword2 & 0xFF, (dword2 & 0xFF00) >> 8 );
					debug_printf( XHCI_LOG_ANSI("    DriverSlot type %i"), dword3 & 0x0F );
					break;
				}
				case ExtensionID::USBDebugCapability: debug_print( XHCI_LOG_ANSI("XHCI: USB Debug Capability") );
					break;
				case ExtensionID::Reserved:
				case ExtensionID::ExtendedPowerManagement:
				case ExtensionID::IOVirtualization:
				case ExtensionID::MessageInterrupt:
				case ExtensionID::LocalMemory:
				case ExtensionID::ExtendedMessageInterrupt:
				default: debug_printf( "XHCI: Unknown Extension ID %i\n", (int) id );
					break;
			}
			nextOffset = ((extCapEntry & 0xFF00) >> 8) * 4;
			curOffset += nextOffset;
		} while(nextOffset != 0);
	}
	{
		uint32_t const pageSizeReg = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PAGESIZE, PAGE_SIZE );
		debug_printf( XHCI_LOG_ANSI("XHCI: pagesizereg %i Page %i bytes"), pageSizeReg, pageSizeReg << 12 );

	}
}


char const *SlotStateToString( SlotState slotState_ ) {
	switch(slotState_) {
		case SlotState::Disabled: return "Disabled";
		case SlotState::Default: return "Default";
		case SlotState::Addressed: return "Addressed";
		case SlotState::Configured: return "Configured";
		default: return "Reserved";
	}
}

}