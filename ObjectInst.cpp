#include "ObjectInst.h"

CObjectInst::CObjectInst(long id, CClass* cls)
{
	this->id = id;
	this->cls = cls;
	ref = 0;
	valueType = 0;	
	v.s = &vs;
}


CObjectInst::~CObjectInst(void)
{
}


//CObjectInst* CObjectInst::createObject(CClass* c){
//	long id;
//	
//	CObjectInst * p = new CObjectInst(id, c);
//	
//	return p;
//}