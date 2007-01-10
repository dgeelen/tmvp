//---------------------------------------------------------------------------

#ifndef UCmdLineParserH
#define UCmdLineParserH
#include "UTypes.h"
#include <string>
#include <vector>
#include <map>

typedef unsigned int uint; // TODO: include type header

enum AType {
	AT_STRING,
	AT_INT,
	AT_BOOL,
	AT_STRARR,
};

struct CmdLineOption {
	char shortname;  //optional   (\0)
	std::string longname;
	void *args;
	uint numargs;
	uint numparsed;
	AType argtype;
	void* def;
	std::string desc;
};

/* Should we also handle stuff like this?
 * '|'   -- indicates that the option takes no argument;
 * '?'   -- indicates that the option takes an optional argument;
 * ':'   -- indicates that the option takes a required argument;
 * '*'   -- indicates that the option takes zero or more arguments;
 * '+'   -- indicates that the option takes one or more arguments;
 */
/* And what about this:
	* Prompting the user for missing arguments.
	* Allowing keywords (-count=4) and/or options (-c4).
	* Ignoring bad syntax instead of terminating.
	* Ignoring upper/lower case on the command-line.
	* Suppressing the printing of syntax error messages.
	* Controlling the verboseness of usage messages.
	* Controlling whether or not options may be processed after positional parameters have been seen.
	*/

/**
 * What I think we *must* support:
 *  - No argument
 *    > normal boolean
 *    > inverted boolean
 *    > toggle boolean
 *  - Single mandatory argument
 *    ex: '--color green' '-c green' '-cgreen' '-abc green' '-abcgreen'
 *  - use '=' as argument specifier
 *    ex: '--max-count=1'
 *    note: we problably only want this to work with longoptions ( 'grep -m=1' doesnt work )
 *  - Single optional argument
 *    ex: '--color --otheroption' '--color -o' '-c green' '-c -o' '-co??' (what to do with last one?)
 *    note: needs 2 default values, 1 for when option isnt present and 1 for if its present without argument
 *           or needs extra bool variable indicating if it was present...
 *    note: we could also limit optional arguments to only work with the '=' delimiter (grep does this)
 *          'grep --color=always' sets color to always (color has 1 optional arg)
 *          'grep --color always' sets color to default, and uses always as positional arg (pattern or file)
 *          'grep --max-count 1' sets max count to 1 (maxcount has 1 mandatory arg)
 *  - Custom prefixes, like using '-' instead of '--' as option prefix if no shorthands are needed (mplayer does this)
 *    or even '' so everything gets interpretted as an option (dd does this?)
 *  - keywords (-count=4 / -c4 dd of=10) -> du uses the former (`du -h --max-depth=1') while grep uses the latter (`grep -iC5 bla *')
 *    note: what grep does is just shorthand option +argument
 *    note: what we really want is to use '=' as argument specifier
 *
 * Possibilities we might want with lists(vectors) and stuff:
 *  - Fixed amount of mandatory arguments
 *    ex: '--rgb 64 64 64' '-c 64 64 64' ?'-c64 64 64' ?'--rgb=64 64 64'
 *  - variable amount of optional arguments
 *    note: we might not want this, or do we?
 *    ex: '--files file1 file2 --otheroption' '--files file1 --otheroption'
 *  - 'Cumulative' arguments
 *    ex: '-f blaat -f fliep -f floep' results in vector with 3 items
 *    nots: this would then also be used for positional 'non-options'
 *    note: for string concat may be usefull, or do we let app handle this?
 *  - 'override' certain variables for cumulative arguments
 *    so '-f arg1 -f arg2 -f arg3 -f arg4' could put arg1 in 'var1' arg2 in 'var2' and arg3,arg4 in 'restvect' or so...
 *    note: do we want to be able to override the 'last' in the list? cp/mv use this but might be
 *          too unhandy to implement so let the app handle this?
 *
 * What we should *not* support:
 *  - zero or more arguments   -> Can be implemented by issuing multiple optional arguments, as such:
 *                                ./dummy -f blaat -f fliep -f floep -f -
 *                                and specifying that you want this option to concatenate the results.
 *                                Would be different for int, bool, and string (addition/xor/(concat|vector))
 * - one or more arguments     -> Same as zero or more arguments, replace `optional' with `mandatory'
 * - Ignoring upper/lower case -> teh 3v!L
 * - 'Positional parameters'   -> Can be parsed by application as 'Cumulative' arguments (above)
 *  - Conflicting arguments (svn cleanup --force for example)
 *    note: IMHO this is too application specific for us to handle... let the app do these checks
 *
 * What we *might* support:
 * - Prompting the user for missing arguments -> This could be a nice extension, but may interfere with piping
 *
 */

