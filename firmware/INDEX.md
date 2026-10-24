# ReChord firmware — Source Index

## Live build tree (`firmware/`)

```
firmware/
├── rockchip/          # Rockchip RKnanoD SDK (compiled from source)
│   ├── audio/         #   audio control, codecs, RkEQ (Effect.c = DSP hook)
│   ├── bbsystem/      #   Main2, BSP, cru, dma, interrupt2, Delay2, Debug2
│   ├── system/        #   os (Task/Thread/Msg/Win), fileseek, module_overlay,
│   │                  #   sysservice (battery, backlight, Service, Hook)
│   └── include/       #   armcc_compat.h + integration headers
├── startup/
│   └── startup.c      # RKnanoFW header
├── entry_stubs.S     # firmware_entry @ 0x03000010 -> rechord_hw_init -> rechord_main
├── rechord_app.c     # rechord_hw_init (ROM init) + rechord_main (from-source UI)
├── rechord_win.c     # MainMenuWin (WIN structure for the SDK UI framework)
├── stubs.c            # link stubs: globals (zero), weak funcs, newlib syscalls
├── firmware.ld        # linker script (SDK sections + segment-table buffers)
└── resource/          # resource_api.h
```

## Build

```
make                # standalone AP5M (our AP + Core B) -> build/ReChord_AP5M.IMG
make cfw-ap         # AP ELF only -> build/cfw/rechord_ap.elf
make clean-clone-check   # two clean worktrees must produce the same image
```

The SDK BB/stock packing targets (`build-bb`, `link-bb`, `pack-bb-img`,
`all`, `release`, `cfw-img`) were removed in BRNOMT-82: they depended on
`community/` blobs and the stock image. `firmware/rockchip/` stays in the tree
as RE source but is not part of the product build.

## Archived decompilation (spec only, not compiled)

The Ghidra decompilation output that served as the spec for the
decompilation phase is archived under `docs/re/decomp/` (apps, codecs,
drivers, network, usb, stubs, firmware core + headers). It is **not**
part of the live build — the SDK is compiled from `firmware/rockchip/`.
