/*
 * module_loader.c — RMF1 module loader implementation.
 *
 * Freestanding-friendly: no libc, no vendor headers. Byte copy/zero helpers
 * are private so the TU has zero external dependencies (host `cc` and
 * `arm-none-eabi-gcc` compile it identically).
 */
#include "module_loader.h"

/* --- private byte helpers (no libc dependency) -------------------------- */

static void rmf_memcopy(void *dst, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    size_t i;
    for (i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

static void rmf_memzero(void *dst, size_t n)
{
    uint8_t *d = (uint8_t *)dst;
    size_t i;
    for (i = 0; i < n; i++) {
        d[i] = 0;
    }
}

/* --- checksum (the device's documented primitive) ----------------------- */

uint32_t rmf_crc32(const void *data, size_t len)
{
    /* Non-reflected, MSB-first: poly 0x04C10DB7, init 0, xorout 0. */
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = 0;
    size_t i;
    int bit;

    for (i = 0; i < len; i++) {
        crc ^= ((uint32_t)p[i]) << 24;
        for (bit = 0; bit < 8; bit++) {
            if (crc & 0x80000000u) {
                crc = (crc << 1) ^ 0x04C10DB7u;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

/* --- loader ------------------------------------------------------------ */

rmf_result_t rmf_load_module(const void *blob, size_t blob_len,
                             void *window, size_t window_size,
                             rmf_loaded_t *out)
{
    rmf1_header_t h;
    const uint8_t *code;

    if (blob == NULL || window == NULL) {
        return RMF_ERR_ARG;
    }

    /* 1. Structural: a full header must be present before we trust fields. */
    if (blob_len < RMF1_HEADER_SIZE) {
        return RMF_ERR_LENGTH;
    }
    /* Copy through a local: the blob may live at any alignment. */
    rmf_memcopy(&h, blob, sizeof h);

    /* 2. Header identity. */
    if (h.magic != RMF1_MAGIC) {
        return RMF_ERR_MAGIC;
    }
    if (h.version != RMF1_VERSION) {
        return RMF_ERR_VERSION;
    }
    if ((h.flags & ~RMF1_FLAG_KNOWN) != 0u) {
        return RMF_ERR_FLAGS;
    }

    /*
     * 3. Length consistency (RMF_ERR_LENGTH):
     *   - code must exist and hold the entry point;
     *   - entry_offset even (Thumb halfword alignment);
     *   - bss must not overlap code (bss_start >= code_len) and be
     *     word-aligned (the loader zeroes it with byte stores, but 4-byte
     *     alignment is part of the layout contract);
     *   - the file must actually contain the claimed code.
     * Sums are done in 64-bit so hostile u32 fields cannot wrap past a check
     * on the 32-bit target.
     */
    if (h.code_len == 0u) {
        return RMF_ERR_LENGTH;
    }
    if (h.entry_offset >= h.code_len || (h.entry_offset & 1u) != 0u) {
        return RMF_ERR_LENGTH;
    }
    if (h.bss_start < h.code_len || (h.bss_start & 3u) != 0u) {
        return RMF_ERR_LENGTH;
    }
    if ((uint64_t)blob_len <
        (uint64_t)RMF1_HEADER_SIZE + (uint64_t)h.code_len) {
        return RMF_ERR_LENGTH;
    }

    /* 4. Window fit (RMF_ERR_WINDOW): code at base, bss at base+bss_start. */
    if ((uint64_t)h.code_len > (uint64_t)window_size) {
        return RMF_ERR_WINDOW;
    }
    if ((uint64_t)h.bss_start + (uint64_t)h.bss_len > (uint64_t)window_size) {
        return RMF_ERR_WINDOW;
    }

    /* 5. Verify BEFORE deploying anything, so a bad module never runs. */
    code = (const uint8_t *)blob + RMF1_HEADER_SIZE;
    if (rmf_crc32(code, h.code_len) != h.crc32) {
        return RMF_ERR_CRC;
    }

    /* 6. Deploy. */
    rmf_memcopy(window, code, h.code_len);
    rmf_memzero((uint8_t *)window + h.bss_start, h.bss_len);

    if (out != NULL) {
        out->id = h.id;
        out->flags = (uint16_t)h.flags;
        out->base = window;
        out->code_len = h.code_len;
        out->bss_start = h.bss_start;
        out->bss_len = h.bss_len;
        /* Thumb state bit set: the entry is called directly on Cortex-M. */
        out->entry = (uintptr_t)window + (uintptr_t)h.entry_offset + 1u;
    }
    return RMF_OK;
}

const char *rmf_strerror(rmf_result_t r)
{
    switch (r) {
    case RMF_OK:             return "ok";
    case RMF_ERR_ARG:        return "bad argument";
    case RMF_ERR_MAGIC:      return "bad magic";
    case RMF_ERR_VERSION:    return "unsupported version";
    case RMF_ERR_FLAGS:      return "unknown flags";
    case RMF_ERR_LENGTH:     return "bad length";
    case RMF_ERR_WINDOW:     return "window overflow";
    case RMF_ERR_CRC:        return "checksum mismatch";
    case RMF_ERR_DUPLICATE:  return "duplicate id";
    case RMF_ERR_FULL:       return "registry full";
    case RMF_ERR_NOT_FOUND:  return "not found";
    default:                 return "unknown error";
    }
}
