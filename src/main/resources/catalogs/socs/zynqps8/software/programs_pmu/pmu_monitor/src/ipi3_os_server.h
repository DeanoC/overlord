#pragma once

#include "osservices/ipi3_transport.h"
#ifdef __cplusplus
extern "C" {
#endif

	void IPI3_OSServiceInit();
	bool IPI3_OSServiceHandler(IPI_Channel senderChannel);

	void IPI3_OSServer_InlinePrint(const IPI3_Msg * msgBuffer) NON_NULL(1);
	void IPI3_OSServer_DdrLoBlockAlloc(IPI_Channel senderChannel, const IPI3_Msg * msgBuffer) NON_NULL(2);
	void IPI3_OSServer_DdrLoBlockFree(const IPI3_Msg * msgBuffer) NON_NULL(1);
	void IPI3_OSServer_PtrPrint(IPI_Channel senderChannel, const IPI3_Msg * msgBuffer) NON_NULL(2);
	void IPI3_OSServer_DdrHiBlockAlloc(IPI_Channel senderChannel, const IPI3_Msg * msgBuffer) NON_NULL(2);
	void IPI3_OSServer_DdrHiBlockFree(const IPI3_Msg * msgBuffer) NON_NULL(1);

	// for faster pmu direct send to to the UART
	void PutSizedData(uint32_t size, const uint8_t * text);
	ALWAYS_INLINE void PutByte(const uint8_t c) {
		PutSizedData(1, &c);
	}

#ifdef __cplusplus
}
#endif
