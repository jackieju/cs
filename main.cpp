#include "clib.h"
#include "stdio.h"
#include "cocoR/cp.hpp"
#include "cocoR/CR_ERROR.hpp"
#include "compiler.h"
#include "findfile.h"
#include "mem.h"
#include "VirtualMachine.h"
#include "os/osutils.h"
#include "Configure.h"
#include "cscript.h"
#include "thread.h"
#include <signal.h>

CConfigure conf;
//CCompiler c;
//CVirtualMachine vm;
//CPubFuncTable g_PubFuncTable;

///////////////////////////////////////
// function for external lib function
///////////////////////////////////////
CS* cs;
bool b_exit = false;
	char* target = NULL;
bool applyOption(char** argv, int number){
	if (number <= 0)
		return true;
	char* p = argv[0];
	// printf("arg=%s, number=%d\n",p, number);
	if (p[0]=='-'){
		if (strcmp(p+1, "sp")==0){
			printf("set classpath %s\n", argv[1]);

			 if (number-1 <= 0) // will eat two arguments
			 		return false;
             else{
				conf.set("classpath", argv[1]);
//                printf("set classpath to %s\n", conf.get("classpath").c_str());
             }
			return applyOption(argv+2, number-2);
		}else if(strcmp(p+1, "d")==0){
			  conf.set("debug","yes");
			  return applyOption(argv+1, number-1);
		}else{
			printf("unknow option %s", p);
			return false;
		}
	}else{
		target = argv[0];
		return applyOption(argv+1, number-1);
	}
}
/*struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};
*/

void doAbort(int i){
	fprintf(stderr, "abort ...\n");
	cs->execFunction("test::onExit");
}

void doTerminate(int sig){
	fprintf(stderr, "terminate ...\n");
	cs->execFunction("test::onExit");

	return;
}

void sighandler(int arg){
		fprintf(stderr, "handle sig %d ...\n", arg);
	cs->execFunction("test::onExit");	
		b_exit = true;
	//exit(9); 
	//abort();
	//raise(9);
		fprintf(stderr, "run function %s ok!\n", "test::okexit");
}
extern long g_nThreadNum;
int main(int num, char** args){
//	signal(SIGABRT, &doAbort);//If program aborts go to assigned function "doAbort".
  //  signal(SIGTERM, &doTerminate);//If program terminates go to assigned function "doTerminate".
	signal(SIGINT, &sighandler);

	// test std::string
/*	std::string r ="";
	r += "1111";
	printf("%s\n", (char*)r.c_str());return 0;*/
	// test hash_map
/*	hash_map<char*, char*, hash<char*>, eqstr> members;
	char p1[10] = "hello";
	char p2[10] =  "hello";

	char* p3 = "hello";
	members[p1]="1";
	members[p2]="2";
	printf("test hash_map, size=%d\n", members.size());
	printf("%s=%s\n", p1, members[p1]);
	printf("%s=%s\n", p2, members[p2]);
	return 0;*/
/*	union vtype_t {
	char c;
	int i;
	float f;
	short st;
	long l;
	unsigned char uc;
	unsigned int ui;
	unsigned short ust;
	unsigned long ul;
//	std::string* s;
	long fn;
	char* s;
} v;
unsigned long a = 90;
unsigned long* aa = &a;
v.ul = *(unsigned long*)aa;
printf("%d\n", v.l);
return 0;*/
#if 0

	//c.setOutput("compile_err.txt");

	conf.set("debug","yes");
	conf.set("classpath", "../");

	c.setConf(conf);
	BOOL ret = c.Compile("test/test.cs");
	//ret = c.Compile("D:\\studio\\projects\\webmud\\mud\\MudOS\\MudLib\\xkx2001\\d\\city\\dongmen.c");
	printf("compile ret=%d\n", ret);

#ifdef WIN32
//	g_PubFuncTable.LoadLib("baselib.dll", "baselib.int");
#else
//	g_PubFuncTable.LoadLib("libbaselib.so", "../baselib.int");
#endif
	CCompiler::classDesTable.getClass("test\\test");
//	vm.LoadObject(CCompiler::classTable.getClass("test\\test"));
	//int index = 0;
	//CFunction* pfn = CCompiler::classTable.getClass("test")->getFuncTable()->GetFunction("create", &index);
	//vm.LoadFunction(pfn);
//	vm.Run();
	//std::vector<std::string>* files = findfile("*", "D:\\studio\\projects\\webmud\\mud\\MudOS\\MudLib\\xkx2001\\d\\baituo");

	//for (int i =0; i< files->size(); i++){
	//	printf("%s\r\n", ((*files)[i]).c_str());
	//	c.Compile((char*) ((*files)[i]).c_str());
	//}
	//Delete(files);
	//getchar();
#endif
	if (num == 1 || !applyOption(args+1, num-1) || target == NULL){
	
		printf("New C Script 0.0.1 Deveopled by jackie.ju@gmail.com\n");
		printf("New C Script is C-style programming script language.\n");
		printf("Usage: mse {options} (class_name)\n");
		printf("Options:\n");
		printf("-sp \tsource path separated with ':'\n e.g. -sp /usr/local/mse:/root/mse\n");
		printf("-d  \ttoggle debug mode\n");
		printf("www.cs-soft.com\n");
		return 0;
	}
	
	bool bOption = false;

	
//	LOG("FDASDF");
	

//		LOG("dffff");

	cs = new CS();
	
    cs->setOutput(stdout);
    //conf.set("debug","yes");
  //  conf.set("classpath", "/Users/juweihua/studio/projects/WebMudFramework/ScriptEngine/mse/lib;/Users/juweihua/studio/projects/WebMudFramework/ScriptEngine/mse;/Users/juweihua/studio/projects/WebMudFramework/ScriptEngine/mse/test");
  
cs->setConf(conf);
		std::map<std::string, std::string>::iterator it;

//		std::map<std::string, std::string> & _map1 = *(conf.map());

	// for ( it=_map.begin() ; it != _map.end(); it++ ){
	// 	 
	// 	 	printf("%s=%s\n", (*it).first.c_str(), (*it).second.c_str());
	// 		}	
	// 

               printf("set classpath to %s\n", conf.get("classpath").c_str());

	CConfigure* m_conf = (cs->getConf());
		std::map<std::string, std::string> &  _map = *(m_conf->map());
  printf("set classpath to %s\n", m_conf->get("classpath").c_str());
	 printf("\n[Compiler options]:\n");
	
		for ( it=_map.begin() ; it != _map.end(); it++ ){
		 	printf("%s=%s\n", (*it).first.c_str(), (*it).second.c_str());
		 }
		 printf("\n");
		 printf("Classpath:\n");
		std::vector<std::string>& class_path = cs->getCompiler()->getClassPath();
		int i =0;
			for (i = 0;i<class_path.size(); i++){
				printf("\t%s\n", class_path[i].c_str());
		
		}
		 printf("\n");
			
//	cs->loadobj("test/test");
//		printf("target = %s %d\n", target,  OSstricmp((const char*) &target[strlen(target)-3] ,".cs" ));
	if (strlen(target)>3 && OSstricmp((const char*) &target[strlen(target)-3] ,".cs" ) == 0)
		cs->executeFile(target);
	else if (strlen(target)>2 && strchr(target, ':') != NULL)
		cs->execFunction(target);
	else
		cs->loadobj(target);
	
	while (g_nThreadNum>0 && !b_exit){
		JUJU::sleep(100);
	}
	
	fprintf(stderr, "main exited\n");
}


