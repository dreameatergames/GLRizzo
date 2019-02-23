# GLRizzo

This Repository is an attempt to try to port the GL code of Makaqu to Modern Kallitios and Kazade's GLdc OpenGL port, and make it compilable. It is a work and progress, and needs a lot of help to get it working.

## Current Status: Not Working (See Below)

### How to Compile:

If you want to try to compile it while it errors out, you basically just need to clone the repo into any directory, navigate to s_dc and type make. That simple... if it would compile without errors.


### Current Errors:

We have all the compiling errors, but not the linking errors that happen at the end of the compile stage.
We get undefined references to every single OpenGL external call, so it probably has something to do with the makefile or not linking GLdc correctly.