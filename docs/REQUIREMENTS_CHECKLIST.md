# Requirements and submission evidence

The supplied final brief is the assessment specification; the lectures explain course concepts. Historical fixed-function lecture examples do not replace the brief's OpenGL 3.3 core requirement. Status below describes local evidence, not an assigned grade.

| ID | Implementation / evidence | Status |
|---|---|---|
| R1 | Core context in main; VAO/VBO/EBO in Mesh; own external GLSL; wireframe at 0:45 | Built and rendered |
| R2 | Pad, rocket, gantry, buildings, tanks, radome, roads, lights; opening orbit | More than five distinct types |
| R3 | MVP, rocket hierarchy and independent detachment; Simulation.hpp and Kazakhstan terrain tests | Implemented; continuity tested |
| R4 | Per-fragment Blinn-Phong; sun, toggled gantry and moving engine lights; 0:25 and 1:11 | Implemented and rendered |
| R5 | Concrete UVs, repeat, trilinear mipmaps; procedural steppe; 0:55 | Implemented and rendered |
| R6 | Mouse/keyboard camera; launch, pause, reset, separate, chase, light/night/wireframe, speed | Implemented; manual input in normal mode |
| R7 | Continuous FPS overlay; three 720p CSVs/logs in evidence; report FPS table | Local averages exceed 60; TA-machine test pending |
| R8 | C++17 CMake; bundled GLFW/GLAD/GLM/ImGui; exact README steps | macOS verified; exact course setup needs confirmation |

## Topic and rubric coverage

Baikonur core: pad and hierarchical rocket, fire/smoke, launch key, altitude-darkening sky, chase camera. Stretch: independent booster detachment, ignition shake, altitude stars. All are implemented and shown in the demo.

Visual/code evidence: coherent cream/orange/teal palette, correct normal transforms, static geometry uploaded once, two instanced particle draws, depth testing and blend ordering. Original application modules are below 500 lines with separate GLSL. No physical flight/combustion, Earth curvature or shadows are claimed. Report Section 7 explains limits. Occasional frame-time outliers are disclosed; mean FPS does not prove all-frame stability.

Report: supplied template, 20 pages including a Kazakhstan terrain appendix, all sections, equations, screenshots, hardware/FPS table, comparative discussion of 15 journal articles, numbered references, specific AI note and appendix. Editable DOCX accompanies the PDF. Date remains unconfirmed.

Journal qualification: 12 ACM Transactions on Graphics and 3 Computer Graphics Forum articles, with DOI metadata in `references.json`. Both venues are **Q1 in the 2024 SCImago Computer Graphics and Computer-Aided Design category**. Evidence: [Matej Bel University Library SCImago table](https://kniznica.umb.sk/app/cmsFile.php?ID=21899&disposition=i), Scopus data as of March 2025, PDF pages 1 (TOG) and 3 (CGF). This states the category/year convention, not a per-paper quartile. Reconfirm only if the instructor specifies a different database/year.

Presentation: 10 editable slides, native diagrams/tables, actual screenshots and speaker notes; suggested 8:40 talk with a part for each member. Actual delivery, individual contributions and Q&A remain personal responsibilities.

Demo: 200 seconds, 1920x1080, 30 FPS, feature captions, actual renderer and continuous FPS overlay. Zero OpenGL errors in final recording; no speed alteration.

| Time | Evidence |
|---|---|
| 0:00-0:25 | Scene, orbit, pipeline labels and FPS |
| 0:25-0:45 | Night / toggled gantry lighting |
| 0:45-0:55 | Indexed wireframe geometry |
| 0:55-1:05 | Concrete texture and procedural ground |
| 1:05-1:40 | Countdown, ignition/shake, particles, chase, detachment, stars |
| 1:40-2:10 | Pause with camera inspection |
| 2:10-2:44 | Reset and scene inspection |
| 2:44-3:20 | Night launch and final booster formation |

## Current source refresh

The report, slides, captures and demo are refreshed for the Kazakhstan terrain and detailed scenery. Both mission and terrain CTest targets pass locally. The source package is checked byte-for-byte, then built and tested after extraction. A GitHub Actions workflow is configured for Linux, macOS and Windows. All three hosted build/test jobs passed on 22 October 2026 ([run evidence](https://github.com/aqinaq/baiqonyr/actions/runs/39724116302)). Local and hosted results are recorded in `evidence/validation.json`. The report captures the earlier local-validation stage; this checklist records the subsequent hosted results. This does not substitute for a desktop run on the TA machine.

## Team actions before submission

- [ ] Confirm topic registration/approval and member name order. The brief limits a topic to two teams.
- [ ] Review/understand the source. Each member performs and records genuine substantive work; replace proposed ownership with actual contributions.
- [ ] Confirm the course build setup and test 1280x720 on the TA machine. Check mouse/keyboard, toggles, pause/reset and a clean build.
- [ ] Confirm defense date and replace the report's unconfirmed date. Submit three days before defense, by 23:59 Asia/Almaty.
- [ ] Confirm the latest source at https://github.com/aqinaq/baiqonyr, add the TA and retain genuine member commits. Do not fabricate history.
- [ ] Tag the real submission revision `v1.0` on the submission date.
- [ ] Upload repository link/tag, report PDF, slide deck and demo to Moodle.
- [ ] Rehearse the 7-10 minute talk and 3-5 minute Q&A. Every member must explain any module; keep an offline executable/shaders/assets and video backup.
- [ ] Confirm course progress checks already attended. Preparing files cannot retroactively satisfy those milestones.

No instructor messages, account/sharing changes or assignment submissions were performed by this preparation.
