#ifndef __OBJECTINST_H__
#define __OBJECTINST_H__
#include <string>
#include "Class.h"
#ifdef WIN32
using namespace stdext;
#else
#include <ext/hash_map>
#endif


typedef struct _eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
}EQFN_STR;
class CObjectInst;
class CAttribute;
typedef union vtype_t {
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
	CObjectInst * o;
	long fn;
	char* s;
} VTYPE;


class CRef;
class CObjectInst;
#ifdef _MACOS
// sgi 
typedef hash_map<char*, CRef*, hash<char*>, EQFN_STR> STR_HASHMAP;
typedef hash_map<char*, CRef*, hash<char*>, EQFN_STR>::iterator STR_HASHMAP_IT;
#else
typedef	stdext::hash_map<std::string, CObjectInst*> STR_HASHMAP;
typedef	stdext::hash_map<std::string, CObjectInst*>::iterator  STR_HASHMAP_IT;
#endif



class CAttribute{

public:
	
		CAttribute(){
			//isLeaf = true;
			s_temp = "";
			name = "";
				valueType = 0;	
			//v.s = &vs;
			v.s = (char*)s_temp.c_str();
			memset(&v, 0, sizeof(VTYPE));
			ref = 0;
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
			printf("==>setValue, type=%d, value=%d, dtUlong=%d, dtLong=%d, v=%x\n", type, *(long*)value, dtULong, dtLong, v.s);
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
			
			case dtFn:
			v.fn = *(long*)value;
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
			
			case dtGeneral:
			v.o = *(CObjectInst**)value;
			
			default:
			fprintf(stderr, "Cannot set object value type to %d\n", type);
			return;
		}
		valueType = type;
	}
	
	VTYPE getValue(){
		return v;
	}
	
	BYTE* getValueAddress(){
		return (BYTE*)&v;
	}
	
 	char* getSValue(){
		printf("type=%d, v.l=%d\n", valueType, v.l);
	//	printf("v.s=%s",v.s);
		char ss[1024]="";
		
		char* r = NULL;
			switch(valueType){
			case dtInt:
			snprintf(ss, 1000, "%d", v.i);s_temp = ss;r=(char*)s_temp.c_str();
			break;
		
			case dtUInt:
			snprintf(ss, 1000, "%d", v.ui);s_temp = ss;r=(char*)s_temp.c_str();
			break;
			
			case dtShort:
					snprintf(ss, 1000, "%d", v.st);s_temp = ss;	r=(char*)s_temp.c_str();		
			break;
			
			case dtUShort:
					snprintf(ss, 1000, "%d", v.ust);s_temp = ss;r=(char*)s_temp.c_str();
				break;
			
			case dtLong:
					snprintf(ss, 1000, "%d", v.l);s_temp = ss;r=(char*)s_temp.c_str();
			break;
			
			case dtULong:
					snprintf(ss, 1000, "%d", v.ul);s_temp = ss;r=(char*)s_temp.c_str();
			break;
			
			case dtFloat:
			snprintf(ss, 1000, "%d", v.f);s_temp = ss;r=(char*)s_temp.c_str();
			break;
			
			case dtStr:
				//r += vs;
				if (v.s != NULL){
				if (strlen(v.s)>1000){
					r=v.s;
					}else{
				snprintf(ss, 1000, "%s", v.s);s_temp = ss;r=(char*)s_temp.c_str();
			}
			}else{
				v.s = r = (char*)s_temp.c_str();
			}
			break;
			
			case dtGeneral:
					{
				const char* p  = (cls?(const char*)cls->GetFullName():"object");
				snprintf(ss, 1000, "%s@%d", id);
				s_temp = ss;r=(char*)s_temp.c_str();
				}
				break;
				
			default:
				fprintf(stderr, "Cannot convert object value type %d to string", valueType);
		
		}
		printf("get value of string %s", (char*)r);
		return r;
	
	}
	void addRef(){
		ref ++;
	}
	void subRef(){
		ref --;
	}
protected:
	int valueType;
	VTYPE v;
	//std::string vs;
	std::string name;
	long id;
	CClass* cls;
	std::string s_temp;
	// bool isLeaf;
	int ref;
};
class CRef {
public:
	CRef(){
		ref = NULL;
		this->name="";
	};
	CRef(char* name){
		ref = NULL;
		this->name=name;
	};
	~CRef();
	CAttribute* getRef(){
		return ref;
	}
	void setRef(CAttribute *p){
		release();
		ref = p;
		ref->addRef();
	}
	void release(){
		if (ref != NULL)
			ref->subRef();
	}
private:
	CAttribute* ref;
	std::string name;
	
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

	CRef*  getMemberRef(char* szName){
		CRef * r =  members[szName];
		if (r == NULL){
			r = addMember(szName);
		}
		printf("r=%x\n",r);
		return r;
	}
	/*
	CAttribute ** getMemberAddress(char* szName){
		printf("===>%s.getMemberAddress %s.\n", (char*)name.c_str(), szName);
		printf("====>member size=%d", members.size());
		CRef * r =  members[szName];
		printf("====>member size2=%d", members.size());
		printf("====>r=%x", r);
		if (r == NULL)
			r = addMember(szName);

		printf("====>getMemberAddress=%x", r);
		return &(r->getRef());
	}*/
	
	CRef* addMember(char* szName){
	
		// CAttribute *p = CObjectInst::createObject(NULL);
		members[szName] = new CRef(szName);
		return members[szName];
	}

	
/*	std::vector<CAttribute> asArray(){
		std::vector<CAttribute> v;
		STR_HASHMAP_IT it = members.begin();
		while(it!=members.end()) {
			CAttribute *attr = 	(CAttribute*)it->second;
			CAttribute a = *attr;
			v.push_back(a);
		}
	}
	
*/

	static CObjectInst* createObject(CClass* c){
		// TODO should put object to global list, for gc
		CObjectInst* r = NULL;
		r = new CObjectInst(0, c);
		return r;
	};
private:

	


	STR_HASHMAP members;


public:
	CObjectInst(long id, CClass* cls);

public:
	~CObjectInst(void);
};

#endif //__OBJECTINST_H__
