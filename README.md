# Narball
## Building with CMake
Requires CMake 3.24+ and Ninja. The Vulkan SDK is optional; if it isn't found, the Vulkan headers are downloaded automatically.

GCC (MSYS2 UCRT64, `pacman -S mingw-w64-ucrt-x86_64-{gcc,cmake,ninja}`):
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

Shaders (`*.vert`, `*.frag` and `*.comp` in `shader/` and `Narball/shader/`) are compiled with `glslc` from the Vulkan SDK. Each `.spv` is written next to its source, because that's where the app loads it from. Only edited shaders are recompiled, and new shader files are picked up automatically. Build just the shaders with `cmake --build --preset gcc-release --target shaders`. If `glslc` isn't found, or you configure with `-DWF_COMPILE_SHADERS=OFF`, the committed `.spv` files are used as-is. The `.spv` files are build outputs, so a CMake `clean` deletes them, and the next build regenerates them.

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
State as of the move from `WorldFabric.vcxproj` to CMake. Both the `gcc-*` and `x64-*` (MSVC) presets build cleanly and the chess demo runs.

### Needs verification
- **Visual Studio IDE debugging (F5) is untested.** The presets use the Ninja generator, which may ignore the `VS_DEBUGGER_WORKING_DIRECTORY` set in `CMakeLists.txt`. If so, the exe starts in `build\<preset>\` and can't find `./assets`. Either use the `launch.vs.json` workaround above, or make the app locate assets relative to the exe or repo root instead of the current directory.
- **VR in GCC builds is untested on a headset.** See the OpenVR note under compiler compatibility below.
- **Debug configuration changed.** The old Debug|x64 config defined `NDEBUG` and used whole-program optimization; CMake's Debug is a normal debug build, so `assert`s are now active. Restore the old flags in `CMakeLists.txt` if that was intentional.

### GCC/MSVC compatibility rules
The prebuilt SDKs in `lib/` and `dll/` are compiled with MSVC. GCC and MSVC disagree on how C++ member functions return structs by value, so calling such a method on a Steam or OpenVR C++ interface from a GCC build returns garbage or crashes.
- **Steamworks:** `ISteamUser::GetSteamID` and `ISteamMatchmaking::GetLobbyOwner` go through the flat C API in `SteamWorksPlugin.cpp` (`getUserSteamID`, `getLobbyOwner`). Any new Steam call that returns `CSteamID` or another struct by value needs the same treatment (see `steam_api_flat.h`).
- **OpenVR:** `IVRSystem::GetProjectionMatrix` and `GetEyeToHeadTransform` use OpenVR's C function table (`FnTable:IVRSystem_022`) in non-MSVC builds (`OpenXRPlugin.cpp`). The `IVRSystemFnTablePrefix` struct mirrors the start of `VR_IVRSystem_FnTable` in `openvr_capi.h`; recheck it if the OpenVR headers are upgraded. New struct-returning OpenVR calls need the same treatment.
- Calls returning integers, enums, `bool` or pointers, or filling output parameters, are safe with either compiler.

### Cleanup candidates
- **Keep variables initialized.** All members and locals in the project's own code (everything outside `include/`, `vk_mem_alloc.h` and VkBootstrap) now have initializers; this exposed an uninitialized draw-indirect buffer handle that crashed GCC builds and a `Board` constructor that initialized its position from itself. To check new code, run (from an MSYS2 UCRT64 shell, after configuring `gcc-release`; needs `mingw-w64-ucrt-x86_64-clang-tools-extra`):
  ```
  run-clang-tidy -p build/gcc-release -quiet \
    -checks='-*,cppcoreguidelines-init-variables,cppcoreguidelines-pro-type-member-init' \
    -header-filter='.*/(engine/header|header|Narball/header)/.*' -exclude-header-filter='.*(vk_mem_alloc|VkBootstrap).*' \
    '^(?!.*(VkBootstrap|/include/)).*\.cpp$'
  ```
  A GCC build with `-Wuninitialized -Wmaybe-uninitialized` at `-O2` also catches reads of uninitialized values.
- **Missing return.** `Polynomial::operator[](const ComplexNumber&)` in `engine/header/Utilities.h` calls `apply(x)` but never returns a value (undefined behavior if called; GCC warns with `-Wreturn-type`). It should probably be `return apply(x);`.
- **Fence-based destruction for the other GPU resources.** `TriangleModel`'s indirect draw buffers go through `VulkanPlugin::destroyAfterGPU`, which frees them only after the render fence proves their frame finished (`completed_frame`). The other queues in `VulkanPlugin::run` (VMA buffers, images, samplers, descriptor sets) still rely on timing heuristics (`millis_to_hold_buffer` and `frames_to_hold_buffer`). Buffers used by off-thread `immediateSubmit` work would need their own tracking before they can switch over. Also, the Vulkan device is never torn down at exit, so nothing flushes these queues on shutdown.
- **Unused libraries.** `CMakeLists.txt` links every `.lib` the old project did, but the exe only imports `SDL3`, `SDL3_ttf`, `OpenAL32`, `openvr_api`, `steam_api64` and `vulkan-1`. `glew32`, `OpenGL32`, `SDL3_image/mixer/net/rtf`, `SDL3_test`, `glew32s` and `sdkencryptedappticket64` could likely be dropped.
- **Unused DLLs.** All of `dll/` is copied next to the exe. `SDL2.dll`, `freeglut.dll`, `glfw3.dll`, `glew32.dll`, `steam_api.dll` (32-bit) and the SDL3 extension DLLs aren't imported. `ucrtbase.dll` shouldn't be redistributed this way; it ships with Windows.
- **Retire the `compile_shaders.bat` scripts.** CMake now compiles every shader, so `shader/compile_shaders.bat` and `Narball/shader/compile_shaders.bat` are redundant. They had already drifted: neither listed `sky.comp` or `colored_triangle.vert`, and `GLTFShadow.frag.spv` hadn't been rebuilt since the "Fix shadow offset" commit.
- **Shared shaders are duplicated.** Most files in `Narball/shader` have a same-named copy in `shader/` (for example, the two `GLTFShadow.frag` files are identical). A shared directory, or `#include` files (CMake already tracks includes through depfiles), would stop them drifting apart.
- **Only some apps have been run.** The `WF_APP` switch was smoke-tested with `Chess`, `Narball` and `Pyramid`. The other demo states compile but haven't been launched since the CMake move.
- **No CI.** A Windows CI job building both `gcc-release` and `x64-release` would catch the kind of MSVC-only code that broke the GCC build.