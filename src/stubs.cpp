// =============================================================================
// stubs.cpp - Xbox 360 API stubs for GH:WoR PC recompilation
// =============================================================================
// APIs do Xbox 360 que nao existem no PC. Cada stub retorna 0 e loga as
// primeiras 3 chamadas para debug.
// =============================================================================

#include <rex/ppc.h>
#ifdef REWOR_DEV
#include "stub_tracker.h"
#else
#define PPC_STUB_TRACKED(subroutine) PPC_STUB(subroutine)
#endif

// --- XCustom (Guitar Hero dashboard hooks) ---

PPC_STUB_TRACKED(__imp__XCustomGetCurrentGamercard);
PPC_STUB_TRACKED(__imp__XCustomSetDynamicActions);
PPC_STUB_TRACKED(__imp__XCustomGetLastActionPressEx);

// --- XamInput ---
PPC_STUB_TRACKED(__imp__XamInputSendStayAliveRequest);

// --- XamAvatar ---
PPC_STUB_TRACKED(__imp__XamAvatarGetManifestLocalUser);
PPC_STUB_TRACKED(__imp__XamAvatarGetMetadataRandom);
PPC_STUB_TRACKED(__imp__XamAvatarManifestGetBodyType);
PPC_STUB_TRACKED(__imp__XamAvatarGetAssetsResultSize);
PPC_STUB_TRACKED(__imp__XamAvatarGetAssets);
PPC_STUB_TRACKED(__imp__XamAvatarLoadAnimation);

// --- XamShow (UI overlays) ---
PPC_STUB_TRACKED(__imp__XamShowFriendsUI);
PPC_STUB_TRACKED(__imp__XamShowPlayersUI);
PPC_STUB_TRACKED(__imp__XamShowGamerCardUIForXUID);
PPC_STUB_TRACKED(__imp__XamShowPlayerReviewUI);
PPC_STUB_TRACKED(__imp__XamShowMarketplaceUI);
PPC_STUB_TRACKED(__imp__XamShowMarketplaceDownloadItemsUI);
PPC_STUB_TRACKED(__imp__XamShowMessageComposeUI);

// --- XamUser ---
PPC_STUB_TRACKED(__imp__XamUserCreateStatsEnumerator);
PPC_STUB_TRACKED(__imp__XamUserGetMembershipTierFromXUID);
PPC_STUB_TRACKED(__imp__XamUserGetOnlineCountryFromXUID);

// --- XamBackground (DLC downloads) ---
PPC_STUB_TRACKED(__imp__XamBackgroundDownloadGetMode);
PPC_STUB_TRACKED(__imp__XamBackgroundDownloadSetMode);
PPC_STUB_TRACKED(__imp__XamBackgroundDownloadItemGetStatus);
PPC_STUB_TRACKED(__imp__XamBackgroundDownloadItemGetHistoryStatus);

// --- XNet ---
PPC_STUB_TRACKED(__imp__XNetLogonGetTitleID);

// --- NetDll ---
PPC_STUB_TRACKED(__imp__NetDll_getsockopt);

// --- XAudio ---
PPC_STUB_TRACKED(__imp__XAudioOverrideSpeakerConfig);

// --- Kernel I/O (xboxkrnl) ---
PPC_STUB_TRACKED(__imp__IoCheckShareAccess);
PPC_STUB_TRACKED(__imp__IoCompleteRequest);
PPC_STUB_TRACKED(__imp__IoDeleteDevice);
PPC_STUB_TRACKED(__imp__IoDismountVolume);
PPC_STUB_TRACKED(__imp__IoInvalidDeviceRequest);

// --- Kernel Object Manager ---
PPC_STUB_TRACKED(__imp__ObReferenceObject);
PPC_STUB_TRACKED(__imp__ObIsTitleObject);

// --- Kernel Runtime ---
PPC_STUB_TRACKED(__imp__RtlUpcaseUnicodeChar);

// --- LDI (Cabinet decompression) ---
PPC_STUB_TRACKED(__imp__LDICreateDecompression);
PPC_STUB_TRACKED(__imp__LDIDecompress);
PPC_STUB_TRACKED(__imp__LDIDestroyDecompression);
PPC_STUB_TRACKED(__imp__LDIResetDecompression);

