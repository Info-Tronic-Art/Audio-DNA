# GIFlib

Copyright © 2012 Eric S. Raymond

GIFLIB is a package of portable tools and library routines for working with GIF
images.

The Graphics Interchange Format(c) specification is the copyrighted property of
CompuServe Incorporated. GIF(sm) is a service mark property of CompuServe
Incorporated.

This package has been released under an X Consortium-like open-source license.
Use and copy as you see fit. If you make useful changes, add new tools, or find
and fix bugs, please send your mods to the maintainers for general distribution.

The util directory includes programs to clip, rotate, scale, and position GIF
images. These are no replacement for an interactive graphics editor, but they
can be very useful for scripted image generation or transformation.

The library includes program-callable entry points for reading and writing GIF
files, an 8x8 utility font for embedding text in GIFs, and an error handler. GIF
manipulation can be done at a relatively low level by sequential I/O (which
automatically does/undoes image compression) or at a higher level by slurping
an entire GIF into allocated core.

This library speaks both GIF87a and GIF89. The differences between GIF87 and
GIF89 are minor: in the latter, the interpretation of some extension block
types is defined. The library never needs to actually interpret these, but
giftext notices them and there are functions in the API to read and modify them.