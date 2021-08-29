#pragma once
/******************************************************************************
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

void DCacheEnable(void);
void ICacheEnable(void);
void DCacheDisable(void);
void ICacheDisable(void);

void DCacheCleanAndInvalidate(void);
void DCacheCleanAndInvalidateRange(uintptr_t adr, uintptr_t  len);
void DDCacheCleanAndInvalidateLine(uintptr_t  adr);

void ICacheCleanAndInvalidate(void);
void ICacheCleanAndInvalidateRange(uintptr_t adr, uintptr_t  len);
void ICacheCleanAndInvalidateLine(uintptr_t  adr);

uint32_t GetDCacheLineSizeInBytes(uint32_t level);

uint32_t GetDCacheNumWays(uint32_t level);

uint32_t GetDCacheNumSets(uint32_t level);