class CmdLineParser {
public:
	CmdLineParser();
	~CmdLineParser();
	template <typename T>
	T AddOption(T* ptr, char shortname, std::string longname, int anum, AType atype, T def, std::string desc);
	template <typename T>
	std::vector<T> AddArray(std::vector<T>* ptr, char shortname, std::string longname, int anum, AType atype, T def, std::string desc);
	int AddAlias(std::string from, std::string to);

	bool parse(int argc, char* argv[]); // returns array of string of unparsed args
	bool parse(std::vector<std::string> vargv); // returns array of string of unparsed args
private:
	std::map<std::string, struct CmdLineOption> cmdopts;
	uint32 posargcount;
//	uint32 baka;
//	uint32 parsedcount;
};


template <typename T>
T CmdLineParser::AddOption(T* ptr, char shortname, std::string longname, int anum, AType atype, T def, std::string desc) { //template must reside in the same file
	struct CmdLineOption cmdopt;
	cmdopt.shortname = shortname;
	cmdopt.longname = longname;
	cmdopt.args = ptr;
	cmdopt.numargs = anum;
	cmdopt.argtype = atype;
	cmdopt.def = &def;
	cmdopt.desc = desc;
	cmdopts["--"+longname] = cmdopt;
	//if(s+shortname==s) { fprintf(stderr,"EMPTY STRING ERROR\n"); } // yes I knew it was a always false condition, but Just In Case TM
  fprintf(stderr,"AddOption(): Adding string `%s' / '%c'\n", longname.c_str(), shortname);
	cmdopts[std::string::basic_string() + shortname] = cmdopt;
	return def;
};

template <typename T>
std::vector<T> CmdLineParser::AddArray(std::vector<T>* ptr, char shortname, std::string longname, int anum, AType atype, T def, std::string desc)
{
	struct CmdLineOption cmdopt;
	cmdopt.shortname = shortname;
	cmdopt.longname = longname;
	cmdopt.args = ptr;
	cmdopt.numargs = anum;
	cmdopt.argtype = atype;
	cmdopt.def = &def;
	cmdopt.desc = desc;
	cmdopt.numparsed = 0;
	cmdopts["--"+longname] = cmdopt;
	//if(s+shortname==s) { fprintf(stderr,"EMPTY STRING ERROR\n"); }
  fprintf(stderr,"AddArray(): Adding string `%s' / '%c'\n", longname.c_str(), shortname);
	cmdopts[std::string::basic_string() + shortname] = cmdopt;
	std::vector<T> val(anum, def);
	return val;
}


// macro magic!
#define OPT_START(parsername) \
	CmdLineParser parsername; \
	namespace TN1_ ## parsername { \
		CmdLineParser& parser = parsername; \
		\
		namespace TN2_ ## parsername { \
		\

#define OPT_END(parsername) \
		}; \
	}; \
	\
	using namespace TN1_ ## parsername::TN2_ ## parsername; \
	\

#define OPT_ALIAS(from, to)\
	namespace TN1_GeneralAliasParserNamespace { \
		namespace { \
			int dummy = parser.AddAlias(from, to); \
		}; \
	}; \


#define OPT_GENERAL(atype, vtype, vname, ochr, ostr, anum, def, desc)\
	vtype vname = parser.AddOption(&vname, ochr, ostr, anum, atype, (vtype)def, desc)

#define OPT_STRING(vname, ochr, ostr, anum, def, desc)\
	OPT_GENERAL(AT_STRING, std::string, vname, ochr, ostr, anum, def, desc)

#define OPT_INT(vname, ochr, ostr, anum, def, desc)\
	OPT_GENERAL(AT_INT, int, vname, ochr, ostr, anum, def, desc)

#define OPT_BOOL(vname, ochr, ostr, anum, def, desc)\
	OPT_GENERAL(AT_BOOL, bool, vname, ochr, ostr, anum, def, desc)


#define OPT_GARR(atype, vtype, vname, ochr, ostr, anum, def, desc)\
	std::vector< vtype > vname(parser.AddArray(&vname, ochr, ostr, anum, atype, (vtype)def, desc))

#define OPT_STRARR(vname, ochr, ostr, anum, def, desc)\
	OPT_GARR(AT_STRARR, std::string, vname, ochr, ostr, anum, def, desc)

#endif
