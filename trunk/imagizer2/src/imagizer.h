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
#ifndef IMAGIZE_H
#define IMAGIZE_H

void initialize_imagizer(unsigned char * biosfont);
void uninitialize_imagizer();
void texttopng(unsigned char * textdata, unsigned char * palette);
void imagize(unsigned char * img, unsigned char * palette, unsigned char *b800h,unsigned long int img_width,unsigned long int img_height);

class RGBColor {
  public:
    RGBColor();
    ~RGBColor();
    unsigned long int distance(RGBColor &target);
    long int r;
    long int g;
    long int b;

  private:

  };

#endif
