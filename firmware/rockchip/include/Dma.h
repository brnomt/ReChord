/* Dma.h — DMA API (AudioControl.c). */
#ifndef DMA_H
#define DMA_H
#include "typedef.h"
#include "driverlib_def.h"

typedef enum { DMA_MODE_MEM2MEM, DMA_MODE_MEM2PERI, DMA_MODE_PERI2MEM } DMA_MODE;
API rk_err_t DmaStart(uint32 ch, uint32 src, uint32 dst, uint32 size, pDMA_CFGX cfg, void (*cb)(void));
API void DmaStop(uint32 ch);
API uint32 DmaGetState(uint32 ch);
#endif /* DMA_H */
