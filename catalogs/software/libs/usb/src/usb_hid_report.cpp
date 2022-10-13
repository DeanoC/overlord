//
// Created by deano on 8/16/22.
//

#include "core/core.h"
#include "dbg/print.h"
#include "dbg/assert.h"
#include "dbg/ansi_escapes.h"
#include "usb/usb_hid_report.hpp"

namespace USB {

void DumpHIDReport( uint8_t *hidReport, uint8_t const *hidReportEnd ) {
	int level = 0;
	while(hidReport < hidReportEnd) {
		if(*hidReport == 0xFE) {
			// long tag
			uint8_t const size = *(hidReport + 1);
			debug_printf( "Long tag - unsupported\n" );
			hidReport += 1 + size;
		} else {
			// short tag
			uint8_t const header = *hidReport;
			uint8_t const tag = header >> 4;
			uint8_t const size = ((header & 0x3) == 0x3) ? 4 : (header & 0x3);
			auto type = (HIDReportShortItemType) ((header >> 2) & 0x3);

			switch(type) {
				case HIDReportShortItemType::Main: level = DumpHIDReportMainTag( hidReport, tag, size, level );
					break;
				case HIDReportShortItemType::Global: DumpHIDReportGlobalTag( hidReport, tag, size, level );
					break;
				case HIDReportShortItemType::Local: DumpHIDReportLocalTag(hidReport, tag, size, level );
					break;
				case HIDReportShortItemType::Reserved:break;
			}

			hidReport += 1 + size;
		}
	}
}

int DumpHIDReportMainTag( uint8_t const *hidReport, uint8_t const tag, uint8_t const size, int level ) {
	auto mainTag = (HIDReportMainTag) tag;

	if(mainTag == HIDReportMainTag::Collection) {
		auto collectionType = (HIDReportCollectionType) *(hidReport + 1);
		for(int i = 0; i < level; i++) debug_print( "  " );
		debug_printf( "Collection %s\n", HIDReportCollectionTypeToString( collectionType ));
		level++;
	} else if(mainTag == HIDReportMainTag::EndCollection) {
		level--;
		for(int i = 0; i < level; i++) debug_print( "  " );
		debug_printf( "End Collection\n");
	} else {
		for(int i = 0; i < level; i++) debug_print( "  " );
		debug_printf( "Main %s ", HIDReportMainTagToString( mainTag ));
		uint16_t bits = 0;
		switch(size) {
			case 1: bits = *(hidReport + 1);
				break;
			case 2: bits = *(uint16_t *) (hidReport + 1);
				break;
			case 4: bits = *(uint32_t *) (hidReport + 1);
				break;
			default:;
		}
		if(size > 0) {
			if(bits & 0x001) debug_printf( "const " ); else debug_printf( "data " );
			if(bits & 0x002) debug_printf( "var " ); else debug_printf( "array " );
			if(bits & 0x004) debug_printf( "rel " ); else debug_printf( "abs " );
			if(bits & 0x008) debug_printf( "wrap " ); else debug_printf( "nowrap " );
			if(bits & 0x010) debug_printf( "nonlinear " ); else debug_printf( "linear " );
			if(bits & 0x020) debug_printf( "nonprefered " ); else debug_printf( "prefered " );
			if(bits & 0x040) debug_printf( "null " ); else debug_printf( "nonull " );
			if(bits & 0x080) debug_printf( "volatile " );
			if(bits & 0x100) debug_printf( "buffered bytes " ); else debug_printf( "bits" );
		}
		debug_print( "\n" );
	}
	return level;
}

void DumpHIDReportGlobalTag( uint8_t const *hidReport, uint8_t const tag, uint8_t const size, int level ) {
	auto const globalTag = (HIDReportGlobalTag) tag;
	switch(globalTag) {
		case HIDReportGlobalTag::UsagePage: assert( size == 1 || size == 2 );
			for(int i = 0; i < level; i++) debug_print( "  " );
			if(size == 2) {
				// only 2 byte usage page are vendor or FIDO
				debug_printf( "Global 2 byte UsagePage %#06x\n", *(uint16_t *) (hidReport + 1));
			} else {
				auto usagePage = (HIDReportUsagePage) *(hidReport + 1);
				debug_printf( "Global UsagePage %s\n", HIDReportUsagePageToString( usagePage ));
			}
			break;
		case HIDReportGlobalTag::LogicalMinimum:
		case HIDReportGlobalTag::LogicalMaximum:
		case HIDReportGlobalTag::PhysicalMinimum:
		case HIDReportGlobalTag::PhysicalMaximum:
		case HIDReportGlobalTag::UnitExponent:
		case HIDReportGlobalTag::Unit:
		case HIDReportGlobalTag::ReportSize:
		case HIDReportGlobalTag::ReportId:
		case HIDReportGlobalTag::ReportCount:
		case HIDReportGlobalTag::Push:
		case HIDReportGlobalTag::Pop:
		default: for(int i = 0; i < level; i++) debug_print( "  " );
			debug_printf( "Global %s ", HIDReportGlobalTagToString((HIDReportGlobalTag) tag ));
			switch(size) {
				case 0: debug_print( "\n" );
					break;
				case 1: debug_printf( "%#04x\n", *(hidReport + 1));
					break;
				case 2: debug_printf( "%#06x\n", *(uint16_t *) (hidReport + 1));
					break;
				case 4: debug_printf( "%i\n", *(uint32_t *) (hidReport + 1));
					break;
				default:;
			}
			break;
	}
}
void DumpHIDReportLocalTag( uint8_t const *hidReport, uint8_t const tag, uint8_t const size, int level ) {
	for(int i = 0; i < level; i++) debug_print( "  " );
	auto localTag = (HIDReportLocalTag) tag;
	switch(localTag) {
		case HIDReportLocalTag::Usage: {
			auto usage = (HIDReportGlobalDesktopUsage) *(hidReport + 1);
			debug_printf( "Local Usage %s\n", HIDReportGlobalDesktopUsageToString( usage ));
			break;
		}
		case HIDReportLocalTag::UsageMinimum:
		case HIDReportLocalTag::UsageMaximum:
		case HIDReportLocalTag::DesignatorIndex:
		case HIDReportLocalTag::DesignatorMinimum:
		case HIDReportLocalTag::DesignatorMaximum:
		case HIDReportLocalTag::StringIndex:
		case HIDReportLocalTag::StringMinimum:
		case HIDReportLocalTag::StringMaximum:
		case HIDReportLocalTag::Delimiter:
		default:
			debug_printf( "Local %s ", HIDReportLocalTagToString((HIDReportLocalTag) tag ));
			switch(size) {
				case 0: debug_print( "\n" );
					break;
				case 1: debug_printf( "%#04x\n", *(hidReport + 1));
					break;
				case 2: debug_printf( "%#06x\n", *(uint16_t *) (hidReport + 1));
					break;
				case 4: debug_printf( "%i\n", *(uint32_t *) (hidReport + 1));
					break;
				default:;
					break;
			}
			break;
	}
}


const char *HIDReportShortItemTypeToString( HIDReportShortItemType tag_ ) {
	switch(tag_) {
		case HIDReportShortItemType::Main: return "Main";
		case HIDReportShortItemType::Global:return "Global";
		case HIDReportShortItemType::Local:return "Local";
		case HIDReportShortItemType::Reserved:return "Reserved";
		default: return "UNKNOWN";
	}
}

const char *HIDReportMainTagToString( HIDReportMainTag tag_ ) {
	switch(tag_) {
		case HIDReportMainTag::Input: return "Input";
		case HIDReportMainTag::Output: return "Output";
		case HIDReportMainTag::Collection: return "Collection";
		case HIDReportMainTag::Feature: return "Feature";
		case HIDReportMainTag::EndCollection: return "EndCollection";
		default: return "UNKNOWN";
	}
}

const char *HIDReportCollectionTypeToString( HIDReportCollectionType tag_ ) {
	switch(tag_) {
		case HIDReportCollectionType::Physical: return "Physical";
		case HIDReportCollectionType::Application: return "Application";
		case HIDReportCollectionType::Logical: return "Logical";
		case HIDReportCollectionType::Report: return "Report";
		case HIDReportCollectionType::NamedArray: return "NamedArray";
		case HIDReportCollectionType::UsageSwitch: return "UsageSwitch";
		case HIDReportCollectionType::UsageModifier: return "UsageModifier";
		default: return "UNKNOWN";
	}
}

const char *HIDReportGlobalTagToString( HIDReportGlobalTag tag_ ) {
	switch(tag_) {
		case HIDReportGlobalTag::UsagePage: return "UsagePage";
		case HIDReportGlobalTag::LogicalMinimum: return "LogicalMinimum";
		case HIDReportGlobalTag::LogicalMaximum: return "LogicalMaximum";
		case HIDReportGlobalTag::PhysicalMinimum: return "PhysicalMinimum";
		case HIDReportGlobalTag::PhysicalMaximum: return "PhysicalMaximum";
		case HIDReportGlobalTag::UnitExponent: return "UnitExponent";
		case HIDReportGlobalTag::Unit: return "Unit";
		case HIDReportGlobalTag::ReportSize: return "ReportSize";
		case HIDReportGlobalTag::ReportId: return "ReportId";
		case HIDReportGlobalTag::ReportCount: return "ReportCount";
		case HIDReportGlobalTag::Push: return "Push";
		case HIDReportGlobalTag::Pop: return "Pop";
		default: return "UNKNOWN";
	}
}

const char *HIDReportLocalTagToString( HIDReportLocalTag tag_ ) {
	switch(tag_) {
		case HIDReportLocalTag::Usage: return "Usage";
		case HIDReportLocalTag::UsageMinimum: return "UsageMinimum";
		case HIDReportLocalTag::UsageMaximum: return "UsageMaximum";
		case HIDReportLocalTag::DesignatorIndex: return "DesignatorIndex";
		case HIDReportLocalTag::DesignatorMinimum: return "DesignatorMinimum";
		case HIDReportLocalTag::DesignatorMaximum: return "DesignatorMaximum";
		case HIDReportLocalTag::StringIndex: return "StringIndex";
		case HIDReportLocalTag::StringMinimum: return "StringMinimum";
		case HIDReportLocalTag::StringMaximum: return "StringMaximum";
		case HIDReportLocalTag::Delimiter: return "Delimiter";
		default: return "UNKNOWN";
	}
}

const char *HIDReportUsagePageToString( HIDReportUsagePage tag_ ) {
	switch(tag_) {
		case HIDReportUsagePage::Undefined: return "Undefined";
		case HIDReportUsagePage::GenericDesktop: return "GenericDesktop";
		case HIDReportUsagePage::SimulationControlsPage: return "SimulationControlsPage";
		case HIDReportUsagePage::VRControls: return "VRControls";
		case HIDReportUsagePage::SportControls: return "SportControls";
		case HIDReportUsagePage::GameControls: return "GameControls";
		case HIDReportUsagePage::GenericDeviceControls: return "GenericDeviceControls";
		case HIDReportUsagePage::Keyboard: return "Keyboard";
		case HIDReportUsagePage::LED: return "LED";
		case HIDReportUsagePage::Buttons: return "Buttons";
		case HIDReportUsagePage::Ordinal: return "Ordinal";
		case HIDReportUsagePage::TelephonyDevice: return "TelephonyDevice";
		case HIDReportUsagePage::Consumer: return "Consumer";
		case HIDReportUsagePage::Digitizers: return "Digitizers";
		case HIDReportUsagePage::Haptics: return "Haptics";
		case HIDReportUsagePage::PhysicalInputDevice: return "PhysicalInputDevice";
		case HIDReportUsagePage::Unicode: return "Unicode";
		case HIDReportUsagePage::EyeAndHeadTrackers: return "EyeAndHeadTrackers";
		case HIDReportUsagePage::AuxiliaryDisplay: return "AuxiliaryDisplay";
		case HIDReportUsagePage::Sensors: return "Sensors";
		case HIDReportUsagePage::MedicalInstrument: return "MedicalInstrument";
		case HIDReportUsagePage::BrailleDisplay: return "BrailleDisplay";
		case HIDReportUsagePage::LightingAndIllumination: return "LightingAndIllumination";
		case HIDReportUsagePage::Monitor: return "Monitor";
		case HIDReportUsagePage::MonitorEnumerated: return "MonitorEnumerated";
		case HIDReportUsagePage::VESAVirtualControls: return "VESAVirtualControls";
		case HIDReportUsagePage::Power: return "Power";
		case HIDReportUsagePage::BatterySystem: return "BatterySystem";
		case HIDReportUsagePage::BarcodeScanner: return "BarcodeScanner";
		case HIDReportUsagePage::Scales: return "Scales";
		case HIDReportUsagePage::MagneticStripeReader: return "MagneticStripeReader";
		case HIDReportUsagePage::CameraControl: return "CameraControl";
		case HIDReportUsagePage::ArcadePage: return "ArcadePage";
		case HIDReportUsagePage::GamingDevice: return "GamingDevice";
		default: return "UNKNOWN";
	}
}
const char *HIDReportGlobalDesktopUsageToString( HIDReportGlobalDesktopUsage tag_ ) {
	switch(tag_) {
		case HIDReportGlobalDesktopUsage::Undefined: return "Undefined";
		case HIDReportGlobalDesktopUsage::Pointer: return "Pointer";
		case HIDReportGlobalDesktopUsage::Mouse: return "Mouse";
		case HIDReportGlobalDesktopUsage::Joystick: return "Joystick";
		case HIDReportGlobalDesktopUsage::Gamepad: return "Gamepad";
		case HIDReportGlobalDesktopUsage::Keyboard: return "Keyboard";
		case HIDReportGlobalDesktopUsage::Keypad: return "Keypad";
		case HIDReportGlobalDesktopUsage::MultiAxisController: return "MultiAxisController";
		case HIDReportGlobalDesktopUsage::TabletPCSystemControls: return "TabletPCSystemControls";
		case HIDReportGlobalDesktopUsage::WaterCoolingDevice: return "WaterCoolingDevice";
		case HIDReportGlobalDesktopUsage::ComputerChassisDevice: return "ComputerChassisDevice";
		case HIDReportGlobalDesktopUsage::WirelessRadioControls: return "WirelessRadioControls";
		case HIDReportGlobalDesktopUsage::PortableDeviceControl: return "PortableDeviceControl";
		case HIDReportGlobalDesktopUsage::SystemMultiAxisController: return "SystemMultiAxisController";
		case HIDReportGlobalDesktopUsage::SpatialController: return "SpatialController";
		case HIDReportGlobalDesktopUsage::AssistiveControl: return "AssistiveControl";
		case HIDReportGlobalDesktopUsage::DeviceDock: return "DeviceDock";
		case HIDReportGlobalDesktopUsage::DockableDevice: return "DockableDevice";
		case HIDReportGlobalDesktopUsage::CallStateManagementControl: return "CallStateManagementControl";
		case HIDReportGlobalDesktopUsage::X: return "X";
		case HIDReportGlobalDesktopUsage::Y: return "Y";
		case HIDReportGlobalDesktopUsage::Z: return "Z";
		case HIDReportGlobalDesktopUsage::Rx: return "Rx";
		case HIDReportGlobalDesktopUsage::Ry: return "Ry";
		case HIDReportGlobalDesktopUsage::Rz: return "Rz";
		case HIDReportGlobalDesktopUsage::Slider: return "Slider";
		case HIDReportGlobalDesktopUsage::Dial: return "Dial";
		case HIDReportGlobalDesktopUsage::Wheel: return "Wheel";
		case HIDReportGlobalDesktopUsage::HatSwitch: return "HatSwitch";
		case HIDReportGlobalDesktopUsage::CountedBuffer: return "CountedBuffer";
		case HIDReportGlobalDesktopUsage::ByteCount: return "ByteCount";
		case HIDReportGlobalDesktopUsage::MotionWakeup: return "MotionWakeup";
		case HIDReportGlobalDesktopUsage::Start: return "Start";
		case HIDReportGlobalDesktopUsage::Select: return "Select";
		case HIDReportGlobalDesktopUsage::Vx: return "Vx";
		case HIDReportGlobalDesktopUsage::Vz: return "Vz";
		case HIDReportGlobalDesktopUsage::Vbrx: return "Vbrx";
		case HIDReportGlobalDesktopUsage::Vbry: return "Vbry";
		case HIDReportGlobalDesktopUsage::Vbrz: return "Vbrz";
		case HIDReportGlobalDesktopUsage::Vno: return "Vno";
		case HIDReportGlobalDesktopUsage::FeatureNotification: return "FeatureNotification";
		case HIDReportGlobalDesktopUsage::ResolutionMultiplier: return "ResolutionMultiplier";
		case HIDReportGlobalDesktopUsage::Qx: return "Qx";
		case HIDReportGlobalDesktopUsage::Qy: return "Qy";
		case HIDReportGlobalDesktopUsage::Qz: return "Qz";
		case HIDReportGlobalDesktopUsage::Qw: return "Qw";
		case HIDReportGlobalDesktopUsage::SysteControl: return "SysteControl";
		case HIDReportGlobalDesktopUsage::SystePowerDown: return "SystePowerDown";
		case HIDReportGlobalDesktopUsage::SysteSleep: return "SysteSleep";
		case HIDReportGlobalDesktopUsage::SysteWakeUp: return "SysteWakeUp";
		case HIDReportGlobalDesktopUsage::SysteContextMenu: return "SysteContextMenu";
		case HIDReportGlobalDesktopUsage::SysteMainMenu: return "SysteMainMenu";
		case HIDReportGlobalDesktopUsage::SysteAppMenu: return "SysteAppMenu";
		case HIDReportGlobalDesktopUsage::SysteMenuHelp: return "SysteMenuHelp";
		case HIDReportGlobalDesktopUsage::SysteMenuExit: return "SysteMenuExit";
		case HIDReportGlobalDesktopUsage::SysteMenuSelect: return "SysteMenuSelect";
		case HIDReportGlobalDesktopUsage::SysteMenuRight: return "SysteMenuRight";
		case HIDReportGlobalDesktopUsage::SysteMenuLeft: return "SysteMenuLeft";
		case HIDReportGlobalDesktopUsage::SysteMenuUp: return "SysteMenuUp";
		case HIDReportGlobalDesktopUsage::SysteMenuDown: return "SysteMenuDown";
		case HIDReportGlobalDesktopUsage::SysteColdRestart: return "SysteColdRestart";
		case HIDReportGlobalDesktopUsage::SysteWarmRestart: return "SysteWarmRestart";
		case HIDReportGlobalDesktopUsage::DPadUp: return "DPadUp";
		case HIDReportGlobalDesktopUsage::DPadDown: return "DPadDown";
		case HIDReportGlobalDesktopUsage::DPadRight: return "DPadRight";
		case HIDReportGlobalDesktopUsage::DPad: return "DPad";
		case HIDReportGlobalDesktopUsage::IndexTrigger: return "IndexTrigger";
		case HIDReportGlobalDesktopUsage::PalmTrigger: return "PalmTrigger";
		case HIDReportGlobalDesktopUsage::Thumbstick: return "Thumbstick";
		case HIDReportGlobalDesktopUsage::SystemFunctionShift: return "SystemFunctionShift";
		case HIDReportGlobalDesktopUsage::SystemFunctionShiftLock: return "SystemFunctionShiftLock";
		case HIDReportGlobalDesktopUsage::SystemFunctionShiftLockIndiactor: return "SystemFunctionShiftLockIndiactor";
		case HIDReportGlobalDesktopUsage::SystemDismissNotification: return "SystemDismissNotification";
		case HIDReportGlobalDesktopUsage::SystemDoNotDisturb: return "SystemDoNotDisturb";
		case HIDReportGlobalDesktopUsage::SystemDock: return "SystemDock";
		case HIDReportGlobalDesktopUsage::SystemUndock: return "SystemUndock";
		case HIDReportGlobalDesktopUsage::SystemSetup: return "SystemSetup";
		case HIDReportGlobalDesktopUsage::SystemBreak: return "SystemBreak";
		case HIDReportGlobalDesktopUsage::SystemDebuggerBreak: return "SystemDebuggerBreak";
		case HIDReportGlobalDesktopUsage::ApplicationBreak: return "ApplicationBreak";
		case HIDReportGlobalDesktopUsage::ApplicationDebuggerBreak: return "ApplicationDebuggerBreak";
		case HIDReportGlobalDesktopUsage::SystemSpeakerMute: return "SystemSpeakerMute";
		case HIDReportGlobalDesktopUsage::SystemHibernate: return "SystemHibernate";
		case HIDReportGlobalDesktopUsage::SystemDisplayInvert: return "SystemDisplayInvert";
		case HIDReportGlobalDesktopUsage::SystemDisplayInternal: return "SystemDisplayInternal";
		case HIDReportGlobalDesktopUsage::SystemDisplayExternal: return "SystemDisplayExternal";
		case HIDReportGlobalDesktopUsage::SystemDisplayBoth: return "SystemDisplayBoth";
		case HIDReportGlobalDesktopUsage::SystemDisplayDual: return "SystemDisplayDual";
		case HIDReportGlobalDesktopUsage::SystemDisplayToggleIntExtMode: return "SystemDisplayToggleIntExtMode";
		case HIDReportGlobalDesktopUsage::SystemDisplaySwapPrimarySecondary: return "SystemDisplaySwapPrimarySecondary";
		case HIDReportGlobalDesktopUsage::SystemDisplayToggleLCDAutoscale: return "SystemDisplayToggleLCDAutoscale";
		case HIDReportGlobalDesktopUsage::SensorZone: return "SensorZone";
		case HIDReportGlobalDesktopUsage::RPM: return "RPM";
		case HIDReportGlobalDesktopUsage::CoolantLevel: return "CoolantLevel";
		case HIDReportGlobalDesktopUsage::CoolantCriticalLevel: return "CoolantCriticalLevel";
		case HIDReportGlobalDesktopUsage::CoolantPump: return "CoolantPump";
		case HIDReportGlobalDesktopUsage::ChassisEnclosure: return "ChassisEnclosure";
		case HIDReportGlobalDesktopUsage::WirelessRadioButton: return "WirelessRadioButton";
		case HIDReportGlobalDesktopUsage::WirelessRadioLED: return "WirelessRadioLED";
		case HIDReportGlobalDesktopUsage::WirelessRadioSliderSwitch: return "WirelessRadioSliderSwitch";
		case HIDReportGlobalDesktopUsage::SystemDisplayRotationLockButton: return "SystemDisplayRotationLockButton";
		case HIDReportGlobalDesktopUsage::SystemDisplayRotationLockSliderSwitch: return "SystemDisplayRotationLockSliderSwitch";
		case HIDReportGlobalDesktopUsage::ControlEnable: return "ControlEnable";
		case HIDReportGlobalDesktopUsage::DockableDeviceUniqueID: return "DockableDeviceUniqueID";
		case HIDReportGlobalDesktopUsage::DockableDeviceVendorID: return "DockableDeviceVendorID";
		case HIDReportGlobalDesktopUsage::DockableDevicePrimaryUsagePage: return "DockableDevicePrimaryUsagePage";
		case HIDReportGlobalDesktopUsage::DockableDevicePrimaryUsageID: return "DockableDevicePrimaryUsageID";
		case HIDReportGlobalDesktopUsage::DockableDeviceDockingState: return "DockableDeviceDockingState";
		case HIDReportGlobalDesktopUsage::DockableDeviceDisplayOcclusion: return "DockableDeviceDisplayOcclusion";
		case HIDReportGlobalDesktopUsage::DockableDeviceObjectType: return "DockableDeviceObjectType";
		case HIDReportGlobalDesktopUsage::CallActiveLED: return "CallActiveLED";
		case HIDReportGlobalDesktopUsage::CallMuteToggle: return "CallMuteToggle";
		case HIDReportGlobalDesktopUsage::CallMuteLED: return "CallMuteLED";
		default: return "UNKNOWN";
	}
}


} // USB