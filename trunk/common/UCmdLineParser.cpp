
#include "UCmdLineParser.h"
#include <string>
#include <map>
#include <vector>

using namespace std;
/** PUBLIC **/

CmdLineParser::CmdLineParser() {
}

vector<string>* VectorizeArgs(uint32 argc, char *argv[]) { //Converts an array of char* strings to vector<string> 'array'
	vector<string> *v = new vector<string>;
	for(uint32 i=0; i<argc; i++) {
		string s=argv[i];
		v->push_back(s);
		}
	return v;
	}

vector<string>* CmdLineParser::parse(int argc, char* argv[]){ // returns array of string of unparsed args
	vector<string>* vargv = VectorizeArgs(argc, argv);
	for(uint32 i=0; i< argc; i++) {
		fprintf(stderr,"current argument: %s\n", (*vargv)[i].c_str());
		if(cmdopts.count((*vargv)[i])>0) { //if this is a long-option name
			fprintf(stderr, "  Found long option: `%s'\n",(*vargv)[i].c_str());
			}
		else { //either a short or non option
			if((*vargv)[i].size() > 1 && (*vargv)[i][0]=='-' && (*vargv)[i][1]!='-') { // (list of) short options
				fprintf(stderr, "  Found short option list: `%s'\n",(*vargv)[i].c_str());
				string s= std::string::basic_string();
				for(uint32 j=1; j<((*vargv)[i]).size(); j++){
					fprintf(stderr,"    Possible option: `%c'\n",((*vargv)[i])[j]);
					if( cmdopts.count( s+((*vargv)[i])[j] ) > 0) {
						fprintf(stderr,"      Is an valid option (longname == `%s')\n",cmdopts[s+((*vargv)[i])[j]].longname.c_str());
					}
					else {
						fprintf(stderr,"      Is an INVALID option\n");
					}
				}
			}
			else {
				fprintf(stderr, "  Found non option: `%s'\n",(*vargv)[i].c_str());
			}
		}
	}
	delete vargv;
	return vargv;
/*/BWEH, let's make better use of our <map> then this below ...

	fprintf(stderr,"argc=%i\n", argc);
	bool parseError=false;
	const map<string, struct CmdLineOption>::iterator end=cmdopts.end();
	for(map<string, struct CmdLineOption>::iterator i = cmdopts.begin(); i!=end; i++){
		fprintf(stderr,"Parsing option -%c / --%s : %s\n",i->second.shortname, i->second.longname.c_str(),i->second.desc.c_str());
		for(uint32 j = 1; j<argc; j++) {
			// loop through all arguments, looking for (argv[j][0]=='-' /\ ((argv[j][1]=='-' /\ (A_k : k>2 : argv[j][k]==i.longname[k]))
			//                                                           \/ (argv[j][1]!='-' /\ argv[j][2]==i.shortname}))
			bool match = false;
			if(argv[j][0]=='-') {
				if(argv[j][1]=='-') { // Long option
					if(argv[j][2]==0) { // option only consists of '--', aka 'end of arglist'
						j=argc;
						}
					char *k=&argv[j][2];
					const char *l=i->second.longname.c_str();
					match=true;
					while(match && *k!=0 && *l!=0) {
						match= (*k==*l) && match;
						k++;
						l++;
						}
					match=match&&(*k==*l);
					}
				else {                // Short option(s)
					fprintf(stderr,"Parsing short opts\n");
					char *k=&argv[j][1];
					const char l=i->second.shortname;
					while(*k!=0 && !match){
						fprintf(stderr,"match=%s, k=%c, l=%c\n", match?"true":"false", *k, l);
						match=match || *k==l;
						k++;
					}
					if(match && i->second.numargs!=0 && *k!=0) { // invalid use of a short option with argument
						match=false;
						parseError=true;
					}
				}
			}
			if(match) { //found a good match, assign Var
				fprintf(stderr,"found a match!\n");
				if(i->second.argtype == AT_BOOL) { // bool variables shall not have arguments
					*(bool*)(i->second.args)=true;
				}
				if(i->second.argtype == AT_INT) {
					fprintf(stderr,"ATOI\n");
					if(j<argc){                     // There is an argument
						*(int*)(i->second.args)=atoi(argv[j+1]);
						}
					else {
						parseError=true;
						}
				}
			}
			if(parseError){
				break;
			}
		}
		if(parseError){
			break;
		}
	}
	if(parseError){
		fprintf(stderr,"Parsing aborted due to parseError!\n");
		return NULL;
	}
	else {
		return NULL;
	}
*/
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
