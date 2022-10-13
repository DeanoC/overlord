#include "core/core.h"
#include "utils/boot_psi.h"
#include "utils/busy_sleep.h"

void raw_debug_printf(const char *format, ...);

#define PSI_INIT_TIMEOUT 100000000

typedef struct psi_InternalState
{
  PSI_IWord const *pc;
  uint32_t *curBankAddr;
} psi_InternalState;

#define EXTRACT_REG(wordA) ((wordA & PSIC_REGISTER_MASK) >> PSIC_REGISTER_SHIFT)
#define EXTRACT_LOOP(wordA) ((wordA & PSIC_LOOP_MASK) >> PSIC_LOOP_SHIFT)
#define WRITE_REG(state, reg, value) *(state.curBankAddr + reg) = value
#define READ_REG(state, reg) *(state.curBankAddr + reg)

void psi_RunRegisterProgram(PSI_IWord const *program)
{
  psi_InternalState state = {0};
  uint32_t instructionCount = 0;
  uint32_t* backUp = 0;
  state.pc = program;

  while ((*state.pc & PSIC_INSTRUCTION_MASK) != PSIC_END_PROGRAM)
  {
    uint32_t pcincr = 0;
    uint32_t wordA = *(state.pc + pcincr);
    pcincr++;

    switch ((wordA & PSIC_INSTRUCTION_MASK))
    {
    case PSIC_SET_BANK:
    {
      state.curBankAddr = (uint32_t *)(uintptr_t)(wordA & 0xFFFF0000);
      break;
    }
    case PSIC_WRITE_32BIT:
    {
PSIC_WRITE_32BIT_label:;
      uint32_t loopCount = EXTRACT_LOOP(wordA);
      uint32_t reg = EXTRACT_REG(wordA);
      uint32_t wordB = *(state.pc + pcincr);
      pcincr++;
      while (loopCount)
      {
        WRITE_REG(state, reg, wordB);
        reg++;
        loopCount--;
      }
      break;
    }
    case PSIC_WRITE_MASKED_32BIT:
    {
PSIC_WRITE_MASKED_32BIT_label:;
		uint32_t mask = *(state.pc + pcincr);
      pcincr++;
      uint32_t wordB = *(state.pc + pcincr);
      pcincr++;
      uint32_t loopCount = EXTRACT_LOOP(wordA);
      uint32_t reg = EXTRACT_REG(wordA);
      while (loopCount)
      {
        uint32_t v = (READ_REG(state, reg) & (~mask)) | (wordB & mask);
        WRITE_REG(state, reg, v);
        reg++;
        loopCount--;
      }
      break;
    }
    case PSIC_MULTI_WRITE_32BIT:
    {
      uint32_t loopCount = EXTRACT_LOOP(wordA);
      uint32_t reg = EXTRACT_REG(wordA);
      while (loopCount)
      {
        uint32_t wordB = *(state.pc + pcincr);
        pcincr++;
        WRITE_REG(state, reg, wordB);
        reg++;
        loopCount--;
      }
      break;
    }
    case PSIC_MULTI_WRITE_MASKED_32BIT:
    {
      uint32_t mask = *(state.pc + pcincr);
      pcincr++;
      uint32_t loopCount = EXTRACT_LOOP(wordA);
      uint32_t reg = EXTRACT_REG(wordA);
      while (loopCount)
      {
        uint32_t wordB = *(state.pc + pcincr);
        pcincr++;
        uint32_t v = (READ_REG(state, reg) & (~mask)) | (wordB & mask);
        WRITE_REG(state, reg, v);
        reg++;
        loopCount--;
      }
      break;
    }
    case PSIC_MULTI_WRITE_MULTI_MASKED_32BIT:
    {
      uint32_t loopCount = EXTRACT_LOOP(wordA);
      uint32_t reg = EXTRACT_REG(wordA);

      while (loopCount)
      {
        uint32_t mask = *(state.pc + pcincr);
        pcincr++;
        uint32_t wordB = *(state.pc + pcincr);
        pcincr++;
        uint32_t v = (READ_REG(state, reg) & (~mask)) | (wordB & mask);
        WRITE_REG(state, reg, v);
        reg++;
        loopCount--;
      }
      break;
    }

    case PSIC_MULTI_WRITE_MASKED_16BIT:
    {
      uint32_t mask = *(state.pc + pcincr);
      pcincr++;
      uint32_t loopCount = EXTRACT_LOOP(wordA);
      uint32_t reg = EXTRACT_REG(wordA);
      bool oddCount = loopCount & 0x1;
      loopCount = loopCount / 2;
      while (loopCount)
      {
        uint32_t wordB = *(state.pc + pcincr);
        pcincr++;
        uint16_t bhi = (wordB >> 16) & 0xFFFF;
        uint16_t blo = wordB & 0xFFFF;

        uint32_t v0 = (READ_REG(state, reg) & (~mask)) | (bhi & mask);
        WRITE_REG(state, reg, v0);
        reg++;
        uint32_t v1 = (READ_REG(state, reg) & (~mask)) | (blo & mask);
        WRITE_REG(state, reg, v1);
        reg++;

        loopCount--;
      }
      if (oddCount)
      {
        uint32_t wordB = *(state.pc + pcincr);
        pcincr++;
        uint16_t bhi = (wordB >> 16) & 0xFFFF;
        uint32_t v0 = (READ_REG(state, reg) & (~mask)) | (bhi & mask);
        WRITE_REG(state, reg, v0);
        reg++;
      }
      break;
    }

    case PSIC_POLL_MASKED_32BIT:
    {
PSIC_POLL_MASKED_32BIT_label:;
		int timeout = 0;
      uint32_t mask = *(state.pc + pcincr);
      pcincr++;
      while (!(READ_REG(state, EXTRACT_REG(wordA)) & mask))
      {
        if (timeout > PSI_INIT_TIMEOUT)
        {
        	raw_debug_printf("init program timeout! ERROR at instruction %d\r\n", instructionCount);
          break;
        }
        timeout++;
      }
      break;
    }
    case PSIC_DELAY: {
      int us = wordA & PSIC_INSTRUCTION_MASK;
      Utils_BusyMicroSleep(us);
      break;
    }
    case PSIC_FAR_POLL_MASKED_32BIT:
    {
      backUp = state.curBankAddr;
      uint32_t bank = *(state.pc + pcincr);
      state.curBankAddr = (uint32_t *)(uintptr_t)(bank & 0xFFFF0000);
      pcincr++;
      goto PSIC_POLL_MASKED_32BIT_label;
      break;
    }
    case PSIC_FAR_WRITE_32BIT: {
        backUp = state.curBankAddr;
        uint32_t bank = *(state.pc + pcincr);
        state.curBankAddr = (uint32_t *)(uintptr_t)(bank & 0xFFFF0000);
        pcincr++;
        goto PSIC_WRITE_32BIT_label;
        break;
    }

    case PSIC_FAR_WRITE_MASKED_32BIT:
    {
        backUp = state.curBankAddr;
        uint32_t bank = *(state.pc + pcincr);
        state.curBankAddr = (uint32_t *)(uintptr_t)(bank & 0xFFFF0000);
        pcincr++;
        goto PSIC_WRITE_MASKED_32BIT_label;
        break;
    }

    default:
      break;
    }

    if(backUp != 0) {
    	state.curBankAddr = backUp;
    	backUp = 0;
    }

    instructionCount++;
    state.pc += pcincr;
  }
}