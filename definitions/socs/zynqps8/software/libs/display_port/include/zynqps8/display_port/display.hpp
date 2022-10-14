#pragma once

#include "core/core.h"

namespace DisplayPort::Display {

// 12 bit colours are fairly common. the MSB of this should be zero
typedef uint16_t uint12_t;
// 1.3.12 fixed point (sign bit, 2 decimal, 12 fraction)
struct ColourTransformMatrix {
	uint16_t preTranslate[3];
	uint16_t matrix[9];
	uint16_t postTranslate[3];
};

enum class LinkRate : uint8_t {
	Rate_1_62Gbps = 6,
	Rate_2_7Gbps = 10,
	Rate_5_4Gbps = 20,
};

enum class OutputPixelFormat : uint8_t {
	RGB = 0,
	YCBCR_422 = 2,
	YCBCR_444 = 1,
	OTHER = 3
};

enum class BitsPerPrimaryChannel : uint8_t {
	SIX_BITS = 0,
	EIGHT_BITS = 1,
	TEN_BITS = 2,
	TWELVE_BITS = 3,
	SIXTEEN_BITS = 4,
};
//
struct OutputPixelConfig {
	OutputPixelFormat format;
	BitsPerPrimaryChannel bitsPerChannel;
	bool yOnly;
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

// must be 256 byte aligned
struct PACKED DMADescriptor {
	union {
		uint32_t CONTROL;
		struct {
			uint32_t preamble: 8;
			uint32_t enableCompletionInterrupt: 1;
			uint32_t enableDescriptorUpdate: 1;
			uint32_t ignoreDone: 1;
			uint32_t burstType: 1; // 0 = INCR, 1 = FIXED
			uint32_t axiCache: 4;
			uint32_t axiProt: 2;
			uint32_t fragmented: 1;
			uint32_t last: 1;
			uint32_t enableCrc: 1;
			uint32_t lastOfFrame: 1;
			uint32_t : 0;
		};
	};

	union {
		uint32_t DSCR_ID;
		struct {
			uint32_t dscrId: 16;
			uint32_t : 0;
		};
	};

	union {
		uint32_t XFER_SIZE;
		struct {
			uint32_t transferSize;
		};
	};
	union {
		uint32_t LINE_SIZE_STRIDE;
		struct {
			uint32_t width: 18;
			uint32_t stride: 14;
		};
	};

	union {
		uint32_t LSB_Timestamp;
		struct {
			uint32_t timestampLSB;
		};
	};
	union {
		uint32_t MSB_Timestamp;
		struct {
			uint32_t timestampMSB: 10;
			uint32_t padd0: 21;
			uint32_t timestampDone: 1;
		};
	};

	union {
		uint32_t ADDR_EXT;
		struct {
			uint32_t nextDescriptorAddressExt: 16;
			uint32_t sourceAddressExt: 16;
		};
	};
	union {
		uint32_t NEXT_DESC;
		struct {
			uint32_t nextDescriptorAddress;
		};
	};

	union {
		uint32_t SRC_ADDR;
		struct {
			uint32_t sourceAddress;
		};
	};

	union {
		uint32_t ADDR_EXT_23;
		struct {
			uint32_t source2AddressExt: 16;
			uint32_t source3AddressExt: 16;
		};
	};
	union {
		uint32_t ADDR_EXT_45;
		struct {
			uint32_t source4AddressExt: 16;
			uint32_t source5AddressExt: 16;
		};
	};
	union {
		uint32_t SRC_ADDR2;
		struct {
			uint32_t source2Address;
		};
	};
	union {
		uint32_t SRC_ADDR3;
		struct {
			uint32_t source3Address;
		};
	};
	union {
		uint32_t SRC_ADDR4;
		struct {
			uint32_t source4Address;
		};
	};
	union {
		uint32_t SRC_ADDR5;
		struct {
			uint32_t source5Address;
		};
	};

