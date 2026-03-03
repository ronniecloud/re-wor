#include "generated/re_wor_config.h"
#include "generated/re_wor_init.h"

#include "discord_rpc.h"

#include <rex/rex_app.h>
#include <rex/ppc.h>
#include <rex/runtime.h>
#include <rex/audio/sdl/sdl_audio_system.h>
#include <rex/input/input_system.h>
#include <rex/ui/keybinds.h>

#include <cstdio>
#include <unordered_set>
#include <cstring>
#include <unordered_map>
#include <fstream>
#ifdef REWOR_DEV
#include "stub_tracker.h"
#endif

#ifdef _WIN32
#include <windows.h>
#include <rex/ui/window_win.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

#ifdef REWOR_DEV
static std::ofstream& dbg() {
  static std::ofstream log("re_wor_trace.log", std::ios::out | std::ios::trunc);
  return log;
}
#endif
#ifdef REWOR_DEV
#define TRACE(msg) do { dbg() << msg << std::endl; dbg().flush(); } while(0)
#else
#define TRACE(msg) ((void)0)
#endif

static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
  std::ofstream log("re_wor_crash_detail.log", std::ios::out | std::ios::trunc);
  log << "Exception: 0x" << std::hex << ep->ExceptionRecord->ExceptionCode << "\n";
  log << "RIP: 0x" << std::hex << ep->ContextRecord->Rip << "\n";
  if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
    log << "Access: " << (ep->ExceptionRecord->ExceptionInformation[0] == 0 ? "READ" :
                          ep->ExceptionRecord->ExceptionInformation[0] == 1 ? "WRITE" : "EXEC") << "\n";
    log << "Target: 0x" << std::hex << ep->ExceptionRecord->ExceptionInformation[1] << "\n";
  }
  void* stack[64];
  USHORT frames = CaptureStackBackTrace(0, 64, stack, NULL);
  log << "\nStack (" << std::dec << frames << " frames):\n";
  for (USHORT i = 0; i < frames; i++)
    log << "  [" << std::dec << i << "] 0x" << std::hex << (uintptr_t)stack[i] << "\n";
  log.flush();
  return EXCEPTION_CONTINUE_SEARCH;
}
#endif

// ---------------------------------------------------------------------------
// Safe CRT init ctor caller — uses SEH to catch crashes in individual ctors
// ---------------------------------------------------------------------------
static void safe_call_ctor(PPCContext& ctx, uint8_t* base, uint32_t ppc_addr,
                           const char* source, int& called, int& skipped, int& crashed) {
  auto* fn = PPC_LOOKUP_FUNC(base, ppc_addr);
  if (!fn) {
    skipped++;
    return;
  }
#ifdef _WIN32
  __try {
    fn(ctx, base);
    called++;
  } __except(EXCEPTION_EXECUTE_HANDLER) {
    TRACE("[ctor] CRASH in 0x" << std::hex << ppc_addr << " (" << source 
          << ") exception=0x" << GetExceptionCode() << " - skipping");
    crashed++;
  }
#else
  fn(ctx, base);
  called++;
#endif
}

// ---------------------------------------------------------------------------
// Hook: sub_822CCB60 — CRT static initializer callback iterator (SAFE VERSION)
// ---------------------------------------------------------------------------
extern "C" PPC_FUNC(sub_822CCB60) {
  TRACE("[sub_822CCB60] Safe CRT init started");
  int called = 0, skipped = 0, crashed = 0;

  // Stage 1: Single callback at 0x82064074
  uint32_t fn_addr = 0;
  bool stage1_readable = false;
#ifdef _WIN32
  __try {
    fn_addr = PPC_LOAD_U32(0x82064074);
    stage1_readable = true;
  } __except(EXCEPTION_EXECUTE_HANDLER) {
    TRACE("[sub_822CCB60] Cannot read stage1 addr 0x82064074 (exc=0x" 
          << std::hex << GetExceptionCode() << ") - skipping stage1");
  }
#else
  fn_addr = PPC_LOAD_U32(0x82064074);
  stage1_readable = true;
#endif
  if (stage1_readable && fn_addr != 0) {
    safe_call_ctor(ctx, base, fn_addr, "stage1", called, skipped, crashed);
  }

  // Stage 2: Table 1 (0x827D2720 to 0x827D272C)
  TRACE("[sub_822CCB60] Stage 2: table1 (0x827D2720-0x827D272C)");
  for (uint32_t ea = 0x827D2720; ea < 0x827D272C; ea += 4) {
    fn_addr = PPC_LOAD_U32(ea);
    if (fn_addr != 0) {
      safe_call_ctor(ctx, base, fn_addr, "table1", called, skipped, crashed);
    }
  }

  // Stage 3: Table 2 (0x827D0414 to 0x827D271C)  
  TRACE("[sub_822CCB60] Stage 3: table2 (0x827D0414-0x827D271C)");
  for (uint32_t ea = 0x827D0414; ea < 0x827D271C; ea += 4) {
    fn_addr = PPC_LOAD_U32(ea);
    if (fn_addr != 0 && fn_addr != 0xFFFFFFFF) {
      safe_call_ctor(ctx, base, fn_addr, "table2", called, skipped, crashed);
    }
  }

  TRACE("[sub_822CCB60] Done: called=" << std::dec << called 
        << " skipped=" << skipped << " crashed=" << crashed);
}

