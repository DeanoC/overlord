#include "core/core.h"
#include "zynqps8/display_port/aux.hpp"
#include "zynqps8/display_port/display.hpp"
#include "zynqps8/display_port/eedid.hpp"
#include "dbg/print.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/eedid.h"
#include "dbg/assert.h"

namespace DisplayPort::Display::Eedid {

static const uint8_t EdidAddress = 0x50;

#define XDPPSU_EDID_EXT_BLOCK_COUNT      0x7E

void DumpBlock0(uint8_t const *data) {
	debug_printf("EDID Version %d.%d\n", data[EEDID_DISPLAY_PORT_MAJOR_VERSION_OFFSET], data[EEDID_DISPLAY_PORT_MINOR_VERSION_OFFSET]);
	if(HW_REG_DECODE_BIT(EEDID, VIDEO_INPUT_DEFINITION, DIGITAL_VIDEO, data[EEDID_VIDEO_INPUT_DEFINITION_OFFSET])) {
		debug_print("Digital Video: ");
		switch(HW_REG_DECODE_FIELD(EEDID, VIDEO_INPUT_DEFINITION, DIGITAL_COLOUR_BIT_DEPTH, data[EEDID_VIDEO_INPUT_DEFINITION_OFFSET])) {
			default:
				debug_printf("Unknown colour bits per primary colour %d\n", HW_REG_DECODE_FIELD(EEDID, VIDEO_INPUT_DEFINITION, DIGITAL_COLOUR_BIT_DEPTH, data[EEDID_VIDEO_INPUT_DEFINITION_OFFSET])); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_COLOUR_BIT_DEPTH_UNDEFINED:
				debug_printf("Unknown colour bits per primary colour\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_COLOUR_BIT_DEPTH_6_BITS_PER_PRIMARY_COLOUR:
				debug_print("6 Bit Colour per primary colour\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_COLOUR_BIT_DEPTH_8_BITS_PER_PRIMARY_COLOUR:
				debug_print("8 Bit Colour per primary colour\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_COLOUR_BIT_DEPTH_10_BITS_PER_PRIMARY_COLOUR:
				debug_print("10 Bit Colour per primary colour\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_COLOUR_BIT_DEPTH_12_BITS_PER_PRIMARY_COLOUR:
				debug_print("12 Bit Colour per primary colour\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_COLOUR_BIT_DEPTH_14_BITS_PER_PRIMARY_COLOUR:
				debug_print("14 Bit Colour per primary colour\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_COLOUR_BIT_DEPTH_16_BITS_PER_PRIMARY_COLOUR:
				debug_print("16 Bit Colour per primary colour\n"); break;
		}
		switch(HW_REG_DECODE_FIELD(EEDID, VIDEO_INPUT_DEFINITION, DIGITAL_VIDEO_INTERFACE_SUPPORT, data[EEDID_VIDEO_INPUT_DEFINITION_OFFSET])) {
			default:
				debug_printf("Unknown digital video interface %d\n", HW_REG_DECODE_FIELD(EEDID, VIDEO_INPUT_DEFINITION, DIGITAL_VIDEO_INTERFACE_SUPPORT, data[EEDID_VIDEO_INPUT_DEFINITION_OFFSET])); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_VIDEO_INTERFACE_SUPPORT_NOT_DEFINED:
				debug_print("Unknown digital video interface\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_VIDEO_INTERFACE_SUPPORT_DVI_SUPPORTED:
				debug_print("DVI video interface\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_VIDEO_INTERFACE_SUPPORT_HDMI_A_SUPPORTED:
				debug_print("HDMI-a video interface\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_VIDEO_INTERFACE_SUPPORT_HDMI_B_SUPPORTED:
				debug_print("HDMI-b video interface\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_VIDEO_INTERFACE_SUPPORT_MDDI_SUPPORTED:
				debug_print("MDDI video interface\n"); break;
			case EEDID_VIDEO_INPUT_DEFINITION_DIGITAL_VIDEO_INTERFACE_SUPPORT_DISPLAY_PORT_SUPPORTED:
				debug_print("Display Port video interface\n"); break;
		}
		switch(HW_REG_DECODE_FIELD(EEDID, FEATURE_SUPPORT, COLOUR_FORMATS_SUPPORTED, data[EEDID_FEATURE_SUPPORT_OFFSET])) {
			default:
			case EEDID_FEATURE_SUPPORT_COLOUR_FORMATS_SUPPORTED_RGB_444:
				debug_print("RGB 4:4:4\n"); break;
			case EEDID_FEATURE_SUPPORT_COLOUR_FORMATS_SUPPORTED_RGB_444_YCRCB444:
				debug_print("RGB 4:4:4 + YCrCb 4:4:4\n"); break;
			case EEDID_FEATURE_SUPPORT_COLOUR_FORMATS_SUPPORTED_RGB_444_YCRCB422:
				debug_print("RGB 4:4:4 + YCrCb 4:2:2\n"); break;
			case EEDID_FEATURE_SUPPORT_COLOUR_FORMATS_SUPPORTED_RGB_444_YCRCB444_YCRCB422:
				debug_print("RGB 4:4:4 + YCrCb 4:4:4 + YCrCb 4:2:2\n"); break;
		}
		if(HW_REG_DECODE_BIT(EEDID, FEATURE_SUPPORT, SRGB_STANDARD_IS_DEFAULT, data[EEDID_FEATURE_SUPPORT_OFFSET])) {
			debug_print("sRGB standard is default\n");
		}

		bool PTMIsNative = false;
		if(HW_REG_DECODE_BIT(EEDID, FEATURE_SUPPORT, PTM_IS_NATIVE, data[EEDID_FEATURE_SUPPORT_OFFSET])) {
			debug_print("Primary Timing Mode is native\n");
			PTMIsNative = true;
		}
		if(HW_REG_DECODE_BIT(EEDID, FEATURE_SUPPORT, CONTINUOUS_FREQUENCY, data[EEDID_FEATURE_SUPPORT_OFFSET])) {
			debug_print("Continuous Frequency is supported\n");
		}
		debug_print("Established Timings:\n");

		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_1, 720_400_70HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("720 x 400 @ 70Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_1, 720_400_88HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("720 x 400 @ 88Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_1, 640_480_60HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("640 x 480 @ 60Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_1, 640_480_67HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("640 x 480 @ 67Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_1, 640_480_72HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("640 x 480 @ 72Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_1, 640_480_75HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("640 x 480 @ 75Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_1, 800_600_56HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("800 x 600 @ 56Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_1, 800_600_60HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("800 x 600 @ 60Hz\n");
		}

		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_2, 800_600_72HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("800 x 600 @ 72Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_2, 800_600_75HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("800 x 600 @ 75Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_2, 832_624_75HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("832 x 624 @ 75Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_2, 1024_768_87HZ_I, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("1024 x 768 @ 87Hz Interlaced\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_2, 1024_768_60HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("1024 x 768 @ 60Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_2, 1024_768_70HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("1024 x 768 @ 70Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_2, 1024_768_75HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("1024 x 768 @ 75Hz\n");
		}
		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_2, 1280_1024_75HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("1280 x 1024 @ 75Hz\n");
		}

		if(HW_REG_DECODE_BIT(EEDID, ESTABLISHED_TIMING_3, 1152_870_75HZ, data[EEDID_ESTABLISHED_TIMING_1_OFFSET])) {
			debug_print("1152 x 870 @ 75Hz\n");
		}

		for(int i = 0;i < 4;++i) {
#define DTD_REG(x) data[baseReg + (EEDID_DTD_##x##_OFFSET - EEDID_DTD_PIXEL_CLOCK_LSB_1_OFFSET)]
			uint32_t const baseReg = EEDID_DTD_PIXEL_CLOCK_LSB_1_OFFSET + i * 18;
			if(!(DTD_REG(PIXEL_CLOCK_LSB_1) == 0 && DTD_REG(PIXEL_CLOCK_MSB_1) == 0) ) {
				uint32_t const pixelClock = ((DTD_REG(PIXEL_CLOCK_MSB_1) << 8) | DTD_REG(PIXEL_CLOCK_LSB_1)) * 10;
				if(i == 0 && PTMIsNative) debug_print("Native DTD\n");
				else debug_printf("DTD %d\n", i);

				debug_printf("Pixel Clock %uKhz\n", (unsigned int)pixelClock);

				uint8_t const hfp_hsp_vfp_vsp = DTD_REG(HFP_HSP_VFP_VSP_MSB_1);

				debug_printf("H Addressable %d H Blank %d H Front Porch %d H Sync Pulse %d\n",
										 DTD_REG(HORIZONTAL_ADDRESSABLE_LSB_1) + (HW_REG_DECODE_FIELD(EEDID, DTD_HA_HB_MSB_1, HA_MSB, DTD_REG(HA_HB_MSB_1)) << 8),
										 DTD_REG(HORIZONTAL_BLANKING_LSB_1) + (HW_REG_DECODE_FIELD(EEDID, DTD_HA_HB_MSB_1, HB_MSB, DTD_REG(HA_HB_MSB_1)) << 8),
										 DTD_REG(HORIZONTAL_FRONT_PORCH_LSB_1) + (HW_REG_DECODE_FIELD(EEDID, DTD_HFP_HSP_VFP_VSP_MSB_1, HFP_MSB, hfp_hsp_vfp_vsp) << 8),
										 DTD_REG(HORIZONTAL_SYNC_PULSE_LSB_1) + (HW_REG_DECODE_FIELD(EEDID, DTD_HFP_HSP_VFP_VSP_MSB_1, HSP_MSB, hfp_hsp_vfp_vsp) << 8)
				);
				uint8_t const vfp_vsp_lsb = DTD_REG(VERTICAL_FRONT_PORCH_VERTICAL_SYNC_PULSE_LSB_1);
				uint8_t const vfp_lsb = HW_REG_DECODE_FIELD(EEDID, DTD_VERTICAL_FRONT_PORCH_VERTICAL_SYNC_PULSE_LSB_1, VFP_LSB, vfp_vsp_lsb);
				uint8_t const vsp_lsb = HW_REG_DECODE_FIELD(EEDID, DTD_VERTICAL_FRONT_PORCH_VERTICAL_SYNC_PULSE_LSB_1, VSP_LSB, vfp_vsp_lsb);

				debug_printf("V Addressable %d, V Blank %d V Front Porch %d V Sync Pulse %d\n",
										 DTD_REG(VERTICAL_ADDRESSABLE_LSB_1) + (HW_REG_DECODE_FIELD(EEDID, DTD_VA_VB_MSB_1, VA_MSB, DTD_REG(VA_VB_MSB_1)) << 8),
										 DTD_REG(VERTICAL_BLANKING_LSB_1) + (HW_REG_DECODE_FIELD(EEDID, DTD_VA_VB_MSB_1, VB_MSB, DTD_REG(VA_VB_MSB_1)) << 8),
										 vfp_lsb + (HW_REG_DECODE_FIELD(EEDID, DTD_HFP_HSP_VFP_VSP_MSB_1, VFP_MSB, hfp_hsp_vfp_vsp) << 4),
										 vsp_lsb + (HW_REG_DECODE_FIELD(EEDID, DTD_HFP_HSP_VFP_VSP_MSB_1, VSP_MSB, hfp_hsp_vfp_vsp) << 4)
										 );
			}
#undef DTD_REG
		}

	}
}

bool ReadBlock(struct Connection* display, uint32_t block, uint8_t* data) {
	return I2CRead(display, EdidAddress, block * EedidBlockSize, EedidBlockSize, data);
}

} // end namespace