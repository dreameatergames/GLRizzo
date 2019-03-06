#ifdef GLQUAKE
/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// vid_null.c -- null video driver to aid porting efforts

#include "quakedef.h"
#include <kos.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glkos.h>

viddef_t vid; // global video state

#define WARP_WIDTH 320
#define WARP_HEIGHT 200

unsigned short d_8to16table[256];
unsigned d_8to24table[256];
unsigned char d_15to8table[1 << 15];

/* cvar_t		vid_mode = {"vid_mode","5",false}; */
cvar_t vid_redrawfull = {"vid_redrawfull", "0", false};
cvar_t vid_waitforrefresh = {"vid_waitforrefresh", "0", true};

int texture_mode = GL_LINEAR;

int texture_extension_number = 1;

float gldepthmin, gldepthmax;
float vid_gamma;

cvar_t gl_ztrick = {"gl_ztrick", "0"};

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;
void (*qglColorTableEXT)(int, int, int, int, int, const void *);
qboolean is8bit = false;

qboolean isPermedia = false;
qboolean gl_mtexable = false;

#define PACK_RGB565(r, g, b) ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))

int basewidth, baseheight, screenheight, kos_vidmode, dcvidmode = 4;

int scr_width, scr_height;

void CheckMultiTextureExtensions(void)
{
	gl_mtexable = true;
}

void VID_SetPalette(unsigned char *palette)
{
	byte *pal;
	unsigned r, g, b;
	unsigned v;
	int r1, g1, b1;
	int j, k, l, m;
	unsigned short i;
	unsigned *table;
	char s[255];
	int dist, bestdist;
	static qboolean palflag = false;

	//
	// 8 8 8 encoding
	//
	pal = palette;
	table = d_8to24table;
	for (i = 0; i < 256; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;

		v = (255 << 24) + (r << 0) + (g << 8) + (b << 16);
		*table++ = v;
	}
	d_8to24table[255] &= 0xffffff; // 255 is transparent

	// JACK: 3D distance calcs - k is last closest, l is the distance.
	for (i = 0; i < (1 << 15); i++)
	{
		/* Maps
		000000000000000
		000000000011111 = Red  = 0x1F
		000001111100000 = Blue = 0x03E0
		111110000000000 = Grn  = 0x7C00
		*/
		r = ((i & 0x1F) << 3) + 4;
		g = ((i & 0x03E0) >> 2) + 4;
		b = ((i & 0x7C00) >> 7) + 4;
		pal = (unsigned char *)d_8to24table;
		for (v = 0, k = 0, bestdist = 10000 * 10000; v < 256; v++, pal += 4)
		{
			r1 = (int)r - (int)pal[0];
			g1 = (int)g - (int)pal[1];
			b1 = (int)b - (int)pal[2];
			dist = (r1 * r1) + (g1 * g1) + (b1 * b1);
			if (dist < bestdist)
			{
				k = v;
				bestdist = dist;
			}
		}
		d_15to8table[i] = k;
	}
}

void VID_ShiftPalette(unsigned char *palette)
{
	VID_SetPalette(palette);
}

void VID_Shutdown(void)
{
}

