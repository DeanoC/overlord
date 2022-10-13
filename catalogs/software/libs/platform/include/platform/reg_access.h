#pragma once

// HW offsets are 32bit currently even on 64 bit platforms

// field helper
#define HW_REG_FIELD(registerlist, reg, field) registerlist##_##reg##_##field
#define HW_REG_FIELD_LSHIFT(registerlist, reg, field) registerlist##_##reg##_##field##_##LSHIFT
#define HW_REG_FIELD_MASK(registerlist, reg, field) registerlist##_##reg##_##field##_MASK
#define HW_REG_FIELD_ENUM(registerlist, reg, field, enm) registerlist##_##reg##_##field##_##enm

// codecs
#define HW_REG_ENCODE_FIELD(registerlist, reg, field, value) ((value) << HW_REG_FIELD_LSHIFT(registerlist, reg, field))
#define HW_REG_DECODE_FIELD(registerlist, reg, field, value) (((value) & HW_REG_FIELD_MASK(registerlist, reg, field)) >> HW_REG_FIELD_LSHIFT(registerlist, reg, field))
#define HW_REG_ENCODE_ENUM(registerlist, reg, field, enm) HW_REG_ENCODE_FIELD(registerlist,reg,field, HW_REG_FIELD_ENUM(registerlist, reg, field, enm))
#define HW_REG_DECODE_BIT(registerlist, reg, field, value) (!!((value) & HW_REG_FIELD_MASK(registerlist, reg, field)))

// newer style (takes address so easy to use register instances) macros
#define HW_REG_READ(addr, registerlist, reg) hw_RegRead(addr, registerlist##_##reg##_OFFSET)
#define HW_REG_WRITE(addr, registerlist, reg, value) hw_RegWrite(addr, registerlist##_##reg##_OFFSET, (value))
#define HW_REG_RMW(addr, registerlist, reg, mask, value) hw_RegRMW(addr, registerlist##_##reg##_OFFSET, (mask), (value))

#define HW_REG_READ64(addr, registerlist, reg) hw_RegRead64(addr, registerlist##_##reg##_OFFSET)
#define HW_REG_WRITE64(addr, registerlist, reg, value) hw_RegWrite64(addr, registerlist##_##reg##_OFFSET, (value))
#define HW_REG_RMW64(addr, registerlist, reg, mask, value) hw_RegRMW64(addr, registerlist##_##reg##_OFFSET, (mask), (value))

// bit access helpers
#define HW_REG_SET_BIT(addr, registerlist, reg, field) HW_REG_RMW( addr, registerlist, reg, HW_REG_FIELD_MASK(registerlist, reg, field), HW_REG_FIELD_MASK(registerlist, reg, field))
#define HW_REG_CLR_BIT(addr, registerlist, reg, field) HW_REG_RMW( addr, registerlist, reg, HW_REG_FIELD_MASK(registerlist, reg, field), !HW_REG_FIELD_MASK(registerlist, reg, field))
#define HW_REG_GET_BIT(addr, registerlist, reg, field) HW_REG_DECODE_BIT(registerlist, reg, field, HW_REG_READ(addr, registerlist, reg))

// field and enum helpers
#define HW_REG_GET_FIELD(addr, registerlist, reg, field) HW_REG_DECODE_FIELD(registerlist, reg, field, HW_REG_READ(addr, registerlist, reg))
#define HW_REG_SET_FIELD(addr, registerlist, reg, field, value) HW_REG_RMW(addr, registerlist, reg, HW_REG_FIELD_MASK(registerlist, reg, field), HW_REG_ENCODE_FIELD(registerlist, reg, field, value))
#define HW_REG_SET_ENUM(addr, registerlist, reg, field, enm) HW_REG_RMW(addr, registerlist, reg, HW_REG_FIELD_MASK(registerlist, reg, field), HW_REG_ENCODE_ENUM(registerlist, reg, field, enm))

// for easier access to single instance register banks
#define HW_REG_GET_ADDRESS(instance) instance##_BASE_ADDR
#define HW_REG_READ1(registerlist, reg) HW_REG_READ(HW_REG_GET_ADDRESS(registerlist), registerlist, reg)
#define HW_REG_WRITE1(registerlist, reg, value) HW_REG_WRITE(HW_REG_GET_ADDRESS(registerlist), registerlist, reg, value)
#define HW_REG_RMW1(registerlist, reg, mask, value) HW_REG_RMW(HW_REG_GET_ADDRESS(registerlist), registerlist, reg, mask, value)
#define HW_REG_SET_BIT1(registerlist, reg, field) HW_REG_RMW( HW_REG_GET_ADDRESS(registerlist), registerlist, reg, HW_REG_FIELD_MASK(registerlist, reg, field), HW_REG_FIELD_MASK(registerlist, reg, field))
#define HW_REG_CLR_BIT1(registerlist, reg, field) HW_REG_RMW( HW_REG_GET_ADDRESS(registerlist), registerlist, reg, HW_REG_FIELD_MASK(registerlist, reg, field), !HW_REG_FIELD_MASK(registerlist, reg, field))
#define HW_REG_GET_BIT1(registerlist, reg, field) HW_REG_DECODE_BIT(registerlist, reg, field, HW_REG_READ(HW_REG_GET_ADDRESS(registerlist), registerlist, reg))
#define HW_REG_GET_FIELD1(registerlist, reg, field) HW_REG_DECODE_FIELD(registerlist, reg, field, HW_REG_READ(HW_REG_GET_ADDRESS(registerlist), registerlist, reg))
#define HW_REG_SET_FIELD1(registerlist, reg, field, value) HW_REG_RMW(HW_REG_GET_ADDRESS(registerlist), registerlist, reg, HW_REG_FIELD_MASK(registerlist, reg, field), HW_REG_ENCODE_FIELD(registerlist, reg, field, value))

#ifdef __cplusplus
EXTERN_C
{
#endif

ALWAYS_INLINE uint32_t hw_RegRead(const uintptr_t addr_, const uint32_t offset_in_bytes_) {
	return *(((const volatile uint32_t *)addr_) + offset_in_bytes_/sizeof(uint32_t));
}
ALWAYS_INLINE void hw_RegWrite(const uintptr_t addr_, const uint32_t offset_in_bytes_, const uint32_t value_) {
	*(((volatile uint32_t *)addr_) + offset_in_bytes_/sizeof(uint32_t)) = value_;
}
ALWAYS_INLINE void hw_RegRMW(const uintptr_t addr_, const uint32_t offset_in_bytes_, const uint32_t mask_, const uint32_t value_) {
	hw_RegWrite(addr_, offset_in_bytes_, (hw_RegRead(addr_, offset_in_bytes_) & ~(mask_)) | ((value_) & (mask_)));
}

// these read/write 64 bit registers
ALWAYS_INLINE uint64_t hw_RegRead64(const uintptr_t addr, const uint64_t offset_in_bytes){
	return *(((const volatile uint64_t *)addr) + offset_in_bytes/sizeof(uint64_t));
}
ALWAYS_INLINE void hw_RegWrite64(const uintptr_t addr, const uint64_t offset_in_bytes, const uint64_t value){
	*(((volatile uint64_t *)addr) + offset_in_bytes/sizeof(uint64_t)) = value;
}
ALWAYS_INLINE void hw_RegRMW64(const uintptr_t addr_, const uint64_t offset_in_bytes_, const uint64_t mask_, const uint64_t value_) {
	hw_RegWrite64(addr_, offset_in_bytes_, (hw_RegRead64(addr_, offset_in_bytes_) & ~(mask_)) | ((value_) & (mask_)));
}


#ifdef __cplusplus
}
#endif