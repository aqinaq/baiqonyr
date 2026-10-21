# Baikonur Rocket Launch

Native C++17 / OpenGL 3.3 core final project for Computer Graphics Fundamentals. Launch a stylized rocket, follow it through a darkening sky, and inspect four independently detached boosters.

## Build and run

System prerequisites: CMake 3.20+, a C++17 compiler, desktop window system and OpenGL 3.3 core driver. GLFW 3.4, GLM 1.0.1, Dear ImGui 1.91.8 and GLAD are bundled under `vendor`; configuration does not download packages. No external models or textures are needed.

On macOS you can double-click `run.command` after installing the course compiler and CMake. It builds and launches the application.

macOS/Linux, from the project directory:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
./build/baikonur
```

Windows, from a Visual Studio developer terminal with the C++ desktop workload and CMake:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
.\build\Release\baikonur.exe
```

macOS requires Xcode Command Line Tools. Linux uses GLFW's X11 backend, requiring the usual compiler, X11, Xrandr, Xinerama, Xcursor, Xi and OpenGL development headers. On Ubuntu these are provided by `build-essential cmake xorg-dev libgl1-mesa-dev`. They are system prerequisites; all project libraries are bundled. Confirm compatibility with the course assignment setup before submission. Windows/Linux commands have not been executed locally. The repository includes a GitHub Actions build/test matrix for Windows, macOS and Linux; its first hosted run is pending publication.

The build copies `shaders/` and `assets/` beside the executable. Keep both folders with the executable if moving it. These folders are also accepted from the working directory. Run from a writable directory for screenshots. Context-creation errors usually require checking the graphics driver/OpenGL support or the desktop session.

Verified locally: Apple M1, 8 GiB RAM, macOS 26.3.1, AppleClang 21.0.0, Release, OpenGL 4.1 core / Metal 90.5. Code uses the 3.3 feature set.

## Controls

| Action | Control |
|---|---|
| Launch eight-second countdown | Space / Launch button |
| Pause or resume mission/particles | P |
| Reset mission, particles and camera | R |
| Separate boosters after T+5 | B; automatic at T+24 |
| Toggle chase camera | C |
| Orbit and pitch | Left drag / A,D and W,S |
| Zoom | Wheel / Q,E |
| Toggle ignition shake | K |
| Toggle gantry light | L |
| Toggle day/night | N |
| Toggle wireframe | F |
| Show/hide help | H |
| Save `output/screenshot.ppm` | F12 |
| Exit | Esc |
| Change playback speed | On-screen slider |

Camera movement remains available while paused. Input over the control panel belongs to the interface. The flight path is a compressed visual trajectory, not physical aerospace simulation.

## Features and source layout

- Indexed procedural cubes, cylinders, cones and spheres form the pad, rocket, boosters, gantry, buildings, tanks, radome, roads and lights.
- The terrain follows Kazakhstan's outline, using a bundled Natural Earth boundary, concave triangulation and solid side walls. North is -Z, east is +X, and the launch pad sits at the projected Baikonur location. The country is scaled to 10,000 scene units wide; its full outline becomes visible from high altitude. `python3 tools/generate_kazakhstan.py` regenerates the baked boundary without downloads.
- The launch-site background includes a ribbed assembly hangar, fuel farm and pipework, lattice communications mast, service vehicles, containers, railway, fencing and utility cables. Sparse steppe scrub, stones, irregular ridges, mottled ground and drifting clouds add detail before liftoff. These are stylized procedural scenery, not a reconstruction of the actual facility.
- Explicit MVP matrices and parent/child transforms; boosters detach in world space with continuous position and initial vertical velocity.
- Own per-fragment Blinn-Phong GLSL, inverse-transpose normals, directional sun, toggled gantry light and moving engine light.
- Concrete texture with UVs, repeat wrapping and mipmaps; procedural ground and sky.
- Wireframe inspection draws deduplicated triangle edges with `GL_LINES`, avoiding the slow polygon-mode path on the tested driver. Use F or start with `--wireframe`.
- Bounded fire/smoke pool, back-to-front alpha smoke and additive fire, two instanced billboard passes.
- All topic stretch features: independent booster detachment, ignition shake and altitude stars.
- Keyboard/mouse camera, scene controls, ImGui help, FPS/frame time and captioned demo replay.

`src/` separates Shader, Mesh, Camera, Texture, Scene, Particles, Mission, UI and capture. Original application files are below 500 lines; vendored libraries retain upstream structure. `shaders/` has separate vertex/fragment files; `tests/` checks mission invariants. See [course mapping](docs/COURSE_MAP.md), [defense guide](docs/DEFENSE_GUIDE.md), [requirements checklist](docs/REQUIREMENTS_CHECKLIST.md) and [credits/licenses](CREDITS.md).

