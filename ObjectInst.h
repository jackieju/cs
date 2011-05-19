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
//	std::string* s;
	char* s;
} ;
class CAttribute{

public:
	
		CAttribute(){
			//isLeaf = true;
		};
		~CAttribute(){};
		CAttribute(const CAttribute& src){
			//this->name = src->getName();
			//setValue(src->getType(), &(src->getValue()));
			this->name = src.name;
			this->v = src.v;
			this->valueType = src.valueType;
		//	this->vs = src.vs;
		};
		void setName(char* name){
			this->name = name;
		}
		char* getName(){
			return (char*)name.c_str();
		}
		void setValue(int type, void* value){
			printf("==>setValue, type=%d, value=%d\n", type, *(long*)value);
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
			//vs = (char*)value;
			v.s = *(char**)value;
			printf("set value to string %s", *(char**) value);
			break;
			
			default:
			fprintf(stderr, "Cannot set object value type to %d\n", type);
			return;
		}
		valueType = type;
	}
	
	vtype_t getValue(){
		return v;
	}
	
	BYTE* getValueAddress(){
		return (BYTE*)&v;
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
				//r += vs;
			r += v.s;
			break;
			
		//	case dtGeneral:
				
			default:
				fprintf(stderr, "Cannot convert object value type %d to string", valueType);
		
		}
		printf("get value of string %s", (char*)r.c_str());
		return r;
	
	}
	
protected:
	int valueType;
	vtype_t v;
	//std::string vs;
	std::string name;
	
	// bool isLeaf;
};

class CObjectInst : public CAttribute
{
public:
	
	void addRef(){
		ref ++;
	}

	int getRef(){
		return ref;
	}

	CObjectInst * getMemberAddress(char* name){
		
		CObjectInst * r =  members[name];
		if (r == NULL)
			r = addMember(name);
		return r;
	}
	
	CObjectInst * addMember(char* name){
		CObjectInst *p = CObjectInst::createObject(NULL);
		members[name] = p;
		return p;
	}


	std::vector<CAttribute> asArray(){
		std::vector<CAttribute> v;
		hash_map<char*, CObjectInst*>::iterator it = members.begin();
		while(it!=members.end()) {
			CAttribute *attr = 	(CAttribute*)it->second;
			CAttribute a = *attr;
			v.push_back(a);
		}
	}

	static CObjectInst* createObject(CClass* c){
		// TODO should put object to global list, for gc
		CObjectInst* r = NULL;
		r = new CObjectInst(0, c);
		return r;
	};
private:
	long id;
	CClass* cls;
	int ref;


	
#ifdef _MACOS
	hash_map<char*, CObjectInst*> members;
#else
	stdext::hash_map<std::string, CAttribute*> members;
#endif


public:
	CObjectInst(long id, CClass* cls);

public:
	~CObjectInst(void);
};

#endif //__OBJECTINST_H__
