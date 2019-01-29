# GLRizzo

This Repository is an attempt to try to port the GL code of Makaqu to Modern Kallitios and Kazade's GLdc OpenGL port, and make it compilable. It is a work and progress, and needs a lot of help to get it working.

## Current Status: Not Working (See Below)

### How to Compile:

If you want to try to compile it while it errors out, you basically just need to clone the repo into any directory, navigate to s_dc and type make. That simple... if it would compile without errors.


### Current Errors:

Currently, it errors out at GLdraw.c. A few other places have variable warnings, but for now, I believe they can be ignored. Here are the current errors I'm getting:


gl_draw.c:1188:23: error: 'GL_KOS_COLOR_INDEX8_WITHALPHA' undeclared (first use in this function)

gl_draw.c:1200:82: error: 'GL_COLOR_INDEX' undeclared (first use in this function)



Even when changing the first line error to ''' GL_COLOR_INDEX8_WITHALPHA ''' It does not work. I fear it might have to do with paletted texture support, but I might be missing something. After commenting this out, I then get errors for all the references to ''' luminence '''. At this point I stopped, for now, as I have not had time or knowledge to work on it. If anyone could help, I'd really appreciate it!