## Validation and reproduction

```sh
./build/baikonur --duration 12 --no-retina --no-vsync --csv output/ready.csv
./build/baikonur --launch --start-time -3 --duration 12 --no-retina --no-vsync --csv output/ascent.csv
./build/baikonur --launch --start-time 24 --duration 12 --no-retina --no-vsync --csv output/separation.csv
```

CSV output excludes the first two seconds. `python3 tools/benchmark.py` repeats these tests and updates `docs/evidence/metrics.json` and raw evidence; it expects the single-configuration `build/baikonur` path. Windows users can run the commands directly with their executable path. Close GPU-heavy work before measuring.

Local 720p mean frame-loop throughput for the current terrain/scenery: 504.4 FPS ready, 477.6 FPS ignition/ascent, 586.2 FPS separation/stars. Respective p95 frame times: 3.93, 2.73, 2.14 ms. Occasional frame outliers remain; averages do not guarantee every frame meets the target. These are CPU frame-loop measurements including swaps, not GPU-only timings. TA-machine performance still requires verification.

Replay the captioned 200-second sequence:

```sh
./build/baikonur --demo --duration 200
```

Optional recording requires an external FFmpeg encoder. **FFmpeg is not needed to build, run or assess the application**; the submitted MP4 is already encoded. To record again:

```sh
./build/baikonur --demo --width 1920 --height 1080 --duration 200 --record output/demo.mp4 --ffmpeg /absolute/path/to/ffmpeg
```

Retina rendering is enabled by default. Use `--no-retina` for a physical 1280x720 benchmark on macOS. Both window rendering and recording use four-sample MSAA. Curved meshes have finer tessellation, the concrete texture is 1024x1024 with mipmaps, and bundled Roboto text uses a high-resolution font atlas. Keep both `assets/` and `shaders/` beside the executable; CMake copies them automatically.

Recording captures the actual framebuffer at the requested resolution and 30 FPS, without playback speed alteration. The refreshed demo records the current terrain and scenery in one continuous native 1920x1080 take. Captions and FPS are rendered by the application; see CREDITS.md. To capture native 4K, use `--width 3840 --height 2160`; recording requires more GPU and encoding throughput. For recording, `--no-vsync` avoids limiting throughput to display refresh intervals. On macOS, `--hardware-encode` selects FFmpeg VideoToolbox H.264 at 50 Mbps to reduce CPU encoding load. The default remains portable software H.264. Manual input is locked only in `--demo`. See `./build/baikonur --help` for other options.

## Team

| Member | Group | Proposed review/presentation ownership |
|---|---|---|
| Akbope Bakytkeldy | SE2426 | Scope, requirements, build; slides 1-2 |
| Moldir Omar | SE2426 | Pipeline, geometry, hierarchy; slides 3-4 |
| Ayaulym Talgat | SE2427 | Lighting, textures, particles; slides 5-7 |
| Talant Altaikhan | SE2425 | Interaction, tests, results; slides 8-10 |

These are suggested responsibilities, not completed student contributions. The team must record actual individual work before submission. Defense/submission date is unconfirmed. Instructor: Nazira Shaimerdenova.

## AI Use

OpenAI Codex substantially assisted with C++ code, procedural geometry/textures, GLSL, tests, builds, research/reference checks, report, slides, documentation, measurements and recorded demo. The user selected the topic, supplied the assignment documents and lectures, and provided team identities. The team has not yet confirmed independent contributions or understanding.

AI-assisted validation fixed GLAD definition scope, particle emission ratio at high FPS, help overlap, fixed-size capture, framebuffer sample compatibility, demo input interference and camera framing. Builds, simulation tests and live rendering checks followed. See the report's detailed AI Use Note. Members must review and explain the source, verify their course setup, perform genuine substantive work, and update this disclosure. No student history or participation has been fabricated.

## Submission

The package includes source, template-based report DOCX/PDF, ten-slide PPTX with notes, and a 3:20 captioned 1080p demo. Complete the external/personal items in the checklist: topic approval, actual member contributions and commits, TA access to [the repository](https://github.com/aqinaq/baiqonyr), `v1.0` tag on the actual submission revision, dates, TA-machine checks and Moodle upload. Deadline: three days before defense at 23:59 Asia/Almaty. This package does not establish those actions or guarantee a grade.

The current refresh passes both `mission_invariants` and `kazakhstan_terrain`. The source ZIP includes the current terrain/scenery files and is checked byte-for-byte against the workspace. See `docs/evidence/validation.json` for the latest verification record.
