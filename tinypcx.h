/*
  TINYPCX
  Copyright (C) 2016 Florian Kesseler
  This project is free software; you can redistribute it and/or modify it
  under the terms of the MIT license. See LICENSE.md for details.
*/

#ifndef TINYPCX_H
#define TINYPCX_H

/***************************************************************************
 Tiny PCX file reader.

 This library reads a very specific type of PCX file: 8 bit paletted.
 That's probably the most useful one for simple 16 bit DOS games.

 Usage:
   1. Open the file:

      PCXFile pcx_file;
      int err = pcx_open_file("filename.pcx", &pcx_file);
      // error checks

   2. Read pcx_file.height scanlines of pcx_file.width pixels:

      for(i = 0; i < pcx_file.height; ++i) {
        int err = pcx_read_scanline(pcx_file, &buf);
        // error checks
      }

   3. Read the palette size:

      int palette_size = pcx_get_palette_size(&pcx_file);
      // error checks

   4. Read the palette into a buffer of appropriate size:

      PCXPaletteEntry palette[256];
      int err = pcx_read_palette(&pcx_file, &palette);
      // error checks

   5. Close the file:

      pcx_close_file(&pcx_file);

   Note that these steps have to be performed in this specific order,
   otherwise the decoding will fail. Especially the palette size can only
   be determined once all scanlines have been read and subsequently, the
   palette can only be read once its size is known.

   You should close the file on every error except on errors returned from
   pcx_open_file.

   All functions except pcx_close_file return an error code, which is
   explained below.

   All functions except pcx_get_palette_size that return an error return
   PCX_ERR_NONE if no error occured. pcx_get_palette_size returns the size
   of the palette (i.e. number of entries) instead.

****************************************************************************/

typedef unsigned char  pcx_uint8;
typedef unsigned short pcx_uint16;

typedef struct {
  pcx_uint8 red;   /* Red value of palette entry (0-255)   */
  pcx_uint8 green; /* Green value of palette entry (0-255) */
  pcx_uint8 blue;  /* Blue value of palette entry (0-255)  */
} PCXPaletteEntry;

typedef struct {
  FILE       *file;           /* Internal use */
  pcx_uint16  width;          /* Width of image in pixels */
  pcx_uint16  height;         /* Height of image in scanlines */
  pcx_uint16  scanlines_read; /* Internal use */
  pcx_uint16  palette_size;   /* Internal use */
  pcx_uint8   version;        /* Internal use */
} PCXFile;

#define PCX_ERR_NONE        0 /* No error occured */
#define PCX_ERR_OPEN        1 /* File could not be opened */
#define PCX_ERR_INVALID     2 /* File content doesn't seem to be PCX */
#define PCX_ERR_UNSUPPORTED 3 /* File uses an unsupported PCX feature */
#define PCX_ERR_USAGE       4 /* Incorrect use of this library. */
#define PCX_PALETTE_16     16 /* Palette has 16 entries */
#define PCX_PALETTE_256   256 /* Palette has 256 entries */

int pcx_open_file(char const *filename, PCXFile *data);
int pcx_read_scanline(PCXFile far *data, pcx_uint8 far *scanline);
int pcx_get_palette_size(PCXFile far * file);
int pcx_read_palette(PCXFile far * file, PCXPaletteEntry far * palette);
void pcx_close_file(PCXFile far *data);

#endif
