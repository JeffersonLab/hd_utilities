

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <stack>
using namespace std;

#include <sys/stat.h>
#include <time.h>
#include <expat.h>
#include <string.h>

#include "DClassDef.h"

string XML_FILENAME = "/home/davidl/work/latest/sim-recon/src/libraries/HDDM/event.xml";
string HDDM_CLASS = "x";
string CXXFLAGS = ""; // first and last character should be space (or empty string)
//string CXXFLAGS = " -pg -g ";

map<string, DClassDef> CLASSES;
unsigned int MAX_DEPTH=0;
unsigned int TARGET_DEPTH=1;

void startElement(void *data, const char *el, const char **attr);
void endElement(void *data, const char *el);
void CreateCopyRoutines(void);
void CreateHDDM2ROOT_tool(void);
string NameToHDDMname(string name);
string HDDMPlural(string name_hddm);
void Usage(void);
void ParseCommandLineArguments(int narg, char *argv[]);


//--------------
// main
//--------------
int main(int narg, char *argv[])
{
	// Parse Command line args
	ParseCommandLineArguments(narg, argv);

	// Parse XML file to get class definitions
	stack<string> ancestory;

	XML_Parser parser = XML_ParserCreate(NULL);
	XML_SetUserData(parser, &ancestory);
	XML_SetElementHandler(parser, startElement, endElement);

	ifstream ifs(XML_FILENAME.c_str());

	// get length of file:
	ifs.seekg (0, ios::end);
	int length = ifs.tellg();
	ifs.seekg (0, ios::beg);
	
	if(length > 102400)length = 102400; // limit size to 100kB in case this is hddm file

	char *buff = new char[length];
	ifs.read (buff,length);
	ifs.close();
	
	// Find terminating "</HDDM>" string and shorten buffer to this
	char *end = strstr(buff, "</HDDM>");
	if(end){
		end[strlen("</HDDM>")] = 0;
		length = strlen(buff);
	}
	
	// Create sub-directory for all source just to keep things a little cleaner
	mkdir("hddm_root_generated", S_IRWXU | S_IRWXG | S_IRWXO);

	// Write hddm XML definition to file
	ofstream hddmxmlofs("hddm_root_generated/hddm_def.xml");
	if(hddmxmlofs.is_open()){
		hddmxmlofs << buff;
		hddmxmlofs.close();
		
		// If we were able to write the XML file, then use it to
		// generate the I/O routines in our source directory. This
		// is preferred since it will guarantee compatibility with
		// this file. In case we don't get here, the tool will need
		// to compile using I/O routines from sim-recon.
		mkdir("hddm_root_generated/HDDM", S_IRWXU | S_IRWXG | S_IRWXO);
		system("cd hddm_root_generated/HDDM ; hddm-c ../hddm_def.xml");
		system("cd hddm_root_generated/HDDM ; hddm-cpp ../hddm_def.xml");
		// sed -e '/\/HDDM/,$d'
	}

	int done = 0;
	XML_Parse(parser, buff, length, done);

	delete[] buff;


	// Create a header file for each class. This is needed to make
	// the ROOT dictionaries (cint doesn't seem to like it when they
	// are all in the same file).
	
	// Print class definitions
	// Here we must print them in reverse-depth order so the classes
	// that show up as members of others are defined first.
	map<string, DClassDef>::iterator iter;
	for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
		string name = iter->first;
		string type = iter->second.name;
		map<string,string> &members = iter->second.members;

		//if(iter->second.depth != depth)continue; // printing in reverse-depth order
		if(name == "HDDM_t")continue;  // Skip outer HDDM tags
		
		// Open output file
		string fname = string("hddm_root_generated/") + name + ".h";
		cout<<"writing "<<fname<<endl;
		ofstream ofs(fname.c_str());

		// Write header to file
		ofs << "#include<vector>" << endl;
		ofs << "using namespace std;" << endl;
		ofs << endl;
		ofs << "#include<Rtypes.h>" << endl;
		ofs << "#include<TObject.h>" << endl;
		ofs << endl;
		
		ofs << "#ifndef _" << name << "_"<<endl;
		ofs << "#define _" << name << "_"<<endl;
		
		ofs << "typedef int Particle_t;" << endl;
		ofs << endl;
		
		// Add relevant includes for other classes this depends on
		set<string> &include_types = iter->second.include_types;
		set<string>::iterator iter_inc;
		for(iter_inc=include_types.begin(); iter_inc!=include_types.end(); iter_inc++){
			ofs << "#include \"" << *iter_inc << "_t.h\""<<endl;
		}
		ofs << endl;
		
		// Write class definition to file
		ofs<<"class " << name << ":public TObject{" << endl;
		ofs<<"	public:"<<endl;
		ofs<<endl;
		
		map<string,string>::iterator iter2;
		for(iter2=members.begin(); iter2!=members.end(); iter2++){
			ofs << "		" << iter2->second << " " << iter2->first << ";" << endl;
		}
		ofs<<endl;
		ofs<<"		ClassDef("<<name<<",1)"<<endl;
		ofs<<"};"<<endl;
		ofs<<endl;

		ofs << "#endif" << endl;
		
		// Close file
		ofs.close();
	}
	
	// Create script that will generate ROOT dictionaries for each header file
	ofstream ofs("hddm_root_generated/makefile");
	ofs << endl << endl;
	ofs << "all: dictionaries copy_routines tool" << endl;
	ofs << endl;
	ofs << "dictionaries: *.h"<<endl;
	for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
		string name = iter->first;
		if(name == "HDDM_t")continue;  // Skip outer HDDM tags

		string cmd = "	rootcint -f " + name + "_Dict.cc -c " + name + ".h";
		ofs << cmd << endl;
	}

	for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
		string name = iter->first;
		if(name == "HDDM_t")continue;  // Skip outer HDDM tags

		string cmd = "	c++ `root-config --cflags` -c " + name + "_Dict.cc" + CXXFLAGS;
		ofs << cmd << endl;
	}
	ofs << "	ar -r libhddm_root.a *_t_Dict.o" << endl;
	ofs << endl;
	ofs << endl;
	
	ofs << "copy_routines: hddm_root_CopyRoutines.cc hddm_root_CopyRoutines.h" << endl;
	string cmd = "	c++ `root-config --cflags` -c -I${HALLD_RECON_HOME}/${BMS_OSNAME}/include -I. hddm_root_CopyRoutines.cc" + CXXFLAGS;
	ofs << cmd << endl;
	ofs << "	ar -r libhddm_root.a hddm_root_CopyRoutines.o" << endl;
	ofs << endl;

	ofs << "tool: hddm2root_*.cc" << endl;
	cmd = "	c++ `root-config --cflags --libs` -I${HALLD_RECON_HOME}/${BMS_OSNAME}/include -I. hddm2root_" + HDDM_CLASS + ".cc -o hddm2root_" + HDDM_CLASS + CXXFLAGS + " ./libhddm_root.a -lbz2 -lz -L${HALLD_RECON_HOME}/${BMS_OSNAME}/lib -lHDDM -lxstream";
	ofs << cmd << endl;
	ofs << endl;
	ofs << endl;

	ofs.close();
	
	// Create Copy Routines
	CreateCopyRoutines();
	
	// Create main tool file
	CreateHDDM2ROOT_tool();

	// Notify user of final step
	cout<<endl;
	cout<<"Code generation complete. Issue the following"<<endl;
	cout<<"to build the hddm2root_"<<HDDM_CLASS<<" tool:"<<endl;
	cout<<endl;
	cout<<"  make -C hddm_root_generated"<<endl;
	cout<<endl;
	cout<<"The tool will be left as hddm_root_generated/hddm2root_"<<HDDM_CLASS<<endl;
	cout<<endl;
	
	return 0;
}