// ---------------------------------------------------------------------------
// Safe stub for unmapped PPC functions — prevents RIP=0x0 crashes
// ---------------------------------------------------------------------------
#ifdef REWOR_DEV
static std::unordered_set<uint32_t> g_logged_unmapped;
#endif

static PPC_FUNC(unmapped_func_stub) {
  uint32_t target = ctx.ctr.u32;
  uint32_t caller = static_cast<uint32_t>(ctx.lr);
#ifdef REWOR_DEV
  if (g_logged_unmapped.find(target) == g_logged_unmapped.end()) {
    g_logged_unmapped.insert(target);
    TRACE("[UNMAPPED] PPC call to 0x" << std::hex << target 
          << " from 0x" << caller << " - stubbed (total unmapped: " 
          << std::dec << g_logged_unmapped.size() << ")");
  }
#endif
#ifdef REWOR_DEV
  StubTracker::instance().record_unmapped(target, caller);
#endif
  ctx.r3.u64 = 0;  // Safe default return value for unmapped functions
}

// Fill null entries in the dispatch table with our safe stub
static void fill_dispatch_table(uint8_t* base) {
  TRACE("[dispatch] Filling null entries in function dispatch table...");
  
  auto* table_base = reinterpret_cast<PPCFunc**>(
      base + PPC_IMAGE_BASE + PPC_IMAGE_SIZE);
  
  size_t num_entries = PPC_CODE_SIZE / 4;
  size_t filled = 0;
  
  for (size_t i = 0; i < num_entries; i++) {
    if (table_base[i] == nullptr) {
      table_base[i] = &unmapped_func_stub;
      filled++;
    }
  }
  
  TRACE("[dispatch] Filled " << std::dec << filled << " null entries out of " 
        << num_entries << " total (" << (num_entries - filled) << " mapped)");
}


class ReWorApp : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  ~ReWorApp() override { discord_shutdown(); }

  void OnPreSetup(rex::RuntimeConfig& config) override {
    TRACE("[OnPreSetup] entered");
    config.audio_factory = REX_AUDIO_BACKEND(rex::audio::sdl::SDLAudioSystem);
    config.input_factory = REX_INPUT_BACKEND(rex::input::CreateDefaultInputSystem);
  }

  void OnPostSetup() override {
    TRACE("[OnPostSetup] entered - XEX loaded, runtime ready");
    
    uint8_t* base = runtime()->virtual_membase();
    fill_dispatch_table(base);

    discord_init();
    discord_update();

    // F11: Toggle borderless fullscreen
    rex::ui::RegisterBind("fullscreen_toggle", "F11", "Toggle fullscreen", [this]() {
        window()->SetFullscreen(!window()->IsFullscreen());
    });
#ifdef REWOR_DEV
    StubTracker::instance().start_reporter(5);
#endif
    TRACE("[OnPostSetup] StubTracker reporter started (5s interval -> re_wor_stub_freq.log)");
    
    TRACE("[OnPostSetup] sub_822CCB60 is hooked via PPC_FUNC - will be called by runtime during module init");
  }

  void OnCreateDialogs(rex::ui::ImGuiDrawer* imgui) override {
#ifdef _WIN32
    // Set window icon from embedded resource
    auto* win32_window = dynamic_cast<rex::ui::Win32Window*>(window());
    if (win32_window) {
      HWND hwnd = win32_window->hwnd();
      if (hwnd) {
        HICON icon = LoadIconA(GetModuleHandleA(NULL), "IDI_ICON1");
        if (icon) {
          SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
          SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
        }
      }
    }
#endif
    // Start in borderless fullscreen by default
    window()->SetFullscreen(true);
  }

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
#ifdef _WIN32
    SetUnhandledExceptionFilter(CrashHandler);
#endif
TRACE("[Create] constructing ReWorApp");
    auto app = std::unique_ptr<ReWorApp>(new ReWorApp(ctx, "Guitar Hero: Warriors of Rock",
        {PPC_CODE_BASE, PPC_CODE_SIZE, PPC_IMAGE_BASE,
         PPC_IMAGE_SIZE, PPCFuncMappings}));
    TRACE("[Create] done");
    return app;
  }
};

REX_DEFINE_APP(re_wor, ReWorApp::Create)
