#ifndef __CS_H__
#define __CS_H__


class CCompiler;
class CPubFuncTable;
class CVirtualMachine;
class CConfigure;
class CS{
public:
	
	CCompiler *c;
	CVirtualMachine *vm;
	//CPubFuncTable *g_PubFuncTable;
	
public:
	CS();
	~CS();
	
	void setConf(CConfigure& conf);	
	CConfigure* getConf();	
	void setOutput(FILE* file);	
	CCompiler* getCompiler();
	
	void executeFile(std::string s);
	void execFunction(std::string s, void* p = NULL);
	void loadobj(std::string s, void* p = NULL);

};
#endif