//--------------
// startElement
//--------------
void startElement(void *data, const char *el, const char **attr)
{
	// Setup reference to our ancestory stack
	stack<string> &ancestory = *(stack<string>*)data;
	
	// Convert element tag to string
	string name(el);
	
	// Loop over attributes, temporarily storing them into map
	map<string, string> members;
	bool is_unbounded = false;
	for (int i = 0; attr[i]; i += 2) {
		string name(attr[i]);
		string type(attr[i+1]);
		
		// Some attributes are HDDM-specific
		if(name == "class") HDDM_CLASS = type;
		if(name == "minOccurs")continue;
		if(name == "version")continue;
		if(name == "maxOccurs"){
			if(type == "unbounded"){
				is_unbounded = true;
				continue;
			}
			if(atoi(type.c_str())>1){
				is_unbounded = true;
				continue;
			}
			continue;
		}
		
		if(type.find("GeV")== 0 )continue; // skip Eunit and punit stuff
		if(type == "rad")continue;
		if(type == "ns")continue;
		if(type == "cm")continue;
		
		if(type=="boolean") type = "bool"; // (sigh...)
		
		// Add member to list
		members[name] = type;
	}

	
	// If there is a parent, add this element as a member
	if(!ancestory.empty()){
		if(is_unbounded){
			CLASSES[ancestory.top()].members[name+"s"] = string("vector<") + name + "_t>";
		}else{
			CLASSES[ancestory.top()].members[name] = name + "_t";
		}
		CLASSES[ancestory.top()].include_types.insert(name);
	}

	// Get reference to class definition, creating it if necessary
	DClassDef &class_def = CLASSES[name + "_t"];
	class_def.members.insert(members.begin(), members.end());
	class_def.depth = ancestory.size();
	if(class_def.depth > MAX_DEPTH) MAX_DEPTH = class_def.depth;

	ancestory.push(name + "_t");
}

