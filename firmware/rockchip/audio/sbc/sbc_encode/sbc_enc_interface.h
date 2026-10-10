#ifndef __SBC_ENC_INTERFACE_H
#define __SBC_ENC_INTERFACE_H

#define RAW_FRAME_LENGTH	512		/* 每帧的输入采样数据长度 byte  */
void *  sbc_enc_init();
//void *  sbc_init();
uint32 sbc_enc(void *hEnc,uint8 *input,uint32 in_len,uint8 *output,uint32 *Frame_size);



#endif 