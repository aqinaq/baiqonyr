# Lecture-to-code map

Page references are **PDF page numbers**, which sometimes differ from the printed slide numbers. The final project brief requires modern OpenGL 3.3 core, even where historical lecture examples show GLUT or fixed-function calls. GLFW/GLAD, GLM and Dear ImGui are explicitly permitted by that brief and are bundled from source. Confirm that this CMake setup matches the practical assignments before submission.

| Supplied lecture | PDF pages | Concept | Implementation |
|---|---|---|---|
| L-1-2 computer graphics fundamentals | 1–56 | Pipeline, buffers, shaders and window/event responsibilities | `main.cpp`, `Mesh.cpp`, `Shader.cpp` |
| L-3-1 Input | 1–51 | Event-driven input, mouse, keyboard, resize and double buffering | GLFW polling/callbacks and ImGui capture checks in `main.cpp` |
| L-3-2 Geometry | 1–45 | Points, vectors, coordinate frames | GLM vectors and matrix interfaces |
| L-4-1 Transformations | 9–10, 22–25 | Homogeneous translation and concatenation | `Scene.cpp` TRS, `Simulation.hpp` world transforms |
| L-4-2 Models | 1–49 | Modeling objects from geometry and transforms | Shared cube/cylinder/cone/sphere meshes |
| L-5-1 Viewing | 11–12, 16–22, 26 | LookAt, perspective, division and pipeline | `Camera.hpp`, `scene.vert` |
| L-5-2 Shading | 29, 58–59, 66–70 | Modified Phong/Blinn model and per-fragment shading | `scene.frag`, inverse-transpose normals in `scene.vert` |
| L-6 CurvesSurfaces | 1–106 | Parametric curves and surfaces | Parametric cylinder and sphere tessellation in `Mesh.cpp`; no Bezier surface claim |
| L-8 TextureMapping | 30–34, 38–40, 43 | UVs, interpolation, filtering and mipmaps | `Texture.cpp`, `Mesh.cpp`, `scene.frag` |
| L-8-1-2D TextureMapping | 1–62 | Texture sampling and illumination | Repeating concrete texture and per-fragment material color |
| L-9 HierchModels (1) | 8–11, 29–32, 45 | Parent-child matrices, traversal and dynamic trees | Core-body children and boosters; detachment switches to world coordinates |
| L-9 SolidModel 2 | 1–49 | Solid representation and primitives | Primitive construction; CSG is not claimed |
| L-10 ProcMeths | 5, 14–22, 32, 51 | Particle birth, attributes, lifetime, extinction and integration | `Particles.cpp`, bounded pool, age-based color and alpha |

The three L-10 files cover the same 103-page lecture; they are treated as one source, not three independent references. Introductory/history slides support context but do not count toward the required fifteen journal sources.

## Oral explanation sequence

1. Follow one vertex from a cylinder parameter to VBO/EBO, model matrix, view matrix, projection, interpolation, fragment lighting and framebuffer.
2. Show how a booster gets its parent transform before separation and a world-space trajectory afterward.
3. Explain the half-vector, diffuse clamping and inverse-transpose normal matrix.
4. Explain why smoke uses sorted alpha blending and fire uses additive blending, with depth writes disabled only for particles.
5. Explain mipmaps and the distinction between MSAA on edges and texture minification filtering.
6. Distinguish the visual trajectory and altitude-based sky interpolation from aerospace physics and physical atmospheric scattering.
