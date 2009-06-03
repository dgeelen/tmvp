//---------------------------------------------------------------------------
#include <assert.h>
#include <string>
#include <list>
#include <algorithm>
#include <vector>
#include "../common/UTypes.h"
#include "../common/UImage.h"
#include <limits>
//#include "UCmdLineParser.h"
using namespace std;
//---------------------------------------------------------------------------

//TODO: Clean up old code and useless comments

enum alignment { align_top, align_centre, align_bottom, align_left, align_right };
#define Mleft 0
#define Mright 1
#define Mtop 2
#define Mbottom 3

struct srtsub {
	list<list<string> > lines;
	uint32 start;
	uint32 end;
	uint32 index;
};

list<string> parseSRT(list<string> words, uint32 max_len) {
	list<string> l;
	string s="";
//  fprintf(stderr, "(maxlen=%u)\n",max_len);
	signed long int k=max_len;
	for(list<string>::iterator i=words.begin(); i!=words.end(); i++) { //loop through words, concatting them into strings of the correct size for display
		//fprintf(stderr, "%s", i->c_str());
	//  fprintf(stderr, "s.size=%u, i.size=%u, k=%u, s.size+i.size=%u, s=`%s', i=`%s'\n", s.size(), i->size(), k, s.size()+i->size(),s.c_str(), i->c_str());
		//if((s.size()+i->size()) <= k) {
		if( k - (signed long int)(i->size()) >= 0 ){
			s+=(*i)+" ";
			k-=((i->size())+1);
		}
		else {
			//fprintf(stderr, "resetting..\n");
			s.erase(s.size()-1, s.size());
			l.push_back(s);
			s="";
			k=max_len;
		}
	}
	s.erase(s.size()-1, s.size());
	l.push_back(s);
	/*fprintf(stderr, "parser:\n");
	for(list<string>::iterator i=l.begin(); i!=l.end(); i++){
		fprintf(stderr, "  `%s'\n",i->c_str());
	}*/
	return l;
}

list<string> split(string s) {
	string t;
	list<string> v;
	t="";
	for(uint32 i=0; i<s.size(); i++) {
		if(s[i]!=' '){
			t=t+s[i];
		}
		else {
			if(t!=""){
				v.push_back(t);
				t="";
			}
		}
	}
	if((t!="")||(v.size()==0)){
		v.push_back(t);
		t="";
	}
	return v;
}

struct srtsub lastsrt;

struct srtsub getNextSub(FILE* f) {
	struct srtsub s;
	list<list<string> > q;
	s.index=0;
	s.end=0;
	s.start=0;
	s.lines=q;

	char c;
	//fprintf(stderr,"reading index first char\n");
	fread(&c,1,1,f);
	//fprintf(stderr,"reading index following chars\n");
	string t="";
	if(feof(f)){
		return s;
	}
	while((c != '\n')) {
		t=t+c;
		fread(&c,1,1,f);
	}
	//fprintf(stderr,"read index\n");
	s.index = atol(t.c_str());
	//fprintf(stderr,"reading sub #%u\n", s.index);
	if(! (s.index > lastsrt.index)) {
		fprintf(stderr, "assertion failed : Non consecutive index numbers for subs %u and %u!\n", lastsrt.index, s.index);
		s.index=0;
		fseek(f,0,SEEK_END);
		fread(&c,1,1,f);
		return s;
	}
	else {
		lastsrt=s;
	}
	t="";
	fread(&c,1,1,f);
	while(c != '\n') {
		t=t+c;
		fread(&c,1,1,f);
	}
	//t=time index string
	uint32 a=t.find(" --> ",0);
	const char *p=t.c_str();
	if(a<0xffffffff){ //all ok
		s.start = atol(p)*60*60*1000 + atol(p+3)*60*1000 + atol(p+6)*1000 + atol(p+9) ;
		p+=a+5;
		s.end = atol(p)*60*60*1000 + atol(p+3)*60*1000 + atol(p+6)*1000 + atol(p+9) ;
	}
	else { // error looking for `-->'
		fprintf(stderr,"Error while parsing time index for subtitle %u!\n",s.index);
		s.start=0;
		fseek(f,0,SEEK_END);
		fread(&c,1,1,f);
		s.end=0;
		return s;
	}
	fread(&c,1,1,f);
	while(c!='\n') {
		t="";
		while(c != '\n') {
			t=t+c;
			fread(&c,1,1,f);
		}
		//s.l.push_back();
		s.lines.push_back(split(t));
		fread(&c,1,1,f);
	}
	/*/
	fprintf(stderr, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	for(uint32 i=0; i<s.lines.size(); i++){
		for(uint32 j=0; j<s.lines[i].size(); j++){
			fprintf(stderr, "%s /\\ ", s.lines[i][j].c_str());
		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(stderr, "start=%u, end=%u\n",s.start, s.end); //*/
	//fprintf(stderr, "%u\n", s.index);
	return s;
}