//--------------
// endElement
//--------------
void endElement(void *data, const char *el)
{
	stack<string> &ancestory = *(stack<string>*)data;
	
	ancestory.pop();
}

//--------------
// CreateCopyRoutines
//--------------
void CreateCopyRoutines(void)
{
	// Open header file to write into
	cout << "Writing hddm_root_generated/hddm_root_CopyRoutines.h" << endl;
	ofstream ofs("hddm_root_generated/hddm_root_CopyRoutines.h");

	// Add file header
	time_t t = time(NULL);
	ofs << "//" << endl;
	ofs << "// Auto-generated HDDM to ROOT copy routines DO NOT EDIT" << endl;
	ofs << "//" << endl;
	ofs << "// "<<ctime(&t);
	ofs << "// Generated from: "<<XML_FILENAME<<endl;
	ofs << "//" << endl;
	ofs << endl;
	
	ofs << "#define Particle_t Particle_tt"<<endl;
	map<string, DClassDef>::iterator iter;
	for(unsigned int depth=0; depth<MAX_DEPTH; depth++){
		for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
			if(iter->second.depth != depth) continue;
			if(iter->first == "HDDM_t")continue;
			ofs << "#include \"" << iter->first << ".h\"" << endl;
		}
	}
	ofs << "#undef Particle_t" << endl;
	ofs << endl;
	ofs << endl;
	
	ofs << "#include <HDDM/hddm_"<<HDDM_CLASS<<".hpp>"<<endl;
	ofs << "using namespace hddm_"<<HDDM_CLASS<<";"<<endl;
	ofs << endl;

	// Copy Routine declarations
	for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
		string name = iter->first;
		string varname = name.substr(0, name.length()-2);
		string name_hddm = NameToHDDMname(varname);

		if(varname == "HDDM")continue;
		
		ofs << "void Copy"<<name_hddm<<"("<<name<<" &"<<varname<<", class "<<name_hddm<<" &"<<varname<<"_hddm, bool hddm_invalid=false);"<<endl;
	}
	ofs << endl;
	
	// Clear Routine declarations
	for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
		string name = iter->first;
		string varname = name.substr(0, name.length()-2);
		string name_hddm = NameToHDDMname(varname);

		if(varname == "HDDM")continue;
		
		ofs << "void Clear"<<name_hddm<<"("<<name<<" &"<<varname<<");"<<endl;
	}
	ofs << endl;
	ofs << endl;
	ofs.close();  // header file
	
	// Open implementation file to write into
	cout << "Writing hddm_root_generated/hddm_root_CopyRoutines.cc" << endl;
	ofs.open("hddm_root_generated/hddm_root_CopyRoutines.cc");
	
	// Add file header
	ofs << "//" << endl;
	ofs << "// Auto-generated HDDM to ROOT copy routines DO NOT EDIT" << endl;
	ofs << "//" << endl;
	ofs << "// "<<ctime(&t);
	ofs << "// Generated from: "<<XML_FILENAME<<endl;
	ofs << "//" << endl;
	ofs << endl;
	ofs << "#include \"hddm_root_CopyRoutines.h\"" << endl;
	ofs << endl;
	ofs << endl;

	// Copy Routines
	for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
		string name = iter->first;
		string varname = name.substr(0, name.length()-2);
		string name_hddm = NameToHDDMname(varname);
		string name_hddm_plural = HDDMPlural(name_hddm);
		map<string, string> &members = iter->second.members;
		
		if(varname == "HDDM")continue;
		
		ofs << "void Copy"<<name_hddm<<"("<<name<<" &"<<varname<<", class "<<name_hddm<<" &"<<varname<<"_hddm, bool hddm_invalid)"<<endl;
		ofs << "{"<<endl;
		ofs << "	if(hddm_invalid)return; // FIXME!!!" << endl;
		ofs << endl;
		
		map<string, string>::iterator iter_members;
		for(iter_members=members.begin(); iter_members!=members.end(); iter_members++){
			string myname = iter_members->first;
			string mytype = iter_members->second;
			string myvarname = myname;
			string myname_hddm = NameToHDDMname(myvarname);
			
			if(   mytype == "int"
			   || mytype == "float"
			   || mytype == "bool"
			   || mytype == "Particle_t"
			   || mytype == "string"){
			   
			   // Atomic type uses equals
				ofs << "	" << varname<<"."<<myvarname<<" = "<<varname<<"_hddm.get"<<myname_hddm<<"();" << endl;
			}else if(mytype.find("vector<") == 0){
				// STL vector requires iteration
				mytype = myname.substr(0, myname.length()-1); // chop off "s" that was added
				myname_hddm = NameToHDDMname(mytype);
				string myname_hddm_plural = HDDMPlural(mytype);
				string iter_name = "iter_" + mytype;
				ofs << endl;
				//ofs << "--- skipping "<<mytype<<":"<<myname<<":"<<myname_hddm_plural<<endl;
				ofs << "	"<<myname_hddm<<"List &"<<myname_hddm_plural<<" = "<<varname<<"_hddm.get"<<HDDMPlural(myname_hddm)<<"();"  <<endl;
				ofs << "	"<<myname_hddm<<"List::iterator "<<iter_name<<";"<<endl;
				ofs << "	for("<<iter_name<<"="<<myname_hddm_plural<<".begin(); "<<iter_name<<"!="<<myname_hddm_plural<<".end(); "<<iter_name<<"++){"<<endl;
				ofs << "		"<<mytype<<"_t a;"<<endl;
				ofs << "		Copy"<<myname_hddm<<"(a, *"<<iter_name<<");"<<endl;
				ofs << "		"<<varname<<"."<<myname<<".push_back(a);"<<endl;
				ofs << "	}"<<endl;
			}else{
				// Non-vector calls copy routine
				string myname_hddm_plural = HDDMPlural(myname_hddm);
				if(myname_hddm_plural == "Properties") myname_hddm_plural = "PropertiesList";
				ofs << "	Copy"<<myname_hddm<<"("<<varname<<"."<<myvarname<<", "<<varname<<"_hddm.get"<<myname_hddm<<"(), "<<varname<<"_hddm.get"<< myname_hddm_plural<< "().empty());" << endl;
			}
		}
		
		ofs << "}" << endl;
		ofs << endl;
	}
	ofs << endl;
	ofs << endl;

	// Clear Routines
	for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
		string name = iter->first;
		string varname = name.substr(0, name.length()-2);
		string name_hddm = NameToHDDMname(varname);
		string name_hddm_plural = HDDMPlural(name_hddm);
		map<string, string> &members = iter->second.members;
		
		if(varname == "HDDM")continue;
		
		ofs << "void Clear"<<name_hddm<<"("<<name<<" &"<<varname<<")"<<endl;
		ofs << "{"<<endl;
		ofs << endl;
		
		map<string, string>::iterator iter_members;
		for(iter_members=members.begin(); iter_members!=members.end(); iter_members++){
			string myname = iter_members->first;
			string mytype = iter_members->second;
			string myvarname = myname;
			string myname_hddm = NameToHDDMname(myvarname);
			
			if(mytype == "int" || mytype == "Particle_t"){
				ofs << "	" << varname<<"."<<myvarname<<" = 0;" << endl;
			}else if(mytype == "float"){
				ofs << "	" << varname<<"."<<myvarname<<" = 0.0;" << endl;
			}else if(mytype == "string"){
				ofs << "	" << varname<<"."<<myvarname<<" = \"\";" << endl;
			}else if(mytype == "bool"){
				ofs << "	" << varname<<"."<<myvarname<<" = false;" << endl;
			}else if(mytype.find("vector<") == 0){
				ofs << "	" << varname<<"."<<myname<<".clear();"<< endl;
			}else{
				// Non-vector calls copy routine
				string myname_hddm_plural = HDDMPlural(myname_hddm);
				if(myname_hddm_plural == "Properties") myname_hddm_plural = "PropertiesList";
				ofs << "	Clear"<<myname_hddm<<"("<<varname<<"."<<myvarname<<");" << endl;
			}
		}
		
		ofs << "}" << endl;
		ofs << endl;
	}
	ofs << endl;
	ofs << endl;
	
	ofs.close();
}

