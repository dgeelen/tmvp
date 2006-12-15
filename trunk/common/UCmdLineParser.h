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
 *  - Single mandatory argument
 *  - Single optional argument
 *  - keywords (-count=4 / -c4 dd of=10) -> du uses the former (`du -h --max-depth=1') while grep uses the latter (`grep -iC5 bla *')
 *  - Conflicting arguments (svn cleanup --force for example)
 *
 * What we should *not* support:
 *  - zero or more arguments   -> Can be implemented by issuing multiple optional arguments, as such:
 *                                ./dummy -f blaat -f fliep -f floep -f -
 *                                and specifying that you want this option to concatenate the results.
 *                                Would be different for int, bool, and string (addition/xor/(concat|vector))
 * - one or more arguments     -> Same as zero or more arguments, replace `optional' with `mandatory'
 * - Ignoring upper/lower case -> teh 3v!L
 * - 'Positional parameters'   -> Can be parsed by application as 'unparsed arguments'
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
	//if(s+shortname==s) { fprintf(stderr,"EMPTY STRING ERROR\n"); }
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