uint32 blitSRT(TextImage* ti, vector<srtsub>::iterator i, alignment *align, uint32 *margins, uint32 ctime, char color) {
//  fprintf(stderr, "  blitSRT: starting blit with srtsub #%u (start=%u, end=%u), ctime=%u\n", i->index, i->start, i->end, ctime);
	//fprintf(stderr, "#lines: %u\n", i->lines.size());
	list<string> blitstr;
	list<string> t;
	//fprintf(stderr, "i->[]==`%s'\n",i->lines.front().front().c_str());
	for(;(i->start <= ctime) && (ctime < i->end);i++){
		for(list<list<string> >::iterator j=i->lines.begin(); j!=i->lines.end(); j++){
			//fprintf(stderr, "parsing `%s'\n",j->front().c_str());
			t=parseSRT(*j, 80-margins[Mleft]-margins[Mright]);
			blitstr.splice(blitstr.end(), t);
		}
	}
	//blitstr holds all lines of text wordwrapped to fit margins
	list<string>::iterator k=blitstr.begin();
	#define linecount (50-margins[Mtop]-margins[Mbottom])
	#define blitcnt (blitstr.size()<linecount?blitstr.size():linecount)
	//fprintf(stderr, "blitcnt=%u\n", blitcnt);
	for(uint32 i=0; i<blitcnt; i++) { // render at most as much lines as will fit on screen
		uint32 x=0;
		uint32 y=0;
		if(align[0]==align_left){
			x=margins[Mleft];
		}
		else if(align[0]==align_centre) {
			x=40-((k->size()+1)>>1);
		}
		else if(align[0]==align_right) {
			x=80-margins[Mright]-(k->size());
		}
		else {
			fprintf(stderr,"error: Invalid H_ALIGN\n");
		}
		if(align[1]==align_top){
			y=margins[Mtop];
		}
		else if(align[1]==align_centre){
			y=25-((blitcnt+1)>>1);
		}
		else if(align[1]==align_bottom){
			y=50-margins[Mbottom]-(blitcnt);
		}
		else {
			fprintf(stderr,"error: Invalid V_ALIGN\n");
		}
		y+=i;
		uint16 *p=(ti->data)+x+80*y;
		char r[2];
		r[1]=color;
		uint16 *q=(uint16*)&r;
		//fprintf(stderr,"Blitting @(%u, %u): `",x, y);
		for(uint32 j=0; j<k->size(); j++){
			//fprintf(stderr,"%c",(*k)[j]);
			r[0]=(*k)[j];
			(*p)=(*q);
			p++;
		}
//    fprintf(stderr,"'\n");
	k++; //next string
	}
//  fprintf(stderr,"'\n");
/*    ti->data[0]=uint16('id');
		ti->data[1]=uint16(' t');
		ti->data[2]=uint16('si');
		ti->data[3]=uint16('r ');
		ti->data[4]=uint16('aa');
		ti->data[5]=uint16('!r'); /**/

/*  fprintf(stderr, "Result:\n");
	for(list<string>::iterator j=blitstr.begin(); j!=blitstr.end(); j++){
		fprintf(stderr, "  %s\n",j->c_str());
	}
/*
	#define MSize (?MVSize:MHSize)
	#define MHSize (80-margins[0]-margins[1])
	#define MVSize (50-margins[2]-margins[3])
	fprintf(stderr, "  blitSRT: blitting srtsub #%u\n", s.index);
	list<string> bLines;
	for(list<list<string> >::iterator i=s.lines.begin(); i!=s.lines.end(); i++){
		bLines=parseSRT(*i, MSize);
		if(){
			fprintf(stderr, "  >>> FIXME : Can't render ly yet! <<<"); //TODO: make  renderer
		}
		else { // render Horizontal
			/*FIXME: [BLOCKER] Cumulative renders overlap* /
			for(list<string>::iterator j=bLines.begin(); j!=bLines.end(); j++ ){// loop through lines to render
				if(alignment[0]==align_left){
				}
			}
		}
	}
	return q; */
}

char bestcontrast(TextPal *p) {
	RGBColor black(0, 0,0);
	RGBColor white(0xff, 0xff, 0xff);
	uint32 fg=0;
	uint32 bg=0;
	uint32 dist=0;
	uint32 best_fg_dist=std::numeric_limits<uint32>::max();
	uint32 best_bg_dist=std::numeric_limits<uint32>::max();
	for(uint32 i=0; i<16; i++) {
		dist=MRGBDistInt( p->GetColor( i ), black);
		if(dist<best_bg_dist){
			best_bg_dist=dist;
			bg=i;
		}
		dist=MRGBDistInt( p->GetColor( i ), white);
		if(dist<best_fg_dist){
			best_fg_dist=dist;
			fg=i;
		}
	}
	return (bg<<4)|fg;
}

  class SubSortFunc {
    public:
      bool operator()(const srtsub &a,const srtsub &b) {
        return a.start < b.start;
      }
  };

