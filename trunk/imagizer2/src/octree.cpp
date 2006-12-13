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

#include <unistd.h>
#include "octree.h"
using std::list;

#define max(x,y) (x>y?(x):(y))
#define min(x,y) (x<y?(x):(y))
#define abs(x)   (x<0?(-x):(x))
#define distance(r1,g1,b1,r2,g2,b2) ((((r2)-(r1))*((r2)-(r1)))+(((g2)-(g1))*((g2)-(g1)))+(((b2)-(b1))*((b2)-(b1))))

/***********************************************
	PRIVATE
 ***********************************************/


/***********************************************
	PUBLIC
 ***********************************************/

octree::octree(unsigned long int numlevels,unsigned long numcolors) {
	root=new octreenode;
	root->references=0;
	root->red=0;
	root->green=0;
	root->blue=0;
	root->isLeaf=false;
	root->parent=NULL;
	root->child_id=-1;
	root->child_count=0;

	for(int i=0; i<8; i++) {
		root->child[i]=NULL;
		}
	count=0;
	maxlevel=min(max(numlevels,1),8);
	maxcolors=min(numcolors,1);
	}

bool octree::fill_palette(char * palette) {
	reduce();
	}

void octree::reduce() {
	unsigned long int mindist=0xffffffff;
	unsigned long int pixrep =0xffffffff;
	octreenode *q=NULL;
	while(count>maxcolors) {
		list<octreenode*>::iterator i = leaflist.begin();
		octreenode * q = *i ;
		unsigned long int min_references=(*i)->references;
		while( (i != leaflist.end()) ) {
			if((*i)->references > min_references) {
				min_references = (*i)->references;
				q=*i;
				}
			i++;
			}
		// q = color with minimal references, and i points to q
		octreenode* p = q->parent;
		octreenode* n = NULL;
		unsigned long int x=q->child_id;
		if( p->child_count > 1) {
			// find first neighbour            *FIXME* : Does not find _actual_ neighbours, but nodes in the same segment
			while(n==NULL && --x >= 0) {
				if(p->child[x] != NULL) {
					n=p->child[x];
					}
				}
			if(n==NULL) {
				x=q->child_id;
				while(n==NULL && ++x <= bpc) {
					if(p->child[x] != NULL) {
						n=p->child[x];
						}
					}
				}
			// n is a neighbour of q, merge them
			n->red   = (unsigned long int)(((( n->red   * n->references) + ( q->red   * q->references)) / (n->references + q->references)));
			n->green = (unsigned long int)(((( n->green * n->references) + ( q->green * q->references)) / (n->references + q->references)));
			n->blue  = (unsigned long int)(((( n->blue  * n->references) + ( q->blue  * q->references)) / (n->references + q->references)));
			n->references = (n->references + q->references);
			leaflist.erase(i);
			p->child[q->child_id]=NULL;
			delete q;
			}
		else if (p->child_count == 1) { // p has only a single color left, make p=q
			leaflist.erase(i);
			if(p->parent != NULL) { // p != root
				p->parent->child[p->child_id]=q;
				delete p;
				q=p;
				}
			else {
				fprintf(stderr, "error: p->parent != NULL");
				}
			leaflist.push_back(q);
			}
		count--;
		}
	return;
	}

octreenode * octree::create_new_node(unsigned char r, unsigned char g, unsigned char b, bool asLeaf) {
	octreenode * q = new octreenode;
	q->references=0;
	q->red=r;
	q->green=g;
	q->blue=b;
	q->isLeaf=asLeaf;
	for(int i=0; i<8; i++) {
		q->child[i]=NULL;
		}
	return q;
	}

void octree::insert_color(unsigned char r, unsigned char g, unsigned char b) {
	int level = 1;
	octreenode *p=root;
	octreenode *q=NULL;
	while(level <= maxlevel) {
		unsigned long int index=( ((r>>(bpc-level)) & 1) | ((g>>(bpc-level-1)) & 2) | ((b>>(bpc-level-2)) & 4)  );
		if (p->child[index] != NULL) {
			//p->references++;
			p=p->child[index];
			}
		else {
			q=create_new_node(r,g,b,level==maxlevel);
			q->parent=p;
			q->child_id=index;
			p->child[index]=q;
			p->child_count++;
			count++;
//       p->references++;
			p=q;
			leaflist.push_front(q);
			}
		level++;
		}
	//we're at leaf node p
	p->references++;
	}

void octree::free_subtree(octreenode *p) {
	for(int i=0; i<8; i++) {
		if(p->child[i]!=NULL) {
			free_subtree(p->child[i]);
			p->child[i];
			}
		}
	delete p;
	count--;
	p=NULL;
	}

octree::~octree() {
	/* FIXME: Walk the tree and delete all nodes */
	free_subtree(root);
	//count=0;
	}
