/***************************************************************************
 *   Copyright (C) 2006 by Da Fox   *
 *   dafox@shogoki   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <iostream>
#include <png.h>
#include "readpng.h"

int readpngintobuffer(FILE * ip, unsigned char* &imgdata, unsigned long int * w, unsigned long int * h) {
  char *header=new char[8];
  short bytesread = fread(header, 1, 8, ip);
  if( !png_sig_cmp((png_byte*)header, 0, 8) && bytesread==8) {  // is png
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
    if(png_ptr!=NULL) {
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if(info_ptr!=NULL) {
        png_infop end_info = png_create_info_struct(png_ptr);
        if(end_info != NULL) {
          png_init_io(png_ptr, ip);
          png_set_sig_bytes(png_ptr, bytesread);
          png_set_read_user_chunk_fn(png_ptr, /*user_chunk_ptr*/ NULL, read_chunk_callback);
          /* Now we can read the image into memory (I think) */
          png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY | PNG_TRANSFORM_STRIP_ALPHA , NULL);
          png_bytep *row_pointers=NULL;
          row_pointers = png_get_rows(png_ptr, info_ptr);
          *w = png_get_image_width(png_ptr,info_ptr);
          *h = png_get_image_height(png_ptr,info_ptr);
          if ( (*w)!=160 || (*h)!=100 ) {
            fprintf(stderr,"Image width is not 160, or image height is not 100");
            return EXIT_FAILURE;
            }
          /* Assume: Files that are converted with this utility are 24bits, 8bpp, RGB images */
//          char * imgdata=new char[(*w)*(*h)*3];
          for(int i=0; i<*h; i++) {
            memcpy(imgdata+03*(*w)*i,row_pointers[i],(*w)*03);
            }
          // deinit libpng && exit-if to continue with AA-ification
          png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
          delete header;
//          buffer = imgdata;
          return EXIT_SUCCESS;
//          fclose(ip);
          }
        else {
          fprintf(stderr, "Eror: Failed to initialise libpng in png_create_info_struct().\n");
          png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
          return EXIT_FAILURE;
          }
        }
      else {
        fprintf(stderr, "Eror: Failed to initialise libpng in png_create_info_struct().\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        return EXIT_FAILURE;
        }
      }
    else {
      fprintf(stderr, "Eror: Failed to initialise libpng in png_create_read_struct().\n");
      return EXIT_FAILURE;
      }
    }
  else {
    fprintf(stderr, "Error: This is not a png file.\n");
    delete header;
    return EXIT_FAILURE;
    }
  }

int read_chunk_callback(png_structp png_ptr, png_unknown_chunkp chunk) {
  fprintf(stderr, "Warning: unknown chunk, ignoring.\n");
  return 1;
  }
