/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "core/core.h"
#include "dbg/print.h"
#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "library_defines/library_defines.h"

#if IKUY_HAVE_LIB_XILINX_ZYNQPS8
#include "zynqmp/sdcard.h"
#endif

// Definitions of physical drive number for each drive
#define DEV_SDCARD	0	// Map MMC/SD card to physical drive 0

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
// Physical drive nmuber to identify the drive
DSTATUS disk_initialize (BYTE pdrv)
{
	switch (pdrv) {
		case DEV_SDCARD :
#if IKUY_HAVE_LIB_XILINX_ZYNQPS8
			if( FATFS_ZynqMPSDCardInit() ) {
				return 0;
			} else return STA_NOINIT;
#else
			return STA_NOINIT;
#endif
	}
	return STA_NOINIT;
}

// Get Drive Status
// Physical drive nmuber to identify the drive
DSTATUS disk_status ( BYTE pdrv )
{
	switch (pdrv) {
		case DEV_SDCARD:
#if IKUY_HAVE_LIB_XILINX_ZYNQPS8
			return 0;
#else
			return STA_NOINIT;
#endif
			//		result = MMC_disk_status();
			//		return stat;

	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv) {
	case DEV_SDCARD :
#if IKUY_HAVE_LIB_XILINX_ZYNQPS8
		if(FATFS_ZynqMPSDCardRead(sector, buff, count) ) {
			return RES_OK;
		} else return RES_ERROR;
#else
		return RES_PARERR;
#endif
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	switch (pdrv) {
	case DEV_SDCARD :
#if IKUY_HAVE_LIB_XILINX_ZYNQPS8
			if(FATFS_ZynqMPSDCardWrite(sector, buff, count) ) {
				return RES_OK;
			} else return RES_ERROR;
#else
			return RES_PARERR;
#endif
	}

	return RES_PARERR;
}

#endif

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff) {
	switch(cmd) {
#if IKUY_HAVE_LIB_XILINX_ZYNQPS8
		case CTRL_SYNC:
			FATFS_ZynqMPSDCardIdle();
			return 0;
#endif
		default:
			return RES_ERROR;
	}
}
