#pragma once

struct HostInterface {
	enum class State : uint8_t {
		RECEIVING_COMMAND,
		PROCESSING_COMMAND,
		POST_RECEIVE_DATA
	} currentState;

	enum class DownloadTarget : uint8_t {
		A53,
		R5F,
		DATA
	} currentDownloadTarget;

	uintptr_all_t downloadAddress;
	uintptr_all_t currentDownloadAddress;


	void Loop() __attribute__((noreturn));

	void EchoCmd(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int findCount);
	void DownloadCmd(uint8_t *cmdBuffer, uint32_t cmdBufferHead,unsigned int const *finds, unsigned int findCount);
	uint8_t * ReceiveData(uintptr_t tmpBufferAddr, uint8_t *tmpBuffer, uint32_t tmpBufferSize);

	void PostDownload();
	void ZModemDownload();
};
