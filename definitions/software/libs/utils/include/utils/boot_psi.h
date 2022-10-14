#include <stdint.h>

#define PSIC_INSTRUCTION_SHIFT (0)
#define PSIC_INSTRUCTION_MASK (0xF << PSIC_INSTRUCTION_SHIFT)
#define PSIC_LOOP_SHIFT (4)
#define PSIC_LOOP_MASK (0xFF << PSIC_LOOP_SHIFT)
#define PSIC_REGISTER_SHIFT (16)
#define PSIC_REGISTER_MASK (0xFFFF << PSIC_REGISTER_SHIFT)

typedef enum PSI_CONTROL
{
  PSIC_END_PROGRAM = 0 << PSIC_INSTRUCTION_SHIFT,
  PSIC_SET_BANK = 1 << PSIC_INSTRUCTION_SHIFT, // upper 16 bits, is upper 16 bits of address bank
  PSIC_WRITE_32BIT = 2 << PSIC_INSTRUCTION_SHIFT,
  PSIC_WRITE_MASKED_32BIT = 3 << PSIC_INSTRUCTION_SHIFT,
  PSIC_POLL_MASKED_32BIT = 4 << PSIC_INSTRUCTION_SHIFT,

  PSIC_MULTI_WRITE_32BIT = 5 << PSIC_INSTRUCTION_SHIFT,
  PSIC_MULTI_WRITE_MASKED_32BIT = 6 << PSIC_INSTRUCTION_SHIFT,
  PSIC_MULTI_WRITE_MULTI_MASKED_32BIT = 7 << PSIC_INSTRUCTION_SHIFT,
  PSIC_MULTI_WRITE_MASKED_16BIT = 8 << PSIC_INSTRUCTION_SHIFT,

  PSIC_DELAY = 9 << PSIC_INSTRUCTION_SHIFT,

  PSIC_FAR_POLL_MASKED_32BIT = 10 << PSIC_INSTRUCTION_SHIFT,
  PSIC_FAR_WRITE_32BIT = 11 << PSIC_INSTRUCTION_SHIFT,
  PSIC_FAR_WRITE_MASKED_32BIT = 12 << PSIC_INSTRUCTION_SHIFT,

} PSI_CONTROL;

typedef uint32_t PSI_IWord;

#define PSI_END_PROGRAM PSIC_END_PROGRAM
#define PSI_SET_REGISTER_BANK(bank) (PSIC_SET_BANK | ((bank##_BASE_ADDR) & 0xFFFF0000))
#define PSI_DELAY_US(us) (PSIC_DELAY | (us))
#define PSI_DELAY_MS(ms) PSI_DELAY_US((ms) * 1000)

#define PSI_REG(registerlist, reg) ((((uint32_t)(registerlist##_##reg##_OFFSET>>2))) << PSIC_REGISTER_SHIFT)

#define PSI_WRITE_32(type, reg, value) (PSIC_WRITE_32BIT | (1 << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), (value)
#define PSI_WRITE_MASKED_32(type, reg, mask, value) (PSIC_WRITE_MASKED_32BIT | (1 << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), (mask), (value)
#define PSI_POLL_MASKED_32(type, reg, mask) (PSIC_POLL_MASKED_32BIT | PSI_REG(type,reg)), (mask)

// allow a constant set to continuous registers (no more than 255 register per loop)
#define PSI_WRITE_N_32(type, reg, n, value) (PSIC_WRITE_32BIT | ((n) << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), (value)
#define PSI_WRITE_N_MASKED_32(type, reg, n, mask, value) (PSIC_WRITE_MASKED_32BIT | ((n) << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), (mask), (value)

// write multiple values to continuous registers with a shared mask (if masked)
#define PSI_MULTI_WRITE_32(type, reg, n, ...) (PSIC_MULTI_WRITE_32BIT | ((n) << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), __VA_ARGS__
#define PSI_MULTI_WRITE_MASKED_32(type, reg, n, mask, ...) (PSIC_MULTI_WRITE_MASKED_32BIT | ((n) << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), (mask), __VA_ARGS__
// write multiple values to continuous registers each with its own mask
#define PSI_MULTI_WRITE_MULTI_MASKED_32(type, reg, n, ...) (PSIC_MULTI_WRITE_MULTI_MASKED_32BIT | ((n) << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), __VA_ARGS__

// TODO generalize 16 bit, this was implemented for MIO specifically
// odd number of transfer must be padded to 32 bit boundary
// for odd transfers set n = odd number and add a dummy low
#define PSI_MULTI_WRITE_MASKED_16(type, reg, n, mask, ...) (PSIC_MULTI_WRITE_MASKED_16BIT | ((n) << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), (mask), __VA_ARGS__
#define PSI_PACK_16(a, b) ((uint32_t)(a) << 16U) | ((uint32_t)(b))


// a far call takes 4 bytes more per register, so FAR avoids a following SET_REGISTER_BANK to restore the bank
#define PSI_FAR_POLL_MASKED_32(type, reg, mask)  (PSIC_FAR_POLL_MASKED_32BIT | (1 << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), ((type##_BASE_ADDR) & 0xFFFF0000), (mask)
#define PSI_FAR_WRITE_32(type, reg, val) (PSIC_FAR_WRITE_32BIT | (1 << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), ((type##_BASE_ADDR) & 0xFFFF0000), (val)
#define PSI_FAR_WRITE_MASKED_32(type, reg, mask, val) (PSIC_FAR_WRITE_MASKED_32BIT  | (1 << PSIC_LOOP_SHIFT) | PSI_REG(type,reg)), ((type##_BASE_ADDR) & 0xFFFF0000), (mask), (val)

#ifdef __cplusplus
EXTERN_C
{
#endif

void psi_RunRegisterProgram(PSI_IWord const *program);

#ifdef __cplusplus
}
#endif