	union {
		uint32_t CRC;
		struct {
			uint32_t crc;
		};
	};
};
#pragma GCC diagnostic pop

static_assert( sizeof(DMADescriptor) == 64);
static_assert( offsetof(DMADescriptor, CONTROL) == 0);
static_assert( offsetof(DMADescriptor, DSCR_ID) == 4);
static_assert( offsetof(DMADescriptor, XFER_SIZE) == 8);
static_assert( offsetof(DMADescriptor, LINE_SIZE_STRIDE) == 12);
static_assert( offsetof(DMADescriptor, LSB_Timestamp) == 16);
static_assert( offsetof(DMADescriptor, MSB_Timestamp) == 20);
static_assert( offsetof(DMADescriptor, ADDR_EXT) == 24);
static_assert( offsetof(DMADescriptor, NEXT_DESC) == 28);
static_assert( offsetof(DMADescriptor, SRC_ADDR) == 32);
static_assert( offsetof(DMADescriptor, ADDR_EXT_23) == 36);
static_assert( offsetof(DMADescriptor, ADDR_EXT_45) == 40);
static_assert( offsetof(DMADescriptor, SRC_ADDR2) == 44);
static_assert( offsetof(DMADescriptor, SRC_ADDR3) == 48);
static_assert( offsetof(DMADescriptor, SRC_ADDR4) == 52);
static_assert( offsetof(DMADescriptor, SRC_ADDR5) == 56);
static_assert( offsetof(DMADescriptor, CRC) == 60);

struct DisplayVideoPlane {
	enum class Source {
		DISABLED = 3,
		BUFFER = 1,
		FPGA = 0,
		TEST_GENERATOR = 2
	} source;

	enum class Format {
		CbYfCrYs8_422 = 0,
		CrYfCbYs8_422 = 1,
		YfCrYsCb8_422 = 2,
		YfCbYsCr8_422 = 3,
		Y8_Cr8_Cb8_422 = 4,
		Y8_Cr8_Cb8_444 = 5,
		Y8_CrCb8_422 = 6,
		Y8 = 7,
		Y8_CbCr8_422 = 8,
		YCrCb8_444 = 9,
		RGB8 = 10,
		RGBX8 = 11,
		RGB10 = 12,
		YCrCb10_444 = 13,
		Y10_CrCb10_422 = 14,
		Y10_CbCr10_422 = 15,
		Y10_Cr10_Cb10_422 = 16,
		Y10_Cr10_Cb10_444 = 17,
		Y10 = 18,
		Y8_Cr8_Cb8_420 = 19,
		Y8_CrCb8_420 = 20,
		Y8_CbCr8_420 = 21,
		Y10_Cr10_Cb10_420 = 22,
		Y10_CrCb10_420 = 23,
		Y10_CbCr10_420 = 24,
	} format;

	ColourTransformMatrix toRGBTransform;

	uintptr_all_t simpleDescPlane0Address;
	uintptr_all_t simpleDescPlane1Address;
	uintptr_all_t simpleDescPlane2Address;

	[[nodiscard]] static bool IsFormatRGB(Format format) {
		return (format == DisplayVideoPlane::Format::RGB8 ||
				format == DisplayVideoPlane::Format::RGBX8 ||
				format == DisplayVideoPlane::Format::RGB10);
	}
	[[nodiscard]] static bool NeedColourUpSampling(Format format) {
		switch (format) {
			case Format::CbYfCrYs8_422:
			case Format::CrYfCbYs8_422:
			case Format::YfCrYsCb8_422:
			case Format::YfCbYsCr8_422:
			case Format::Y8_Cr8_Cb8_422:
			case Format::Y8_CrCb8_422:
			case Format::Y8_CbCr8_422:
			case Format::Y10_CrCb10_422:
			case Format::Y10_CbCr10_422:
			case Format::Y10_Cr10_Cb10_422:
			case Format::Y8_CrCb8_420:
			case Format::Y8_CbCr8_420:
			case Format::Y10_Cr10_Cb10_420:
			case Format::Y10_CrCb10_420:
			case Format::Y10_CbCr10_420:
			case Format::Y8_Cr8_Cb8_420:
				return true;
			default: return false;
		}
	}
	[[nodiscard]] static int NumberOfPlanes(Format format) {
		switch (format) {
			case Format::Y8_Cr8_Cb8_422:
			case Format::Y8_Cr8_Cb8_420:
			case Format::Y10_Cr10_Cb10_420:
			case Format::Y10_Cr10_Cb10_422:
			case Format::Y10_Cr10_Cb10_444:
			case Format::Y8_Cr8_Cb8_444: return 3;

			case Format::Y10_CrCb10_420:
			case Format::Y10_CbCr10_420:
			case Format::Y8_CrCb8_420:
			case Format::Y8_CbCr8_420:
			case Format::Y10_CrCb10_422:
			case Format::Y10_CbCr10_422:
			case Format::Y8_CrCb8_422:
			case Format::Y8_CbCr8_422: return 2;
			default: return 1;
		}
	}
};

struct DisplayGfxPlane {
	enum class Source {
		DISABLED = 3,
		BUFFER = 1,
		FPGA = 2,
	} source;

