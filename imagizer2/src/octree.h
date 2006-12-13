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

#ifndef OCTREE_H
#define OCTREE_H
#include <list>
using std::list;

#define bpc 8 // bits per color


struct octreenode {
	bool isLeaf;
	unsigned long int references;
	unsigned long int red;
	unsigned long int green;
	unsigned long int blue;
	octreenode *child[8];
	unsigned long int child_count;
	octreenode *parent;
	unsigned long int child_id;
//  struct octreenode parent;
	};

class octree {
	public:
		void clear();
		void insert_color(unsigned char r, unsigned char g, unsigned char b);
		bool fill_palette(char * palette);
		octree(unsigned long int numlevels,unsigned long numcolors);
		~octree();
	private:
		void reduce();
		void free_subtree(octreenode *p);
		octreenode * create_new_node(unsigned char r, unsigned char g, unsigned char b, bool asLeaf);
		unsigned long int maxlevel;
		unsigned long int maxcolors;
		octreenode *root;
		unsigned long int count;
		list<octreenode*> leaflist;
	};
#endif
