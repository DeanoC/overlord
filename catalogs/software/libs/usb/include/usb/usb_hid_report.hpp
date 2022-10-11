//
// Created by deano on 8/16/22.
//

#pragma once

#include "core/core.h"

namespace USB {

enum class HIDReportShortItemType : uint8_t {
	Main = 0,
	Global = 1,
	Local = 2,
	Reserved = 3
};

enum class HIDReportMainTag : uint8_t {
	Input = 8,
	Output = 9,
	Collection = 10,
	Feature = 11,
	EndCollection = 12
};

enum class HIDReportCollectionType : uint8_t {
	Physical = 0,
	Application = 1,
	Logical = 2,
	Report = 3,
	NamedArray = 4,
	UsageSwitch = 5,
	UsageModifier = 6,
};

enum class HIDReportGlobalTag : uint8_t {
	UsagePage = 0,
	LogicalMinimum = 1,
	LogicalMaximum = 2,
	PhysicalMinimum = 3,
	PhysicalMaximum = 4,
	UnitExponent = 5,
	Unit = 6,
	ReportSize = 7,
	ReportId = 8,
	ReportCount = 9,
	Push = 10,
	Pop = 11,
};

enum class HIDReportLocalTag : uint8_t {
	Usage = 0,
	UsageMinimum = 1,
	UsageMaximum = 2,
	DesignatorIndex = 3,
	DesignatorMinimum = 4,
	DesignatorMaximum = 5,
	StringIndex = 7,
	StringMinimum = 8,
	StringMaximum = 9,
	Delimiter = 10,
};

enum class HIDReportUsagePage : uint16_t {
	Undefined = 0x0,
	GenericDesktop,
	SimulationControlsPage,
	VRControls,
	SportControls,
	GameControls,
	GenericDeviceControls,
	Keyboard,
	LED,
	Buttons,
	Ordinal,
	TelephonyDevice,
	Consumer,
	Digitizers,
	Haptics,
	PhysicalInputDevice,
	Unicode,
	EyeAndHeadTrackers = 0x12,
	AuxiliaryDisplay = 0x14,
	Sensors = 0x20,
	MedicalInstrument = 0x40,
	BrailleDisplay = 0x41,
	LightingAndIllumination = 0x59,
	Monitor = 0x80,
	MonitorEnumerated = 0x81,
	VESAVirtualControls = 0x82,
	Power = 0x84,
	BatterySystem = 0x85,
	BarcodeScanner = 0x8C,
	Scales = 0x8D,
	MagneticStripeReader = 0x8E,
	CameraControl = 0x90,
	ArcadePage = 0x91,
	GamingDevice = 0x92,
};

enum class HIDReportGlobalDesktopUsage : uint8_t {
	Undefined = 0,
	Pointer,
	Mouse,
	Joystick = 0x4,
	Gamepad,
	Keyboard,
	Keypad,
	MultiAxisController,
	TabletPCSystemControls,
	WaterCoolingDevice,
	ComputerChassisDevice,
	WirelessRadioControls,
	PortableDeviceControl,
	SystemMultiAxisController,
	SpatialController,
	AssistiveControl,
	DeviceDock,
	DockableDevice,
	CallStateManagementControl,
	X = 0x30,
	Y,
	Z,
	Rx,
	Ry,
	Rz,
	Slider,
	Dial,
	Wheel,
	HatSwitch,
	CountedBuffer,
	ByteCount,
	MotionWakeup,
	Start,
	Select,
	Vx = 0x40,
	Vz,
	Vbrx,
	Vbry,
	Vbrz,
	Vno,
	FeatureNotification,
	ResolutionMultiplier,
	Qx,
	Qy,
	Qz,
	Qw,
	SysteControl = 0x80,
	SystePowerDown,
	SysteSleep,
	SysteWakeUp,
	SysteContextMenu,
	SysteMainMenu,
	SysteAppMenu,
	SysteMenuHelp,
	SysteMenuExit,
	SysteMenuSelect,
	SysteMenuRight,
	SysteMenuLeft,
	SysteMenuUp,
	SysteMenuDown,
	SysteColdRestart,
	SysteWarmRestart,
	DPadUp,
	DPadDown,
	DPadRight,
	DPad,
	IndexTrigger,
	PalmTrigger,
	Thumbstick,
	SystemFunctionShift,
	SystemFunctionShiftLock,
	SystemFunctionShiftLockIndiactor,
	SystemDismissNotification,
	SystemDoNotDisturb,
	SystemDock = 0xA0,
	SystemUndock,
	SystemSetup,
	SystemBreak,
	SystemDebuggerBreak,
	ApplicationBreak,
	ApplicationDebuggerBreak,
	SystemSpeakerMute,
	SystemHibernate,
	SystemDisplayInvert = 0xB0,
	SystemDisplayInternal,
	SystemDisplayExternal,
	SystemDisplayBoth,
	SystemDisplayDual,
	SystemDisplayToggleIntExtMode,
	SystemDisplaySwapPrimarySecondary,
	SystemDisplayToggleLCDAutoscale,
	SensorZone = 0xC0,
	RPM,
	CoolantLevel,
	CoolantCriticalLevel,
	CoolantPump,
	ChassisEnclosure,
	WirelessRadioButton,
	WirelessRadioLED,
	WirelessRadioSliderSwitch,
	SystemDisplayRotationLockButton,
	SystemDisplayRotationLockSliderSwitch,
	ControlEnable,
	DockableDeviceUniqueID = 0xD0,
	DockableDeviceVendorID,
	DockableDevicePrimaryUsagePage,
	DockableDevicePrimaryUsageID,
	DockableDeviceDockingState,
	DockableDeviceDisplayOcclusion,
	DockableDeviceObjectType,
	CallActiveLED = 0xE0,
	CallMuteToggle,
	CallMuteLED,
};

void DumpHIDReportGlobalTag( uint8_t const *hidReport, uint8_t tag, uint8_t size, int level );
int DumpHIDReportMainTag( uint8_t const *hidReport, uint8_t tag, uint8_t size, int level );
void DumpHIDReportLocalTag( uint8_t const *hidReport, uint8_t tag, uint8_t size, int level );
void DumpHIDReport( uint8_t *hidReport, uint8_t const *hidReportEnd);

const char *HIDReportShortItemTypeToString( HIDReportShortItemType tag_ );
const char *HIDReportMainTagToString( HIDReportMainTag tag_ );
const char *HIDReportCollectionTypeToString( HIDReportCollectionType tag_ );
const char *HIDReportGlobalTagToString( HIDReportGlobalTag tag_ );
const char *HIDReportLocalTagToString( HIDReportLocalTag tag_ );
const char *HIDReportUsagePageToString( HIDReportUsagePage tag_ );
const char *HIDReportGlobalDesktopUsageToString( HIDReportGlobalDesktopUsage tag_ );

} // USB
