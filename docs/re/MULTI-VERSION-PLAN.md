# FiiO Echo Mini Firmware — Multi-Version Decompilation Plan (HANDOFF DOCUMENT)

> **⚠️ READ THIS FIRST.** This document is the complete knowledge base for the
> multi-version decompilation project. Any AI continuing this work **MUST**
> read this file, `docs/decompilation-plan.md`, `docs/symbol-index.md`,
> `docs/memory-map.md`, `docs/sdk-matching-progress.md`, and
> `docs/changelog-string-diff.md` before doing anything. This file exists so
> the process is never lost.
>
> **Last updated:** Aug 2026 · **ALL 22 versions analyzed** (v3.7.0 → v1.2.5) +
> **changelog-anchored naming phase in progress** (see §5c).
>
> **CURRENT STATE (this session):**
> - v3.7.0 primary: **852 / 2,776 named (30.7%)** — +82 this session
>   (changelog-cluster lineage push; cumulative campaign +177: +37 lineage
>   + 34 structural v2 + 82 cluster push + 24 callgraph/SDK)
>   (+37 lineage + 34 structural v2 + 82 cluster push)
> - **150+ new .c files** exported to `firmware/` (FLAC decoder internals,
>   media library service, OGG/WMA helpers, audio state, LCD, config, GPIO,
>   GUI, recorder, SDIO, wifi, crypto, USB)
> - New tool: `tools/version_lineage.py` — traces every v3.7.0 function
>   backwards through all 21 adjacent version pairs → `first_seen` version
>   (functions introduced in vX = changelog features of vX)
> - `build/function_lineage.json` — the lineage database (2,776 funcs)
> - **NEXT UP:** continue naming the remaining introduction clusters
>   (v3.1.0 genre/sleep, v1.7.0 Custom EQ/OGG, v1.4.6 lyrics, v1.4.0, 1.6.2, 1.8.0…)
>   then export each to `firmware/`.

---

## 1. PROJECT GOAL

Reverse-engineer the **FiiO Echo Mini (Snowsky Echo Mini)** portable music player
firmware into accurate, modifiable C source code.

