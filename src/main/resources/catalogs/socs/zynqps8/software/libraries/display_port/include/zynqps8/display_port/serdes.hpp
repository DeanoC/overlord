#pragma once

namespace DisplayPort::Display::Serdes {
enum LinkStatus {
	LS_OKAY = 0,
	LS_CLOCK_RECOVERY_FAILED = 0x1,
	LS_CHANNEL_EQUALISATION_FAILED = 0x2,
	LS_LANES_ALIGNED_FAILED = 0x4
};

void UpdateLaneStatusAdjReqs(struct Connection* display);

void ResetPhy(Connection* display);
void SetLinkRate(struct Connection* display, LinkRate rate);
void SetLaneCount(struct Connection* display, unsigned int count);
bool SetDownSpread(struct Connection* display, bool enable);
bool SetEnhancedFrameMode(struct Connection* display, bool enable);
bool SetVoltageSwingAndPreEmphasis(struct Connection* display);

bool CheckLanesAligned(struct Connection const* display);
bool CheckLanesClockRecovery(struct Connection const* display);
bool CheckLanesChannelEqualisation(struct Connection const* display);
LinkStatus CheckLinkStatus(struct Connection const* display);

void StallForPhyReady(struct Connection *display);

} // end namespace