//--------------
// CreateHDDM2ROOT_tool
//--------------
void CreateHDDM2ROOT_tool(void)
{
	
	// Open implementation file to write into
	string fname = string("hddm_root_generated/hddm2root_") + HDDM_CLASS + ".cc";
	ofstream ofs(fname.c_str());
	cout << "Writing " << fname << endl;
	
	// Add file header
	time_t t = time(NULL);
	ofs << "//" << endl;
	ofs << "// Auto-generated HDDM to ROOT converion tool for class " << HDDM_CLASS << endl;
	ofs << "//" << endl;
	ofs << "// "<<ctime(&t);
	ofs << "// Generated from: "<<XML_FILENAME<<endl;
	ofs << "//" << endl;
	ofs << endl;
	ofs << endl;

	// Add headers
	ofs << "#include <stdlib.h>" << endl;
	ofs << "#include <fstream>" << endl;
	ofs << "#include <string>" << endl;
	ofs << "#include <TTree.h>" << endl;
	ofs << "#include <TFile.h>" << endl;
	ofs << "using namespace std;" << endl;
	ofs << endl;
	ofs << "#include \"hddm_root_CopyRoutines.h\"" << endl;
	ofs << endl;
	ofs << endl;
	
	// Add main
	ofs << "int main(int narg, char *argv[])" << endl;
	ofs << "{" << endl;
	ofs << endl;
	ofs << "	string usage = \"Usage:\\n\\n   hddm2root_"<<HDDM_CLASS<<" [-n nevents] [-o outfile.root] file.{xml|hddm}\";" << endl;
	ofs << endl;
	ofs << "	if(narg<2){cout << endl << usage << endl << endl; return -1;}"<<endl;
	ofs << endl;
	ofs << "	// Parse command line arguments" << endl;
	ofs << "	int MAX_EVENTS = 0;"<<endl;
	ofs << "	string ofname=\"hddm2root_" << HDDM_CLASS << ".root\";"<<endl;
	ofs << "	string fname=\"hdgeant_smeared.hddm\";"<<endl;
	ofs << "	for(int i=1; i<narg; i++){" << endl;
	ofs << "		string arg = argv[i];" << endl;
	ofs << "		if(arg==\"-n\"){" << endl;
	ofs << "			if(++i<narg){" << endl;
	ofs << "				MAX_EVENTS = atoi(argv[i]);" << endl;
	ofs << "			}else{" << endl;
	ofs << "				cerr<<\"-n option requires an argument!\"<<endl;" << endl;
	ofs << "				return -1;" << endl;
	ofs << "			}" << endl;
	ofs << "		}else if(arg==\"-o\"){" << endl;
	ofs << "			if(++i<narg){" << endl;
	ofs << "				ofname = argv[i];" << endl;
	ofs << "			}else{" << endl;
	ofs << "				cerr<<\"-o option requires an argument!\"<<endl;" << endl;
	ofs << "				return -2;" << endl;
	ofs << "			}" << endl;
	ofs << "		}else if(arg==\"-h\"){" << endl;
	ofs << "			cout << endl << usage << endl << endl;" << endl;
	ofs << "			return 0;" << endl;
	ofs << "		}else{" << endl;
	ofs << "			fname = argv[i];" << endl;
	ofs << "		}" << endl;
	ofs << "	}" << endl;
	ofs << endl;
	ofs << "	TFile *f = new TFile(ofname.c_str(), \"RECREATE\");" << endl;
	ofs << "	TTree *t = new TTree(\"T\", \"A Tree\", 99);" << endl;
	ofs << endl;

	// Create variable for each depth=TARGET_DEPTH class
	char branchname[2] = "A";
	map<string, DClassDef>::iterator iter;
	for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
		if(iter->second.depth!=TARGET_DEPTH)continue;
		string name = iter->first;
		string varname = name.substr(0, name.length()-2);

		ofs << "	" << name << " *"<<varname << " = new " << name << "();" << endl;
		ofs << "	t->Branch(\""<<branchname<<"\", &"<<varname<<");"<<endl;
		
		branchname[0]++; // sketchy trick to name branches A,B,C, ....
	}
	ofs << endl;	
	ofs << "	// Open hddm file for reading" << endl;
	ofs << "	ifstream ifs(fname.c_str());" << endl;
	ofs << endl;
	ofs << "	// Associate input file stream with HDDM record" << endl;
	ofs << "	hddm_"<<HDDM_CLASS<<"::istream istr(ifs);" << endl;
	ofs << endl;
	ofs << "	// Loop over events" << endl;
	ofs << "	unsigned int N = 0;"<<endl;
	ofs << "	while(!ifs.eof()){" << endl;
	ofs << "		try{" << endl;
	ofs << "			HDDM xrec;" << endl;
	ofs << "			istr >> xrec;"<<endl;

	// Call copy routine for each depth=TARGET_DEPTH class
	for(iter=CLASSES.begin(); iter!=CLASSES.end(); iter++){
		if(iter->second.depth!=TARGET_DEPTH)continue;
		string name = iter->first;
		string varname = name.substr(0, name.length()-2);
		string name_hddm = NameToHDDMname(varname);
		
		ofs << "			Clear" << name_hddm << "(*"<<varname<<");"<<endl;
		ofs << "			Copy" << name_hddm << "(*"<<varname<<", xrec.get"<<name_hddm<<"());"<<endl;
	}
	ofs << endl;
	ofs << "			t->Fill();" << endl;
	ofs << "			if(++N%10 == 0){cout<<\" \"<<N<<\" events processed    \\r\"; cout.flush();}"<<endl;
	ofs << "			if(MAX_EVENTS>0 && N>=MAX_EVENTS)break;"<<endl;
	ofs << "		}catch(...){" << endl;
	ofs << "			break;" << endl;
	ofs << "		}" << endl;
	ofs << "	}" << endl;
	ofs << endl;
	ofs << "	f->Write();" << endl;
	ofs << "	f->Close();" << endl;
	ofs << "	delete f;" << endl;
	ofs << endl;
	ofs << "	cout<<endl<<N<<\" events processed total\"<<endl;" << endl;
	ofs << "	return 0;" << endl;
	ofs << "}" << endl;
	ofs << endl;
	
	ofs.close();
}