	enum class Format {
		RGBA8 = 0,
		ABGR8 = 1,
		RGB8 = 2,
		BGR8 = 3,
		RGBA5551 = 4,
		RGBA4 = 5,
		RGB565 = 6,
		CLUT8 = 7,
		CLUT4 = 8,
		CLUT2 = 9,
		CLUT1 = 10,
		YUV8_422 = 11, //?? this isn't in the register docs but in Xilinx driver
	} format;
	[[nodiscard]] static bool IsFormatRGB(Format format) {
		return (format != DisplayGfxPlane::Format::YUV8_422);
	}

	[[nodiscard]] static bool NeedColourUpSampling(Format format) { return false; }

	ColourTransformMatrix toRGBTransform;
	uintptr_all_t simpleDescBufferAddress;
};

struct VideoTiming {
	uint16_t width;
	uint16_t hFrontPorch;
	uint16_t hSyncPulseWidth;
	uint16_t hBackPorch;
	uint16_t hTotal;
	uint16_t height;
	uint16_t vFrontPorch;
	uint16_t vSyncPulseWidth;
	uint16_t vBackPorch;
	uint16_t vTotal;
	bool hSyncPolarity;
	bool vSyncPolarity;
	float frameRateHz;
};

enum class MixerFunction {
	VIDEO,                  // Video plane only
	GFX,                    // Gfx plane only
	CHROMA_KEY_VIDEO,        // ranged colour key punch through using video plane
	CHROMA_KEY_GFX,          // ranged colour key punch through using gfx plane
	GLOBAL_PORTER_DUFF,      // single value alpha blend
	PER_PIXEL_PORTER_DUFF    // per pixel alpha blend
};

enum class StandardVideoMode {
	VM_640_480_60,
	VM_800_600_60,
	VM_1280_720_60,
	VM_1920_1080_60,
};

// mixer takes a background colour + 2 planes (1 video + 1 gfx)
// They are mixed in linear rgb space and then displayed in output pixel format.
// Each plane has a colour transform matrix to convert its inputs into linear RGB
struct Mixer {
	MixerFunction function;

	DisplayVideoPlane videoPlane;
	DisplayGfxPlane gfxPlane;

	ColourTransformMatrix outRgb2YCrCbMatrix;

	uint12_t backgroundColour[3];
	uint12_t chromaKeyMin[3];
	uint12_t chromaKeyMax[3];
	uint8_t globalAlpha;
};

struct Display {
	OutputPixelConfig pixelConfig;

	VideoTiming videoTiming;

	uint8_t synchronousClock: 1;
	uint8_t dynamicRange: 1;
	uint8_t ycbcrColourimety: 1;

};

struct Connection {
	LinkRate linkRate;

	uint8_t numLanes;
	uint8_t lane_0_1Status;
	uint8_t laneAlignStatus;
	uint8_t voltageSwing: 4;
	uint8_t preEmphasis: 4;

	uint8_t supportsDownSpread: 1;
	uint8_t supportsEnhancedFrame: 1;
	uint8_t supportsTrainingPattern3: 1;
	uint8_t enhancedFrameEnabled: 1;
	uint8_t downSpreadEnabled: 1;
	uint8_t msaTimingParIgnored: 1;
	uint8_t connected: 1;
	uint8_t : 0;

	uint32_t delayRateUS;

	static uint8_t const MaxVoltageSwing = 3;
	static uint8_t const MaxPreEmphasis = 2;
};

void Init(Connection *link);
void Init(Display *display);
void Init(Mixer *mixer);
void Init(DMADescriptor* dma);

bool IsDisplayConnected(Connection *link);
bool Connect(Connection *link);
bool CopyNativeResolution(Connection *link, VideoTiming *videoTiming);
bool CopyStandardVideoMode(StandardVideoMode videoMode, VideoTiming *videoTiming);
void SetDisplay(Connection *link, Display *display, Mixer *mixer);
void SetMixerDMA(Mixer* mixer);

void SetVBlankInterrupt(bool enable);
void SetCounterMatch0Interrupt(bool enable);
void SetCounterMatch1Interrupt(bool enable);

} // end namespace