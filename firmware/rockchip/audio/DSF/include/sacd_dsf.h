
//#ifndef _SACD_DSDIFF_H_INCLUDED
//#define _SACD_DSDIFF_H_INCLUDED
//#ifdefien DSF_DEC_INCLUDE

#include <stdint.h>
#include "endianess.h"
#include "dsf2pcm_conv.h"
// #include"my.h"
//#include "FsInclude.h"
//#include "File.h"


typedef struct CK_ID {
	uint8_t ckID[4];
}CK_ID;

typedef struct Chunk{
	uint8_t ckID[4];
	//uint64_t ckDataSize;
	uint8_t ckDataSize[8];
}Chunk;

#define DST_DECODER_THREADS 8

#define DST_BUFFER_SIZE (MAX_DSDBITS_INFRAME / 8 * MAX_CHANNELS)
#define DSD_BUFFER_SIZE (MAX_DSDBITS_INFRAME / 8 * MAX_CHANNELS)


typedef struct sacd_dsf_t{
	FILE * pfile;
	uint32_t m_version;
	uint32_t m_samplerate;
	uint32_t m_bit_per_sample;
	uint32_t m_channel_count;
	uint32_t m_channel_type;
    //uint32_t m_sampleCount;
	uint32_t m_data_offset;
	uint32_t m_data_size;
	uint32_t m_file_size;
	uint16_t m_framerate;
	uint32_t m_frame_size;
	uint32_t m_frame_count;
	uint32_t m_decode_frame_count;

	int                   pcm_out_channels;
	unsigned int          pcm_out_channel_map;
	int                   pcm_out_samplerate;
	//int                   pcm_out_bits_per_sample;
	int                   pcm_out_samples;
	int                   dsd_samplerate;
	uint16_t bps;
	int bitrate;
//	uint8_t dsd_data[DST_BUFFER_SIZE];
	uint8_t dsd_data[4096*2];
	uint32_t dsd_size;
    double frameLength;//Ö¡Ê±³¤
}sacd_dsf_t;

BOOL dsf_open(void);
BOOL dsf_close(void);
BOOL dsf_read_frame(uint8_t* frame_data, uint32_t* frame_size);
BOOL dsf_seek(int seconds);
int dsf_decode_init(FILE* pfile);
BOOL dsf_decode(uint8* pcm_out_buf, uint32_t* OutLength);


//#endif
