#include "ObjectInst.h"

CObjectInst::CObjectInst(long id, CClass* cls)
{
	this->id = id;
	this->cls = cls;
	ref = 0;

	// isLeaf = false;
}



/*CObjectInst::~CObjectInst(void)
{	
//	STR_HASHMAP_IT it = members.begin();
//	while(it!=STR_HASHMAP_IT.end()){
//		if (it->second)
//			delete (CRef*)(it->second);
//		it++;
//	}
}

*/
//CObjectInst* CObjectInst::createObject(CClass* c){
//	long id;
//	
//	CObjectInst * p = new CObjectInst(id, c);
//	
//	return p;
//}