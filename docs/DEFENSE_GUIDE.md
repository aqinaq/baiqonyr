# Defense preparation

The deck has 10 slides and a suggested 8 minute 40 second talk. The demo is 3 minutes 20 seconds. Assignments below are proposed speaking and review ownership, not claims of work already completed.

| Member | Slides | Target | Review responsibility |
|---|---|---|---|
| Akbope Bakytkeldy, SE2426 | 1–2 | 1:30 | Scope, requirements, source/build workflow |
| Moldir Omar, SE2426 | 3–4 | 1:50 | Pipeline, geometry, hierarchy and separation |
| Ayaulym Talgat, SE2427 | 5–7 | 2:50 | Lighting, textures, fire/smoke and atmosphere |
| Talant Altaikhan, SE2425 | 8–10 | 2:30 | Interaction, tests, measured performance, limitations |

Every member must also understand the other modules. Read the report AI Use Note aloud if asked about authorship. Describe only contributions you actually made. Keep the source, Release executable, shaders and local demo available offline.

## Questions and concise explanations

- **Why VAO, VBO and EBO?** A VBO stores vertices, an EBO stores indices, and a VAO remembers how the vertex attributes and EBO are connected. Indexed drawing reuses vertices.
- **Which profile?** A 3.3 core context is requested. macOS provided OpenGL 4.1 core; the code and shaders use the 3.3 feature set.
- **Matrix order?** `clip = projection * view * parent * local * position` for column vectors. The rightmost operation happens first.
- **Why inverse transpose?** Nonuniform scaling changes normals differently from positions. The inverse transpose keeps the transformed normal perpendicular to the transformed tangent plane. Normalize again in the fragment shader.
- **What is the hierarchy?** The core translation parents the body, nose and attached boosters. Each booster has a local radial translation and its own part transforms. Detachment preserves its position and initial vertical velocity, then uses a separate ballistic arc.
- **Is this realistic rocket physics?** No. The trajectory is `h = 0.55 * max(t,0)^2` and `v = 1.1 * max(t,0)`, selected for a short demonstration. There is no mass loss, drag, thrust integration, orbital mechanics or certified geometry.
- **Why Blinn-Phong?** It exposes ambient, diffuse and specular terms clearly and fits the course. The half-vector is the normalized sum of light and viewing directions. Shininess controls highlight width.
- **Which lights?** Directional sun, toggled blue gantry point light, and an orange engine point light attached to the ascending rocket.
- **Why separate fire/smoke blend modes?** Fire adds light to the destination. Smoke combines source and destination using opacity and needs back-to-front ordering. Both test depth against the opaque scene without writing new depth.
- **How are particles efficient?** A bounded CPU pool holds lifetimes and velocities. One shared quad and an instance buffer render smoke and fire in two calls. Only dynamic instance data uploads each frame.
- **What fixed the high-FPS emission bug?** A global spawn ordinal assigns the 2:1 fire/smoke ratio. Restarting that ordinal each frame biased the result whenever few particles spawned per frame.
- **Why mipmaps?** They prefilter a texture for minification and reduce shimmering. The sampler uses `GL_LINEAR_MIPMAP_LINEAR` and repeating UVs.
- **Do you implement the literature algorithms?** The report compares them. The application uses simpler course-level methods; it does not implement Navier-Stokes, wavelet turbulence, Cook-Torrance PBR or physical atmospheric scattering.
- **What do the FPS values mean?** They are measured frame-loop throughput, not an isolated GPU timer. VSync, capture and hardware affect them. The TA must repeat the exact 1280x720 test.
- **What is missing?** Physical flight/combustion, shadows, Earth curvature, high-order smoke simulation and validated user studies. These are future work rather than claimed features.

## Live demonstration fallback

Build using README, run, orbit with drag or WASD, toggle L and N, toggle F, press Space, enable C, wait for T+24 separation, pause with P, orbit around the frozen formation, then R. If the video fails, this covers the same core features live.

## Team actions before submission

Confirm names/capitalization, actual contributions, course build setup and defense date. Register the topic if not already approved. Create the hosted repository, add the TA, make genuine member commits while reviewing and improving the project, and tag the actual submission revision `v1.0`. Do not fabricate backdated history. Upload the four deliverables three days before defense, by 23:59 Asia/Almaty. These account-based and personal actions are not performed by this package.
