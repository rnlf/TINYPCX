/*
  TINYPCX
  Copyright (C) 2016 Florian Kesseler
  This project is free software; you can redistribute it and/or modify it
  under the terms of the MIT license. See LICENSE.md for details.
*/

#include <stdio.h>
#include "tinypcx.h"

/* Header fields for PCX */
typedef struct {
  pcx_uint8  identification;
  pcx_uint8  pcx_version;
  pcx_uint8  compression;
  pcx_uint8  bpp;
  pcx_uint16 coords[4];
  pcx_uint16 dpi[2];
  PCXPaletteEntry color_map[16];
  pcx_uint8  reserved;
  pcx_uint8  plane_count;
  pcx_uint16 row_stride;
  pcx_uint16 palette_information;
  pcx_uint16 width;
  pcx_uint16 height;
} PCXFileHeader;

#define FILE_IDENTIFICATION 10
#define FILE_DATA_OFFSET 128
#define RLE_LEN_FLAG 0xC0
#define RLE_LEN_MASK 0x3F
#define PALETTE_FLAG_OFFSET (-3*256-1)
#define PALETTE_FLAG 0x0C

int pcx_open_file(char const *filename, PCXFile *data) {
  PCXFileHeader header;

  data->file = fopen(filename, "rb");
  data->scanlines_read = 0;
  data->palette_size = 0;

  if(data->file == NULL) {
    return PCX_ERR_OPEN;
  }

  if(fread(&header, sizeof(PCXFileHeader), 1, data->file) != 1) {
    fclose(data->file);
    return PCX_ERR_INVALID;
  }

  if(header.identification != FILE_IDENTIFICATION) {
    getch();
    fclose(data->file);
    return PCX_ERR_INVALID;
  }

  if(header.bpp != 8) {
    fclose(data->file);
    return PCX_ERR_UNSUPPORTED;
  }

  if(header.plane_count != 1) {
    fclose(data->file);
    return PCX_ERR_UNSUPPORTED;
  }

  data->width  = header.coords[2] - header.coords[0] + 1;
  data->height = header.coords[3] - header.coords[1] + 1;
  data->version = header.pcx_version;

  fseek(data->file, FILE_DATA_OFFSET, SEEK_SET);

  return PCX_ERR_NONE;
}

void pcx_close_file(PCXFile far *file) {
  fclose(file->file);
}

int pcx_read_scanline(PCXFile far *data, pcx_uint8 far *scanline) {
  int count = 0;

  if(++data->scanlines_read > data->height) {
    return PCX_ERR_USAGE;
  }

  while(count < data->width) {
    pcx_uint8 rcount;
    pcx_uint8 byte;
    if(fread(&byte, 1, 1, data->file) != 1) {
      return PCX_ERR_INVALID;
    }

    if((byte & RLE_LEN_FLAG) == RLE_LEN_FLAG) {
      rcount = byte & RLE_LEN_MASK;
      if(fread(&byte, 1, 1, data->file) != 1) {
        return PCX_ERR_INVALID;
      }
    } else {
      rcount = 1;
    }

    while(rcount --> 0) {
      scanline[count++] = byte;
    }
  }

  return PCX_ERR_NONE;
}

int pcx_get_palette_size(PCXFile far * file) {
  if(file->scanlines_read != file->height) {
    return PCX_ERR_USAGE;
  }

  if(file->version != 5) {
    file->palette_size = PCX_PALETTE_16;
  } else {
    long pos = ftell(file->file);
    long pos2;

    pcx_uint8 byte;
    fseek(file->file, PALETTE_FLAG_OFFSET, SEEK_END);
    pos2 = ftell(file->file);

    if(pos2 < pos) {
      file->palette_size = PCX_PALETTE_16;
    } else {
      fread(&byte, 1, 1, file->file);
      if(byte == PALETTE_FLAG) {
        file->palette_size = PCX_PALETTE_256;
      } else {
        file->palette_size = PCX_PALETTE_16;
      }
    }
  }

  return file->palette_size;
}

#define offsetof(STRUCT, MEMBER) \
  ((long)(((STRUCT*)0)->MEMBER))

int pcx_read_palette(PCXFile far * file, PCXPaletteEntry far * palette) {
  int i;
  PCXPaletteEntry palette_buf[256];

  if(file->palette_size == PCX_PALETTE_16) {
    fseek(file->file, offsetof(PCXFileHeader, color_map), SEEK_SET);
  } else if(file->palette_size == PCX_PALETTE_256) {
    fseek(file->file, PALETTE_FLAG_OFFSET+1, SEEK_END);
  } else {
    return PCX_ERR_USAGE;
  }
  fread(palette_buf, sizeof(PCXPaletteEntry), file->palette_size, file->file);
  for(i = 0; i < file->palette_size; ++i) {
    palette[i] = palette_buf[i];
  }

  return PCX_ERR_NONE;
}
