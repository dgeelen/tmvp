
#include "UCmdLineParser.h"

CmdLineParser::CmdLineParser()
{
}


//-----rest is testing stuff----

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
