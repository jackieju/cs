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

void CS::execute(std::string s){
	CClassDes* pc = NULL;

	BOOL ret = c->Compile((char*)(s).c_str());
	if (!ret){
			printf("==== compile failed ===\n");
			return;
	}
	pc = CCompiler::classDesTable.getClass("_global_");
	printf("-->pc=%x",pc);


	if (pc == NULL){
		ERR1p("Cannnot load class %s", s.c_str());
		return;
	}
	vm->LoadObject(pc);
	return;
}

void CS::loadobj(std::string s){
	CClassDes* pc = NULL;
	pc = CCompiler::classDesTable.getClass((char*)(s.c_str()));
	if (pc == NULL){
		 //	BOOL ret = c.Compile("test/test.cs");
			BOOL ret = c->Compile((char*)(s+".cs").c_str());
			if (!ret){
					printf("==== compile failed ===\n");
			//		return "==== compiler error ===\n";
					return;
			}
			pc = CCompiler::classDesTable.getClass((char*)(s.c_str()));
			printf("-->pc=%x",pc);
	}
	
	if (pc == NULL){
		ERR1p("Cannnot load class %s", s.c_str());
		return;
	}
	vm->LoadObject(pc);
	return;
};
CS::CS(){

	vm = new CVirtualMachine();
	c = new CCompiler();
	CConfigure conf;
	conf.set("debug","yes");
	conf.set("classpath", ".");

	c->setConf(conf);
	
};
CS::~CS(){
	delete vm;
	delete c;
};

void CS::setConf(CConfigure& conf){
	c->setConf(conf);
};

CConfigure* CS::getConf(){
	return c->getConf();
};

void CS::setOutput(FILE* file){
	if (file)
		c->setOutput(file);
}

CCompiler* CS::getCompiler(){
	return c;
}
;