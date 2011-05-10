#ifndef __CS_H__
#define __CS_H__

class CS{
public:
	
	CCompiler c;
	CVirtualMachine vm;
	CPubFuncTable g_PubFuncTable;
	
public:
	CS(){
			CConfigure conf;
			conf.set("debug","yes");
			conf.set("classpath", ".");
			c.setConf(conf);
		};
	~CS(){};
	
	void setConf(CConfigure& conf){
		c.setConf(conf);
	};
	
	CConfigure* getConf(){
		return c.getConf();
	};
	
	void setOutput(FILE* file){
		if (file)
			c.setOutput(file);
	}
	
	CCompiler* getCompiler(){
		return &c;
	}
	

	void loadobj(std::string s);

};
#endif
