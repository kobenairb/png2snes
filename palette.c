#include <png.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "palette.h"

#define CONVERT_TO_BGR15(red, green, blue) (((blue & 0xF8) << 7) | ((green & 0xF8) << 2) | (red >> 3))

uint16_t* convert_palette(png_structp png_ptr, png_infop info_ptr, int* size)
{
  uint16_t* palette;
  png_color* png_palette;

  //Fetch Palette from PNG file
  png_get_PLTE(png_ptr, info_ptr, &png_palette, size);

  //Allocate memory for palette
  palette = malloc(*size * sizeof(uint16_t));
  for(size_t i = 0; i < *size; i++)
    palette[i] = CONVERT_TO_BGR15(png_palette[i].red, png_palette[i].green, png_palette[i].blue);

  for(size_t i = 1; i < *size; i++)
  {
    for(size_t j = 1; j < i; j++)
    {
      if(palette[i] == palette[j])
        fprintf(stderr, "Found duplicate colors %lu and %lu\n", i, j);
    }
  }

  return palette;
}

void output_palette_binary(char* basename, uint16_t* data, int words)
{
  FILE* fp;
  char* filename;

  if(asprintf(&filename, "%s.cgr", basename) == -1)
  {
    perror("output_palette_binary");
    return;
  }

  fp = fopen(filename, "wb");

  if(!fp)
  {
    perror(filename);
    free(filename);
    return;
  }

  for(size_t i = 0; i < words; i++)
  {
    putc(data[i] & 0xFF, fp);
    putc(data[i] >> 8, fp);
  }

  free(filename);
  fclose(fp);
}

void output_palette_wla(char* basename, uint16_t* data, int words)
{
  FILE* fp;
  char* filename;

  if(strcmp(basename, "-") == 0)
    fp = stdout;
  else
  {
    if(asprintf(&filename, "%s_cgram.asm", basename) == -1)
    {
      perror("output_palette_wla");
      return;
    }

    fp = fopen(filename, "w");

    if(!fp)
    {
      perror(filename);
      free(filename);
      return;
    }

    free(filename);
  }

  for(size_t i = 0; i < words; i++)
  {
    if((i & 0x07) == 0)
      fprintf(fp, "\n\t.dw ");

    fprintf(fp, "$%04X", data[i]);

    if(((i & 0x07) < 7) && (i < (words-1)))
        fprintf(fp, ", ");
   }

  fprintf(fp, "\n");

  if(fp != stdout && fp != NULL)
    fclose(fp);
}
