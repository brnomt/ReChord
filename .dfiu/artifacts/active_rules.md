# Active rules: ReChord

- [warning] `R-e9b5d8222673` (AP<->BB mailbox reply paths): Any mailbox reply (even DEC_OPEN_ERR) breaks the AP's wait; silence is the only deadlock. -> Guarantee a reply for every mailbox command path, including error answers; treat any silent command as a deadlock.
- [blocker] `R-e9e18de6c692` (fw1 scatter/loader model (tools/make_fw1.py)): Scatter row src addresses live in the loaded RAM image, not XIP; verify byte-for-byte against the reference image before flashing. -> Validate every scatter row against the reference fw1 boot model before any on-device flash.
- [blocker] `R-9461aeaecbe2` (tools/pack_fw1.py fw1 scatter table): Never flash a custom fw1 until the scatter table is validated; no scatter row may target 0x03000000..0x03004000. -> Keep the pack_fw1.py safety gate; require scatter-table review + reference-image diff before any fw1 write.
- [blocker] `R-1f1c5d829375` (tools/pack_img.py section_3 flash region): Never zero-fill the stock section_3 tail; the AP reads string/pointer tables from inside that region. -> Keep --keep-stock-tail as default; byte-diff the section_3 tail against stock after every repack.