/*
===============
GL_Init
===============
*/
void GL_Init(void)
{
	gl_vendor = glGetString(GL_VENDOR);
	Con_Printf("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString(GL_RENDERER);
	Con_Printf("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString(GL_VERSION);
	Con_Printf("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString(GL_EXTENSIONS);
	Con_Printf("GL_EXTENSIONS: %s\n", gl_extensions);

	//	Con_Printf ("%s %s\n", gl_renderer, gl_version);

	CheckMultiTextureExtensions();

	glClearColor(1, 0, 0, 0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	//	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_FLAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/*
=================
GL_BeginRendering

=================
*/
void GL_BeginRendering(int *x, int *y, int *width, int *height)
{
	extern cvar_t gl_clear;

	*x = *y = 0;
	*width = scr_width;
	*height = scr_height;

	//    if (!wglMakeCurrent( maindc, baseRC ))
	//		Sys_Error ("wglMakeCurrent failed");

	//	glViewport (*x, *y, *width, *height);
}

void GL_EndRendering(void)
{
	//glFlush();
	glKosSwapBuffers();
}

qboolean VID_Is8bit(void)
{
	return is8bit;
}

void VID_Init8bitPalette(void)
{
	// Check for 8bit Extensions and initialize them.
	int i;
	void *prjobj;

	if (COM_CheckParm("-no8bit"))
		return;

	 else if (strstr(gl_extensions, "GL_EXT_shared_texture_palette")) {
		char thePalette[256 * 3];
		char *oldPalette, *newPalette;

		Con_SafePrintf("... Using GL_EXT_shared_texture_palette\n");
		glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
		oldPalette = (char *)d_8to24table; //d_8to24table3dfx;
		newPalette = thePalette;
		for (i = 0; i < 256; i++)
		{
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			oldPalette++;
		}
		qglColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, (void *)thePalette);
		is8bit = true;
	}
}

static void Check_Gamma(unsigned char *pal)
{
	float f, inf;
	unsigned char palette[768];
	int i;

	if ((i = COM_CheckParm("-gamma")) == 0)
	{
		if ((gl_renderer && strstr(gl_renderer, "Voodoo")) ||
			(gl_vendor && strstr(gl_vendor, "3Dfx")))
			vid_gamma = 1;
		else
			vid_gamma = 0.7; // default to 0.7 on non-3dfx hardware
	}
	else
		vid_gamma = Q_atof(com_argv[i + 1]);

	for (i = 0; i < 768; i++)
	{
		f = pow((pal[i] + 1) / 256.0, vid_gamma);
		inf = f * 255 + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		palette[i] = inf;
	}

	memcpy(pal, palette, sizeof(palette));
}

void SelectVidMode()
{
	switch (dcvidmode)
	{
	case 8:
		kos_vidmode = DM_800x608;
		basewidth = 800;
		baseheight = 608;
		screenheight = 608;
		break;
	case 7:
		kos_vidmode = DM_768x576;
		basewidth = 768;
		baseheight = 576;
		screenheight = 576;
		break;
	case 6:
		kos_vidmode = DM_768x480;
		basewidth = 768;
		baseheight = 480;
		screenheight = 480;
		break;
	case 5:
		kos_vidmode = DM_640x480;
		basewidth = 640;
		baseheight = 480;
		screenheight = 480;
		break;
	case 4:
		kos_vidmode = DM_640x480;
		basewidth = 640;
		baseheight = 400;
		screenheight = 480;
		break;
	case 3:
		kos_vidmode = DM_640x480;
		basewidth = 640;
		baseheight = 240;
		screenheight = 480;
		break;
	case 2:
		kos_vidmode = DM_640x480;
		basewidth = 320;
		baseheight = 480;
		screenheight = 480;
		break;
	case 1:
		kos_vidmode = DM_320x240;
		basewidth = 320;
		baseheight = 240;
		screenheight = 240;
		break;
	default: //case 0:
		kos_vidmode = DM_320x240;
		basewidth = 320;
		baseheight = 200;
		screenheight = 240;
		break;
	}
}
// Manoel Kasimier - end

void VID_Init(unsigned char *palette)
{
	SelectVidMode(); // Manoel Kasimier

	// BlackAura (15-08-2004) - Fix waterwarp crashes at higher resolution
	//	vid.maxwarpwidth = WARP_WIDTH; // Manoel Kasimier - hi-res waterwarp & buffered video - removed
	//	vid.maxwarpheight = WARP_HEIGHT; // Manoel Kasimier - hi-res waterwarp & buffered video - removed
	vid.width = vid.conwidth = basewidth;
	vid.height = vid.conheight = baseheight;
	vid.maxwarpwidth = vid.width;   // Manoel Kasimier - hi-res waterwarp & buffered video
	vid.maxwarpheight = vid.height; // Manoel Kasimier - hi-res waterwarp & buffered video
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong(*((int *)vid.colormap + 2048));
	vid.rowbytes = vid.conrowbytes = basewidth;

	// Initialise the KOS crap
	vid_set_mode(kos_vidmode, PM_RGB565);

	// Set up the default palette
	VID_SetPalette(palette);
}

// Adapted from nxDoom
void VID_Update(vrect_t *rects)
{
}

#endif