# Procedural Visual Sources — Comprehensive Research

> **Scope**: Exhaustive catalog of procedural/algorithmic visual generators for implementation in C++/OpenGL/GLSL. These are "Sources" — they GENERATE visual content from scratch (unlike Effects which modify existing content). Organized by category with GLSL feasibility, parameter lists, complexity ratings, and prioritization.

> **Related**: [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md) | [VIDEO_vj_frameworks.md](VIDEO_vj_frameworks.md)

---

## Table of Contents

1. [Industry Survey: What VJ Apps Offer](#1-industry-survey)
2. [Mathematical / Fractal Sources](#2-mathematical--fractal-sources)
3. [Geometric Pattern Sources](#3-geometric-pattern-sources)
4. [Particle Systems](#4-particle-systems)
5. [3D Primitive Sources (Ray Marching)](#5-3d-primitive-sources)
6. [Noise / Organic Sources](#6-noise--organic-sources)
7. [Concert Lighting Simulations](#7-concert-lighting-simulations)
8. [Text / Typography Sources](#8-text--typography-sources)
9. [Audio-Visual Sources](#9-audio-visual-sources)
10. [ArKaos Generator Effects](#10-arkaos-generator-effects)
11. [Priority Matrix](#11-priority-matrix)
12. [Implementation Architecture](#12-implementation-architecture)

---

## 1. Industry Survey

### What VJ Apps Offer as Generators

#### Resolume Wire
Resolume Wire is a node-based patching environment within Resolume Arena. Its generator capabilities include:
- **2D Shape System**: Circles, rectangles, morphing blobs, psychedelic patterns — all as procedural sources
- **Text Rendering**: Real-time animated text with fonts and colors
- **ISF Shader Generators**: Hundreds of community-created generators in ISF format
- **Beat-responsive Sources**: Generators that lock to BPM transport

#### TouchDesigner Generator TOPs
TouchDesigner's texture operators that generate from scratch:
- **Noise TOP**: Perlin, Simplex, Sparse, Alligator noise with animation
- **Ramp TOP**: Linear, radial, circular gradients
- **Circle TOP / Rectangle TOP**: Geometric primitives
- **Constant TOP**: Solid color
- **Text TOP**: Font-rendered text
- **Render TOP**: 3D scene rendering
- **Function TOP**: Mathematical function visualization
- **Pattern TOP**: Grid, stripe, checkerboard patterns
- **CHOP-to-TOP**: Audio data converted to visual texture (key for audio-reactive generation)

#### ISF (Interactive Shader Format) Generators
From the Vidvox ISF-Files repository (~200+ shaders), the following are confirmed GENERATORS (create from scratch, no input required):

| ISF Generator | Category | Description |
|---------------|----------|-------------|
| Audio Waveform Shape | Audio-Visual | Waveform display from audio input |
| Basic Shape | Geometric | Configurable geometric shapes |
| Bounce | Animation | Bouncing element |
| Brick Pattern | Pattern | Brick/tile pattern |
| Checkerboard | Pattern | Animated checkerboard |
| Circle | Geometric | Circle generator |
| City Lights | Noise/Organic | Procedural city lights |
| Color Bars | Test | Color bar test pattern |
| Color Organ Polyphonic | Audio-Visual | Audio-reactive color organ |
| Color Scales | Pattern | Color scale display |
| Color Schemes | Pattern | Color scheme generator |
| Color Test Grid | Test | Test grid pattern |
| Crazy Parametric Fun | Mathematical | Parametric curves |
| Digital Clock | Text | Time display |
| Doodler | Drawing | Interactive drawing |
| FFT Color Lines | Audio-Visual | FFT-driven color lines |
| FFT Filled Waveform | Audio-Visual | FFT-driven waveform |
| FFT Spectrogram | Audio-Visual | Rolling spectrogram |
| Graph Paper | Pattern | Grid pattern |
| Heart | Geometric | Heart shape |
| Life | Cellular Automata | Conway's Game of Life |
| Line Group | Geometric | Line patterns |
| Lines | Geometric | Animated lines |
| Linear Gradient | Gradient | Linear gradient |
| Multi Gradient | Gradient | Multi-stop gradient |
| Noise | Noise | Procedural noise |
| Polar Function | Mathematical | Polar coordinate function |
| Poly Star | Geometric | Polygon/star shapes |
| Radial Gradient | Gradient | Radial gradient |
| Radial Spectrogram | Audio-Visual | Circular spectrogram |
| Random Characters | Text/Glitch | Matrix-style characters |
| Random Checkerboard | Pattern | Randomized checkerboard |
| Random Lines | Pattern | Random line patterns |
| Random Shape Blast | Geometric | Exploding shapes |
| Random Shape | Geometric | Random shapes |
| Random Squares | Pattern | Random square grid |
| Random Stripes | Pattern | Random stripe patterns |
| Solid Color | Solid | Solid color fill |
| Spiral | Geometric | Spiral pattern |
| Star | Geometric | Star shape |
| Stripes | Pattern | Stripe pattern |
| Test Pattern Generator | Test | Broadcast test pattern |
| Triangle | Geometric | Triangle shape |
| Triangles | Pattern | Triangle pattern grid |
| Truchet Tile | Pattern | Truchet tiling pattern |
| TV Static | Noise/Glitch | Static noise |
| VU Meter | Audio-Visual | VU meter display |

#### VDMX / Magic Music Visuals
These applications primarily consume ISF shaders and Quartz Composer compositions as generators. Their built-in generation is limited to ISF-compatible sources plus basic solid colors and gradients.

#### Max/MSP + Jitter
Jitter provides `jit.gl.gridshape` (3D primitives), `jit.gl.mesh` (custom geometry), `jit.gl.shader` (GLSL), and `jit.gl.pix` (Gen-based pixel shaders). The Gen environment allows visual programming of fragment shaders — effectively a visual GLSL editor.

---

## 2. Mathematical / Fractal Sources

### 2.1 Mandelbrot Set

**What it is**: The set of complex numbers c for which z(n+1) = z(n)^2 + c does not diverge. Produces infinitely detailed fractal boundaries.

**GLSL Implementation**: Single fragment shader. Each pixel maps to a complex number, iterate the formula, color based on escape iteration count.

```glsl
// Core algorithm (simplified)
vec2 z = vec2(0.0);
vec2 c = uv * zoom + center;
int iter = 0;
for (int i = 0; i < maxIter; i++) {
    z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
    if (dot(z, z) > 4.0) break;
    iter++;
}
float t = float(iter) / float(maxIter);
// Color using cosine palette or smooth iteration count
```

**Parameters** (all routable to audio):
| Parameter | Range | Audio Mapping Idea |
|-----------|-------|--------------------|
| `zoom` | [0.5, 1e12] | Bass energy → zoom level (deep zoom on drops) |
| `centerX/Y` | [-2, 2] | Spectral centroid → X, pitch → Y |
| `maxIterations` | [32, 1024] | RMS → detail level |
| `colorSpeed` | [0, 10] | Beat phase → color cycling |
| `colorOffset` | [0, 1] | Hue shift from chroma |
| `power` | [2, 8] | For Multibrot generalization |

**Complexity**: Simple (single fragment shader, no state)
**Performance**: High iteration counts can be GPU-intensive. 256 iterations at 1080p runs fine on modern GPUs. Deep zooms (>1e12) need double precision (mediump won't work).

### 2.2 Julia Set

**What it is**: Related to Mandelbrot — same formula but c is fixed and z(0) varies per pixel. Different c values produce wildly different fractals.

**GLSL Implementation**: Single fragment shader. Nearly identical to Mandelbrot but parameters swap roles.

**Parameters**:
| Parameter | Range | Audio Mapping Idea |
|-----------|-------|--------------------|
| `cReal` | [-2, 2] | Spectral centroid → real part |
| `cImag` | [-2, 2] | Spectral flux → imaginary part |
| `zoom` | [0.5, 100] | RMS → zoom |
| `rotation` | [0, 2pi] | Beat phase → rotation |
| `maxIterations` | [32, 512] | Dynamic range → detail |
| `colorScheme` | palette index | Key detection → color palette |

**Complexity**: Simple
**Performance**: Same as Mandelbrot. Animating c creates fluid morphing — excellent for audio-reactive use.

### 2.3 Lorenz Attractor

**What it is**: A system of 3 ODEs that produces a butterfly-shaped chaotic trajectory. Classic chaos theory visualization.

```
dx/dt = sigma * (y - x)
dy/dt = x * (rho - z) - y
dz/dt = x * y - beta * z
```

**GLSL Implementation**: Requires compute shader or CPU-side integration of the ODE system. The trajectory must be accumulated over time (stateful). A fragment shader alone cannot simulate this — it needs a trail buffer.

**Alternative GLSL approach**: Render as a density field. For each pixel, iterate the attractor equations and count how many trajectory points land near that pixel. This works as a fragment shader but is computationally expensive (each pixel must simulate many steps).

**Better approach**: CPU computes trajectory into a vertex buffer, GPU renders as GL_LINE_STRIP with additive blending and glow. Or use a compute shader to update particle positions.

**Parameters**:
| Parameter | Range | Audio Mapping Idea |
|-----------|-------|--------------------|
| `sigma` | [1, 30] | Spectral centroid |
| `rho` | [1, 50] | RMS (chaos threshold at ~24.7) |
| `beta` | [0.5, 8] | Spectral flatness |
| `rotation` | [0, 2pi] | Beat phase → rotate view |
| `trailLength` | [100, 10000] | Transient density → trail |
| `lineWidth` | [0.5, 5] | Peak amplitude |
| `colorMode` | enum | Key detection → coloring |

**Complexity**: Medium (needs CPU or compute shader for state)
**Performance**: Fine if trajectory length is bounded (~10k points)

### 2.4 Strange Attractors (General)

Beyond Lorenz, other visually striking attractors implementable the same way:

| Attractor | Equations | Visual Character |
|-----------|-----------|-----------------|
| **Lorenz** | 3 ODEs (sigma, rho, beta) | Butterfly wings |
| **Rössler** | 3 ODEs (a, b, c) | Folded band, simpler than Lorenz |
| **Clifford** | x' = sin(a*y) + c*cos(a*x), y' = sin(b*x) + d*cos(b*y) | 2D, dense swirling patterns |
| **De Jong** | x' = sin(a*y) - cos(b*x), y' = sin(c*x) - cos(d*y) | 2D, similar to Clifford |
| **Aizawa** | 3 ODEs | Torus-like structure |
| **Thomas** | 3 ODEs (b parameter) | Organic flowing loops |
| **Halvorsen** | 3 ODEs (a parameter) | Triangular symmetric form |

**Clifford and De Jong are ideal for fragment shaders** — they're 2D, stateless per-pixel if rendered as density maps, and 4 parameters each map perfectly to audio features.

**Complexity**: Medium (same as Lorenz for 3D; Clifford/De Jong can be fragment-only as density)

### 2.5 Flame Fractals

**What it is**: Iterated function systems (IFS) with nonlinear "variation" functions. Popularized by Apophysis/Chaotica. Produces organic, ethereal fractal flames.

**GLSL Implementation**: Extremely difficult as a fragment shader. The algorithm is inherently stochastic (random iteration of affine transforms + nonlinear variations). Traditional implementation uses millions of particles accumulated into a density histogram.

**Practical approach**: CPU-side chaos game with GPU-accelerated accumulation (render points to a texture with additive blending). Or use a compute shader for the iteration.

**Parameters**: Affine transform coefficients, variation weights (sinusoidal, spherical, swirl, horseshoe, etc.), color palette

**Complexity**: Complex (needs compute or CPU + GPU accumulation)
**Performance**: Real-time at lower quality. High-quality renders need millions of samples.

### 2.6 Reaction-Diffusion (Gray-Scott Model)

**What it is**: Two chemical species (U and V) diffuse and react. Different feed/kill rates produce spots, stripes, waves, mitosis, coral, fingerprints. One of the most visually rich generative systems.

**GLSL Implementation**: Requires ping-pong FBOs (read from texture A, write to texture B, swap). Each frame, a fragment shader reads the current U/V concentrations from neighbors, applies the reaction-diffusion equations, and writes the new state. This is a feedback system — it needs previous frame state.

```glsl
// Core Gray-Scott equations per pixel per timestep:
float laplacianU = /* sum of 4 neighbors minus 4*center */ ;
float laplacianV = /* same for V */ ;
float uvv = u * v * v;
float du = Du * laplacianU - uvv + F * (1.0 - u);
float dv = Dv * laplacianV + uvv - (F + k) * v;
u += du * dt;
v += dv * dt;
```

**Parameters**:
| Parameter | Range | Audio Mapping Idea |
|-----------|-------|--------------------|
| `feedRate` (F) | [0.01, 0.1] | Spectral centroid → pattern type |
| `killRate` (k) | [0.04, 0.07] | Spectral flatness → pattern type |
| `diffusionU` (Du) | [0.1, 0.3] | Low-frequency energy |
| `diffusionV` (Dv) | [0.01, 0.1] | High-frequency energy |
| `dt` (timestep) | [0.5, 2.0] | BPM → simulation speed |
| `colorScheme` | palette | Key detection → coloring |
| `seedMode` | enum | Onset → inject new seeds |

**Complexity**: Medium (needs ping-pong FBOs, but the shader itself is simple)
**Performance**: Excellent on GPU. Run multiple timesteps per frame for faster evolution.

### 2.7 Cellular Automata

**What it is**: Grid of cells with simple rules producing complex emergent behavior.

| Type | Dimensions | Visual Character |
|------|-----------|-----------------|
| **Game of Life** | 2D, binary | Classic: gliders, oscillators, spaceships |
| **Rule 110 / Elementary CA** | 1D → 2D (as history) | Triangular fractal patterns |
| **Brian's Brain** | 2D, 3-state | Sparkling, chaotic patterns |
| **Wireworld** | 2D, 4-state | Circuit-like patterns |
| **Langton's Ant** | 2D, directional | Emergent highway patterns |
| **Multiple Neighborhoods** | 2D, continuous | Lenia — smooth, lifelike creatures |

**GLSL Implementation**: Ping-pong FBOs, same architecture as reaction-diffusion. Fragment shader reads neighbor states, applies rules, writes new state.

**Parameters**:
| Parameter | Range | Audio Mapping Idea |
|-----------|-------|--------------------|
| `birthRules` | bitmask | Key detection → rule set |
| `surviveRules` | bitmask | Chroma → rule variation |
| `speed` | [1, 60] steps/frame | BPM → simulation speed |
| `colorMode` | enum | Structural state → coloring |
| `seedDensity` | [0, 1] | Onset → spawn new cells |
| `zoom` | [1, 20] | RMS → zoom level |

**Complexity**: Medium (ping-pong FBOs)
**Performance**: Excellent on GPU.

### 2.8 L-Systems

**What it is**: Formal grammar producing fractal trees, plants, space-filling curves. Rules like "F → F[+F]F[-F]F" with turtle graphics interpretation.

**GLSL Implementation**: Not practical as a pure fragment shader. L-systems are inherently sequential (string rewriting + turtle traversal). Must be computed on CPU, then rendered as line geometry on GPU.

**Alternative**: Pre-compute several L-system generations and store as textures or vertex buffers. Animate by interpolating between generations.

**Parameters**: Axiom, rules, angle, iterations, segment length
**Complexity**: Medium-Complex (CPU geometry generation)
**Performance**: Fine once geometry is computed. Real-time rule changes require regeneration.

---

## 3. Geometric Pattern Sources

### 3.1 Lissajous / Oscilloscope Patterns

**What it is**: x = A*sin(a*t + delta), y = B*sin(b*t). When a/b is rational, produces closed curves. Classic oscilloscope display.

**GLSL Implementation**: Single fragment shader. For each pixel, compute signed distance to the parametric curve. Use SDF rendering for anti-aliased lines.

```glsl
// Compute closest distance from pixel to Lissajous curve
// Sample many points along the curve, find minimum distance
float minDist = 1e10;
for (float t = 0.0; t < TWO_PI; t += 0.01) {
    vec2 p = vec2(sin(a * t + phase), sin(b * t));
    minDist = min(minDist, length(uv - p));
}
float line = smoothstep(lineWidth, 0.0, minDist);
```

**Parameters**:
| Parameter | Range | Audio Mapping Idea |
|-----------|-------|--------------------|
| `freqRatioA` | [1, 8] | Detected pitch → frequency ratio |
| `freqRatioB` | [1, 8] | Second harmonic |
| `phase` | [0, 2pi] | Beat phase → phase offset |
| `lineWidth` | [0.001, 0.05] | RMS → line thickness |
| `decay` | [0, 1] | Smoothing → trail persistence |
| `rotation` | [0, 2pi] | Spectral centroid → rotation |

**Complexity**: Simple
**Performance**: Moderate (many distance samples per pixel). Can optimize with analytical SDF.

### 3.2 Spirograph / Hypotrochoid

**What it is**: Parametric curves from rolling circles: x = (R-r)*cos(t) + d*cos((R-r)/r * t)

**GLSL Implementation**: Same SDF approach as Lissajous. Single fragment shader.

**Parameters**: outerRadius (R), innerRadius (r), penDistance (d), lineWidth, rotation, colorCycle
**Complexity**: Simple
**Performance**: Same as Lissajous

### 3.3 Sacred Geometry

| Pattern | GLSL Approach | Complexity |
|---------|--------------|------------|
| **Flower of Life** | SDF circles arranged at 60-degree intervals, 7+ circles | Simple |
| **Metatron's Cube** | SDF lines connecting circle centers, hexagonal grid | Simple |
| **Sri Yantra** | 9 interlocking triangles + circles, precise geometry | Medium |
| **Seed of Life** | 7 overlapping circles in hexagonal arrangement | Simple |
| **Tree of Life** | 10 circles + 22 connecting lines | Simple |
| **Vesica Piscis** | 2 overlapping circles with intersection highlight | Simple |
| **Torus / Tube Torus** | 3D ray marching needed for true torus | Medium |

**All can be done in a single fragment shader** using SDF circle and line primitives. The key technique: draw circle at center, then 6 circles at radius distance at 60-degree intervals, then 12 more at the next ring, etc.

**Parameters**: scale, rotation, lineWidth, glowIntensity, layerVisibility (which rings to show), colorScheme, animationSpeed
**Complexity**: Simple to Medium
**Audio mapping**: Rotation → beat phase, line glow → RMS, layers appearing → onset triggers

### 3.4 Voronoi Patterns

**What it is**: Partition of space into regions closest to each seed point.

**GLSL Implementation**: Single fragment shader. Tile space into grid cells, place random point in each cell, check current pixel against neighboring cells' points.

```glsl
// Standard Voronoi in GLSL
vec2 ip = floor(uv * scale);
vec2 fp = fract(uv * scale);
float minDist = 1.0;
for (int y = -1; y <= 1; y++) {
    for (int x = -1; x <= 1; x++) {
        vec2 neighbor = vec2(x, y);
        vec2 point = hash22(ip + neighbor); // random per cell
        point = 0.5 + 0.5 * sin(time + 6.28 * point); // animate
        float d = length(neighbor + point - fp);
        minDist = min(minDist, d);
    }
}
```

**Parameters**: scale (cell count), edgeWidth, animate (point movement speed), colorMode (cell fill, distance, edge), distortion
**Complexity**: Simple
**Performance**: Excellent. 3x3 neighbor loop is fast.

### 3.5 Moire Patterns

**What it is**: Interference patterns from overlapping regular grids.

**GLSL Implementation**: Single fragment shader. Render two overlapping grids (line, circle, or dot) with slight offset/rotation/scale difference. The interference creates moire.

```glsl
float grid1 = step(0.5, fract(uv.x * freq1));
float grid2 = step(0.5, fract((uv * rot(angle)) .x * freq2));
float moire = grid1 * grid2; // or abs(grid1 - grid2)
```

**Parameters**: freq1, freq2, angle, offset, lineWidth, pattern (lines/circles/dots)
**Complexity**: Simple
**Performance**: Excellent
**Audio mapping**: Frequency ratio → detected pitch, angle → beat phase (creates slow sweeping moire)

### 3.6 Kaleidoscope Generator

**What it is**: Not the effect (which mirrors an input image) — this is a pure generator that creates kaleidoscope patterns from noise or procedural shapes.

**GLSL Implementation**: Single fragment shader. Apply polar coordinate transformation, fold UV space by angle divisions, then render a procedural pattern (noise, shapes) in the folded space.

```glsl
vec2 p = uv - 0.5;
float angle = atan(p.y, p.x);
float radius = length(p);
angle = mod(angle, TWO_PI / segments);
angle = abs(angle - PI / segments); // mirror fold
// Now render any pattern in this folded space
```

**Parameters**: segments (4-32), zoom, rotation, innerPattern (noise/shapes/lines), colorCycle, symmetry
**Complexity**: Simple
**Performance**: Excellent

### 3.7 Grid Patterns

**What it is**: Dots, lines, crosshatch, checkerboard, brick, hexagonal grid.

**GLSL Implementation**: All single fragment shader. Use `fract()` for tiling, `step()`/`smoothstep()` for edges.

| Pattern | Core Technique |
|---------|---------------|
| Dot grid | `length(fract(uv*scale) - 0.5) < radius` |
| Line grid | `fract(uv.x * scale) < width` for vertical |
| Crosshatch | Two overlapping line grids at different angles |
| Checkerboard | `mod(floor(uv.x*s) + floor(uv.y*s), 2.0)` |
| Brick | Offset every other row by 0.5 |
| Hexagonal | Hex grid tiling (well-documented technique) |

**Parameters**: scale, lineWidth, rotation, offset, dotSize, color1, color2
**Complexity**: Simple
**Performance**: Excellent

### 3.8 Concentric Circles / Rings

**What it is**: Expanding rings from center, like a target or radar display.

**GLSL**: `sin(length(uv - center) * frequency - time * speed)`

**Parameters**: frequency, speed, center, lineWidth, decay, colorScheme
**Complexity**: Simple

### 3.9 Radial Burst / Starburst

**What it is**: Lines radiating from center point, like a sunburst.

**GLSL**: `sin(atan(uv.y, uv.x) * numRays + time)`

**Parameters**: numRays, rotation, lineWidth, taper (fade with distance), pulse (radial animation)
**Complexity**: Simple

---

## 4. Particle Systems

### 4.1 Overview: Fragment Shader vs Compute Shader

**Fragment shader particles**: Render each particle as an SDF (circle) by iterating over all particle positions in the shader. Works for <1000 particles. Particle positions can be encoded in a texture and read per frame.

**Compute shader particles**: True GPU particle system. Compute shader updates positions/velocities, fragment shader renders. Handles millions of particles. Requires OpenGL 4.3+ (NOT available on macOS with OpenGL 4.1).

**Audio-DNA constraint**: OpenGL 4.1 on macOS means NO compute shaders. Options:
1. CPU-side particle update + GPU rendering (practical for <100k particles)
2. Fragment shader "fake" particles using noise functions (no true particle state)
3. Transform feedback (OpenGL 3.0+ — available) for GPU particle update without compute

**Recommended**: CPU particle update + instanced rendering for true particles. Fragment shader noise-based "particle fields" for ambient effects.

### 4.2 Simple Particle Emitter

**Implementation**: CPU maintains array of particles (position, velocity, life, size, color). Each frame: update physics, upload positions to VBO, render as point sprites or instanced quads.

**Parameters**:
| Parameter | Range | Audio Mapping Idea |
|-----------|-------|--------------------|
| `emitRate` | [0, 1000] /sec | RMS → emission rate |
| `gravity` | [-2, 2] | Bass energy → gravity |
| `wind` | [-1, 1] xy | Spectral centroid → wind direction |
| `turbulence` | [0, 2] | Spectral flux → turbulence |
| `particleLife` | [0.5, 5] sec | Transient density → lifespan |
| `particleSize` | [1, 50] px | Peak → size |
| `spread` | [0, 360] deg | Band energy → spread angle |
| `initialVelocity` | [0, 5] | Onset strength → burst velocity |
| `colorScheme` | palette | Key → color palette |

**Complexity**: Medium (CPU update + GPU render)
**Performance**: Good for <50k particles at 60fps

### 4.3 Fragment Shader "Fake" Particles

**What it is**: Use layered noise with different scales and speeds to simulate particle-like motion. No actual particle state — purely procedural per pixel.

```glsl
float particles = 0.0;
for (int i = 0; i < NUM_LAYERS; i++) {
    vec2 p = uv * scale + vec2(time * speed, 0.0);
    p += hash2(float(i)) * 100.0; // per-layer offset
    vec2 cell = floor(p);
    vec2 local = fract(p) - 0.5;
    vec2 point = hash2(cell + float(i)) - 0.5;
    float dist = length(local - point * 0.8);
    float size = hash1(cell) * maxSize;
    particles += smoothstep(size, 0.0, dist) * brightness;
}
```

**This approach works for**: Snow, rain, sparkles, stars, dust, fireflies
**Does NOT work for**: Physics-based particles (gravity, collision, trails)

**Complexity**: Simple (single fragment shader)
**Performance**: Excellent

### 4.4 Specific Particle Types

| Type | Fragment Shader? | Key Technique |
|------|-----------------|---------------|
| **Fireworks** | Hybrid — CPU burst logic + GPU render | Burst on onset, gravity falloff, trail fade |
| **Snow** | Yes (layered noise) | Multiple layers at different speeds for parallax |
| **Rain** | Yes (stretched noise) | Elongated along Y, fast movement |
| **Sparkles** | Yes (noise + threshold) | Random bright points that flash and fade |
| **Smoke/Fog** | Yes (FBM noise) | Layered noise with upward drift |
| **Confetti** | CPU particles | Rectangular sprites with rotation |
| **Bubbles** | Yes (layered circles) | Rising circles with wobble |

---

## 5. 3D Primitive Sources (Ray Marching)

### 5.1 Ray Marching in Fragment Shader

**What it is**: A technique to render 3D scenes entirely in a fragment shader. Cast a ray from the camera through each pixel, march along the ray using signed distance functions (SDFs) to find the surface. No vertex geometry needed — just a fullscreen quad.

**This is the single most powerful technique for 3D generators in a fragment shader.**

Available 3D SDF primitives (from Inigo Quilez):

| Category | Primitives |
|----------|-----------|
| **Basic** | Sphere, Box, Round Box, Torus, Cylinder, Cone, Plane, Capsule |
| **Extended** | Box Frame, Capped Torus, Link, Hexagonal Prism, Rounded Cylinder, Capped Cone, Pyramid, Octahedron |
| **Exotic** | Death Star, Solid Angle, Cut Sphere, Rhombus, Vesica Segment |
| **Operations** | Union, Intersection, Subtraction, Smooth Union/Intersect/Subtract, Elongation, Rounding, Onion (hollow), Repetition (infinite/finite), Twist, Bend |

### 5.2 Specific 3D Sources

#### Rotating Primitives
```glsl
// Ray march a rotating torus
float map(vec3 p) {
    p = rotateY(p, time); // rotate around Y
    return sdTorus(p, vec2(1.0, 0.3)); // major/minor radius
}
```

**Parameters**: shape (enum), rotationSpeed XYZ, scale, material (wireframe/solid/emissive), position, smoothing

#### Wireframe Rendering
Apply `abs(sdf) - thickness` instead of just `sdf` to get wireframe/outline. Or use grid overlay on the surface UVs.

#### Multiple Objects
Use SDF domain repetition: `p = mod(p + halfSize, size) - halfSize` creates infinite grid of objects. Finite repetition with clamping.

#### Tunnel Effect
```glsl
// Classic tunnel: camera inside a cylinder
float map(vec3 p) {
    return -(length(p.xy) - radius) + noise(p.z * freq + time);
}
// UV mapping: angle for x, z-depth for y
// Advance camera: p.z += time * speed
```

**Parameters**: speed, radius, tiling, curvature, wallPattern (noise/grid/bricks), lightPosition
**This was one of ArKaos's most popular effects.**

#### Terrain
```glsl
float map(vec3 p) {
    return p.y - fbm(p.xz * 0.1); // FBM noise heightmap
}
```
**Parameters**: heightScale, noiseFreq, noiseOctaves, flySpeed, cameraAngle

### 5.3 Ray Marching Performance

Ray marching is the most GPU-intensive technique. Budget:
- Simple scene (1 object, no shadows): ~1-2ms at 1080p
- Complex scene (multiple objects, shadows, AO): ~4-8ms at 1080p
- Very complex (fractals, many iterations): ~8-16ms at 1080p

**For VJ use at 60fps**: Keep scenes simple. One or two objects, basic lighting, no ambient occlusion. The visual impact is still high.

**Complexity**: Medium to Complex (depending on scene complexity)

---

## 6. Noise / Organic Sources

### 6.1 Perlin Noise (2D, 3D)

**What it is**: Gradient noise with smooth interpolation between grid points. The foundation of procedural textures.

**GLSL Implementation**: Single fragment shader. Well-established implementations available. 2D uses 4 gradient lookups + bicubic interpolation. 3D uses 8 lookups.

**Fractal Brownian Motion (FBM)**: Layer multiple octaves of noise for natural-looking patterns:
```glsl
float fbm(vec2 p) {
    float value = 0.0, amplitude = 0.5;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * noise(p);
        p *= 2.0; // lacunarity
        amplitude *= 0.5; // gain
    }
    return value;
}
```

**Parameters**:
| Parameter | Range | Audio Mapping Idea |
|-----------|-------|--------------------|
| `scale` | [0.5, 50] | Inverse of frequency band energy |
| `octaves` | [1, 8] | Spectral complexity → detail |
| `lacunarity` | [1.5, 3.0] | Spectral rolloff |
| `gain` | [0.3, 0.7] | RMS → amplitude persistence |
| `speed` | [0, 5] | BPM → drift speed |
| `colorScheme` | palette | Key → palette |
| `warp` | [0, 2] | Domain warping intensity (noise feeds into itself) |

**Complexity**: Simple
**Performance**: Excellent (2D). Good (3D with FBM, 4-8 octaves).

### 6.2 Simplex Noise

**What it is**: Perlin's improvement — uses simplex grid (triangles in 2D) instead of square grid. Fewer artifacts, faster in higher dimensions.

**GLSL**: Same interface as Perlin. Widely available implementations. For 3D+, simplex is preferred.

**Complexity**: Simple
**Performance**: Slightly better than Perlin in 3D+

### 6.3 Worley / Cellular Noise

**What it is**: Distance to nearest random feature point in a grid. Produces cell-like, organic patterns (like biological cells, stone textures, cracked earth).

**GLSL**: Single fragment shader with 3x3 neighbor loop (see section 3.4 Voronoi — same algorithm, different coloring).

**Variations**:
- F1 (distance to nearest): smooth cells
- F2 (distance to second nearest): highlighted edges
- F2-F1: cell edges only (cracked/veined pattern)

**Parameters**: scale, F-function (F1/F2/F2-F1), animate, jitter, metric (euclidean/manhattan/chebyshev)
**Complexity**: Simple
**Performance**: Excellent

### 6.4 Plasma Effect

**What it is**: Classic demoscene effect. Layered sine waves with time-varying frequencies producing psychedelic color patterns.

```glsl
float plasma = sin(uv.x * 10.0 + time)
             + sin(uv.y * 10.0 + time * 1.1)
             + sin((uv.x + uv.y) * 10.0 + time * 0.7)
             + sin(length(uv) * 10.0 + time * 1.3);
plasma = plasma * 0.25 + 0.5; // normalize to [0,1]
vec3 color = palette(plasma); // cosine palette
```

**Parameters**: numWaves, frequencies, speeds, colorPalette, complexity
**Complexity**: Simple
**Performance**: Excellent
**Audio mapping**: Wave frequencies → band energies. Color speed → beat phase. Perfect for audio-reactive use.

### 6.5 Fire / Flame Simulation

**What it is**: Upward-drifting noise with color gradient (black → red → orange → yellow → white).

**GLSL Implementation**: Single fragment shader. Use FBM noise with upward scroll, apply fire color ramp.

```glsl
vec2 p = uv;
p.y -= time * speed; // upward drift
float n = fbm(p * scale);
n *= smoothstep(1.0, 0.0, uv.y); // fade at top
n *= smoothstep(0.0, 0.3, uv.y); // shape at bottom
vec3 color = fireGradient(n); // black→red→yellow→white
```

**Parameters**: speed, scale, intensity, colorRamp, turbulence, width, height
**Complexity**: Simple
**Performance**: Excellent

### 6.6 Water Caustics

**What it is**: Light patterns at the bottom of a pool caused by surface refraction.

**GLSL**: Single fragment shader. Multiple layers of Voronoi/cellular noise with different scales and animation speeds, combined with max or multiply.

```glsl
float caustics = 0.0;
for (int i = 0; i < 3; i++) {
    float scale = 1.0 + float(i) * 0.5;
    float speed = 1.0 + float(i) * 0.3;
    caustics = max(caustics, voronoi(uv * scale + time * speed));
}
```

**Parameters**: scale, speed, intensity, numLayers, color
**Complexity**: Simple
**Performance**: Good

### 6.7 Clouds

**What it is**: Soft, billowing FBM noise with cloud-like coloring.

**GLSL**: FBM with many octaves (6-8), soft color mapping, optional 3D noise for volumetric feel.

**Parameters**: coverage, density, scale, speed, windDirection, color, altitude
**Complexity**: Simple (2D FBM), Medium (3D volumetric)
**Performance**: Good

---

## 7. Concert Lighting Simulations

### 7.1 Moving Head Beams

**What it is**: Simulated concert moving-head spotlights with animated pan/tilt. Visible beams in haze.

**GLSL Implementation**: Single fragment shader. Each beam is a cone/wedge shape from a source point. Use SDF for cone shape, multiply by haze density, additive blend multiple beams.

```glsl
// Single beam: cone from top of screen
float beam(vec2 uv, vec2 origin, float angle, float width, float intensity) {
    vec2 dir = vec2(sin(angle), -cos(angle));
    vec2 toPixel = uv - origin;
    float along = dot(toPixel, dir);
    if (along < 0.0) return 0.0;
    float perp = abs(dot(toPixel, vec2(-dir.y, dir.x)));
    float coneWidth = along * width;
    float beam = smoothstep(coneWidth, coneWidth * 0.5, perp);
    beam *= exp(-along * falloff); // distance falloff
    beam *= intensity;
    return beam;
}
```

**Parameters**:
| Parameter | Range | Audio Mapping Idea |
|-----------|-------|--------------------|
| `numBeams` | [1, 16] | Fixed (scene setup) |
| `panAngle[]` | [-90, 90] deg | Beat phase → sweep, LFO |
| `tiltAngle[]` | [0, 90] deg | Spectral centroid → tilt |
| `beamWidth` | [0.02, 0.3] | Dynamic range → width |
| `intensity` | [0, 2] | RMS → brightness |
| `beamColor` | RGB | Key detection → color |
| `hazeIntensity` | [0, 1] | Spectral flatness → haze |
| `goboPattern` | texture/procedural | Fixed (per beam) |

**Complexity**: Simple to Medium
**Performance**: Excellent (purely analytical shapes)

### 7.2 Laser Beam Patterns

**What it is**: Geometric laser show patterns — lines, fans, tunnels, cones, grids drawn by laser beams.

**GLSL**: Single fragment shader. Lines with sharp glow (inverse distance to line segment with power falloff).

```glsl
// Laser line with glow
float laser(vec2 uv, vec2 a, vec2 b) {
    vec2 ba = b - a;
    float t = clamp(dot(uv - a, ba) / dot(ba, ba), 0.0, 1.0);
    float d = length(uv - a - ba * t);
    return pow(0.002 / d, 1.5); // sharp glow
}
```

**Patterns**: Fan (multiple lines from one point), tunnel (concentric shapes receding), grid (crossing lines), spiral, star, abstract geometric
**Parameters**: numLines, speed, pattern (fan/tunnel/grid/spiral), color, intensity, spread
**Complexity**: Simple
**Performance**: Excellent

### 7.3 LED Wall Patterns

**What it is**: Simulated pixel-grid display. Each "pixel" is a visible rectangle with gaps between.

**GLSL**: Grid of rounded rectangles with content (scrolling text, patterns, gradients, video).

**Parameters**: pixelSize, gapSize, brightness, scanlineEffect, content (color/gradient/pattern)
**Complexity**: Simple
**Performance**: Excellent

### 7.4 Spotlight with Gobos

**What it is**: Patterned light projection. A gobo is a stencil in front of a light, projecting a pattern.

**GLSL**: Beam shape multiplied by a procedural pattern (circles, stars, breakup, leaves).

**Parameters**: goboPattern, rotation, beamAngle, sharpness, color
**Complexity**: Simple
**Performance**: Excellent

### 7.5 Strobe Arrays

**What it is**: Grid of lights that flash in patterns (sequential, random, chase, all-at-once).

**GLSL**: Grid of bright circles/rectangles with per-cell timing.

```glsl
vec2 cell = floor(uv * gridSize);
float phase = hash(cell) * TWO_PI; // per-cell random phase
float flash = step(0.9, sin(time * strobeSpeed + phase));
```

**Parameters**: gridSize, strobeSpeed, pattern (random/chase/wave/all), intensity, color, flashDuration
**Complexity**: Simple
**Performance**: Excellent

### 7.6 Atmospheric Haze with Beams

**What it is**: Volumetric haze/fog that makes light beams visible. Combined with any beam source.

**GLSL**: Add noise-based density variation to beam intensity. Multiply beam by `fbm(uv * noiseScale + time * drift)`.

**Parameters**: hazeDensity, noiseScale, drift, visibility
**Complexity**: Simple (adds to beam shaders)

### 7.7 Par Can Wash

**What it is**: Wide, soft colored light gradients simulating wash lights on a stage.

**GLSL**: Multiple soft radial gradients with color mixing.

```glsl
vec3 wash = vec3(0.0);
for (int i = 0; i < numLights; i++) {
    vec2 pos = lightPositions[i];
    float d = length(uv - pos);
    wash += lightColors[i] * exp(-d * d * falloff);
}
```

**Parameters**: numLights, positions, colors, falloff, intensity
**Complexity**: Simple
**Performance**: Excellent

---

## 8. Text / Typography Sources

### 8.1 Text Rendering in OpenGL

**Challenge**: GLSL fragment shaders cannot render arbitrary text. Text requires either:
1. **Pre-rendered font atlas texture** (bitmap font) — most practical
2. **SDF font rendering** (signed distance field fonts) — scalable, sharp at any size
3. **CPU text rendering** to texture (JUCE can do this) — most flexible

**Recommended approach for Audio-DNA**: Use JUCE's `Graphics::drawText()` to render text to an `Image`, upload as OpenGL texture, then apply shader effects. This supports any system font, Unicode, word wrap, etc.

### 8.2 Scrolling Text

**Implementation**: Render text to a wide texture, animate UV offset.

**Parameters**: text, font, fontSize, scrollSpeed, direction (L/R/U/D), color, backgroundColor, repeat
**Complexity**: Simple (JUCE renders text, shader scrolls)

### 8.3 Text with Effects

**Implementation**: Render text to texture, then apply glow (Gaussian blur of text + additive blend), outline (dilate - original), shadow (offset copy), RGB split, glitch displacement.

**Parameters**: text, font, glowAmount, glowColor, outlineWidth, shadowOffset, effectType
**Complexity**: Simple to Medium

### 8.4 Character Matrix (Matrix Rain)

**What it is**: Columns of random characters scrolling downward at different speeds. Classic "Matrix" digital rain.

**GLSL Implementation**: Can be done in a single fragment shader using a font texture atlas.

```glsl
// Per column: different speed, random character sequence
vec2 cell = floor(uv * vec2(numColumns, numRows));
float columnSpeed = hash(cell.x) * 2.0 + 1.0;
float charIndex = hash(cell + floor(time * columnSpeed)) * numChars;
// Sample font atlas at charIndex position
// Brightness fades down column (trail effect)
float trail = exp(-fract(uv.y * numRows - time * columnSpeed) * trailLength);
```

**Parameters**: numColumns, speed, trailLength, color, characterSet, randomize
**Complexity**: Medium (needs font atlas texture)
**Performance**: Good

---

## 9. Audio-Visual Sources

These generators create visuals DIRECTLY from the audio feature data in FeatureSnapshot. They are unique to audio-reactive applications.

### 9.1 Waveform Display

**What it is**: Real-time audio waveform rendered as a line or filled shape. ArKaos has this as a generator effect.

**Implementation**: Pass audio samples (from ring buffer or FeatureSnapshot time-domain data) as a 1D texture. Fragment shader draws the waveform.

```glsl
uniform sampler1D u_waveform; // 512 or 1024 samples
float sample = texture(u_waveform, uv.x).r; // -1 to 1
float waveY = sample * amplitude + 0.5;
float dist = abs(uv.y - waveY);
float line = smoothstep(lineWidth, 0.0, dist);
```

**Parameters**: amplitude, lineWidth, color, fill (line vs filled), mirror (top+bottom), smoothing
**Complexity**: Simple
**Performance**: Excellent

### 9.2 Spectrum Analyzer Bars

**What it is**: Classic frequency spectrum as vertical bars.

**Implementation**: Pass band energies or FFT magnitude bins as 1D texture. Render as bars.

```glsl
uniform sampler1D u_spectrum; // band energies
float band = floor(uv.x * numBands) / numBands;
float energy = texture(u_spectrum, band).r;
float bar = step(uv.y, energy) * step(band, uv.x) * step(uv.x, band + barWidth);
```

**Parameters**: numBands, barWidth, gap, colorMode (solid/gradient/per-band), orientation, smoothing, peak hold
**Complexity**: Simple
**Performance**: Excellent

### 9.3 Circular Spectrum

**What it is**: Spectrum displayed radially — bars or line extending outward from center.

**Implementation**: Convert to polar coordinates, map angle to frequency band.

```glsl
vec2 p = uv - 0.5;
float angle = atan(p.y, p.x) / TWO_PI + 0.5; // 0-1
float radius = length(p);
float energy = texture(u_spectrum, angle).r;
float ring = smoothstep(baseRadius + energy * height, baseRadius + energy * height - lineWidth, radius)
           * smoothstep(baseRadius, baseRadius + lineWidth, radius);
```

**Parameters**: baseRadius, height, lineWidth, rotation, colorMode, mirror (both sides), smoothing
**Complexity**: Simple
**Performance**: Excellent

### 9.4 Audio-Driven Particle Emission

**What it is**: Particles spawn on beats, velocity from RMS, color from spectral features.

**Implementation**: CPU particle system with audio-reactive parameters (see Section 4.2). Onset triggers bursts, RMS controls emission rate, spectral centroid controls color temperature.

**Complexity**: Medium
**Performance**: Good

### 9.5 Chromagram Display

**What it is**: 12 pitch classes displayed as colored segments or a circle of fifths.

**Implementation**: Pass `chromagram[12]` as uniform array. Display as 12 colored bars, a radial chart, or a piano-style display.

**Parameters**: displayMode (bars/radial/piano), smoothing, colorScheme, scale
**Complexity**: Simple

### 9.6 Beat Pulse

**What it is**: A visual pulse synchronized to beat phase. Can be a expanding ring, flash, or shape transformation.

```glsl
uniform float u_beatPhase; // 0-1 sawtooth
float pulse = exp(-u_beatPhase * decay); // sharp attack, exponential decay
// Apply pulse to size, brightness, color, displacement, etc.
```

**Parameters**: shape (circle/ring/flash/star), decay, size, color, intensity
**Complexity**: Simple
**Performance**: Excellent

---

## 10. ArKaos Generator Effects

From the ArKaos VJ 3.6.1 release notes, these effects GENERATE content rather than modify existing input:

### True Generators (create from nothing)

| Effect | Category | What It Generates | Parameters |
|--------|----------|-------------------|------------|
| **Waveform** | Artistic | Audio waveform display from audio input | Mode (fill/draw), width, RGB color |
| **Digital Noiz** | Artistic | Animated noise patterns (5 presets) | BW mode, preset, speed |
| **Stroboscope** | Artistic | Strobe flashes (black/white/invert/background) | Mode, speed, shape (square/sine) |
| **Color Bars** (standard) | Test | Broadcast color bars | None |

### Pseudo-Generators (transform input into 3D scenes)

| Effect | Category | What It Does | Parameters |
|--------|----------|-------------|------------|
| **Tunnel** | 3D | Places visual inside a tunnel with camera movement | Speed, tiling, light, curve, orientation |
| **Screen Room** | 3D | Camera inside rotating sphere of visuals | Size, rotation XY, light rotation |
| **Cube Inside** | 3D | Camera inside rotating cube with visual on faces | Rotation XY, tiling, light |
| **3D Objects** | 3D | Map visual onto 3D primitives (plane, cube, sphere, cylinder, donut) | Size, rotation, tiling, light |
| **Plane** | 3D | Visual on planes scrolling above/below camera | Speed, rotation, altitude, tiling |
| **RotoZoom** | 3D | Infinite zoom with rotation | Rotation speed, zoom speed |
| **Larsen** (3 variants) | Artistic | Feedback loop effects | Size, rotation, transparency |

**Key insight from ArKaos**: Their most popular generators are the Tunnel, 3D Objects, and Waveform. The Tunnel effect is essentially a ray-marched cylinder — one of the simpler ray marching scenes. The 3D Objects map an input texture onto primitives, which is a vertex-shader approach (not fragment-only).

---

## 11. Priority Matrix

### Tier 1 — Maximum Impact, Minimum Effort (Implement First)

These cover the broadest range of VJ scenarios and are all single fragment shaders:

| # | Source | Why Priority | Fragment Shader? | Complexity |
|---|--------|-------------|-----------------|------------|
| 1 | **Plasma** | Classic VJ staple, infinitely varied, all params audio-routable | Yes | Simple |
| 2 | **Noise/FBM** (Perlin/Simplex) | Foundation of organic textures, domain warping creates infinite variations | Yes | Simple |
| 3 | **Tunnel** | Iconic VJ effect, hypnotic, simple ray march | Yes | Simple-Medium |
| 4 | **Waveform Display** | Direct audio visualization, every VJ app has this | Yes | Simple |
| 5 | **Spectrum Bars** | Essential audio visualization | Yes | Simple |
| 6 | **Concentric Rings** | Simple, hypnotic, perfect beat-sync target | Yes | Simple |
| 7 | **Radial Burst / Starburst** | High energy, great for drops | Yes | Simple |
| 8 | **Moving Head Beams** | Concert lighting feel, instant pro look | Yes | Simple |
| 9 | **Strobe Flash** | Essential for energy moments | Yes | Simple |
| 10 | **Voronoi / Cellular** | Organic cell patterns, very visual | Yes | Simple |

### Tier 2 — High Impact, Moderate Effort

| # | Source | Why Priority | Fragment Shader? | Complexity |
|---|--------|-------------|-----------------|------------|
| 11 | **Julia Set** | Infinitely varied fractals, smooth animation via c parameter | Yes | Simple |
| 12 | **Reaction-Diffusion** | Mesmerizing organic patterns, unique look | Ping-pong FBO | Medium |
| 13 | **Kaleidoscope Generator** | Crowd-pleaser, works with any inner pattern | Yes | Simple |
| 14 | **Laser Beams** | Concert/club staple | Yes | Simple |
| 15 | **Fire / Flame** | Dramatic, visceral | Yes | Simple |
| 16 | **Game of Life / Cellular Automata** | Emergent complexity from simple rules | Ping-pong FBO | Medium |
| 17 | **Grid Patterns** (dots, lines, hex) | Clean geometric look, design-friendly | Yes | Simple |
| 18 | **Circular Spectrum** | Visually striking audio visualization | Yes | Simple |
| 19 | **Water Caustics** | Beautiful organic, aquatic mood | Yes | Simple |
| 20 | **Moire Patterns** | Hypnotic interference, simple to implement | Yes | Simple |

### Tier 3 — Specialized / Higher Effort

| # | Source | Why Priority | Fragment Shader? | Complexity |
|---|--------|-------------|-----------------|------------|
| 21 | **Mandelbrot** | Deep zoom is iconic but less audio-reactive | Yes | Simple |
| 22 | **3D Primitives** (ray march) | Rotating shapes in space | Yes | Medium |
| 23 | **Spirograph** | Beautiful curves, math art | Yes | Simple |
| 24 | **Sacred Geometry** | Niche but devoted audience | Yes | Simple-Medium |
| 25 | **Matrix Rain** | Pop culture icon | Yes + font atlas | Medium |
| 26 | **Particle Emitter** | Physics-based, requires CPU + GPU | CPU + GPU | Medium |
| 27 | **Clouds** | Atmospheric, slower-paced | Yes | Simple |
| 28 | **Spotlight with Gobos** | Specialized lighting look | Yes | Simple |
| 29 | **LED Wall Patterns** | Niche venue simulation | Yes | Simple |
| 30 | **Chromagram Display** | Music theory visualization | Yes | Simple |

### Tier 4 — Advanced / Niche

| # | Source | Why Priority | Fragment Shader? | Complexity |
|---|--------|-------------|-----------------|------------|
| 31 | **Strange Attractors** (Clifford/De Jong) | Beautiful but niche, density render is slow | Yes (slow) or CPU | Medium |
| 32 | **Lorenz Attractor** | Iconic chaos visual | CPU + GPU lines | Medium |
| 33 | **Flame Fractals** | Stunning but very complex | CPU + GPU | Complex |
| 34 | **L-Systems** | Fractal trees, botanical | CPU geometry | Medium-Complex |
| 35 | **Terrain** | 3D landscape, less VJ, more art | Yes (ray march) | Medium |
| 36 | **Beat Pulse** | Simple but useful as overlay | Yes | Simple |
| 37 | **Scrolling Text** | Needs JUCE text rendering | JUCE + GPU | Medium |
| 38 | **Worley Noise** | Variant of Voronoi, already covered | Yes | Simple |
| 39 | **Par Can Wash** | Soft lighting, subtle | Yes | Simple |
| 40 | **Haze/Fog** | Atmospheric addon | Yes | Simple |

### Coverage Analysis

With just the **Tier 1 sources (10 generators)**, a VJ can cover:
- **Abstract/Psychedelic**: Plasma, Noise/FBM, Voronoi
- **Geometric/Clean**: Concentric Rings, Starburst
- **3D/Immersive**: Tunnel
- **Audio Visualization**: Waveform, Spectrum Bars
- **Concert/Club**: Moving Head Beams, Strobe
- **Organic/Natural**: Noise/FBM, Voronoi

Adding **Tier 2 (10 more)** brings:
- **Fractal/Mathematical**: Julia Set
- **Biological/Organic**: Reaction-Diffusion, Cellular Automata
- **Reflective/Hypnotic**: Kaleidoscope, Moire
- **Elemental**: Fire, Water Caustics
- **Concert Extended**: Laser Beams
- **Design/Pattern**: Grid Patterns
- **Audio Extended**: Circular Spectrum

**20 generators cover virtually every VJ style.** The remaining 20 are specialized niches.

---

## 12. Implementation Architecture

### Source vs Effect: Architectural Difference

| Aspect | Effect (current) | Source (new) |
|--------|-----------------|-------------|
| **Input** | Receives texture from previous stage | Generates its own output |
| **Shader** | Samples `u_inputTexture` | Does NOT sample input texture |
| **Chain position** | Any position in effect chain | Must be FIRST in chain (or standalone) |
| **FBO requirement** | Ping-pong pair (existing) | Writes to FBO like any effect |
| **State** | Stateless (per-frame) | Some need state (reaction-diffusion, GoL) |

### Stateless Sources (Tier 1 priority)

These need NO special infrastructure — they render entirely from uniforms + time:

```
SourceShader(uniforms: time, resolution, audio features) → FBO → [Effect Chain]
```

Implementation: Same as an Effect, but the shader ignores `u_inputTexture` and generates its output procedurally. Can literally be loaded into the existing EffectChain as the first element.

### Stateful Sources (Ping-Pong FBO)

Reaction-Diffusion, Cellular Automata, and similar feedback systems need previous frame state:

```
Read FBO_A → SourceShader → Write FBO_B → [Effect Chain reads FBO_B]
Next frame: swap A and B
```

This is the same ping-pong architecture the EffectChain already uses. The Source just needs its own dedicated FBO pair that persists across frames.

### Audio Data as Textures

For waveform/spectrum sources, audio data must be uploaded as a 1D texture:
- Waveform: 512 or 1024 float samples → `GL_TEXTURE_1D` or `GL_TEXTURE_2D` with height=1
- Spectrum: FFT magnitude bins → same approach
- Band energies: 7 floats → uniform array (no texture needed)
- Chromagram: 12 floats → uniform array

### Shared Uniforms for All Sources

Every source shader receives these standard uniforms (already in the rendering pipeline):

```glsl
uniform float u_time;           // seconds since start
uniform vec2  u_resolution;     // output resolution
uniform float u_rms;            // [0,1]
uniform float u_beatPhase;      // [0,1) sawtooth
uniform float u_bpm;            // BPM
uniform float u_spectralCentroid; // Hz, normalized
uniform bool  u_onsetDetected;  // onset flag
uniform float u_bandEnergies[7]; // 7-band spectrum
// ... all FeatureSnapshot fields
```

### Parameter Mapping

Source parameters should follow the same [0, 1] normalization as effect parameters. The existing MappingEngine routes audio features to parameters — sources should plug into this same system with zero changes.

### Recommended Uniform Naming for Sources

```
u_[sourceName]_[paramName]

Examples:
u_plasma_frequency
u_plasma_speed
u_plasma_colorOffset
u_tunnel_speed
u_tunnel_radius
u_tunnel_curvature
u_beams_numBeams
u_beams_panAngle
u_beams_intensity
```

---

## References

- Inigo Quilez, "2D SDF Functions" — https://iquilezles.org/articles/distfunctions2d/
- Inigo Quilez, "3D SDF Functions" — https://iquilezles.org/articles/distfunctions/
- Inigo Quilez, "Cosine Palettes" — https://iquilezles.org/articles/palettes/
- Inigo Quilez, "Ray Marching SDFs" — https://iquilezles.org/articles/raymarchingdf/
- The Book of Shaders, Chapter 11: Noise — https://thebookofshaders.com/11/
- The Book of Shaders, Chapter 12: Cellular Noise — https://thebookofshaders.com/12/
- Vidvox ISF Shader Repository — https://github.com/Vidvox/ISF-Files
- Gray-Scott Reaction-Diffusion — https://en.wikipedia.org/wiki/Reaction-diffusion_system
- ArKaos VJ 3.6.1 Release Notes (local PDF)
- TouchDesigner TOP Reference — https://docs.derivative.ca/Category:TOPs
