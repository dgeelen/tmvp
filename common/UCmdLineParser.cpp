
#include "UCmdLineParser.h"
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <stdlib.h>

using namespace std;
/** PUBLIC **/

CmdLineParser::CmdLineParser() {
	posargcount = 0;
}

bool CmdLineParser::parse(int argc, char *argv[]) { //Converts an array of char* strings to vector<string> 'array'
	vector<string> v;
	for(int i=0; i<argc; i++) {
		string s=argv[i];
		v.push_back(s);
	}
	return parse(v);
}

bool CmdLineParser::parse(const vector<string> vargv){ // returns array of string of unparsed args
	char cbuf[101];
	CmdLineOption* curopt;
	for(int i=0; i< vargv.size(); i++) {
//		fprintf(stderr,"current argument: %s\n", vargv[i].c_str());
		if(cmdopts.count(vargv[i])>0) { //if this is a long-option name
//			fprintf(stderr, "  Found long option: `%s'\n",vargv[i].c_str());
			curopt = &cmdopts[vargv[i]];
hopt:
			if (i + curopt->numargs >= vargv.size()) {
//				fprintf(stderr, "Fatal error: no argument found for option %s", vargv[i].c_str());
				return false;
			}

			for (int j = i + curopt->numargs; i < j; ++i) {
				switch (curopt->argtype) {
					case AT_STRING: {
						*((string*)(curopt->args)) = vargv[i + 1];
					}; break;
					case AT_INT: {
						*((int*)(curopt->args)) = atoi(vargv[i + 1].c_str());
					}; break;
					default: {

					}
				}
			}

			if (curopt->numargs == 0) {
				if (curopt->argtype == AT_BOOL) {
					*((bool*)(curopt->args)) = !*((bool*)(curopt->args));
				}
			}

		}
		else { //either a short or non option
			if(vargv[i].size() > 1 && vargv[i][0]=='-' && vargv[i][1]!='-') { // (list of) short options
//				fprintf(stderr, "  Found short option list: `%s'\n",vargv[i].c_str());
				string s= std::string::basic_string();
				for(uint32 j=1; j<(vargv[i]).size(); j++){
//					fprintf(stderr,"    Possible option: `%c'\n",(vargv[i])[j]);
					if( cmdopts.count( s+(vargv[i])[j] ) > 0) {
						curopt = &cmdopts[s+(vargv[i])[j]];
//						fprintf(stderr,"      Is an valid option (longname == `%s')\n",cmdopts[s+(vargv[i])[j]].longname.c_str());
						if (curopt->numargs == 0 || j+1 == (vargv[i]).size()) {
							goto hopt;
						}
					}
					else {
//						fprintf(stderr,"      Is an INVALID option\n");
					}
				}
			}
			else {
//				fprintf(stderr, "  Found non option: `%s'\n", vargv[i].c_str());
				snprintf(cbuf, 100, "----%i", posargcount++);
				if (cmdopts.count(cbuf) > 0) {
					curopt = &cmdopts[cbuf];
					--i;
					goto hopt;
				} //else if cmdopts.count((string)"--"
			}
		}
	}
	return true;
}


CmdLineParser::~CmdLineParser() {
}

/** PRIVATE **/


/*/-----rest is testing stuff----

OPT_START(myparser)
OPT_INT   (myintopt, 'o', "ooo", 1, 123        , "nice one");
OPT_STRING(mystropt, 'o', "ooo", 1, "defstring", "nice one");
OPT_END(myparser);

#include <stdio.h>

int main(int argc, char* argv[])
{
	//look myparser is in our current scope!
	myparser.AddOption((int*)NULL, '0', "", 0, AT_INT		, (int)0, "bladiebla");
	//myparse.parse(argc,argv) // or something...

 	// look our option vars are in current scope too
	printf("myintopt:%i\n", myintopt);
	printf("mystropt:%s	\n", mystropt.c_str());
	return 0;
}
/*/