// --- Additional Kernel/Driver APIs ---
PPC_STUB_TRACKED(__imp__IoRemoveShareAccess);
PPC_STUB_TRACKED(__imp__IoSetShareAccess);
PPC_STUB_TRACKED(__imp__ExAllocatePoolWithTag);

// --- Microphone/Peripheral device ---
PPC_STUB_TRACKED(__imp__MicDeviceRequest);
PPC_STUB_TRACKED(__imp__RmcDeviceRequest);
PPC_STUB_TRACKED(__imp__CurlOpenTitleBackingFile);

// --- XamInput (additional) ---
PPC_STUB_TRACKED(__imp__XamInputControl);
PPC_STUB_TRACKED(__imp__XamInputRawState);

// --- XamVoice ---
PPC_STUB_TRACKED(__imp__XamVoiceSubmitPacket);

// --- NetDll (additional networking) ---
PPC_STUB_TRACKED(__imp__NetDll_XNetConnect);
PPC_STUB_TRACKED(__imp__NetDll_XNetGetConnectStatus);
PPC_STUB_TRACKED(__imp__NetDll_XNetQosLookup);
PPC_STUB_TRACKED(__imp__NetDll_XNetServerToInAddr);
PPC_STUB_TRACKED(__imp__NetDll_XNetTsAddrToInAddr);
PPC_STUB_TRACKED(__imp__NetDll_XNetUnregisterInAddr);
PPC_STUB_TRACKED(__imp__NetDll_XNetUnregisterKey);

// --- XeCrypt (Xbox cryptography) ---
PPC_STUB_TRACKED(__imp__XeCryptMd5Init);
PPC_STUB_TRACKED(__imp__XeCryptMd5Update);
PPC_STUB_TRACKED(__imp__XeCryptMd5Final);

// ---------------------------------------------------------------------------
// XNotifyCreateListener(ULONGLONG qwAreas) -> HANDLE
// Return a dummy handle
// ---------------------------------------------------------------------------
extern "C" PPC_FUNC(__imp__XNotifyCreateListener) {
#ifdef REWOR_DEV
  StubTracker::instance().record_unmapped(0, 0);
#endif
  ctx.r3.u64 = 0xDEAD0002; // dummy notification listener handle
}


// =============================================================================
// VIDEO/RENDERING-CRITICAL STUBS
// =============================================================================

#include <cstring>

// ---------------------------------------------------------------------------
// XGetVideoMode(XVIDEO_MODE* pVideoMode) -> void
// XVIDEO_MODE layout (all big-endian u32):
//   +0x00: dwDisplayWidth
//   +0x04: dwDisplayHeight
//   +0x08: fIsInterlaced
//   +0x0C: fIsWideScreen
//   +0x10: fIsHiDef
//   +0x14: RefreshRate (float, big-endian)
//   +0x18: VideoStandard
// ---------------------------------------------------------------------------
extern "C" PPC_FUNC(__imp__XGetVideoMode) {
#ifdef REWOR_DEV
  StubTracker::instance().record_unmapped(0, 0);
#endif
  uint32_t mode_addr = static_cast<uint32_t>(ctx.r3.u64);
  if (mode_addr == 0) return;

  // Zero the struct first (48 bytes)
  uint8_t* dest = reinterpret_cast<uint8_t*>(PPC_RAW_ADDR(mode_addr));
  std::memset(dest, 0, 48);

  PPC_STORE_U32(mode_addr + 0x00, 1280);  // dwDisplayWidth
  PPC_STORE_U32(mode_addr + 0x04, 720);   // dwDisplayHeight
  PPC_STORE_U32(mode_addr + 0x08, 0);     // fIsInterlaced = false
  PPC_STORE_U32(mode_addr + 0x0C, 1);     // fIsWideScreen = true
  PPC_STORE_U32(mode_addr + 0x10, 1);     // fIsHiDef = true

  // RefreshRate = 60.0f (IEEE 754 big-endian: 0x42700000)
  PPC_STORE_U32(mode_addr + 0x14, 0x42700000);

  // VideoStandard = NTSC (1)
  PPC_STORE_U32(mode_addr + 0x18, 1);
}

