/*** 
 * Read OLD_CODE
	 output OLD_CODE
	 CHARACTER = OLD_CODE
	 WHILE there are still input characters DO
		 Read NEW_CODE
		 IF NEW_CODE is not in the translation table THEN
			 STRING = get translation of OLD_CODE
			 STRING = STRING+CHARACTER
		 ELSE
			 STRING = get translation of NEW_CODE
		 END of IF
		 output STRING
		 CHARACTER = first character in STRING
		 add OLD_CODE + CHARACTER to the translation table
		 OLD_CODE = NEW_CODE
	 END of WHILE                                        *
										                                 ***/
#include <iostream>
#include <string>
#include <map>
using std::string;
using std::map;

#define CODE_MAX 0xEFFF
map<short int,string> dict;
unsigned long int ncodes;
unsigned long int oldcode;
unsigned long int newcode;
unsigned long int character;
string unlzw_s="";

bool init_unlzw() {
	dict.clear();
	ncodes=256;
	for(unsigned long int i=0 ; i<256 ; i++) {
		string s= "a";
		s[0]=(char)i;
		dict[i]=s;
		}
	oldcode=0xffff;
	newcode=0xffff;
	character=oldcode;
	unlzw_s="";
	return true;
	}

int main(int argc, char *argv[]) {
	int cr=0;
	init_unlzw();
	if(argc>2) {
		FILE *ip = fopen(argv[1], "rb");
		FILE *op = fopen(argv[2], "wb");
restart:
		cr=fread(&oldcode,1,2,ip);
		newcode=oldcode;
		if(cr==0){
			fprintf(stderr,"File empty!\n");
			fclose(ip);
			fclose(op);
			return EXIT_FAILURE;
			}
		fwrite(dict[oldcode].data(),1,dict[oldcode].size(),op);
//    fprintf(stderr, "first code=%04x, which is `%s' in the table\n", oldcode, dict[oldcode].c_str());
		while(!feof(ip)){
			cr=fread(&newcode,1,2,ip);
			if(cr==0){
				fprintf(stderr,"End of file!\n");
				fclose(ip);
				fclose(op);
				return EXIT_SUCCESS;
				}
			if(newcode==0xFFFE) {
				fprintf(stderr, "resetting decompressor\n");
				init_unlzw();
				goto restart;
				}
//      fprintf(stderr,"read %04x,",newcode);
			if(dict.count(newcode) == 0 ) {
//        fprintf(stderr, "not in table\n");
				unlzw_s = dict[oldcode];
				unlzw_s = unlzw_s + unlzw_s[0];
				fwrite(unlzw_s.data(), 1/*unlzw_s.size_type*/, unlzw_s.size(), op );        
				if(ncodes < CODE_MAX) {
					dict[ncodes++]=unlzw_s;
					}
				oldcode=newcode;
				}
			else { // Does exist in table
				unlzw_s = dict[newcode];
//        fprintf(stderr,"which is `%s' in the table (length %i)\n",unlzw_s.c_str(),unlzw_s.size());
//        fwrite(unlzw_s.data(), 1/*unlzw_s.size_type*/, unlzw_s.size(), stderr); fprintf(stderr,"\n");
				fwrite(unlzw_s.data(), 1/*unlzw_s.size_type*/, unlzw_s.size(), op );
				unlzw_s = dict[oldcode] + unlzw_s[0];
				if(ncodes < CODE_MAX) {
					dict[ncodes++]=unlzw_s;
					}
				oldcode=newcode;
				}
			}
		fclose(ip);
		fclose(op);
		}
	}
	//*/
	
