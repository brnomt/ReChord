# ReChord BB baseline from audio/bbsystem/system after NON2_EXCLUDE and SPECIAL_EXCLUDE.
BB_SRCS := \
  firmware/rockchip/audio/AAC/pAAC2.c \
  firmware/rockchip/audio/AudioControl/AudioControl.c \
  firmware/rockchip/audio/AudioControl/HoldonPlay.c \
  firmware/rockchip/audio/AudioControl/Pcm.c \
  firmware/rockchip/audio/Common/audio_track_control.c \
  firmware/rockchip/audio/Common/pCODECS.c \
  firmware/rockchip/audio/DSDIFF/pDSDIFF2.c \
  firmware/rockchip/audio/DSF/pDSF2.c \
  firmware/rockchip/audio/HIFI/alac/p_hifi_alac2.c \
  firmware/rockchip/audio/HIFI/ape/p_hifi_Ape2.c \
  firmware/rockchip/audio/HIFI/flac/p_hifi_flac2.c \
  firmware/rockchip/audio/HIFI/hifi_get_bits.c \
  firmware/rockchip/audio/Mp3/pMP32.c \
  firmware/rockchip/audio/Ogg/pOGG2.c \
  firmware/rockchip/audio/RkEQ/Effect/Effect.c \
  firmware/rockchip/audio/sbc/sbc_encode/sbc_enc_interface.c \
  firmware/rockchip/audio/SSRC/resample_interface.c \
  firmware/rockchip/audio/Wav/pWAV.c \
  firmware/rockchip/audio/Wav/pWAV2.c \
  firmware/rockchip/audio/Wav/pWAVEnc.c \
  firmware/rockchip/bbsystem/audio_file_access2.c \
  firmware/rockchip/bbsystem/BSP2.c \
  firmware/rockchip/bbsystem/cru2.c \
  firmware/rockchip/bbsystem/Debug2.c \
  firmware/rockchip/bbsystem/Delay2.c \
  firmware/rockchip/bbsystem/dma2.c \
  firmware/rockchip/bbsystem/interrupt2.c \
  firmware/rockchip/bbsystem/Main2.c \
  firmware/rockchip/bbsystem/SysTickHandler2.c \
  firmware/rockchip/system/debug/Debug.c \
  firmware/rockchip/system/fileseek/SysFindFile.c \
  firmware/rockchip/system/module_overlay/ModuleOverlay.c \
  firmware/rockchip/system/module_overlay/SysReservedOperation.c \
  firmware/rockchip/system/os/Msg.c \
  firmware/rockchip/system/os/OsHook.c \
  firmware/rockchip/system/os/Task.c \
  firmware/rockchip/system/os/Thread.c \
  firmware/rockchip/system/os/Win.c \
  firmware/rockchip/system/sysservice/Backlight.c \
  firmware/rockchip/system/sysservice/battery.c \
  firmware/rockchip/system/sysservice/bb_core.c \
  firmware/rockchip/system/sysservice/Hook.c \
  firmware/rockchip/system/sysservice/Service.c \
  firmware/rockchip/system/sysservice/UsbAdapterProbe.c

# Include directories preserved from SDK_INCLUDES in the current Makefile.
BB_INCLUDE_DIRS := \
  firmware/rockchip/include \
  firmware/rockchip \
  firmware/rockchip/driver/MemDev \
  firmware/rockchip/audio/Include \
  firmware/rockchip/audio/AudioControl \
  firmware/rockchip/audio/Common \
  firmware/rockchip/audio/RkEQ/Effect \
  firmware/rockchip/audio/RecordControl \
  firmware/rockchip/audio/ID3 \
  firmware/rockchip/audio/Wav/WAV_LIB \
  firmware/rockchip/audio/SSRC/resampler \
  firmware/rockchip/system/os \
  firmware/rockchip/system/fileseek \
  firmware/rockchip/system/module_overlay \
  firmware/rockchip/system/sysservice \
  firmware/rockchip/bbsystem
