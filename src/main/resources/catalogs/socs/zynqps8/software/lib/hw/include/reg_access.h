#pragma once

// HW registers and offsets are 32bit currently even on 64 bit platforms

#define HW_REG_FIELD(type, reg, field) (type##_FIELD(reg,field))
#define HW_REG_FIELD_LSHIFT(type, reg, field) (type##_FIELD_LSHIFT(reg, field))
#define HW_REG_FIELD_MASK(type, reg, field) (type##_FIELD_MASK(reg, field))

#define HW_REG_ENCODE_FIELD(type, reg, field, value) (value << HW_REG_FIELD_LSHIFT(type, reg, field))
#define HW_REG_DECODE_FIELD(type, reg, field, value) ((value & (HW_REG_FIELD_MASK(type, reg, field))) >> HW_REG_FIELD_LSHIFT(type, reg, field))

#define HW_REG_GET(type, reg) hw_RegRead(type##_BASE_ADDR, type##_REGISTER(reg))
#define HW_REG_SET(type, reg, value) hw_RegWrite(type##_BASE_ADDR, type##_REGISTER(reg), (value))

#define HW_REG_SET_BIT(type, reg, field) HW_REG_SET( type, reg, ((HW_REG_GET(type, reg) & ~HW_REG_FIELD_MASK(type, reg, field)) | HW_REG_FIELD(type, reg, field)) )
#define HW_REG_CLR_BIT(type, reg, field) HW_REG_SET( type, reg, (HW_REG_GET(type, reg) & ~HW_REG_FIELD_MASK(type, reg, field)) )
#define HW_REG_GET_BIT(type, reg, field) (!!(HW_REG_GET(type, reg) & HW_REG_FIELD_MASK(type, reg, field)))

#define HW_REG_GET_FIELD(type, reg, field) HW_REG_DECODE_FIELD(type, reg, field, HW_REG_GET(type, reg))
#define HW_REG_MERGE(type, reg, mask, value) HW_REG_SET(type, reg, (HW_REG_GET(type, reg) & ~(mask)) | ((value) & (mask)))
#define HW_REG_MERGE_FIELD(type, reg, field, value) HW_REG_MERGE(type, reg, HW_REG_FIELD_MASK(type, reg, field), HW_REG_ENCODE_FIELD(type, reg, field, value))

#define HW_REG_ARRAY_SET_BIT(type, index, reg, field) HW_REG_SET_BIT(type##index, reg, field)
#define HW_REG_ARRAY_CLR_BIT(type, index, reg, field) HW_REG_CLR_BIT(type##index, reg, field)
#define HW_REG_ARRAY_GET_BIT(type, index, reg, field) HW_REG_GET_BIT(type##index, reg, field)
#define HW_REG_ARRAY_GET(type, index, reg) HW_REG_GET(type##index, reg)
#define HW_REG_ARRAY_GET_FIELD(type, index, reg, field) HW_REG_GET_FIELD( type##index, reg, field)
#define HW_REG_ARRAY_SET(type, index, reg, value) HW_REG_SET(type##index, reg, value)
#define HW_REG_ARRAY_MERGE(type, index, reg, mask, value) HW_REG_MERGE(type##index, reg, mask, value)
#define HW_REG_ARRAY_MERGE_FIELD(type, index, reg, field, value) HW_REG_MERGE_FIELD(type##index, reg, field, value)

#ifdef __cplusplus
extern "C"
{
#endif

ALWAYS_INLINE uint32_t hw_RegRead(const uintptr_t addr, const uint32_t offset_in_bytes) { return *(((const volatile uint32_t *)addr) + offset_in_bytes/sizeof(uint32_t)); }
ALWAYS_INLINE void hw_RegWrite(const uintptr_t addr, const uint32_t offset_in_bytes, const uint32_t value) { *(((volatile uint32_t *)addr) + offset_in_bytes/sizeof(uint32_t)) = value; }

#ifdef __cplusplus
}
#endif

