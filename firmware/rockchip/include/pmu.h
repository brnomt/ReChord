/* pmu.h — PMU register access (RecordControl.c). */
#ifndef PMU_H
#define PMU_H
#include "typedef.h"

#define PMU_BASE  0x400E0000UL
typedef struct {
    volatile uint32_t PMU_SYS_REG0;
    volatile uint32_t PMU_SYS_REG1;
    volatile uint32_t PMU_SYS_REG2;
    volatile uint32_t PMU_SYS_REG3;
    volatile uint32_t PMU_SYS_REG4;
    volatile uint32_t PMU_SYS_REG5;
    volatile uint32_t PMU_SYS_REG6;
    volatile uint32_t PMU_SYS_REG7;
    volatile uint32_t PMU_SYS_REG8;
    volatile uint32_t PMU_SYS_REG9;
} RKNANO_PMU_REGS;

#define PmuReg ((volatile RKNANO_PMU_REGS *)PMU_BASE)
API void PmuWriteReg(uint32 reg, uint32 val);
API uint32 PmuReadReg(uint32 reg);
#endif /* PMU_H */