**The user's ultimate aim:** have the **most accurate possible reconstruction of
the stock FiiO firmware code**, verified by **differential analysis across ALL
23 released firmware versions** (v1.2.5 → v3.8.0), each diff anchored to the
official **changelog** (English + Chinese, in each version's `Read me.txt`).

**Why multi-version diffing works:** the changelog tells us *what* changed each
version; the binary diff tells us *where*; the Ghidra decompilation tells us
*how*. Every changelog line is a ground-truth label for a cluster of functions.

---

## 2. HARDWARE & FIRMWARE FACTS

| Item | Value |
|------|-------|
| SoC | Rockchip RKnanoC (datasheet: LQFP64, Cortex-M3, Thumb-2, LE) |
| SDK | RKnano SDK 1.0 / RKnanoD_MP3_V1.3 + RKnanoD_Wireless_Audio_SDK_V1.5 (leaked, in `firmware/rockchip*`) |
| ROM API | 0x02FE0000–0x02FFFFFF (stub block `ROM_API` in Ghidra) |
| Main firmware | Loaded from IMG section 3 at 0x03000000 |
| IMG total size | 33,554,436 bytes (all versions, identical size) |
| EOF trailer | last 4 bytes LE; v3.7=`0x1EA1C309`, v3.8=`0xE4CA8428` (device may not verify) |
| UI resources | ROCK26IMAGERES partition, **1617 BMP entries**, RGB565 big-endian |
| Formats supported | MP1/MP2/MP3/WMA/WAV/APE/FLAC/AAC/M4A/OGG/MP4/3GP/DFF/DSF |

### IMG layout (verified on v3.7/v3.8; assumed same for all — verify per version)

| Section | IMG range | Content |
|---------|-----------|---------|
| outer_header_reloc | 0x00000000–0x00057820 | header + reloc table |
| bootloader | 0x00057820–0x00081A14 | bootloader |
| **main_fw_code** | **0x00081A14–0x009BAA0E** | **section_3 = the code we decompile (load base 0x03000000)** |
| resources_part5 | 0x009BAA0E–0x01FC41F8 | ROCK26 bitmaps (NOISE — exclude from code diffs) |
| padding | 0x01FC41F8–0x02000000 | zeros |
| trailer | 0x02000000–0x02000004 | CRC |

**Address conversion:** `file_offset = load_addr - 0x03000000 + 0x00081A14`

### Segment table (section 1 descriptors)

- Located at IMG offset **0x200**, entries of **16 bytes** each.
- An entry is real only when its 1st u32 is a plausible RAM address
  (0x03000000–0x03110000): `{load_addr, size, val2, val3}`.
- Known labels (in order): audio_buffer, zero_init, dsp_input, dsp_output,
  codec_work, id3_parse, fat_cache, main_stack, flac_work, ogg_decode,
  mp3_frame, wma_work, ape_work, dsd_buffer, aac_decoder, wav_decode,
  dict_decoder, media_index, ui_framebuffer, bt_stream, usb_dma, record_buf,
  jpeg_decode.
- **Paso 4 usage:** diffing this table between versions shows which buffers
  moved/resized (i.e., which code modules were edited) WITHOUT opening Ghidra.

---

## 3. GHIDRA MCP — CONNECTION & API

### Connection
- **Ghidra MCP plugin running at `http://127.0.0.1:8089`** (JSON-RPC + plain HTTP).
- Ghidra project: `FIIO-3.7.0-Decomp`, program: `section_3_0x00081A14.bin`
  (this is **v3.7.0** section 3).
- To enable scripts: `GHIDRA_MCP_ALLOW_SCRIPTS=1` (see `tools/set_env_var.py`).
- Existing Python clients: `tools/ghidra_client.py`, `tools/ghidra_api.py`.

### ⚠️ CRITICAL API FORMAT GOTCHAS (learned the hard way)

1. **`/decompile_function?address=XXXX` returns RAW C TEXT, not JSON.** Parse
   it as text; first line is the function signature.
2. **`/search_strings?search_term=REGEX&limit=N` returns `{"matches":[{"address":"03013ff4","value":"...","encoding":"ascii"}]}`.** Key is `matches`, not `strings`.
3. **`/get_function_by_address?address=XXXX` returns**
   `Function: NAME at ADDR\nSignature: ...` — parse lines, take the `Function:` line.
4. **`/get_xrefs_to?address=XXXX` returns lines** like
   `From 03014b80 in FUN_03014af2 [PARAM]` — parse `in <FUNCNAME> @ <addr>`.
5. **`/find_undocumented_by_string?address=XXXX` takes an ADDRESS** (of a
   defined string), NOT a text search. Returns only FUN_* (undocumented) funcs.
6. **Addresses must be lowercase hex without `0x`, zero-padded to 8** e.g. `03014b80`.
   Do NOT convert int→str (that gives decimal!) — use `format(addr, '08x')`.
7. **String xrefs in the raw binary are NOT plain LE32 constants.** The
   compiler uses MOVW/MOVT pairs + PC-relative literal pools. Use Ghidra's
   `/get_xrefs_to` instead of scanning for the address bytes.
8. **UI/menu strings (addresses 0x037xxxxx+) often have NO code xrefs** —
   they're referenced via data tables. Only strings in the code region
   (0x0300xxxx–0x0311xxxx) reliably have xrefs.
9. **`/list_functions` returns text blob**; `/list_functions_enhanced` returns
   JSON (key `functions`).

### Useful API endpoints (206 tools total — get full list via `/mcp/schema`)

**Query:** `/get_function_count`, `/get_current_program_info`, `/list_functions`,
`/list_functions_enhanced`, `/search_functions?name_pattern=`,
`/search_functions_enhanced`, `/get_function_by_address`, `/get_function_callees`,
`/get_function_callers`, `/get_function_xrefs`, `/get_xrefs_to`, `/get_xrefs_from`,
`/get_full_call_graph`, `/get_function_call_graph`, `/get_function_hash`,
`/get_bulk_function_hashes`, `/search_strings`, `/list_strings`,
`/find_undocumented_by_string`, `/batch_string_anchor_report`,
`/inspect_memory_content`, `/read_memory`, `/list_segments`, `/get_metadata`

**Decompile:** `/decompile_function`, `/batch_decompile`, `/force_decompile`,
`/disassemble_function`, `/disassemble_bytes`, `/get_function_pcode`

**Rename/annotate:** `/rename_function_by_address`, `/rename_function`,
`/set_plate_comment`, `/set_decompiler_comment`, `/set_disassembly_comment`,
`/add_function_tag`, `/create_function`, `/delete_function`, `/rename_label`,
`/set_function_prototype`, `/set_parameter_type`, `/set_local_variable_type`

**Multi-program / cross-version (PASO 2!):** `/list_open_programs`,
`/open_program`, `/switch_program`, `/import_file` (POST body: file_path,
project_folder, language e.g. `ARM:LE:32:Cortex`, compiler_spec, auto_analyze),
`/set_image_base`, `/bulk_fuzzy_match` (source_program, target_program,
threshold, filter), `/find_similar_functions_fuzzy`, `/diff_functions`
(address_a, address_b, program_a, program_b), `/compare_programs_documentation`,
`/merge_program_documentation`, `/archive_ingest_function`, `/archive_ingest_program`

**Scripts:** `/run_ghidra_script`, `/run_script_inline` (Java source in body)

**Analysis:** `/reanalyze`, `/run_analysis`, `/analysis_status`, `/find_similar_functions`

---

## 4. CURRENT DECOMPILATION STATUS (v3.7.0 — LIVE from Ghidra, Aug 2026)

| Metric | Count |
|--------|------:|
| Total functions in binary | 2,776 (count endpoint: 2,777) |
| **Decompiled (raw pseudocode in `build/all_decompilations.json`)** | **2,764** |
| **Named in Ghidra** | **852 (30.7%)** |
| Named thunks | 16 |
| Named non-thunk | 672 |
| Unnamed (`FUN_*`) | 2,088 |
| SDK functions indexed | 5,333 |
| Decompiled-to-C exported | 3,320 files under `firmware/` (see `firmware/INDEX.md`) |

Top name prefixes: mbedtls (30), rom (27), FLAC (21), hifi (19), thunk (13),
wma (11), parse (9), FW (8), rkos (6), rk (6), FatDev (6), GUI (6),
bitreader (6).

> **Run `python tools/check_decompilation_status.py` for a LIVE status** — it
> queries the Ghidra MCP server (switches to v3.7.0, counts named functions via
> `list_functions_enhanced`), falling back to the frozen cache only if the
> server is down. (The earlier version of this script only read the frozen
> `all_decompilations.json` and under-reported named functions.)

- `build/all_decompilations.json` — 4.6 MB, keyed by function name,
  `{address, code}`.
- Export script: `tools/export_to_c.py` (writes `firmware/<subsystem>/<name>.c`).
- 12 functions still fail to decompile (likely thunks/externals).
- **Boundary repair pending in Ghidra** (do NOT trust oversized bodies):
  `hifi_busy_delay_ovl_09e3`, `_0fd1`, `_0e48` — body starts ~0xb00 before entry;
  needs `delete_function` + `create_function` at true entry.
- `AudioPlayback_Start` @ 0x0302a398 / `MusicService_Init` @ 0x0302a3e0 are
  contiguous — likely one routine; mid-function split revisit later.

### Naming approaches tried (see `docs/sdk-matching-progress.md`)
Functionality matching (constants) +85, structural +195, combined +67,
call-graph propagation +80, similarity +8, ROM API +7, auto-analysis +21,
string matching (exhausted, 0 new). SDK uses different function names than the
binary (`ipc_post_cmd` vs `MailBoxWriteB2ACmd`) so **behavioral fingerprint
matching** (constants ≥0x100, CFG shape, call patterns) is the way.

---

## 5. VERSION CORPUS (stock/)

All 23 versions in `stock/<version>/<folder>/` (each has `Read me.txt` = full
changelog back to v1.2.5, and `HIFIECXX.IMG` or `MINIVXXX.IMG`).

**Paso 0 result (`tools/build_version_corpus.py`): all 23 are UNIQUE code —
no dedup possible.** There are no regional variants with identical section 3.

```
1.2.5  1.2.7  1.3.0  1.4.0  1.4.6  1.5.0  1.6.2  1.7.0  1.8.0
2.4.0  2.5.0  2.6.0  2.7.0  2.8.0  3.0.0  3.1.0  3.2.0  3.3.0
3.4.0  3.5.0  3.6.0  3.7.0  3.8.0
```
- `build/version_corpus.json` has per-version section_3 SHA-256 + sizes.
- 3.8.0 has two variants (512M + 8G) in `stock/3.8.0/`.
- Note: **old versions may not share the exact 0x81A14 layout** — the corpus
  tool falls back to the 3rd `RKnanoFW` occurrence; verify per-version offsets
  when diffing very old pairs.

### Official changelogs (EN, condensed — Chinese has identical meaning)

| Version | Changelog |
|---------|-----------|
| **3.8.0** | OGG "unsupported format" on prev-track; APE ID3 not shown; WMA ID3 not shown; MP3 cover art |
| **3.7.0** | New Button C Mode (single/double press prev/next = volume, long press = track switch); FLAC ID3 tags not shown fixed; prev-track logic: <3s played → go to actual prev track, >3s → restart current track from 0; media library load speed; other |
| **3.6.0** | M4A album sorting fixed; freeze when entering Favorites (rare); Media Library not updating on some micro-SD cards; other |
| **3.5.0** | Favorites resume-playback position; album sorting logic (CD# → track# → filename); other |
| **3.4.0** | Button Modes A/B (A: short=vol, long=ff/rw, dbl=track; B: short=track, long=ff/rw, dbl=vol); battery accuracy; other |
| **3.3.0** | DSD ID3 display (NOT DFF); Favorites accidentally cleared by media-lib update; other |
| **3.2.0** | Click-to-play occasional track skip; volume levels 4-5 smoothness; other |
| **3.1.0** | Genre play-all; sleep timer no longer reset by keypress; other |
| **3.0.0** | Long-press menu = add/remove Favorites; FF/rewind while screen off; "Playlist" renamed "Media Library"; other |
| **2.8.0** | Lyrics switch; channel balance; drive-letter change cleared Favorites; rare media-lib crash; nav speed; other |
| **2.7.0** | Volume button swap; album cover missing on track switch; other |
| **2.6.0** | M4A album sort; RIFF ID3 support; other |
| **2.5.0** | Occasional noise; shuffle logic; memory capacity display; other |
| **2.4.0** | **USB DAC function** (48kHz, driver-free, formats internal memory); folder skip; DAC DRE params |
| **1.8.0** | 3 new UI themes; low-brightness flicker; ID3 "unknown" display; upgrade crash; character drops; translations |
| **1.7.0** | **Custom EQ**; no auto media-lib update on boot; OGG format support; special chars; cover lyrics |
| **1.6.2** | Track-artist priority; playback-start clipping; shuffle rules; album art |
| **1.5.0** | Retro EQ; shutdown logo font; Album Artist priority; low-brightness track-switch flicker; shuffle remembers prev track; ID3 display |
| **1.4.6** | Gear rotation direction; screensaver clock setting; backlight whine; lyrics library; playlist sorts by filename; album sorts by track# |
| **1.4.0** | Cover display + full-screen lyrics; FF/rewind; **120-level volume**; Favorites in file browser; shutdown "pop"; other |
| **1.3.0** | Song title from ID3 (not filename); 12h→24h clock; playback-start drop; charging media-lib re-update; Favorites; other |
| **1.2.7** | Song sort; clock reset after reboot; charging-text flicker; BT auto-reconnect |
| **1.2.5** | Gain settings (menu key in playback); power save default 5min; filter not saved after shutdown; in-folder shuffle; auto-ff+noise; BT popping; removed Warning symbol |

**Changelog→function clusters to look for (from docs/symbol-index.md + our diffs):**
- M4A sorting → `aac_movfile_parser` @ 0x030AEB90, `FormatList_Init` @ 0x03013C10
- OGG → `VorbisOGG_Parser` @ 0x03023BE8, `OGGInfo_Parse` @ 0x0301930E
- APE → `APE_Codec_Open` @ 0x030EEF60, `APEV1_Parse` @ 0x030CB34A
- WMA → `wma_audio_parse` @ 0x0308CDDC
- MP3/ID3 → `mp3_id3v2_handler` @ 0x0306FEC4, `ID3_Picture_Parser` @ 0x03023526
- DSD → `DSD_DecodeBlock` @ 0x030FFA3C, `DSD_IFF_Open` @ 0x030FF2E8
- FLAC → `hifi_flac_dec` @ 0x030DF64C
- BT → `BT_Music_Handler` @ 0x0300DDEA
- USB DAC → `USB_DAC_OpenStream` @ 0x0302B80A
- Volume → `MusicPlay_VolumeDisplay` @ 0x0300AD50, `dac_gain_curve_apply` @ 0x030098E4
- Media library → `MediaLib_GetTotalFiles` @ 0x03000F94, `MediaLib_thunk_GetFiles` @ 0x03012838
- UI/menu → `MainUI_KeyHandler` @ 0x0301020C, `BroMemSelKeyMenu_Handler` @ 0x03013280

---

## 5b. VERSION PROGRESS CHECKLIST (TRACKING — update as you go)

> **How to use:** tick boxes as you complete each step for a version. One row
> per version, processed **backwards** from v3.7.0. After each version:
> save programs + commit. When starting a session, re-open programs with
> `tools/open_all_programs.py` first.

### Legend
- ✅ = fully done · 🟨 = partially done · ⬜ = not started · ▶ = NEXT

| Version | Status | Ghidra program | Funcs | Named | Last step done |
|---------|--------|----------------|------:|------:|----------------|
| 3.8.0 | ⬜ | — | — | — | — (needs its own layout check — big diff vs 3.7) |
| **3.7.0** | ✅ | `section_3_0x00081A14.bin` | 2,776 | **852** | primary: decompiled + exported; +201 this campaign |
| **3.6.0** | 🟨 | `sec3_3_6_0.bin` (Cortex) | 2,217 | **29** | fuzzy match vs 3.7.0 (9 renames); orphan `sec3_3_6_0.bin.0` to ignore |
| **3.5.0** | ✅ | `sec3_3_5_0.bin` | 1,726 | **75** | 3.5→3.6→3.7 chain done (54 chained + 9 direct, 13 weak reverted) |
| **3.4.0** | ✅ | `sec3_3_4_0.bin` | 1,712 | **104** | 73 direct (≥0.9) + 39 chain3 (combo ≥0.7, offset-ok); saved |
| **3.3.0** | ✅ | `sec3_3_3_0.bin` | 1,727 | **106** | 71 direct (≥0.9) + 79 chainN 4-hop (combo ≥0.9, no offset — relink shift); saved |
| **3.2.0** | ✅ | `sec3_3_2_0.bin` | 1,641 | **91** | 77 direct (≥0.9) + 76 chainN 5-hop (combo ≥0.9, no offset — relink shift +512); saved |
| **3.1.0** | ✅ | `sec3_3_1_0.bin` | 1,706 | **94** | 78 direct (≥0.9) + 77 chainN 6-hop (combo ≥0.9, no offset); saved |
| **3.0.0** | ✅ | `sec3_3_0_0.bin` | 1,713 | **94** | 77 direct (≥0.9) + 77 chainN 7-hop (combo ≥0.9, no offset); saved |
| **2.8.0** | ✅ | `sec3_2_8_0.bin` | 1,653 | **95** | 75 direct (≥0.9) + 75 chainN 8-hop (combo ≥0.9, no offset); saved |
| **2.7.0** | ✅ | `sec3_2_7_0.bin` | 1,689 | **92** | 75 direct (≥0.9) + 75 chainN 9-hop (combo ≥0.9, no offset); saved |
| **2.6.0** | ✅ | `sec3_2_6_0.bin` | 1,633 | **94** | 75 direct (≥0.9) + 75 chainN 10-hop (combo ≥0.9, no offset); saved |
| **2.5.0** | ✅ | `sec3_2_5_0.bin` | 1,671 | **89** | 73 direct (≥0.9) + 73 chainN 11-hop (combo ≥0.9, no offset); saved |
| **2.4.0** | ✅ | `sec3_2_4_0.bin` | 1,639 | **89** | 73 direct (≥0.9) + 73 chainN 12-hop (combo ≥0.9, no offset); saved |
| **1.8.0** | ✅ | `sec3_1_8_0.bin` | 1,580 | **89** | 74 direct (≥0.9) + 74 chainN 13-hop (combo ≥0.9, no offset); saved |
| **1.7.0** | ✅ | `sec3_1_7_0.bin` | 1,573 | **90** | 78 direct (≥0.9) + 78 chainN 14-hop (combo ≥0.9, no offset); saved |
| **1.6.2** | ✅ | `sec3_1_6_2.bin` | 1,549 | **92** | 78 direct (≥0.9) + 78 chainN 15-hop (combo ≥0.9, no offset); saved |
| **1.5.0** | ✅ | `sec3_1_5_0.bin` | 1,569 | **94** | 78 direct (≥0.9) + 78 chainN 16-hop (combo ≥0.9, no offset); saved |
| **1.4.6** | ✅ | `sec3_1_4_6.bin` | 1,625 | **94** | 78 direct (≥0.9) + 78 chainN 17-hop (combo ≥0.9, no offset); saved |
| **1.4.0** | ✅ | `sec3_1_4_0.bin` | 1,612 | **92** | 76 direct (≥0.9) + 76 chainN 18-hop (combo ≥0.9, no offset); saved |
| **1.3.0** | ✅ | `sec3_1_3_0.bin` | 1,606 | **87** | 71 direct (≥0.9) + 71 chainN 19-hop (combo ≥0.9, no offset); saved |
| **1.2.7** | ✅ | `sec3_1_2_7.bin` | 1,622 | **91** | 71 direct (≥0.9) + 71 chainN 20-hop (combo ≥0.9, no offset); saved |
| **1.2.5** | ✅ | `sec3_1_2_5.bin` | 1,566 | **87** | 71 direct (≥0.9) + 71 chainN 21-hop (combo ≥0.9, no offset); saved |
| **✅ DONE** | — | — | — | — | **ALL 22 versions (3.7→1.2.5) analyzed + saved in Ghidra** |
| 1.2.5 | ⬜ | — | — | — | — |

### Per-version recipe checklist (copy for each version)

For **X.Y.Z** (target: the NEXT version back, e.g. 3.4.0 when 3.5.0 is done):

**Setup**
- [ ] `python tools/open_all_programs.py` (if Ghidra restarted)
- [ ] `python tools/extract_sec3_for_ghidra.py X.Y.Z`
- [ ] `python tools/import_into_ghidra.py build/sec3_X_Y_Z.bin --language ARM:LE:32:v8-m --base 0x03000000`
- [ ] `python tools/wait_analysis.py X_Y_Z` then `save_program`

**Diff + match (no Ghidra needed for the first two)**
- [ ] `python tools/string_diff_versions.py X.Y.Z <newer> --limit 30`
- [ ] `python tools/segment_table_diff.py X.Y.Z <newer>`
- [ ] `python tools/run_bulk_match.py sec3_X_Y_Z.bin sec3_<newer>.bin --filter FUN_ --threshold 0.5 --out fuzzy_match_vXY_vXZ.json`

**Apply names (Ghidra running)**
- [ ] `python tools/apply_cross_version_names.py sec3_X_Y_Z.bin --matches build/fuzzy_match_vXY_vXZ.json --threshold 0.9`
- [ ] `python tools/chain3_propagate_names.py --m45 build/fuzzy_match_vXY_vXZ.json --m56 build/fuzzy_match_v<prev>_vXY.json --m67 build/fuzzy_match_v<next>_v<newer>.json --program sec3_X_Y_Z.bin --threshold 0.7 --require-offset` (N-hop chain from target back to v3.7; add more match files for versions older than 3.4 — see section 9) (verify the printed per-hop deltas look sane)
- [ ] `python tools/revert_weak_chain_names.py --old-match build/fuzzy_match_vXY_vXZ.json --new-match build/fuzzy_match_v<newer>_<newest>.json --program sec3_X_Y_Z.bin --min-chain 0.6`
- [ ] **`save_program` immediately**

**Changelog anchoring (Paso 3 — the profit step)**
- [ ] Read the version's `Read me.txt` changelog entries
- [ ] For each changelog line, find the changed function cluster (string diff + segment diff + unmatched functions) and name it
- [ ] **Unmatched-function trick:** find v3.7.0 functions that are ABSENT in older versions → those are feature-introduction functions → name per changelog
- [ ] `python tools/check_decompilation_status.py` to confirm counts

**Wrap-up**
- [ ] Update this checklist table (status, named count)
- [ ] Update `docs/symbol-index.md` with new names
- [ ] Save all programs + `git add` + commit

---

## 5c. CHANGELOG-ANCHORED NAMING PHASE (CURRENT WORK — Aug 2026)

> **This is where the project is RIGHT NOW.** All 22 versions are matched and
> saved. The current goal is to USE those matches to NAME v3.7.0 functions
> (the primary program) by correlating function *introduction* with the
> official changelog, then EXPORT each named function to `firmware/`.

### The lineage tool (NEW — `tools/version_lineage.py`)

For every v3.7.0 function, walks the fuzzy-match chain backwards through ALL
21 adjacent version pairs (v3.6→v3.5→…→v1.2.5) and records:
- **`first_seen`** — the OLDEST version where the function exists. A function
  first seen in version X was *introduced* by X → corresponds to a changelog
  feature of X.
- **`versions`** — address per version (ancestor chain).
- **`changes`** — versions where the function was edited (hop score < 0.9).

```bash
python tools/version_lineage.py --out build/function_lineage.json
# --min-score 0.9 detects edit clusters (noisier — relink shifts lower scores)
```

Output: `build/function_lineage.json` → `{versions: [...], functions: {addr: {first_seen, versions, changes}}}`.

**Key result (Aug 2026):** of the 711 v3.7.0 functions traceable to v3.6:

| Version | Functions introduced (first_seen) | Changelog anchor |
|---------|----------------------------------:|------------------|
| 1.2.5 | 462 | baseline SDK core (stable 2 yrs) |
| 1.2.7 | 5 | song sort, clock reset, BT auto-reconnect |
| 1.3.0 | 3 | ID3 title, clock 12/24h, Favorites |
| 1.4.0 | 8 | cover display, FF/RW, 120-level volume |
| 1.4.6 | 26 | gear rotation, screensaver clock, lyrics library |
| 1.5.0 | 2 | Retro EQ, shutdown font, shuffle remembers |
| 1.6.2 | 5 | track-artist priority, shuffle rules |
| 1.7.0 | 18 | **Custom EQ**, OGG support, cover lyrics |
| 1.8.0 | 12 | 3 themes, ID3 unknown, translations |
| 2.4.0 | 5 | **USB DAC**, folder skip, DRE params |
| 2.5.0 | 6 | noise, shuffle logic, memory display |
| 2.6.0 | 4 | M4A sort, RIFF ID3 |
| 2.7.0 | 9 | volume button swap, cover on track switch |
| 2.8.0 | 5 | lyrics switch, channel balance |
| 3.0.0 | 18 | long-press Favorites, FF/RW screen-off, rename Media Library |
| 3.1.0 | 6 | genre play-all, sleep timer keypress |
| 3.2.0 | 5 | click-to-play skip, volume 4-5 smoothness |
| 3.3.0 | 7 | DSD ID3 display, Favorites clearing fix |
| 3.4.0 | 18 | **Button Modes A/B**, battery accuracy |
| 3.5.0 | 27 | album sorting CD#→track#→filename, Favorites resume |
| 3.6.0 | 60 | **FLAC decoder rewrite**, M4A sort, Favorites freeze |

**How to use it for naming (the recipe that worked this session):**
1. Pick a version with a clear changelog (start with the big ones: 3.6.0, 3.5.0, 3.4.0, 3.0.0, 1.7.0).
2. List that version's `first_seen` functions: the intro cluster.
3. Decompile each FUN_* in v3.7.0 (`/decompile_function?address=XXXX` —
   **raw C text**, not JSON; see gotcha 1) and look at:
   - its **address region** (0x030DDxxx = FLAC, 0x0308Cxxx = WMA, 0x0302Bxxx = audio, …)
   - **strings** it references (`s_*` symbols, e.g. `s_PICTURE_OGG_03023ad8` → OGG picture)
   - **callers** (`/get_xrefs_to` — e.g. called by `AudioStop`/`MusicService` → audio state)
4. Name it per the changelog feature + evidence; `save_program`; export.

### Structural matcher v2 (NEW — `tools/match_structure_v2.py`) — the SDK path

**Why v1 failed:** `match_structure.py` counts if/while/for in the C text, which
differs wildly between Ghidra output style and SDK source style.

**v2 signals (weighted):** signature (params+return type) · callee overlap vs
SDK callee names (`sdk_callees.json`) · distinctive constants (≥0x100) ·
code-size ratio · module prefix bonus (binary's calls to already-named
functions sharing the SDK candidate's module prefix).

**The validation that made it trustworthy (callee-overlap):** the SDK file
`build/sdk_callees.json` says *which functions each SDK function calls*. If the
binary function calls the same (already-named) functions that the SDK says its
candidate calls, the match is near-certain. This beat the raw score alone.

**Results (Aug 2026): +34 names validated, e.g.:**
`mbedtls_mpi_div_mpi` (calls mbedtls_mpi_grow) · `wwd_bus_init` (calls
wwd_bus_sdio_set_oob_interrupt) · `MscTestUnitReady` (calls MscSendCSW) ·
`SNOR_Write` (calls SNOR_Erase) · `RecordWriteFile` (calls
RecordCopyEncDataToBuf) · `GetFreeMemory` (calls rkos_memory_malloc) ·
`DeviceListRemoveFs` (calls RKDev_Open) · `RecodTask_GuiInit` (calls
GUITask_CreateWidget) · `MusicPlay_DeleteGuiHandle` (calls
GuiTask_DeleteWidget) · `LunShellTest` (calls LunDev_GetSize) ·
`phybusif_input` (calls pbuf_free) · `Mp3GenreParse` (calls
mp3_bitstream_getbits) · `GUI_TextDisplayID/Buff`, `rkos_semaphore_create`,
`ape_test`, `DisplayDev_ScreenUpdate`, `host_buffer_get`, `wwd_wifi_*`,
`freertos_init_timer`, `BroadMessage_Parse`, `SystemSetTask_Enter`,
`GUI_SpectrumDisplay`, `USBResetPhy`, `MusicPlay_AudioCallBack`,
`cistpl_vers_1`, `sfc_request`, `mbedtls_rsa_check_privkey`,
`rk_wifi_smartconfig`, `main2_entry`.

**Pitfalls learned:**
- The v1 `functionality_matches.json` (516 entries) is FULL of false
  positives — everything scores high as `WavEncodeHeaderInit` (shared generic
  constants). Do NOT trust it.
- `/rename_function_by_address` enforces a name-quality gate (PascalCase,
  ≥8 chars) AND rejects token-subset collisions (`AudioPause` vs
  `AudioFFPause`). `/rename_function` (by old name) bypasses the gate.
- After 2 sweeps the marginal yield dropped to 0 — the callee-overlap
  validation is the binding constraint: once a module's anchors are all
  applied, no new validated matches appear until MORE of its callees get
  named. Next lever: name more FUN_* callees via the lineage clusters (the
  callee-overlap validation then unlocks the remaining SDK candidates).

### Names applied this session (37 total → v3.7.0 now 688 named)

**v3.6.0 cluster — FLAC decoder rewrite (14):**
`flac_bs_bswap32` 030dd6d0 · `flac_clz32` 030dd956 · `flac_bs_getbits_u` 030dd842 ·
`flac_bs_init` 030dd888 · `flac_bs_getbits_s` 030df3ae · `flac_bs_getbits_u_wide` 030df46c ·
`flac_frame_header_parse` 030df30e · `flac_decode_subframe` 030ded62 · `flac_decode_frame` 030df9fa ·
`flac_decode_residual` 030def58 · `flac_lpc_compute` 030ddd0a · `flac_hifi_cfg_write` 030de824 ·
`flac_hifi_ctrl_clear` 030e0920 · `flac_hifi_ctrl_set` 030e0936 → `firmware/codecs/flac/`

**v3.5.0 cluster — media library service (10):**
`media_lib_service_dispatch` 03029730 (cmd dispatcher, reads cmd at struct+0x1c) ·
`media_lib_op_cmd01` 030295d0 · `_cmd04` 030295fc · `_cmd08` 03029628 · `_cmd10` 03029654 ·
`_cmd18` 03029680 · `_cmd20` 030296ac · `_cmd40` 030296d8 · `_cmd80` 03029704 ·
`media_lib_op_dispatch_core` 03029246 → `firmware/firmware/media/`

**v3.5.0 cluster — codecs (3):** `OGG_Picture_Parser` 030241f8 (refs `s_PICTURE_OGG_03023ad8`) ·
`wma_input_read` 0308c9a4 · `wma_debug_helper` 0309fe6a → `firmware/codecs/{ogg,wma}/`

**v3.4.0 cluster — config/GPIO/OGG helpers (3):** `config_parse_helper` 0301ded2 (called by
`HostParseConfig`) · `gpio_int_mode_helper` 030202ce (called by `Gpio_SetPortIntMode`) ·
`ogg_picture_parser_helper` 03026ccc (called by `OGG_Picture_Parser`) → `firmware/{firmware/os,drivers,codecs/ogg}/`

**v3.0.0 cluster — audio/LCD/fs (7):** `AudioStateHandler` 0302a74e (state switch; called by
`AudioStop`/`MusicService`) · `audio_file_input_helper` 0300d090 + `audio_file_input_read` 0300cfd4
(called by `AudioFileInput2`) · `lcd_shell_helper` 0302aaf0 · `lcd_write_helper_a` 03028c9e +
`lcd_write_helper_b` 03028628 (called by `LcdShell`/`Lcd_Write`) · `hifi_file_close_helper` 0306c6c4
(called by `HifiFileClose`) → `firmware/apps/{audio,ui}/`, `firmware/firmware/filesystem/`

### Baseline 1.2.5 match (NEW — `tools/match_baseline_v2.py`) — conservative SDK path

The 1.2.5 baseline (462 funcs, the stable SDK core) is the OLDEST code.
Cautions applied:
- Only SDK-callee-overlap validation is accepted (SDK says candidate calls
  X; binary calls the already-named X). Never the raw score alone.
- **Weak/same-name overlaps REJECTED:** when the overlap = the candidate
  name itself (e.g. overlap `['FW_GetSegmentInfo']` for candidate
  `FW_GetSegmentInfo`), the binary is a CALLER of the already-named function
  — naming it the same would be wrong. All 36 such cases were rejected.
- 16 STRONG matches applied (overlap with a DIFFERENT callee, verified by
  decompile): `FATGetPrevClus` (calls FATGetNextClus), `FREQStopAPP(2)`
  (calls FREQSetFreq), `RecordWriteFile2` (calls RecordCopyEncDataToBuf),
  `FW_GetSegmentInfo`, `sdio_read_fbr`, `SdioDev_SetBlockSize`,
  `MscPreventAllowMediumRemoval`, `FatDev_FileSeek`, `sdio_read_cis`,
  `FileDev_FileSeek(2)`, `FM5807_Tuner_Disable(2/3)` (duplicate copies),
  `GUI_TextBoxTimer`.
- 267 baseline FUN_* remain — mostly callers/orphans. Best attacked as
  more anchors accumulate (each new named callee unlocks new overlaps).

### Remaining introduction clusters (the next names — by version)

**UPDATE (Aug 2026, cluster push done):** only **28 FUN_* remain** across
all introduction clusters. They are the hard cases: broken boundaries
(`030208B8` = "bad instruction data" — needs `delete_function` +
`create_function` at true entry), or orphans with no callers/strings
(`03001E04`, `0304E6A4`, `0304F550`, `030E7500`, `030F0782`, `030F7A94`,
`03100C78`, …). The 1.2.5 baseline (462 functions, the stable SDK core)
is the remaining large pool — best attacked via the SDK structural matcher
(callee-overlap validation) as more anchors accumulate.

| Version | Cluster size (FUN_*) | Hints for the next AI |
|---------|---------------------:|-----------------------|
| **3.1.0** | 3 | genre play-all / sleep timer — look near media lib + power |
| **3.2.0** | 3 | click-to-play skip / volume — near playback + dac_gain |
| **3.3.0** | 2 | DSD ID3 (not DFF) — near DSD_DecodeBlock @ 0x030FFA3C |
| **3.6.0** | 34 remaining | FLAC region 0x030DDxxx-0x030E0xxx + scattered; check callees of flac_decode_frame |
| **3.5.0** | 8 remaining | album sort helpers near 0x030296xx-0x030297xx |
| **1.7.0** | 9 | **Custom EQ** — near DSP_GOODEF @ 0x0300F7DC; OGG support — near VorbisOGG_Parser |
| **1.4.6** | 11 | lyrics library, screensaver clock |
| **1.8.0** | 8 | themes (FLAC_ThemeColor_Select @ 0x030054BA), ID3 unknown |
| **2.4.0** | 3 | **USB DAC** — near USB_DAC_OpenStream @ 0x0302B80A |
| **2.7.0** | 6 | volume button swap — near MusicPlay_VolumeDisplay @ 0x0300AD50 |
| **2.8.0** | 4 | lyrics switch, channel balance |
| **3.0.0** | 4 remaining | long-press Favorites (menu), FF/RW screen-off |
| **1.2.5** | 292 | baseline SDK core — lower priority (exists in every version) |

**Export recipe (used this session):** decompile live via
`/decompile_function?address=XXXX`, write `firmware/<subsystem>/<name>.c` with
a `/** ... */` header noting the version it was introduced in + the changelog
anchor. `tools/export_named_flac.py` is a ready example.

---

## 6. THE MULTI-VERSION STRATEGY (Paso 0–4, from external AI + our implementation)

### Paso 0 — Normalize corpus ✅ DONE
Extract section_3 from every IMG, SHA-256, dedup. All 23 unique.
`python tools/build_version_corpus.py`

### Paso 1 — String diff ✅ DONE (the "quick win")
For each adjacent version pair: strings (ASCII + UTF-16LE, min len 5) per
section_3; `added = vN - vN-1`, `removed = vN-1 - vN`.
Added strings = new menu text, error msgs, EQ preset names, format names —
**direct hints of changelog features**. Then xref the string in Ghidra → function.
`python tools/string_diff_versions.py --all` → `build/string_diffs_all_versions.json`
`python tools/changelog_report.py` → `docs/changelog-string-diff.md`

### Paso 2 — Function diff ✅ DONE for 3.5.0→3.6.0 and 3.6.0→3.7.0
Never diff by address (functions move between versions). Match by **function
shape** (CFG + bytes + references):
- **Option A (MCP, automated):** import v3.6 section_3 as a second Ghidra
  program via `/import_file`, then `/bulk_fuzzy_match` + `/diff_functions`
  between the two programs. Buckets: matched / matched-but-changed /
  unmatched.
- **Option B (GUI):** Ghidra **Version Tracking** tool (File → New → Version
  Tracking), matchers: Exact Function Bytes, Exact Function Instructions,
  Combined Function and Data Reference.
- **Option C:** BinDiff (free) + BinExport plugin — industry standard for 23
  versions at scale.
- Both programs must be **loaded at the same base** (0x03000000, or set via
  `/set_image_base`).

### Paso 3 — The changelog trick (master move; STARTED — 3.5.0→3.6.0 done)
With per-pair diffs, map every changelog entry → cluster of changed functions:
- "Retro EQ" (v1.5) → DSP/EQ region cluster
- "BT popping" (v1.2.5) → bluetooth region cluster
- "New format APE" → functions added near the codec table
Name that cluster in Ghidra per the changelog, then **propagate names via the
existing call-graph tooling** (`tools/propagate_callgraph.py`). Going backwards
from v3.7.0, the audio pipeline self-assembles: every EQ change in history
touches the EQ cluster, so we KNOW where EQ is.

### Paso 4 — Relocation/segment table diff ✅ DONE (cheap, no Ghidra)
Diff the section-1 segment table between versions — moved/resized buffers =
edited modules.
`python tools/segment_table_diff.py 3.6.0 3.7.0`

### The 3 traps (confirmed by our data)
1. **Address shift** — #1 killer. 3.6.0→3.7.0: 89,321 raw byte diff regions,
   most are 4-byte relink fixups. Never compare raw addresses; always shape-match.
2. **Resource noise** — ROCK26 bitmaps change every version (66–71% of total
   diff bytes). Exclude `resources_part5` from code diffs always.
3. **Build noise** — header has build date; version strings like `:3.7.0`,
   `Software:3.6.0`, `Hardware:2.2.0` appear as "added" strings. Don't
   interpret those as features.

---

## 7. FINDINGS SO FAR (verify/extend these)

### String diff highlights (full data in `docs/changelog-string-diff.md`)

| Pair | Notable added strings | Changelog match |
|------|----------------------|-----------------|
| 1.5.0→1.6.2 | "USB DAC", "Shuffle Play", "High Gain", "Favorites", "Bluetooth Toggle" @ 0x037xxxxx | (2.4 added USB DAC per changelog — earlier strings may pre-exist) |
| 2.7.0→2.8.0 | "Channel Balance" @ 0x0372A4A8, "Lyrics switch" @ 0x0372A1A2 | v2.8: balance + lyrics ✓ |
| 3.3.0→3.4.0 | "Double click volume" @ 0x03728324, "Short press volume" @ 0x03728222 | v3.4 button modes ✓ |
| 3.4.0→3.5.0 | FLAC/ALBUM sort strings (see `build/string_diffs_all_versions.json`) | v3.5 album sort logic ✓ |
| 3.5.0→3.6.0 | **FLAC decoder rewrite**: `hifi_flacdec.c`, `invalid subframe padding`, `invalid sync code`, `### bps ###` @ 0x030DE7xx–0x030E02xx | new FLAC decoder internals (first naming win!) |
| 3.4.0→3.5.0 | "Changement des boutons de volume", "Double appui volume", "Gain faible" | v3.5 (button/volume tweaks) |
| 3.6.0→3.7.0 | "Power only, no charging" @ 0x03722524, "UAC charging mode" @ 0x0373241E, "Turn on/off charging power supply" | new USB power control |
| 3.7.0→3.8.0 | "WM/ArtistSortOrder", "WM/AlbumSortOrder", "WM/TrackNumber" @ 0x030401xx + many new "FLAC seek: ..." debug strings @ 0x030DFxxx–0x030E2xxx | v3.8 WMA/FLAC work ✓ |

### Ghidra xref finds (strings → functions, from `/get_xrefs_to`)

| String | Address (v3.7) | Referencing function |
|--------|---------------|---------------------|
| "MP1MP2MP3WMAWAVAPE...DSF" (format list) | 0x03013FF4 | `FormatList_Init` @ 0x03013C18 |
| "MP3/OGG/M4A/WMA" | 0x03014F4C | `FUN_03014af2` (codec table builder) |
| "Play All Album" | 0x03001AD8 | `FUN_030017b4` (album UI) |
| "PICTURE OGG" | 0x03023AD8 | `FUN_030241f8`, `ID3_Picture_Parser` |
| "Cover Art (front)" | 0x03023F84 | `VorbisOGG_Parser` |
| "BT Music" | 0x0300E2D0 | `BT_Music_Handler` |

### Segment table diff 3.6.0→3.7.0 (`tools/segment_table_diff.py`)
- 30/32 segments changed; most shifted **+16 bytes** (relink).
- `audio_buffer` size +16 (101,456→101,472)
- `id3_parse` **size 54→0** ← likely v3.7 "FLAC ID3 fix"
- `aac_decoder` size −16 (27,871→27,855)
- `ui_framebuffer` +16

### v3.7→v3.8 diff (from `stock/3.8.0/firmware-diff-v37-v38.md`, already done)
- Main code 43.5% changed; 513,747 small diff regions (relink fixups).
- Changelog symbols all show ~4KB changed windows → **addresses moved**, so
  never assume v3.7 Ghidra addresses apply to v3.8.

### ✅ 1.2.5→1.2.7 pair (done Aug 2026 session) — FINAL VERSION

**Relink shift (31/32 segments changed).**

- Fuzzy match 1.2.5 vs 1.2.7: 1,136 matches; 422 ≥0.9; 76 named targets.
- Direct renames (threshold 0.9): **71 applied**.
- Chain 21 (chainN): **71 applied** (combo ≥0.9) — `wma_floor_log2`,
  `event_set`, `ipc_post_cmd/arg`, etc.
- **v1.2.5 now has 87 named / 1,566 functions (5.6%)** — was 0 before.

**🏁 MILESTONE: ALL 22 VERSIONS ANALYZED.** Every version v3.7.0 → v1.2.5
now has a Ghidra program with cross-version names (87–651 named each). The
21-hop chainN pipeline is proven at scale. Remaining work: 3.8.0 forward
diff, deeper changelog anchoring per version, boundary repair.

**Ghidra program state (saved Aug 2026):** 23 programs (v3.7.0 primary + 22
older versions). Reopen with `tools/open_all_programs.py` after restart.

Rename history: `build/cross_version_renames_log.json` (per-program keys
`<name>.chainN` / `.chained`).

### ✅ 1.2.7→1.3.0 pair (done Aug 2026 session)

**NEARLY IDENTICAL LAYOUT (1/32 segments changed).**

- Fuzzy match 1.2.7 vs 1.3.0: 1,163 matches; 434 ≥0.9; 73 named targets.
- Direct renames (threshold 0.9): **71 applied**.
- Chain 20 (chainN): **71 applied** (combo ≥0.9) — `wma_floor_log2`,
  `event_set`, `ipc_post_cmd/arg`, etc.
- **v1.2.7 now has 91 named / 1,622 functions (5.6%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 22 programs — add `sec3_1_2_7.bin`
(v1.2.7, 1,622 funcs, **91 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_1_2_7.bin.chainN`).

### ✅ 1.3.0→1.4.0 pair (done Aug 2026 session)

**Relink shift (31/32 segments changed).**

- Fuzzy match 1.3.0 vs 1.4.0: 1,064 matches; 374 ≥0.9; 80 named targets.
- Direct renames (threshold 0.9): **71 applied**.
- Chain 19 (chainN): **71 applied** (combo ≥0.9) — `wma_floor_log2`,
  `event_set`, `ipc_post_cmd/arg`, etc.
- **v1.3.0 now has 87 named / 1,606 functions (5.4%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 21 programs — add `sec3_1_3_0.bin`
(v1.3.0, 1,606 funcs, **87 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_1_3_0.bin.chainN`).

### ✅ 1.4.0→1.4.6 pair (done Aug 2026 session)

**Relink shift (31/32 segments changed).**

- Fuzzy match 1.4.0 vs 1.4.6: 1,092 matches; 396 ≥0.9; 82 named targets.
- Direct renames (threshold 0.9): **76 applied**.
- Chain 18 (chainN): **76 applied** (combo ≥0.9) — `hifi_busy_delay`,
  `event_set`, `ipc_post_cmd/arg`, etc.
- **v1.4.0 now has 92 named / 1,612 functions (5.7%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 20 programs — add `sec3_1_4_0.bin`
(v1.4.0, 1,612 funcs, **92 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_1_4_0.bin.chainN`).

### ✅ 1.4.6→1.5.0 pair (done Aug 2026 session)

**Relink shift (31/32 segments changed).**

- Fuzzy match 1.4.6 vs 1.5.0: 1,109 matches; 403 ≥0.9; 89 named targets.
- Direct renames (threshold 0.9): **78 applied**.
- Chain 17 (chainN): **78 applied** (combo ≥0.9) — `wma_floor_log2`,
  `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, etc.
- **v1.4.6 now has 94 named / 1,625 functions (5.8%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 19 programs — add `sec3_1_4_6.bin`
(v1.4.6, 1,625 funcs, **94 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_1_4_6.bin.chainN`).

### ✅ 1.5.0→1.6.2 pair (done Aug 2026 session)

**IDENTICAL LAYOUT (0/32 segments changed).**

- Fuzzy match 1.5.0 vs 1.6.2: 1,086 matches; 400 ≥0.9; 81 named targets.
- Direct renames (threshold 0.9): **78 applied**.
- Chain 16 (chainN): **78 applied** (combo ≥0.9) — `wma_floor_log2`,
  `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, etc.
- **v1.5.0 now has 94 named / 1,569 functions (6.0%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 18 programs — add `sec3_1_5_0.bin`
(v1.5.0, 1,569 funcs, **94 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_1_5_0.bin.chainN`).

### ✅ 1.6.2→1.7.0 pair (done Aug 2026 session)

**Relink shift (27/32 segments changed).**

- Fuzzy match 1.6.2 vs 1.7.0: 1,073 matches; 403 ≥0.9; 81 named targets.
- Direct renames (threshold 0.9): **78 applied**.
- Chain 15 (chainN): **78 applied** (combo ≥0.9) — `wma_floor_log2`,
  `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, etc.
- **v1.6.2 now has 92 named / 1,549 functions (5.9%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 17 programs — add `sec3_1_6_2.bin`
(v1.6.2, 1,549 funcs, **92 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_1_6_2.bin.chainN`).

### ✅ 1.7.0→1.8.0 pair (done Aug 2026 session)

**Relink shift (31/32 segments changed).**

- Fuzzy match 1.7.0 vs 1.8.0: 1,064 matches; 381 ≥0.9; 83 named targets.
- Direct renames (threshold 0.9): **78 applied**.
- Chain 14 (chainN): **78 applied** (combo ≥0.9) — `RKDeviceUnRegister`,
  `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, `FmFreqToChan`,
  `SetSPIFreq`, etc.
- **v1.7.0 now has 90 named / 1,573 functions (5.7%)** — was 0 before.
  (EQ cluster did NOT chain — v1.7's Custom EQ was heavily rewritten later.)

**Ghidra program state (saved Aug 2026):** 16 programs — add `sec3_1_7_0.bin`
(v1.7.0, 1,573 funcs, **90 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_1_7_0.bin.chainN`).

### ✅ 1.8.0→2.4.0 pair (done Aug 2026 session)

**Relink shift (30/32 segments changed).**

- Fuzzy match 1.8.0 vs 2.4.0: 1,086 matches; 382 ≥0.9; 76 named targets.
- Direct renames (threshold 0.9): **74 applied**.
- Chain 13 (chainN): **74 applied** (combo ≥0.9) — `wma_floor_log2`,
  `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, etc.
- **v1.8.0 now has 89 named / 1,580 functions (5.6%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 15 programs — add `sec3_1_8_0.bin`
(v1.8.0, 1,580 funcs, **89 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_1_8_0.bin.chainN`).

### ✅ 2.4.0→2.5.0 pair (done Aug 2026 session)

**Relink shift (31/32 segments changed).**

- Fuzzy match 2.4.0 vs 2.5.0: 1,135 matches; 392 ≥0.9; 77 named targets.
- Direct renames (threshold 0.9): **73 applied**.
- Chain 12 (chainN): **73 applied** (combo ≥0.9) — `wma_floor_log2`,
  `USBMSCHost_Read` (v2.4 changelog: USB DAC ✓), `event_set`,
  `ipc_post_cmd/arg`, `hifi_busy_delay`, etc.
- **v2.4.0 now has 89 named / 1,639 functions (5.4%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 14 programs — add `sec3_2_4_0.bin`
(v2.4.0, 1,639 funcs, **89 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_2_4_0.bin.chainN`).

### ✅ 2.5.0→2.6.0 pair (done Aug 2026 session)

**IDENTICAL LAYOUT (0/32 segments changed).**

- Fuzzy match 2.5.0 vs 2.6.0: 1,168 matches; 424 ≥0.9; 78 named targets.
- Direct renames (threshold 0.9): **73 applied**.
- Chain 11 (chainN): **73 applied** (combo ≥0.9) — `RKDeviceUnRegister`,
  `udp_server`, `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, etc.
- **v2.5.0 now has 89 named / 1,671 functions (5.3%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 13 programs — add `sec3_2_5_0.bin`
(v2.5.0, 1,671 funcs, **89 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_2_5_0.bin.chainN`).

### ✅ 2.6.0→2.7.0 pair (done Aug 2026 session)

**Relink shift (30/32 segments changed).**

- Fuzzy match 2.6.0 vs 2.7.0: 1,119 matches; 379 ≥0.9; 80 named targets.
- Direct renames (threshold 0.9): **75 applied**.
- Chain 10 (chainN): **75 applied** (combo ≥0.9) — `udp_server`,
  `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, etc.
- **v2.6.0 now has 94 named / 1,633 functions (5.8%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 12 programs — add `sec3_2_6_0.bin`
(v2.6.0, 1,633 funcs, **94 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_2_6_0.bin.chainN`).

### ✅ 2.7.0→2.8.0 pair (done Aug 2026 session)

**Relink shift (31/32 segments changed).**

- Fuzzy match 2.7.0 vs 2.8.0: 1,149 matches; 392 ≥0.9; 79 named targets.
- Direct renames (threshold 0.9): **75 applied**.
- Chain 9 (chainN): **75 applied** (combo ≥0.9) — `udp_server`,
  `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, etc.
- **v2.7.0 now has 92 named / 1,689 functions (5.4%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 11 programs — add `sec3_2_7_0.bin`
(v2.7.0, 1,689 funcs, **92 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_2_7_0.bin.chainN`).

### ✅ 2.8.0→3.0.0 pair (done Aug 2026 session)

**Relink shift (31/32 segments changed).**

- Fuzzy match 2.8.0 vs 3.0.0: 1,128 matches; 376 ≥0.9; 80 named targets.
- Direct renames (threshold 0.9): **75 applied**.
- Chain 8 (chainN): **75 applied** (combo ≥0.9) — `udp_server`,
  `FmFreqToChan`, `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, etc.
- **v2.8.0 now has 95 named / 1,653 functions (5.7%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 10 programs — add `sec3_2_8_0.bin`
(v2.8.0, 1,653 funcs, **95 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_2_8_0.bin.chainN`).

### ✅ 3.0.0→3.1.0 pair (done Aug 2026 session)

**IDENTICAL LAYOUT (0/32 segments changed).**

- Fuzzy match 3.0.0 vs 3.1.0: 1,203 matches; 418 ≥0.9; 79 named targets.
- Direct renames (threshold 0.9): **77 applied**.
- Chain 7 (chainN): **77 applied** (combo ≥0.9) — `udp_server`,
  `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, `dac_gain_curve_apply`,
  etc.
- **v3.0.0 now has 94 named / 1,713 functions (5.5%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 9 programs — add `sec3_3_0_0.bin`
(v3.0.0, 1,713 funcs, **94 named**) to the table above.

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_3_0_0.bin.chainN`).

### ✅ 3.1.0→3.2.0 pair (done Aug 2026 session)

**IDENTICAL LAYOUT (0/32 segments changed)** — first pair since 3.4→3.5
with no relink shift. But the 6-hop chain to v3.7 still crosses the +512
(3.2→3.3) and +24 (3.3→3.4) relink pairs, so offset-ok still fails globally;
kept `--threshold 0.9` without `--require-offset`.

- Fuzzy match 3.1.0 vs 3.2.0: 1,201 matches; 432 ≥0.9; 84 named targets.
- Direct renames (threshold 0.9): **78 applied**.
- Chain 6 (chainN): **77 applied** (combo ≥0.9) — `udp_server`,
  `event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, `dac_gain_curve_apply`,
  `wma_memcmp`, `Unicode2Ascii`, etc.
- **v3.1.0 now has 94 named / 1,706 functions (5.5%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 8 programs in the project —

| Program | Version | Funcs | Named | Notes |
|---------|---------|------:|------:|-------|
| `section_3_0x00081A14.bin` | v3.7.0 | 2,776 | **651** | primary program |
| `sec3_3_6_0.bin` | v3.6.0 (Cortex) | 1,592–2,217 | ~25 | 9 renames |
| `sec3_3_6_0.bin.0` | v3.6.0 (v8-m) | 0 | 0 | ORPHAN — ignore |
| `sec3_3_5_0.bin` | v3.5.0 | 1,726–1,728 | **75** | 9 direct + 54 chained |
| `sec3_3_4_0.bin` | v3.4.0 | 1,712 | **104** | 73 direct + 39 chain3 |
| `sec3_3_3_0.bin` | v3.3.0 | 1,727 | **106** | 71 direct + 79 chainN (4-hop) |
| `sec3_3_2_0.bin` | v3.2.0 | 1,641 | **91** | 77 direct + 76 chainN (5-hop) |
| `sec3_3_1_0.bin` | v3.1.0 (v8-m, base 0x03000000) | 1,706 | **94** | 78 direct + 77 chainN (6-hop) |

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_3_1_0.bin.chainN`). Reopen with `tools/open_all_programs.py`.

### ✅ 3.2.0→3.3.0 pair (done Aug 2026 session)

**ANOTHER RELINK-SHIFT PAIR: 31/32 segments MOVE +512** (bigger than the
+24 shift of 3.3→3.4). The 5-hop chain (3.2→3.3→3.4→3.5→3.6→3.7) was run at
`--threshold 0.9` WITHOUT `--require-offset` — a combo of 6 scores all ≥0.9
is trustworthy despite the physical moves.

- Fuzzy match 3.2.0 vs 3.3.0: 1,154 matches; 408 ≥0.9; 82 named targets;
  dominant delta 0xd4 with wide spread (relink).
- Direct renames (threshold 0.9): **77 applied** (same SDK families).
- Chain 5 (chainN): 306 chained, **76 applied** (combo ≥0.9) — including
  `dac_gain_curve_apply` @ 0x03009418 (v3.2 changelog: volume 4-5 smoothness
  ✓), `event_set`, `ipc_post_cmd/arg`, `udp_server`, `hifi_busy_delay`,
  `wma_memcmp`, `Unicode2Ascii`, etc.
- **v3.2.0 now has 91 named / 1,641 functions (5.6%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 7 programs in the project —

| Program | Version | Funcs | Named | Notes |
|---------|---------|------:|------:|-------|
| `section_3_0x00081A14.bin` | v3.7.0 | 2,776 | **651** | primary program (decomp/export source) |
| `sec3_3_6_0.bin` | v3.6.0 (Cortex) | 1,592–2,217 | ~25 | 9 renames |
| `sec3_3_6_0.bin.0` | v3.6.0 (v8-m) | 0 | 0 | ORPHAN duplicate — close/ignore |
| `sec3_3_5_0.bin` | v3.5.0 | 1,726–1,728 | **75** | 9 direct + 54 chained |
| `sec3_3_4_0.bin` | v3.4.0 | 1,712 | **104** | 73 direct + 39 chain3 |
| `sec3_3_3_0.bin` | v3.3.0 | 1,727 | **106** | 71 direct + 79 chainN (4-hop) |
| `sec3_3_2_0.bin` | v3.2.0 (v8-m, base 0x03000000) | 1,641 | **91** | 77 direct + 76 chainN (5-hop) |

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_3_2_0.bin.chainN`). Reopen with `tools/open_all_programs.py` after any
Ghidra restart.

### ✅ 3.3.0→3.4.0 pair (done Aug 2026 session)

**⚠️ FIRST RELINK-SHIFT PAIR ENCOUNTERED.** Unlike 3.4→3.5 and 3.5→3.6
(0/32 segments changed), **3.3.0→3.4.0 changed 31/32 segments — all MOVE
(load +24, size +0)**. Every buffer moved +24 bytes: the linker inserted
something between 3.3.0 and 3.4.0, so functions are NOT at the same physical
locations. Consequences:

- The 4-hop chain's offset-ok check (all hops same physical location)
  rejected 43 PERFECT matches (score 1.0 on every hop) because the function
  genuinely moved. For shifted pairs, prefer `--threshold 0.9` WITHOUT
  `--require-offset`: a combo of 5 scores all ≥0.9 is trustworthy on its own.
- Fuzzy match 3.3.0 vs 3.4.0: 1,163 matches; 410 ≥0.9; 94 named targets.
  Dominant delta 0x350 but with spread (0x2f8, 0x274, 0x1b8…) — confirms
  the shift is NOT uniform per function.

**Direct renames (threshold 0.9): 71 applied** — same SDK families as
3.4.0 (`event_set`, `ipc_post_cmd/arg`, `hifi_busy_delay`, `wma_memcmp`,
`Unicode2Ascii`, `DmaConfig_for_LLP2`, `SDDecodeCSD`, `mbedtls_*`, etc.).

**Chain propagation — NEW GENERIC N-HOP SCRIPT (`tools/chainN_propagate_names.py`):**
- Generalizes chain3 to any hop count with `--match` (repeatable, oldest-pair
  first) + `--program` + `--threshold` + `--require-offset`.
- 4-hop chain 3.3→3.4→3.5→3.6→3.7: per-hop deltas auto-computed
  (0x350 / -0x3000078 / +0x3000268 / +0xc0); 313 chained, 151 offset-ok,
  115 combo ≥0.7.
- Applied with `--threshold 0.9` (no offset — relink shift): **79 names**, e.g.
  `AmrFunction`, `bitstream_getbits_be`, `pbuf_free`, `CodeResume`,
  `my_bui_clz`, `USBGetRxFIFOIntType`, `cmd_wifi_tcp_server`, `udp_server`,
  `RKDeviceUnRegister`, `hifi_memmove`, `event_set`, `ipc_post_cmd/arg`,
  `FmFreqToChan`, `hifi_busy_delay`, `DSD_DecodeBlock` @ 0x030FE650
  (v3.3 changelog: DSD ID3 display ✓), `flac_bitstream_getbits_s/u`,
  `APE_Set_CFG`, `wma_floor_log2`, `DICTDECODER_InitStream`, etc.
- **v3.3.0 now has 106 named / 1,727 functions (6.1%)** — was 0 before.

**Ghidra program state (saved Aug 2026):** 6 programs in the project —

| Program | Version | Funcs | Named | Notes |
|---------|---------|------:|------:|-------|
| `section_3_0x00081A14.bin` | v3.7.0 | 2,776 | **651** | primary program (decomp/export source) |
| `sec3_3_6_0.bin` | v3.6.0 (Cortex) | 1,592–2,217 | ~25 | 9 renames |
| `sec3_3_6_0.bin.0` | v3.6.0 (v8-m) | 0 | 0 | ORPHAN duplicate — close/ignore |
| `sec3_3_5_0.bin` | v3.5.0 | 1,726–1,728 | **75** | 9 direct + 54 chained |
| `sec3_3_4_0.bin` | v3.4.0 (v8-m, base 0x03000000) | 1,712 | **104** | 73 direct + 39 chain3 |
| `sec3_3_3_0.bin` | v3.3.0 (v8-m, base 0x03000000) | 1,727 | **106** | 71 direct + 79 chainN (4-hop) |

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_3_3_0.bin.chainN`). Reopen with `tools/open_all_programs.py` after any
Ghidra restart.

### ✅ 3.4.0→3.5.0 pair (done Aug 2026 session)

**String diff 3.3.0→3.4.0 (what v3.4.0 ADDED):**
- **Button-mode strings**: `Double click volume` @ 0x03728324, `Short press
  volume` @ 0x03728222, `Button Switching` @ 0x0372A86E (v3.4 changelog:
  Button Modes A/B ✓) — these live in the UI resource region (0x037xxxxx),
  no code xrefs (gotcha 8), and were renamed away by v3.7's Button C Mode.
- **DSD/dff debug strings**: `NO.%d frame dsdiff read frame err!` @
  0x030F68C0, `dff seek_fail` @ 0x030F68E8 (DSD region).

**Segment table diff 3.4.0→3.5.0:** **0/32 segments changed** — identical
memory layout (same as 3.5→3.6). Addresses are very stable across
3.4.0→3.5.0→3.6.0.

**Bulk fuzzy match 3.4.0 vs 3.5.0:** 1,132 matches; 404 ≥0.9; 97 matched to
named targets (71 @ score 1.0).

**Direct renames (threshold 0.9): 73 applied** — `aac_movfile_parser`,
`aac_aac_dec`, `OGGInfo_Parse`, `DICTDECODER_InitStream`, `event_set`,
`ipc_post_cmd/arg`, `hifi_busy_delay`, `hifi_memmove`, `wma_memcmp`,
`dac_gain_curve_apply` @ 0x030096F4 (volume cluster anchored!),
`buffered_fseek`, `find_option`, `SDDecodeCSD`, `AmrFunction`, `pbuf_free`,
`CodeResume`, `my_bui_clz`, `USBGetRxFIFOIntType`, `cmd_wifi_tcp_server`,
`udp_server`, `mbedtls_havege_random`, `StartCASystem`, `IsrDisable2`,
`UsbAdpterProbe`, `FmFreqToChan`, `wma_floor_log2`, `Unicode2Ascii(2)`,
`modinv_u32`, `FsIsLongName`, `SetSPIFreq`, `FatDev_PrevDir`,
`DmaConfig_for_LLP2`, `DmaReConfig2`, `RKDeviceUnRegister`, etc.

**Chain propagation — NEW 3-LEVEL CHAIN (`tools/chain3_propagate_names.py`):**
- The old `chain_propagate_names.py` only chains 2 hops (v3.4→v3.5→v3.6) and
  harvests names from the 2nd match's targets — that only reaches v3.6's ~29
  names (0 applied). Names live in v3.7 (651), so v3.4 needs a 3-hop chain:
  v3.4→v3.5 (`fuzzy_match_v34_v35.json`) → v3.6 (`fuzzy_match_v35_v36.json`)
  → v3.7 (`fuzzy_match_v36_v8m_full.json`).
- Per-hop deltas auto-computed from dominant hi-conf deltas: 45 = -0x3000078
  (v3.5 imported at base 0), 56 = +0x3000268, 67 = +0xc0.
- 314 v3.4 funcs chained to named v3.7 targets; **153 offset-ok** (all 3 hops
  same physical location); **114 combo ≥0.7; 39 applied with
  `--threshold 0.7 --require-offset`** (e.g. `AudioControlTask_Enter`,
  `AudioFileOpen`, `AudioFileMhSeek2`, `ReadFDTInfo`, `RecordStop`,
  `DICTDECODER_InitStream`, `aac_aac_dec`, `aac_movfile_parser`, `OGGInfo_Parse`,
  `mbedtls_havege_random`, `StartCASystem`, `pbuf_free`, `buffered_fseek`,
  `find_option`, `hifi_busy_delay`, `wma_memcmp`, `FW_Ansi2UnicodeStr`,
  `rkos_memory_malloc/free`, `printchar`, `modinv_u32`, `Unicode2Ascii`,
  `event_set`, `ipc_post_cmd/arg`, `DmaConfig_for_LLP2`, `DmaReConfig2`,
  `hifi_busy_delay_ovl_*`).
- **v3.4.0 now has 104 named / 1,712 functions (6.1%)** — was 0 before.
- UI key handlers (`MainUI_KeyHandler`, `BroMemSelKeyMenu_Handler`) do NOT
  chain to v3.4: v3.7's Button C Mode rewrote them. The volume cluster is
  anchored anyway via `dac_gain_curve_apply` @ 0x030096F4.

**Ghidra program state (saved Aug 2026):** 5 programs in the project —

| Program | Version | Funcs | Named | Notes |
|---------|---------|------:|------:|-------|
| `section_3_0x00081A14.bin` | v3.7.0 | 2,776 | **651** | primary program (decomp/export source) |
| `sec3_3_6_0.bin` | v3.6.0 (Cortex) | 1,592–2,217 | ~25 | 9 renames |
| `sec3_3_6_0.bin.0` | v3.6.0 (v8-m) | 0 | 0 | ORPHAN duplicate — close/ignore |
| `sec3_3_5_0.bin` | v3.5.0 | 1,726–1,728 | **75** | 9 direct + 54 chained |
| `sec3_3_4_0.bin` | v3.4.0 (v8-m, base 0x03000000) | 1,712 | **104** | 73 direct + 39 chain3 |

Rename history: `build/cross_version_renames_log.json` (keys include
`sec3_3_4_0.bin.chained`). Reopen with `tools/open_all_programs.py` after any
Ghidra restart.

### ✅ 3.5.0→3.6.0 pair (done Aug 2026 session)

**String diff 3.5.0→3.6.0 (`tools/string_diff_versions.py 3.5.0 3.6.0`):**
- **Added: 37, Removed: 86** strings. The added strings are **almost all FLAC
  decoder** — `..\\..\\Common\\Codec\\Audio\\HIFI\\flac\\hifi_flacdec.c`,
  `invalid subframe padding`, `invalid sync code`, `metadata_size =%d`,
  `### bps ###`, `### samplerate ###`, `# decode_subframe_fixed fail`,
  `s->curr_bps > 32`, `var blocksize not surport` @ 0x030DE7xx–0x030E02xx.
  → **v3.6.0 shipped a full FLAC decoder rewrite** (new file `hifi_flacdec.c`
  with FILE/LINE assert logging). This is the first time we can name the
  FLAC decoder internals (`flac_bitstream_getbits_s` etc.) with certainty.

**Segment table diff 3.5.0→3.6.0:** **0/32 segments changed** — identical
memory layout. Function addresses are therefore very stable between these two
versions (only in-code edits, no relink shift).

**Bulk fuzzy match (`tools/run_bulk_match.py sec3_3_5_0.bin sec3_3_6_0.bin`):**
- **1,141 matches; 406 high-confidence (≥0.9); 10 matched to named targets**
  (`RKDeviceUnRegister`, `AmrFunction`, `SDDecodeCSD`, `pbuf_free`,
  `CodeResume`, `my_bui_clz`, `USBGetRxFIFOIntType`, `cmd_wifi_tcp_server`,
  `udp_server`).

**Chain propagation (`tools/chain_propagate_names.py`):**
- Because 3.5.0 and 3.6.0 share the exact same segment layout, we chained
  matches: v3.5 → v3.6 (1,141 matches) then v3.6 → v3.7
  (`fuzzy_match_v36_v8m_full.json`, **346 v3.6 addrs with named v3.7 targets**)
  → **319 v3.5 functions chained to named v3.7 targets**; 67 applied with
  `--require-offset --threshold 0.7`.
- **v3.5.0 now has 75 named / 1,726 functions** (was 0 before this session),
  including `aac_aac_dec`, `aac_movfile_parser`, `OGGInfo_Parse`,
  `DICTDECODER_InitStream`, `parse_iTunes_frame`,
  `hifi_busy_delay`, `ipc_post_cmd`, `event_set`,
  `Unicode2Ascii`, `FsIsLongName`, `FatDev_PrevDir`, `wma_memcmp`,
  `rkos_memory_malloc/free`, etc.
- ⚠️ **13 weak-chain names were reverted** (chain score < 0.6 — the fuzzy
  matcher's 0.52x floor is unreliable, e.g. `flac_bitstream_getbits_s` was
  proposed for the vector area @ 0x000003e8). Use
  `tools/revert_weak_chain_names.py` to undo any such names.
- ⚠️ **v3.5 section_3 was imported at base 0 (not 0x03000000)** — its
  addresses are raw file offsets. Empirically: **v3.5 addr + 0x03000268 ==
  v3.6 addr** (230 high-conf matches share this delta). The code region
  starts 0x268 bytes into the file. Keep this DELTA in mind for any future
  v3.5 address math.
- ⚠️ **`/rename_function_by_address` operates on the CURRENT program** —
  always `/switch_program` first (the chain script now does this).

**Ghidra program state (saved Aug 2026):** 4 programs in the project —

| Program | Version | Funcs | Named | Notes |
|---------|---------|------:|------:|-------|
| `section_3_0x00081A14.bin` | v3.7.0 | 2,776 | **651** | primary program (decomp/export source) |
| `sec3_3_6_0.bin` | v3.6.0 (Cortex) | 1,592–2,217 | ~25 | 9 renames (RKDeviceUnRegister, AmrFunction, SDDecodeCSD, CodeResume, pbuf_free, my_bui_clz, USBGetRxFIFOIntType, cmd_wifi_tcp_server, udp_server) |
| `sec3_3_6_0.bin.0` | v3.6.0 (v8-m) | 0 | 0 | ORPHAN duplicate — close/ignore |
| `sec3_3_5_0.bin` | v3.5.0 | 1,726–1,728 | **75** | 9 direct + 54 chained (13 weak reverted) |

All saved to the Ghidra project. Rename history (re-apply after a restart):
`build/cross_version_renames_log.json` (per-program dict; keys
`sec3_3_5_0.bin`/`sec3_3_5_0.bin.chained`/`sec3_3_6_0.bin`). Reopen with
`tools/open_all_programs.py` after any Ghidra restart.

---

## 8. TOOLS INVENTORY (all in `tools/`)

### Created this session (multi-version pipeline)
| Tool | Purpose |
|------|---------|
| `build_version_corpus.py` | Paso 0: dedup all IMGs by section_3 hash → `build/version_corpus.json` |
| `string_diff_versions.py` | Paso 1: added/removed strings per pair → `build/string_diffs_all_versions.json` |
| `changelog_report.py` | Changelog × string-diff correlation → `docs/changelog-string-diff.md` |
| `segment_table_diff.py` | Paso 4: segment table diff → `build/segment_diff_*.json` |
| `diff_firmware_versions.py` | Generalized section diff with per-symbol change windows |
| `cross_version_string_map.py` | Shared-string anchors between two versions + Ghidra xrefs |
| `changelog_function_mapper.py` | Keyword → strings in a version's section_3 |
| `string_ref_scanner.py` | Scan for LE32 addr constants (NOTE: doesn't work — see gotcha 7) |
| `changelog_to_functions.py` | `/search_strings` → `/find_undocumented_by_string` → functions |
| `version_analysis_pipeline.py` | Full pipeline: region diff + changelog string mapping |
| `decompile_missing.py` | Decompiled the 175 missing functions (fixed raw-text parsing) |
| `extract_sec3_for_ghidra.py` | Extract section_3 from any version IMG → `build/sec3_<v>.bin` |
| `import_into_ghidra.py` | POST `/import_file` a sec3 bin with language + base |
| `run_bulk_match.py` | Wrapper around `/bulk_fuzzy_match` (parses dict response, saves) |
| `apply_cross_version_names.py` | Rename matches in the OLD program from NEW names (fixed `matches` key + per-program log) |
| `chain3_propagate_names.py` | **NEW** 3-hop chain (v3.4→v3.5→v3.6→v3.7): auto-delta per hop, offset-ok = all 3 hops same physical location, combo = min of 3 link scores; 39 names applied to v3.4.0 |
| `revert_weak_chain_names.py` | Undo chained names whose v3.6→v3.7 chain score < 0.6 (matcher floor is unreliable) |
| `chain3_propagate_names.py` | 3-hop chain (v3.4→v3.5→v3.6→v3.7): per-hop auto-delta, offset-ok = all 3 hops same physical location, combo = min of 3 link scores; 39 names applied to v3.4.0 |
| `chainN_propagate_names.py` | **NEW generic N-hop chain** (`--match` repeatable, oldest-pair first): per-hop auto-delta, offset-ok optional; 79 names applied to v3.3.0 (4 hops, combo ≥0.9, no offset due to relink shift) |
| `version_lineage.py` | **NEW (CURRENT PHASE)** builds `build/function_lineage.json`: walks every v3.7.0 function back through all 21 adjacent pairs → `first_seen` (introduced-in) version + edit clusters (`--min-score 0.9`). The core driver of changelog-anchored naming. |
| `export_named_flac.py` | **NEW example** of live-decompile + export pattern: writes `firmware/codecs/flac/*.c` from Ghidra with provenance headers |
| `match_structure_v2.py` | **NEW structural matcher v2** vs 5,333 SDK functions: signature + callee-overlap + constants + size + module-prefix bonus; **+34 names validated by SDK-callee overlap** (the strongest signal); output `build/structural_matches_v2.json` |
| `match_baseline_v2.py` | **NEW conservative baseline matcher**: matches only the 1.2.5-baseline FUN_*; accepts ONLY SDK-callee-overlap; **+16 applied** (36 weak same-name overlaps rejected as callers); output `build/baseline_matches_v2.json` |
| `callgraph_propagate_names2.py` | **NEW call-graph neighbor sweep**: for each named func, lists in-region FUN_* callees/callers → 641 candidates saved to `build/callgraph_neighbor_candidates.json` |
| `open_all_programs.py` / `close_program.py` / `manage_programs.py` | Open/close/switch Ghidra programs |
| `wait_analysis.py` | Poll `/analysis_status` until a program finishes analyzing |
| `check_close_schema2.py` | Inspect close/switch/save params |
| `cross_version_match.py` | Earlier v3.6→v3.7 matcher (superseded by run_bulk_match.py) |
| `export_to_c.py` | Exported 2,764 decompiled functions → `firmware/**/*.c` + `firmware/INDEX.md` |
| `get_unnamed_functions.py`, `check_decompilation_status.py`, `collect_new_names.py` | Status helpers |
| `list_api_tools.py`, `list_api_tools2.py`, `show_endpoint_params.py` | MCP schema introspection |
| `test_*.py`, `debug_*.py` | Endpoint debugging (decompile text format, xrefs, segments, strings) |

### Pre-existing (SDK matching era)
`ghidra_client.py`, `ghidra_api.py`, `decompile_all.py`, `run_auto_analysis*.py`,
`match_functionality_v2.py`, `match_structure.py`, `match_combined.py`,
`propagate_callgraph.py`, `build_sdk_callees.py`, `extract_sdk_features.py`,
`name_rom_api.py`, `propagate_similar.py`, `extract_fw.py`, `extract_all.py`,
`diff_firmware_sections.py`, `repack_firmware.py`, `extract_resources.py`,
`crc_util.py`, `verify_matches.py`, etc.

### Canonical commands
```bash
# Status
python tools/check_decompilation_status.py

# Paso 0: corpus
python tools/build_version_corpus.py

# Paso 1: string diff (one pair, or --all)
python tools/string_diff_versions.py 3.6.0 3.7.0 --limit 40
python tools/string_diff_versions.py --all --limit 15
python tools/changelog_report.py        # → docs/changelog-string-diff.md

# Paso 4: segment diff
python tools/segment_table_diff.py 3.6.0 3.7.0

# Changelog → functions (Ghidra running, v3.7 loaded)
python tools/changelog_to_functions.py --keywords "ID3,M4A,OGG,APE,DSD,album,favorites,RIFF,shuffle"
python tools/version_analysis_pipeline.py 3.6.0 3.7.0 --max-regions 150

# Decompile missing / export
python tools/decompile_missing.py
python tools/export_to_c.py

# SDK matching (name more FUN_*)
python tools/match_functionality_v2.py
python tools/match_structure.py
python tools/match_combined.py
python tools/propagate_callgraph.py
python tools/collect_new_names.py
```

---

## 9. NEXT STEPS (CURRENT PHASE — changelog-anchored naming)

**Done so far (all versions):** v3.7.0 primary (852 named) + 21 older versions
all imported, matched, chain-propagated and saved (87–106 names each).
`build/function_lineage.json` maps every traceable v3.7.0 function to its
`first_seen` version. See §5c for the full cluster table.

**1. (IMMEDIATE) Continue naming the introduction clusters — see §5c table.**
Recommended order (most changelog certainty first):
1. **v1.7.0** (9 FUN_*) — Custom EQ (near `DSP_GOODEF_*` @ 0x0300F7DC) +
   OGG support (near `VorbisOGG_Parser` @ 0x03023BE8). Most valuable.
2. **v2.4.0** (3 FUN_*) — USB DAC (near `USB_DAC_OpenStream` @ 0x0302B80A).
3. **v3.6.0** remaining 34 — callees of `flac_decode_frame`/`flac_decode_subframe`.
4. **v1.4.6** (11) — lyrics library; **v1.8.0** (8) — themes;
   **v3.1.0** (3) — genre/sleep; **v3.2.0** (3) — volume.

For each cluster: decompile FUN_* → inspect region/strings/callers → name per
changelog → `save_program` → export `.c` to `firmware/<subsystem>/`.

**2. Keep `docs/symbol-index.md` updated** with every function named in Ghidra
(this session added: flac_*, media_lib_*, OGG_*, wma_*, AudioState*, lcd_*,
config_*, gpio_*, audio_file_*, hifi_file_*).

**3. When continuing, re-open programs after a Ghidra restart** with
`python tools/open_all_programs.py`; renames may be lost if unsaved — always
`save_program` after every rename batch (the whole workflow depends on it).

**4. Deeper changelog anchoring per version** (Paso 3, still mostly open):
each changelog line should eventually map to a named function cluster. The
lineage tool gives the introduction clusters; the string diff tool
(`string_diff_versions.py`) gives the UI/text clues for the rest.

**5. Remaining cleanup (documented earlier):**
- 3.8.0 forward diff (its own layout check — big diff vs 3.7).
- Boundary repair: `hifi_busy_delay_ovl_09e3`, `_0fd1`, `_0e48`.
- `AudioPlayback_Start`/`MusicService_Init` mid-function split revisit.
- The 1.2.5 baseline (292 FUN_*) is low priority — stable SDK core.

---

## 10. KEY ADDRESSES (v3.7.0) — from `docs/symbol-index.md`

| Function | Address |
|----------|---------|
| `firmware_entry` | 0x03000010 |
| `MusicInit` | 0x0302B9D8 |
| `MusicService_Init` | 0x0302A3E0 |
| `AudioPlayback_Start` | 0x0302A398 |
| `AudioPause` | 0x0302A69E |
| `FormatList_Init` | 0x03013C10 |
| `DSP_GOODEF_Init/Process/Reload` | 0x0300F7DC / 0x0300FB0E / 0x0301022C |
| `mp3_dec_internal` / `mp3_id3v2_handler` | 0x0306E5DE / 0x0306FEC4 |
| `wma_audio_parse` | 0x0308CDDC |
| `aac_aac_dec` / `aac_movfile_parser` | 0x030AD690 / 0x030AEB90 |
| `hifi_flac_dec` / `hifi_flac_helper` | 0x030DF64C / 0x030DF52A |
| `wav_lib_decode` | 0x030A42EE |
| `VorbisOGG_Parser` | 0x03023BE8 |
| `APE_Codec_Open` / `APEV1_Parse` | 0x030EEF60 / 0x030CB34A |
| `DSD_DecodeBlock` | 0x030FFA3C |
| `ID3_Picture_Parser` | 0x03023526 |
| `DICTDECODER_InitStream` | 0x0304FB44 |
| `USB_DAC_OpenStream` | 0x0302B80A |
| `BT_Music_Handler` | 0x0300DDEA |
| `RecordWinSvc_Start` | 0x0300AABC |
| `HifiFileSeek/Read/Write/Close` | 0x0306B8E6 / 0x0306B94C / 0x0306B9BC / 0x0306BA1E |
| `event_set` / `event_clear` | 0x03073840 / 0x03073882 |
| `MainUI_KeyHandler` | 0x0301020C |
| ROM API block | 0x02FE0000–0x02FFFFFF (stubs named `rom_*`) |

### SDK-matched (Aug 2026 pass — see `docs/symbol-index.md` for full list)
`rk_printf` @0x03000E08, `SDC_Init` @0x03004612, `SNAND_Write` @0x03019246,
`OGGInfo_Parse` @0x0301930E, `dhcpd_add_option` @0x0301A30E,
`DisplayDev_ClrRect` @0x0301C978, `SNOR_Erase` @0x030264DC,
`ScuClockGateCtr` @0x03029822, `TransCodeFromUTF8ToUnicode` @0x0302CAB0,
`FatDev_FileSeek` @0x0304ED3E, `FATDelClusChain` @0x030644C4,
`SDC_UpdateFreq` @0x03082BFA, `GUI_Setfocus` @0x0308CB4C, `SNOR_Init` @0x0308EAF2,
`FSRealname` @0x030B1328, `FW_Ansi2Unicode` @0x030C4DEC, `SDC_WriteData` @0x030F0384

---

## 11b. SDK BUILD PHASE (NEW — Aug 2026) — the custom-firmware path

**All 53 Rockchip SDK source files compile** with arm-none-eabi-gcc (kernel
29 + audio 8 + codec wrappers 16). This unlocks a compile-from-source
firmware so DSP effects can be added in C (the user's goal).

Key facts for continuing:
- **Full status:** `docs/custom-firmware-status.md` (layers, headers, next steps)
- **Compile command:** see that doc (18 include paths + `-include armcc_compat.h`)
- **armcc compat:** the SDK is Keil-written; `armcc_compat.h` maps `__packed`,
  `__irq` (empty — armcc puts it post-declarator), `_ATTR_*` section macros,
  `__asm` (rewritten manually in Delay/Delay2/interrupt/interrupt2 as GCC naked asm)
- **Include-order traps:** PowerManager.h / Service.h redefine `EXT` — do NOT
  include them from SysInclude; include directly in their .c. Shared enums
  (eFREQ_APP) live in `freq_enums.h`.
- **mailbox commands:** DECODE enum in `audio_main.h`; ENCODE/REC in
  `main2_msgbox.h` (macros for Main2.c) + RecordControl.h enum — keep them
  separate, they clash.
- **RKFIO_FOpen:** old-style `()` declaration satisfies both .c variants.
- **Registers derived from Ghidra + SDK:** CRU 0x20000000, DMA 0x40010000
  (DW ahb), INTC 0x400B0000, GRF 0x400C0000, ADC 0x400D0000, I2S/PMU,
  SysTick/NVIC 0xE000E010 (RKnano layout with Irq.Enable/SetPend arrays).
- **DSP mod target:** `firmware/rockchip/audio/RkEQ/Effect/Effect.c`
  compiles — `EffectProcess(pBuffer, PcmLen)` is the per-frame hook.
- **Next:** linker script (place 22 codec .lib + kernel at segment
  addresses from `docs/memory-map.md`), link test, then the FiiO app layer
  (Ghidra-decompiled UI) on top.

---

## 11. LESSONS / RULES FOR ANY CONTINUING AI

1. **Always read this file + the other docs first.** Do not rediscover what's
   here.
2. **Never compare raw addresses between versions.** Always shape-match
   functions (Ghidra Version Tracking / fuzzy match / strings).
3. **Always exclude resources_part5 from code diffs.** Only main_fw_code matters.
4. **Ghidra MCP is text-in/text-out on several endpoints.** Parse responses
   defensively (raw C text, "From X in FUN [TYPE]" lines, `matches` key).
5. **Addresses → `format(addr,'08x')`, never `str(addr)`** (decimal bug).
6. **When naming a function in Ghidra, immediately update `docs/symbol-index.md`.**
7. **Run matching tools in order** (functionality → structure → combined →
   callgraph) and re-run `collect_new_names.py` to harvest renames.
8. **Do not trust oversized decomp bodies** flagged in `docs/symbol-index.md`.
9. **The changelog is the ground truth.** Every changelog line should map to a
   named function cluster before you move to the next version pair.
10. **v3.7.0 remains the primary program** in Ghidra (`FIIO-3.7.0-Decomp`).
    New versions get imported alongside it, never replace it.
11. **`/rename_function_by_address` acts on the CURRENT program.** Always
    `/switch_program` to the target program before renaming (the chain script
    handles this; direct calls do not).
12. **Imported sec3 bins may land at image base 0**, so their addresses are raw
    file offsets. Empirically **v3.5 offset + 0x03000268 == v3.6/3.7 address**
    (verified over 230 matches). Derive the delta per version pair from the
    dominant high-confidence match delta before assuming address math.
13. **Ghidra restarts wipe unsaved renames.** Save after every rename batch
    (`/save_program`). `build/cross_version_renames_log.json` is the backup;
    re-apply with `apply_cross_version_names.py` after reopening.
14. **Duplicate-name programs** (`sec3_3_6_0.bin` + `sec3_3_6_0.bin.0`) confuse
    `/switch_program` and `/close_program`. Avoid double-importing the same
    binary; if it happens, close by `name` closes ALL instances — reopen after.
15. **Chain matching is the force-multiplier:** v3.5→v3.6→v3.7 chaining put
    names into v3.5.0 using only 9 direct names from the v3.6→v3.7 pass. When
    layout is identical between adjacent pairs (0/32 segments changed), prefer
    chaining over direct low-threshold matches.
16. **`/bulk_fuzzy_match` scores have a low-score floor (~0.52) and are NOT
    uniformly trustworthy.** Scores like 0.5214 recur across unrelated
    functions. When propagating a name from a chain, filter on
    `min(direct, chain)` (combo) ≥ 0.7 AND require offset match; revert the
    rest with `revert_weak_chain_names.py`.
17. **The chain threshold must gate the combo score, not just the direct
    score** — names originate from v3.7, so the weaker of the two links bounds
    the confidence.
18. **Relink shifts are the norm below v3.5.** Every pair 3.4→3.3→…→1.2.5
    moves buffers (31/32 segments MOVE; shifts of +24, +512, …). After a
    shift, offset-ok REJECTS perfect matches (score 1.0) because the function
    physically moved — use `--threshold 0.9` WITHOUT `--require-offset` for
    shifted pairs (a combo of 10+ scores all ≥0.9 is trustworthy on its own).
    Check `segment_table_diff.py` output first: `0/32 moved` → offset-ok is
    safe; `30-31/32 moved` → drop it.
19. **`/decompile_function` returns RAW C TEXT, not JSON** (repeat of gotcha 1
    — it bit us again this session). Use `urllib.urlopen(...).read().decode()`
    directly, or the `_get` wrapper will choke trying to `json.loads` it.
20. **The lineage tool is the naming map:** `build/function_lineage.json` tells
    you WHICH version introduced each v3.7.0 function. A function first seen
    in vX is a changelog feature of vX — name it accordingly (see §5c).
    Functions with NO lineage (not in `v36_v8m_full` targets) were introduced
    in 3.7.0 itself (Button C Mode, USB power control).
21. **Name clusters in v3.7.0, not in the old versions.** v3.7.0 is the
    decompile/export source. The 21 old programs already carry their names
    (87–106 each); new discovery happens on v3.7.0 and exports to `firmware/`.
22. **UI/button clusters have no strings** (0x037xxxxx resource region, gotcha
    8) — for those, name by CALLER evidence (`get_xrefs_to`: e.g. called by
    `AudioStop`/`MusicService` → `AudioStateHandler`) + address region. Names
    from caller evidence are conservative; that's fine.
23. **Always re-verify counts after a session:** `list_functions_enhanced` on
    v3.7.0 = 852 named (30.7%) as of this handoff. The `.0` orphan program
    (`sec3_3_6_0.bin.0`, 0 funcs) is still open — ignore it.
