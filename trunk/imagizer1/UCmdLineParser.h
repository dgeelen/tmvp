//---------------------------------------------------------------------------

#ifndef UCmdLineParserH
#define UCmdLineParserH

#include <string>

typedef unsigned int uint; // TODO: include type header

enum AType {
	AT_STRING,
	AT_INT,
	AT_BOOL,
};

struct CmdLineOption {
	char shortname;  //optional   (\0)
	std::string name;
	void *args;
	uint numargs;
	AType argtype;
	void* def;
	std::string desc;
};

class CmdLineParser {
public:
	CmdLineParser();
	template <class T>
	T AddOption(T* ptr, char ochr, std::string ostr, int anum, AType atype, T def, std::string desc) { return def; };
//	int AddOption(int* ptr, char ochr, std::string ostr, int anum, AType atype, int def, std::string desc) { return def; };
//	std::string AddOption(std::string* ptr, char ochr, std::string ostr, int anum, AType atype, std::string def, std::string desc) { return def; };
};


// macro magic!
#define OPT_START(parsername) \
	namespace TN1_ ## parsername { \
		CmdLineParser parser; \
	}; \
	\
	namespace TN2_ ## parsername { \
		using namespace TN1_ ## parsername; \
		namespace TN3_ ## parsername { \

#define OPT_END(parsername) \
		} \
	}; \
	\
	using namespace TN2_ ## parsername::TN3_ ## parsername; \
	\
	CmdLineParser& parsername = TN1_ ## parsername::parser \

#define OPT_GENERAL(atype, vtype, vname, ochr, ostr, anum, def, desc)\
	vtype vname = parser.AddOption(&vname, ochr, ostr, anum, atype, (vtype)def, desc)

#define OPT_STRING(vname, ochr, ostr, anum, def, desc)\
	OPT_GENERAL(AT_STRING, std::string, vname, ochr, ostr, anum, def, desc)

#define OPT_INT(vname, ochr, ostr, anum, def, desc)\
	OPT_GENERAL(AT_INT, int, vname, ochr, ostr, anum, def, desc)

#define OPT_BOOL(vname, ochr, ostr, anum, def, desc)\
	OPT_GENERAL(AT_BOOL, bool, vname, ochr, ostr, anum, def, desc)

#endif
