// #include <stdio.h>
// int main(void) {
//   printf("Content-Type: text/plain;charset=us-ascii\n\n");
//   printf("Hello world\n\n");
//   return 0;
// }
// 
#include "stdio.h"
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
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

CConfigure  conf;
 CCompiler c;
 CVirtualMachine vm;
 CPubFuncTable g_PubFuncTable;
CS* cs;
unsigned int htoi(char s[])
{
    unsigned int val = 0;
    int x = 0;

    if(s[x] == '0' && (s[x+1]=='x' || s[x+1]=='X')) x+=2;

    while(s[x]!='\0')
    {
       if(val > UINT_MAX) return 0;
       else if(s[x] >= '0' && s[x] <='9')
       {
          val = val * 16 + s[x] - '0';
       }
       else if(s[x]>='A' && s[x] <='F')
       {
          val = val * 16 + s[x] - 'A' + 10;
       }
       else if(s[x]>='a' && s[x] <='f')
       {
          val = val * 16 + s[x] - 'a' + 10;
       }
       else return 0;

       x++;
    }
    return val;
}

void init_cs(){
	cs = new CS();
	cs->setOutput(stdout);
	conf.set("debug","yes");
	char* search_path = getenv("CS_PATH");
	debug("search_path=%s\n", search_path);
//conf.set("classpath", "/Users/juweihua/studio/projects/WebMudFramework/ScriptEngine/mse/lib;/Users/juweihua/studio/projects/WebMudFramework/ScriptEngine/mse");
	conf.set("classpath", "./:lib");
	cs->setConf(conf);
}

string exec_cmd(string user, string cmd, std::map<string, string> p){

		

	string r = "hello";
	// load user
	
	// get cmd
	
	cLOG("11");
	// send cmd to user
	// user::onCommand(command name, command param)
#if 0
	// test
 	BOOL ret = c.Compile("test/test.cs");
	// if (!ret){
	// 	printf("==== compiler error ===\n");
	// 	return "==== compiler error ===\n";
	// }
		
//	CCompiler::classDesTable.dump();
	CClassDes* pc = CCompiler::classDesTable.getClass("test/test");
//	printf("this=%x,==>ps=%x", &CCompiler::classDesTable, pc);
	vm.LoadObject(pc);
#endif

	cs->loadobj("test");
	
	cLOG("111");
	return r;
}


int main()
{
	// send header, there must not any printf before this
	printf("Content-Type: text/plain\n\n");
  	printf("Hello world\n\n");
	JUJU::CLog::setFile("/Users/juweihua/studio/projects/jsf/cs/cgi-bin/log");
/*	FILE* file = fopen("/Users/juweihua/studio/projects/jsf/cs/cgi-bin/log", "a+");
	fprintf(file, "=====hello====\n");
	fclose(file);
	cLOG("1");*/
	JUJU::CLog::enableStdOut(false);

//	debug("a=%s, b=%s", "aa", "bb");

    int i,n;
	printf("CONTENT-LENGTH: %s\n", getenv("CONTENT-LENGTH"));
	printf("QUERY_STRING: %s\n", getenv("QUERY_STRING"));
	printf("REQUEST_METHOD: %s\n", getenv("REQUEST_METHOD"));
	char* method = getenv("REQUEST_METHOD");
	char* content_length = getenv("CONTENT-LENGTH");
	char* query_string = getenv("QUERY_STRING");
// parser request
	n=0;
    char qs[2000] ="";
    char* pqs = qs;

// parse post data
    if(stricmp(method, "POST") == 0 && getenv("CONTENT-LENGTH") ){
/*	if (content_length != NULL)
    	n=atoi(getenv("CONTENT-LENGTH"));
	else if (query_string){
		n = strlen(query_string);
	}
    printf("content-length:%d\n",n);
*/
    for (i=0; i<n;i++){
        //int is-eq=0;
        char c=getchar();
//printf("%c", c);
        switch (c){
            case '&':
                *pqs='\n';
                pqs++;
                break;
            case '+':
                *pqs=' ';
                pqs++;
                break;
            case '%':{
                char s[3] = "";
                s[0]=getchar();
                s[1]=getchar();
                s[2]=0;
                *pqs=htoi(s);
                i+=2;
                pqs++;
                }
                break;
//case '=':
//c=':';
//is-eq=1;
//break;
            default:
            *pqs=c;
            pqs++;
        };
    }
    }
//printf("%s\n",getenv("QUERY_STRING"));
printf("qs=%s\n",qs);

try{
	init_cs();
	map<string, string> m;
	//object->command(param), return
	exec_cmd("test", "cmd", m);
}catch(...){
	return 0;	
}

	cLOG("2");


//fprintf(stdout, "=======================\n");

fflush(stdout);
SAFEDELETE(cs);
return 0;
}
