
#ifndef _ENDIANESS_H_INCLUDED
#define _ENDIANESS_H_INCLUDED

#define MAKE_MARKER(a, b, c, d)    ((d) | ((c) << 8) | ((b) << 16) | ((a) << 24))//¸Ä¹ý
#define MAKE_MARKER16(a, b)    ((b) | ((a) << 8))


/*#define htole16(x)                  (uint16_t) (x)
#define htole32(x)                  (uint32_t) (x)
#define htole64(x)                  (uint64_t) (x)

#define hton16(x)            \
    ((((x) & 0xff00) >> 8) | \
     (((x) & 0x00ff) << 8))
#define hton32(x)                 \
    ((((x) & 0xff000000) >> 24) | \
     (((x) & 0x00ff0000) >> 8) |  \
     (((x) & 0x0000ff00) << 8) |  \
     (((x) & 0x000000ff) << 24))
#define hton64(x)                            \
    ((((x) & 0xff00000000000000ULL) >> 56) | \
     (((x) & 0x00ff000000000000ULL) >> 40) | \
     (((x) & 0x0000ff0000000000ULL) >> 24) | \
     (((x) & 0x000000ff00000000ULL) >> 8) |  \
     (((x) & 0x00000000ff000000ULL) << 8) |  \
     (((x) & 0x0000000000ff0000ULL) << 24) | \
     (((x) & 0x000000000000ff00ULL) << 40) | \
     (((x) & 0x00000000000000ffULL) << 56))

#define SWAP16(x)    x = (hton16(x))
#define SWAP32(x)    x = (hton32(x))
#define SWAP64(x)    x = (hton64(x))
	 */
#endif
