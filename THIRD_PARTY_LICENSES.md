# Third-Party Licenses

Attribution for third-party source vendored into Audio-DNA and compiled
directly into the shipped binary.

## Syphon Framework

- **Project:** https://github.com/Syphon/Syphon-Framework
- **Vendored via:** CMake `FetchContent`, pinned to commit
  `71351d4b484cd2d1917867f7846a5cdca724552d` (`main`, 2025-10-06). See the
  Syphon block in `CMakeLists.txt`.
- **License:** BSD 3-Clause ("New"/"Revised")
- **Copyright:** 2010 bangnoise (Tom Butterworth) & vade (Anton Marini). All
  rights reserved.

```
Syphon Framework License:

Copyright 2010 bangnoise (Tom Butterworth) & vade (Anton Marini).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of the Syphon Project nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

## libprojectM 4.1.1 (dynamically linked, locally patched)

Not vendored: libprojectM is a separately installed shared library that the
app links dynamically (optional -- MilkDrop is built only when it is found).

- **Project:** https://github.com/projectM-visualizer/projectm
- **Version:** 4.1.1, upstream commit
  `03aa8a7ffdf81165136ee64643c6a781f5c6a391`, with its submodules.
- **Local modification:** `cmake/projectm/0001-render-frame-fbo.patch` (in
  this repository) backports the upstream-shaped
  `projectm_opengl_render_frame_fbo()` so MilkDrop renders into the app's
  own framebuffer (BF10, s-rta-1002b). `cmake/projectm/build-projectm.sh`
  fetches the source above, applies the patch, and installs the modified
  library into `$HOME/.local/opt/projectm-4.1.1-fbo1`; it also keeps the
  unmodified source archive at `$HOME/.local/src/projectm-03aa8a7.tar.gz`.
  The patch plus the pinned upstream commit are the complete corresponding
  source of the modified library.
- **License:** GNU Lesser General Public License, version 2.1 (the upstream
  `LICENSE.txt`; full text at https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html).
- **Copyright:** (C) 2003-2009 projectM Team, and the contributors listed in
  the upstream `AUTHORS.txt`.
- If Audio-DNA is ever distributed, the LGPL-2.1 terms for a modified library
  apply: offer the modified library's source (the patch + the pinned commit)
  and keep it replaceable (it is dynamically linked).
