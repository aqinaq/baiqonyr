# Credits and licenses

All geometry, the concrete texture, procedural steppe/sky/stars, GLSL scene and particle shaders, mission logic, and camera implementation were authored for this project with OpenAI Codex assistance. No third-party model, photo texture, shader snippet, audio, or tutorial code was copied into application source.

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

The report uses the supplied course template. The presentation uses actual project captures. The MP4 is a real-time framebuffer capture of the application, with the same image displayed in its window, encoded by FFmpeg 7.1 via the imageio-ffmpeg distribution. FFmpeg and temporary authoring tools are not runtime dependencies and are not included in the source archive. Captions are drawn by the application; the video has no music or voice-over. The quality revision captures natively at 3840x2160 with four-sample MSAA. The final demo joins actual capture segments from two takes, with cuts at 45 and 55 seconds to include the optimized wireframe mode. The final paused image is held briefly to complete 200 seconds. No footage is sped up or motion-interpolated. A Lanczos downsampling and H.264 compression pass produces the 1920x1080 submission video while retaining the 200-second duration and 30 FPS timeline.

## AI Use

OpenAI Codex generated the initial implementation, procedural assets, test code, documentation, report prose, and slides, and assisted with compilation, debugging, source verification, and capture. This is substantial AI assistance. The team has supplied its names and topic; personal code changes, independent review, and individual contributions have not yet been confirmed. Each member must review the code, make and document actual changes where needed, and be able to explain the complete submission. Do not represent this initial generated implementation as wholly student-authored.
