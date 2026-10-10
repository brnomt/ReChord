/*
 * module_format.h — ReChord Module Format v1 ("RMF1").
 *
 * Clean-room overlay-module container, written from documented FORMAT FACTS
 * only (docs/re/route-b-minimum.md §4b, docs/re/refcfw-analysis.md §6c):
 * the device's loader accepts small units (e.g. 2980 B code + 20220 B bss),
 * copies them on demand into a named RAM window ("HIGHRAM0",
 * 0x0307A000..0x0309F000), validates a header of the shape
 * {magic, id, key/crc, len, bss_start..bss_end} and verifies the payload with
 * the device's documented checksum (non-reflected MSB-first CRC-32,
 * poly 0x04C10DB7, init 0, xorout 0). RMF1 is OUR format that satisfies those
 * facts; no vendor/CFW structure is reproduced.
 *
 * Byte layout (all fields little-endian, exactly 32 bytes, no padding):
 *
 *   off  size  field
 *   ---- ----- ---------------------------------------------------------
 *    0     4   magic         'R' 'M' 'F' '1'
 *    4     2   version       format version (1)
 *    6     2   id            module id (registry key, e.g. 44, 78)
 *    8     4   flags         RMF1_FLAG_* bitfield (0 in v1)
 *   12     4   code_len      bytes of code in the file after the header
 *   16     4   bss_start     bss offset from the module's load base
 *   20     4   bss_len       bss byte count (0 = no bss)
 *   24     4   entry_offset  entry offset from the load base (even, Thumb)
 *   28     4   crc32         rmf_crc32() over the code_len code bytes
 *   32  code_len  code bytes (Thumb-2 image; see code_len rationale)
 *
 * The header itself is NOT copied into the window: the loader deploys only
 * the code bytes at the window base, then zeroes the bss range.
 */
#ifndef RECHORD_RMF1_H
#define RECHORD_RMF1_H

#include <stdint.h>

/* 'R','M','F','1' in file order == this value read as a little-endian u32. */
#define RMF1_MAGIC   0x31464D52u
#define RMF1_VERSION 1u

/* Fixed for v1: 9 fields = 8*4 + 2 + 2 bytes. */
#define RMF1_HEADER_SIZE 32u

/*
 * flags: bitfield reserved for format evolution (compression, relocation
 * tables, code signing — see docs/rewrite/modules.md "open items"). v1
 * defines NO flag bits: the builder writes 0 and the loader rejects any
 * module with unknown bits set (RMF_ERR_FLAGS) so future meanings can be
 * introduced without old loaders mis-executing new modules.
 */
#define RMF1_FLAG_KNOWN 0x00000000u

/*
 * RMF1 header, packed. Every field is naturally aligned within the struct,
 * so "packed" documents intent (exact 32-byte on-wire layout) without
 * changing the layout on any compiler we use.
 *
 * Field-by-field rationale:
 *
 * magic       — format tag. 4 ASCII bytes make files self-identifying in a
 *               hex dump and give a cheap first reject before any other
 *               check touches untrusted data. Chosen as 'RMF1' (ReChord
 *               Module Format 1) instead of any observed device magic: the
 *               format is ours; interoperability with the stock loader is
 *               explicitly NOT a goal.
 * version     — bumped whenever the layout or semantics change. Separate
 *               from the magic so v2..vN stay recognizably "RMF" while old
 *               loaders can refuse them cleanly (RMF_ERR_VERSION) instead of
 *               misparsing.
 * id          — small integer module identity (device log shows ids like
 *               m44/m45/m47/m48/m78). u16 is plenty (the log format %02u
 *               implies < 100 in practice) and lets the registry key on it.
 * flags       — forward-compat bitfield (rationale above at the #define).
 * code_len    — size of the initialized image that follows the header: the
 *               Thumb-2 code plus any const/initialized data the linker
 *               places in the same load span (the device's "2980 B code"
 *               figures are the same idea). It bounds both the file tail
 *               and the CRC input, so one field cannot disagree with the
 *               other two.
 * bss_start   — offset (bytes) of the zero-filled bss range from the
 *               module's LOAD BASE (the window base at runtime), NOT from
 *               the file start. The device log prints an absolute range
 *               (`bss %08x..%08x`) because its loader knows the window; our
 *               loader is window-agnostic, so the format stores the
 *               window-relative form and the loader computes the absolute
 *               range at deploy time. Must be >= code_len and 4-aligned.
 * bss_len     — byte count of the bss range (the device's "20220 B bss").
 *               Kept separate from bss_start (rather than an end address) so
 *               every size field in the header is a length and overflow
 *               checks are uniform.
 * entry_offset— byte offset of the module entry point from the load base.
 *               Must be even: Thumb code is halfword-aligned. The loader
 *               computes the callable address as base + entry_offset with
 *               bit 0 set (Thumb state), so the file stores a clean offset.
 * crc32       — rmf_crc32() (poly 0x04C10DB7, non-reflected MSB-first,
 *               init 0, xorout 0) over exactly the code_len code bytes.
 *               This is the device's documented image/module checksum
 *               primitive (same family as the IMG trailer), so one verifier
 *               covers modules and, later, flash images. Only the code is
 *               covered: the header is fully validated field-by-field, and
 *               a corrupted header field is caught by the structural checks
 *               or by the code CRC disagreeing with the payload.
 */
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t id;
    uint32_t flags;
    uint32_t code_len;
    uint32_t bss_start;
    uint32_t bss_len;
    uint32_t entry_offset;
    uint32_t crc32;
} __attribute__((packed)) rmf1_header_t;

/* Compile-time proof of the documented on-wire size. */
typedef char rmf1_header_size_check[(sizeof(rmf1_header_t) == RMF1_HEADER_SIZE) ? 1 : -1];

/*
 * Result codes shared by loader and registry. RMF_OK is 0; every failure is
 * a distinct positive value so callers can log the exact cause (the device
 * logs its loader failures as "MODULE m%02u did not load" — we want the
 * reason machine-readable). The five loader classes named in the RMF1 spec
 * (MAGIC/VERSION/LENGTH/CRC/WINDOW) are all present; ARG/FLAGS are extra
 * granularity, DUPLICATE/FULL/NOT_FOUND are registry-side.
 */
typedef enum {
    RMF_OK = 0,
    RMF_ERR_ARG,        /* NULL pointer argument */
    RMF_ERR_MAGIC,      /* magic != 'RMF1' */
    RMF_ERR_VERSION,    /* version != 1 */
    RMF_ERR_FLAGS,      /* unknown flag bits set */
    RMF_ERR_LENGTH,     /* truncated file or inconsistent field lengths */
    RMF_ERR_WINDOW,     /* module does not fit the target RAM window */
    RMF_ERR_CRC,        /* code CRC-32 mismatch */
    RMF_ERR_DUPLICATE,  /* registry: id already registered */
    RMF_ERR_FULL,       /* registry: no free slot */
    RMF_ERR_NOT_FOUND   /* registry: id not registered */
} rmf_result_t;

#endif /* RECHORD_RMF1_H */