int main(int argc, char *argv[]) {
	//alignment align[2]={align_left  , align_bottom};
	//alignment align[2]={align_left  , align_centre};
	//alignment align[2]={align_left  , align_top};
	alignment align[2]={align_centre, align_bottom};
	//alignment align[2]={align_centre, align_centre};
	//alignment align[2]={align_centre, align_top};
	//alignment align[2]={align_right , align_bottom};
	//alignment align[2]={align_right , align_centre};
	//alignment align[2]={align_right , align_top};



	uint32 margins[4] = {4,4,2,2}; //{left, right, top ,bottom}
	lastsrt.index=0;
	lastsrt.start=0;
	lastsrt.end=0;
/*  string ifname = "raw.in";
	string ofname = "raw.out"; /*/
	string ifname = "-";
	string ofname = "-"; /**/

	string sfname = "test.srt";
	FILE* ifhandle = (ifname == "-") ? stdin  : fopen(ifname.c_str(), "r");
	FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "w");
	FILE* sfhandle = fopen(sfname.c_str(), "r");

//  infbuffer<char> *srtbuf = new infbuffer<char>(sfhandle,1024*64);
	vector<srtsub> subs; //1 word in a subtitle is in a list of list of list of string :p
	//fprintf(stderr, "Parsing srt file...\n");
	while(!feof(sfhandle)){ //parse entire srt file
//    fprintf(stderr, "getNextSub\n");
		subs.push_back( getNextSub(sfhandle) );
	}
	subs.pop_back(); //TODO: Improve end-of-srt-file detection
	//fprintf(stderr, "Completed parsing srt file\n");
	//fprintf(stderr, "subtitler: SRT file has %u entries\n",subs.size());
  fclose(sfhandle);
  SubSortFunc ssf;
  sort(subs.begin(), subs.end(), ssf);

//  fprintf(stderr, "done\n");

/*  fprintf(stderr,"Collected subs:\n");
	while(subs.size()>0){
		struct srtsub s=subs.front();
		fprintf(stderr,"Subtitle #%u",s.index);
		fprintf(stderr,", on screen from %u till %u\n",s.start, s.end);
		uint32 x=0;
		for(list<list<string> >::const_iterator i=s.lines.begin(); i!=s.lines.end(); i++) {
			fprintf(stderr,"  line %u = ",x);
			for(list<string>::const_iterator j=i->begin(); j!=i->end(); j++){
				fprintf(stderr,"%s ",j->c_str());
			}
			fprintf(stderr,"\n");
			x++;
		}
		subs.pop_front();
	}
	return 0
	*/
	uint32 ctime=0;
	vector<srtsub>::iterator foss=subs.begin(); //first on screen sub
/*  list<struct srtsub>::iterator loss=foss; //last on screen sub
	while((loss != subs.end()) && (loss->end <= ctime)){
		loss++;
	}*/
	TextImage frame;
	TextFont font;
	TextPal pal;

	//fprintf(stderr, "aarg\n");
	while(!feof(ifhandle)){
		fread(frame.data, 8000, 1, ifhandle);
		fread(&pal, 48, 1, ifhandle);

		//fprintf(stderr, "pre blit - ");
		if(foss!=subs.end()){ //if not all subs done
			//fprintf(stderr, "blit-");
			blitSRT(&frame, foss, align, margins, ctime, bestcontrast(&pal));
			//fprintf(stderr, "ting - ");
		}
		//fprintf(stderr, "post blit\n");
/*
		frame.data[0]=uint16('oh');
		frame.data[1]=uint16(' i');
		frame.data[2]=uint16('la');
		frame.data[3]=uint16('el');
		frame.data[4]=uint16('am');
		frame.data[5]=uint16('la');
	*/
		fwrite(frame.data, 8000, 1, ofhandle);
		fwrite(&pal, 48, 1, ofhandle);
//    fprintf(stderr,"ctime=%u, foss.start=%u, foss.end=%u\n",ctime,foss->start, foss->end);
		ctime+=50; //0.05s == 1/20s
//    fprintf(stderr,"ctime=%u, foss.start=%u, foss.end=%u\n",ctime,foss->start, foss->end);
		while(foss->end <= ctime){
		foss++;
		}
//    fprintf(stderr,"ctime=%u, foss.start=%u, foss.end=%u\n\n",ctime,foss->start, foss->end);
	}

	if (ifname != "-") fclose(ifhandle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}

