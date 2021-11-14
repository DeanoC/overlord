#pragma once

#include "osservices/ipi3_transport.h"
namespace IPI3_OsServer {

void Init();
void Handler(IPI_Channel senderChannel);

void DebugInlinePrint(const IPI3_Msg *msgBuffer) NON_NULL(1);
void DebugPtrPrint(IPI_Channel senderChannel, const IPI3_Msg *msgBuffer) NON_NULL(2);

void DdrLoBlockAlloc(IPI_Channel senderChannel, const IPI3_Msg *msgBuffer) NON_NULL(2);
void DdrLoBlockFree(const IPI3_Msg *msgBuffer) NON_NULL(1);
void DdrHiBlockAlloc(IPI_Channel senderChannel, const IPI3_Msg *msgBuffer) NON_NULL(2);
void DdrHiBlockFree(const IPI3_Msg *msgBuffer) NON_NULL(1);

void ScreenConsoleConfig(const IPI3_Msg *msgBuffer) NON_NULL(1);
void ScreenConsoleInlinePrint(const IPI3_Msg *msgBuffer) NON_NULL(1);
void ScreenConsolePtrPrint(IPI_Channel senderChannel, const IPI3_Msg *msgBuffer) NON_NULL(2);

// for faster pmu direct send to to the UART
void PutSizedData(uint32_t size, const uint8_t *text);
ALWAYS_INLINE void PutByte(const uint8_t c) {
	PutSizedData(1, &c);
}

} // end namespace
