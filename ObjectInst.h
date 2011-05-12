#ifndef __OBJECTINST_H__
#define __OBJECTINST_H__
#include <string>
#include "Class.h"
#ifdef WIN32
using namespace stdext;
#else
#include <ext/hash_map>
#endif

union vtype_t {
	char c;
	int i;
	float f;
	short st;
	long l;
	unsigned char uc;
	unsigned int ui;
	unsigned short ust;
	unsigned long ul;
	std::string* s;
} ;

class CObjectInst
{
public:
	
	void addRef(){
		ref ++;
	}

	int getRef(){
		return ref;
	}

	CObjectInst * getMemberAddress(char* name){
		return members[name];
	}
	void setValue(int type, void* value){
		switch(type){
			case dtInt:
			v.i = *(int*)value;
			break;
		
			case dtUInt:
			v.ui = *(unsigned int*)value;
			break;
			
			case dtShort:
			v.st = *(short*)value;
			break;
			
			case dtUShort:
			v.ust = *(unsigned short*)value;
			break;
			
			case dtLong:
			v.l = *(long*)value;
			break;
			
			case dtULong:
			v.ul = *(unsigned long*)value;
			break;
			
			case dtFloat:
			v.f = *(float*)value;
			break;
			
			case dtStr:
			vs = (char*)value;
			
			default:
			fprintf(stderr, "Cannot set object value type to %d", type);
			return;
		}
		valueType = type;
	}
	
	vtype_t getValue(){
		return v;
	}
	
	std::string getSValue(){
		std::string r="";
	
			switch(valueType){
			case dtInt:
			r += v.i;
			break;
		
			case dtUInt:
				r += v.ui;
			break;
			
			case dtShort:
				r += v.st;
			
			break;
			
			case dtUShort:
				r += v.ust;
				break;
			
			case dtLong:
				r += v.l;
			break;
			
			case dtULong:
				r += v.ul;
			break;
			
			case dtFloat:
				r += v.f;
			break;
			
			case dtStr:
				r += vs;
			
			default:
			fprintf(stderr, "Cannot convert object value type %d to string", valueType);
		
		}
		return r;
	
	}

	static CObjectInst* createObject(CClass* c){
		CObjectInst* r = NULL;
		r = new CObjectInst(0, c);
		return r;
	};
private:
	long id;
	CClass* cls;
	int ref;
	int valueType;
	vtype_t v;
	std::string vs;
	
#ifdef _MACOS
	hash_map<char*, CObjectInst*> members;
#else
	stdext::hash_map<std::string, CObjectInst*> members;
#endif


public:
	CObjectInst(long id, CClass* cls);

public:
	~CObjectInst(void);
};

#endif //__OBJECTINST_H__
