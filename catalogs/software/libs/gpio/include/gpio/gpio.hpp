#include "core/core.h"


namespace Gpio {

enum class Direction : uint8_t  {
	In = 0,
	Out = 1
};

#if SOC_zynqmp
static constexpr int NumBanks = 6;
static constexpr int NumPins = 174;
#endif

/// Write a bank of 32 pins
void WriteBank(uint8_t bank_, uint32_t values_);
/// Read an entire 32 bit bank of pins
uint32_t ReadBank(uint8_t bank_);
/// Set direction of entire bank
void SetBankDirections(uint8_t bank_, uint32_t direction_);
/// Get direction of an entire bank
uint32_t GetBankDirections(uint8_t bank_);
// Enable outputs of an entire bank
void SetBankOutputs(uint8_t bank_, uint32_t outputs_);
// Get outputs of an entire bank
uint32_t GetBankOutputs(uint8_t bank_);
// Convert a pin into its bank and bit offset (0 to 31)
constexpr void PinToBankAndOffset(uint8_t pin_, uint8_t & outBank_, uint8_t & outBitOffset_);
/// write a particular pin
void Write(uint8_t pin_, bool value_);
/// read a particular pin
bool Read(uint8_t pin_);
/// Set direction of pin
void SetDirection(uint8_t pin_, Direction direction_);
/// Get direction of pin
Direction GetDirection(uint8_t pin_);
// Enable outputs of a pin
void SetOutput(uint8_t pin_, bool output_);
// Get outputs of an entire bank
bool GetOutput(uint8_t pin_);


};