//--------------
// NameToHDDMname
//--------------
string NameToHDDMname(string name)
{
	// Convert our class name used for ROOT
	// into the HDDM class name
	string name_hddm = name;
	name_hddm[0] = toupper(name[0]);
	
	return name_hddm;
}

//--------------
// HDDMPlural
//--------------
string HDDMPlural(string name_hddm)
{
	// Re-create HDDM-style plurals

	if(name_hddm.size()<3) return name_hddm + "s";

	if(name_hddm.substr(name_hddm.length()-2, 2) == "ex"){
		name_hddm.erase(name_hddm.length()-2);
		name_hddm += "ices";
	}else if(name_hddm.substr(name_hddm.length()-4, 4) == "ties"){
		// do nothing
	}else if(name_hddm.substr(name_hddm.length()-2, 2) == "ty"){
		name_hddm.erase(name_hddm.length()-2);
		name_hddm += "ties";
	}else if(name_hddm.substr(name_hddm.length()-3, 3) == "tum"){
		name_hddm.erase(name_hddm.length()-3);
		name_hddm += "ta";
	}else if(name_hddm.substr(name_hddm.length()-1, 1) == "s"){
		name_hddm += "es";
	}else{
		name_hddm += "s";
	}
	
	return name_hddm;
}

//--------------
// Usage
//--------------
void Usage(void)
{
	cout<<endl;
	cout<<endl;
	cout<<"Usage:"<<endl;
	cout<<"      hddm2root [-depth d] filename"<<endl;
	cout<<endl;
	cout<<" Generate a tool for converting an HDDM formatted"<<endl;
	cout<<"file into a ROOT file. This does not convert the"<<endl;
	cout<<"file itself, but rather, generates a program than"<<endl;
	cout<<"can be used to convert a file."<<endl;
	cout<<endl;
	cout<<" The file specified by \"filename\" can be either"<<endl;
	cout<<"the original XML file, or an existing HDDM file."<<endl;
	cout<<"If the latter is used, the XML is extracted from"<<endl;
	cout<<"beginning of the file."<<endl;
	cout<<endl;
	cout<<" options:"<<endl;
	cout<<"   -depth d    Set the target depth for objects to"<<endl;
	cout<<"               be written to the ROOT file"<<endl;
	cout<<endl;
	cout<<"   -h          Print this Usage statement"<<endl;
	cout<<endl;
	cout<<endl;
	cout<<"There are several limitations to this system:"<<endl;
	cout<<endl;
	cout<<"1. ROOT does not allow nested containers more"<<endl;
	cout<<"   than a few levels deep. In other words, you can"<<endl;
	cout<<"   have a container of containers, but not a"<<endl;
	cout<<"   container of containers of containers .... For"<<endl;
	cout<<"   practical purposes, this means any tag in "<<endl;
	cout<<"   XML with more than 2 parents having maxOccurs"<<endl;
	cout<<"   set to \"unbounded\" will not be accessible in"<<endl;
	cout<<"   the file. Use the -depth d option to try and"<<endl;
	cout<<"   get around this."<<endl;
	cout<<endl;
	cout<<"2. All tags with a maxOccurs=1 will be treated"<<endl;
	cout<<"   as though a single object of that type exists."<<endl;
	cout<<"   This means that the if there are zero objects"<<endl;
	cout<<"   of that type in the event, values will be written"<<endl;
	cout<<"   anyway, not saving any disk space. The values"<<endl;
	cout<<"   will be filled with zeros and empty strings"<<endl;
	cout<<"   (false for bool types)."<<endl;
	cout<<endl;   
	cout<<"3. Top-level objects can only have single objects."<<endl;
	cout<<"   In other words, if you specify a depth where an"<<endl;
	cout<<"   object at that depth is \"unbounded\", the tool"<<endl;
	cout<<"   will not try and loop over all instances of the"<<endl;
	cout<<"   object type, just the first may be taken."<<endl;
	cout<<endl;
	cout<<"e.g."<<endl;
	cout<<"    > hddm2root $HALLD_RECON_HOME/src/libraries/HDDM/rest.xml"<<endl;
	cout<<"    > hddm2root_r file.hddm"<<endl;
	cout<<endl;
	
	exit(0);
}

//--------------
// ParseCommandLineArguments
//--------------
void ParseCommandLineArguments(int narg, char *argv[])
{
	if(narg<2)Usage();

	for(int i=1; i<narg; i++){
		string arg = argv[i];
		
		if(arg.find("-depth") != arg.npos){
			if(++i >= narg){
				cerr<<endl<<" -depth requires and argument!"<<endl;
				exit(-1);
			}
			TARGET_DEPTH = atoi(argv[i]);
			continue;
		}
		
		if(arg.find("-h") != arg.npos){
			Usage();
		}
		
		XML_FILENAME = arg;
	}
}




