# Narball
## Building with CMake
Requires CMake 3.24+, Ninja and the `glslc` shader compiler. `glslc` comes with the Vulkan SDK, or with MSYS2's `shaderc` package. The rest of the Vulkan SDK is optional: if it isn't found, the Vulkan headers are downloaded automatically.

GCC (MSYS2 UCRT64, `pacman -S mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,shaderc}`):
```
cmake --preset gcc-release
cmake --build --preset gcc-release
```

MSVC (from a "x64 Native Tools" / Developer PowerShell prompt, or open the folder in Visual Studio):
```
cmake --preset x64-release
cmake --build --preset x64-release
```

Run from the repo root so `./assets` and `./Narball` resolve, e.g. `build\gcc-release\WorldFabric.exe`, or `cmake --build --preset gcc-release --target run`. The DLLs from `dll/` are copied next to the exe automatically.

Shaders (`*.vert`, `*.frag` and `*.comp` in `shader/` and `Narball/shader/`) are compiled with `glslc`. Each `.spv` is written next to its source, because that's where the app loads it from. The `.spv` files are gitignored, so a fresh clone has to be built before it can run. Only edited shaders are recompiled, and new shader files are picked up automatically. Build just the shaders with `cmake --build --preset gcc-release --target shaders`. If `glslc` isn't on `PATH` or in the Vulkan SDK, set `-DWF_GLSLC=<path to glslc.exe>`.

### Choosing the app
The `WF_APP` cache variable picks what the exe starts (default `Chess`). Changing it recompiles only `source/Main.cpp`:
```
cmake --preset gcc-release -DWF_APP=Narball
cmake --build --preset gcc-release
```
In PowerShell, quote the argument when it contains a variable (`"-DWF_APP=$app"`). Otherwise PowerShell passes `$app` through literally.

| `WF_APP` | Starts |
| --- | --- |
| `Chess` | `Chess::ChessApp` |
| `Narball`, `NarballServer`, `NarballDesync` | `Narball::main` as the game, dedicated server, or desync checker (`NarballDesync` compares `event_log_1.csv` and `event_log_2.csv` from the working directory) |
| `VulkanDemo`, `SceneDemo`, `SceneDemo2`, `BallTest`, `SocketTest`, `BallThrow`, `Mirror`, `Trace`, `CollisionTest`, `ConstraintTest`, `Pyramid`, `NetPhysics` | the matching `*App` state (`SocketTest` is the `SocketTest` class) |

In Visual Studio, set it under Project > CMake Settings, or in the cache editor. To add an app, append its name to `WF_APPS` in `CMakeLists.txt` and add an entry to the `apps` table in `source/Main.cpp`. A `static_assert` catches a name that's listed in CMake but missing from the table.

## Visual Studio
1. Install Visual Studio with the "Desktop development with C++" workload (includes CMake and Ninja).
2. File > Open > Folder and select the repo root.
3. Pick "MSVC x64 Release" (or Debug) from the configuration dropdown, then build and run WorldFabric.exe.

If assets fail to load when debugging, set the working directory to the repo root: in `.vs\launch.vs.json`, add `"currentDir": "${workspaceRoot}"` to the WorldFabric.exe configuration.

## Follow-up work

### Needs verification
- **Visual Studio IDE debugging (F5) is untested.** The presets use the Ninja generator, which may ignore the `VS_DEBUGGER_WORKING_DIRECTORY` set in `CMakeLists.txt`. If so, the exe starts in `build\<preset>\` and can't find `./assets`. Either use the `launch.vs.json` workaround above, or make the app locate assets relative to the exe or repo root instead of the current directory.
- **VR in GCC builds is untested on a headset.** See the OpenVR note under compiler compatibility below.

### GCC/MSVC compatibility rules
The prebuilt SDKs in `lib/` and `dll/` are compiled with MSVC. GCC and MSVC disagree on how C++ member functions return structs by value, so calling such a method on a Steam or OpenVR C++ interface from a GCC build returns garbage or crashes.
- **Steamworks:** `ISteamUser::GetSteamID` and `ISteamMatchmaking::GetLobbyOwner` go through the flat C API in `SteamWorksPlugin.cpp` (`getUserSteamID`, `getLobbyOwner`). Any new Steam call that returns `CSteamID` or another struct by value needs the same treatment (see `steam_api_flat.h`).
- **OpenVR:** `IVRSystem::GetProjectionMatrix` and `GetEyeToHeadTransform` use OpenVR's C function table (`FnTable:IVRSystem_022`) in non-MSVC builds (`OpenXRPlugin.cpp`). The `IVRSystemFnTablePrefix` struct mirrors the start of `VR_IVRSystem_FnTable` in `openvr_capi.h`; recheck it if the OpenVR headers are upgraded. New struct-returning OpenVR calls need the same treatment.
- Calls returning integers, enums, `bool` or pointers, or filling output parameters, are safe with either compiler.

### Cleanup notes and findings
- clang-tidy can be used to automate code cleanup. It has already been used to initialize variables that were causing compile problems with gcc
- Using GCC with `-Wuninitialized -Wmaybe-uninitialized` at `-O2` catches most uninitialized values.
- **Missing return.** `Polynomial::operator[](const ComplexNumber&)` in `engine/header/Utilities.h` calls `apply(x)` but never returns a value (undefined behavior if called; GCC warns with `-Wreturn-type`). It should probably be `return apply(x);`.
- **Fence-based destruction for the other GPU resources.** `TriangleModel`'s indirect draw buffers go through `VulkanPlugin::destroyAfterGPU`, which frees them only after the render fence proves their frame finished (`completed_frame`). The other queues in `VulkanPlugin::run` (VMA buffers, images, samplers, descriptor sets) still rely on timing heuristics (`millis_to_hold_buffer` and `frames_to_hold_buffer`). Buffers used by off-thread `immediateSubmit` work would need their own tracking before they can switch over. Also, the Vulkan device is never torn down at exit, so nothing flushes these queues on shutdown.
- **Unused libraries.** `CMakeLists.txt` links every `.lib` the old project did, but the exe only imports `SDL3`, `SDL3_ttf`, `OpenAL32`, `openvr_api`, `steam_api64` and `vulkan-1`. `glew32`, `OpenGL32`, `SDL3_image/mixer/net/rtf`, `SDL3_test`, `glew32s` and `sdkencryptedappticket64` could likely be dropped.
- **Unused DLLs.** All of `dll/` is copied next to the exe. `SDL2.dll`, `freeglut.dll`, `glfw3.dll`, `glew32.dll`, `steam_api.dll` (32-bit) and the SDL3 extension DLLs aren't imported. `ucrtbase.dll` shouldn't be redistributed this way; it ships with Windows.
- **The panel pass decides which image is presented.** `shader/PanelPost.comp` (formerly Narball's version) blurs `final_image` under panels and writes the composited frame into `color`. Every render target therefore has to present `color_image`, as `createRenderTarget` does in both `source/Main.cpp` and `Narball/header/NarballMain.h`. A target that presents `final_image` would show the scene without panels.
