# Credits and licenses

The mesh construction, concrete texture, procedural steppe/sky/stars, GLSL scene and particle shaders, mission logic, and camera implementation were authored for this project with OpenAI Codex assistance. Kazakhstan's terrain outline is derived from the public-domain map data credited below. No third-party 3D model, photo texture, shader snippet, audio, or tutorial code was copied into application source.

## Kazakhstan terrain boundary

`assets/maps/kazakhstan.geojson` contains the Kazakhstan polygon from [Natural Earth v5.1.2, 1:110m Admin 0 Countries](https://github.com/nvkelso/natural-earth-vector/blob/v5.1.2/geojson/ne_110m_admin_0_countries.geojson), [public-domain map data](https://www.naturalearthdata.com/about/terms-of-use/). `tools/generate_kazakhstan.py` projects the coordinates at 48° N, centres the launch area at approximately 63.34° E, 45.92° N, scales the country to 10,000 scene units wide, and ear-clips the concave outline into `src/KazakhstanBoundary.hpp`. This is a simplified display map, with a stylized 60-unit-deep base, not a terrain elevation dataset.

| Dependency | Author / project | Version / source | License retained |
|---|---|---|---|
| GLFW | Camilla Löwy and GLFW contributors | 3.4, https://github.com/glfw/glfw/tree/3.4 | zlib/libpng, `vendor/glfw/LICENSE.md` |
| GLAD | David Herberth and contributors | Generated loader shipped in GLFW 3.4, https://github.com/glfw/glfw/blob/3.4/deps/glad/gl.h | MIT / Khronos registry notices embedded in header; GLFW `deps/glad` notices retained |
| GLM | G-Truc Creation and contributors | 1.0.1, https://github.com/g-truc/glm/tree/1.0.1 | MIT or Happy Bunny, `vendor/glm/copying.txt` |
| Dear ImGui | Omar Cornut and contributors | 1.91.8, https://github.com/ocornut/imgui/tree/v1.91.8 | MIT, `vendor/imgui/LICENSE.txt`; embedded ProggyClean font notice retained |
| Roboto Medium | Christian Robertson / Google | TrueType font bundled with Dear ImGui 1.91.8 | Apache 2.0, `assets/fonts/LICENSE-Roboto.txt` |

The GLAD file exposes a compatibility superset but the application explicitly requests a **core-profile context** and only calls modern buffer/shader operations. No legacy GL rendering calls are made by project source. ImGui supplies UI rendering only; it does not generate the 3D lighting shaders.

## Conceptual references

The project uses standard matrix transformations, Blinn-Phong lighting and particle lifecycle concepts. Its implementations are original project code, not copies of the papers. Fifteen journal references and their DOI links are listed in `docs/references.json` and the report. Research techniques that were reviewed but not implemented are labeled as comparisons or future work.

The supplied course lectures include material credited on their slides to E. Angel and D. Shreiner, *Interactive Computer Graphics*, 6th edition, Addison-Wesley, 2012. `docs/COURSE_MAP.md` maps the project to the supplied lecture PDF pages. Lecture files are not redistributed in the code package. The supplied report template's Astana IT University logo is retained only in the report.

## Deliverable tooling

The report uses the supplied course template. The presentation uses actual project captures. The MP4 is a real-time framebuffer capture of the application, with the same image displayed in its window, encoded by FFmpeg 7.1 via the imageio-ffmpeg distribution. FFmpeg and temporary authoring tools are not runtime dependencies and are not included in the source archive. Captions are drawn by the application; the video has no music or voice-over. The refreshed submission records the current application in one continuous native 1920x1080 take with four-sample MSAA and a visible FPS counter. H.264 compression retains the wall-clock timeline at 30 FPS. No footage is sped up or motion-interpolated. Any sub-frame ending adjustment holds the already-paused final image to reach exactly 200 seconds. Capture metadata is recorded in `docs/evidence/capture-edit.json`.

## AI Use

OpenAI Codex generated the initial implementation, procedural assets, test code, documentation, report prose, and slides, and assisted with compilation, debugging, source verification, and capture. This is substantial AI assistance. The team has supplied its names and topic; personal code changes, independent review, and individual contributions have not yet been confirmed. Each member must review the code, make and document actual changes where needed, and be able to explain the complete submission. Do not represent this initial generated implementation as wholly student-authored.
