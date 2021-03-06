#ifndef __CP_UTIL_H
#define __CP_UTIL_H

 class CPUtil{
	public:
		long static typesize(long type, long id)
{
#ifdef _64
/* for 64bit system,
sizeof(long)=8
sizeof(long*)=8
sizeof(int)=4
sizeof(int*)=8
*/
	switch (type)
	{
	case dtChar:
	case dtUChar:
		return sizeof(char);break;
	case dtShort:
	case dtUShort:
		return sizeof(short);break;
	case dtInt:
	case dtUInt:		
	case dtLong:
	case dtULong:
	case dtFloat:
		return sizeof(long);break;
	case dtStr:
		return sizeof(void*);break; //it's a pointer
	case dtGeneral:		
		/*if (id == 0)
		{
			nLOG("OBJECT WITH ERROR ID", 300);
			return -1;
		}
		return m_ObjTable[id-1]->GetSize();*/
		return sizeof(void*);break;//it's a pointer
		break;

/*	case dtDouble:
		return 8;break;
*/	default: return -1;
	}
#else
	switch (type)
	{
	case dtChar:
	case dtUChar:
		return 1;break;
	case dtShort:
	case dtUShort:
		return 2;break;
	case dtInt:
	case dtUInt:		
	case dtLong:
	case dtULong:
	case dtFloat:
		return 4;break;
	case dtStr:
		return sizeof(void*);break; //it's a pointer
	case dtGeneral:		
		/*if (id == 0)
		{
			nLOG("OBJECT WITH ERROR ID", 300);
			return -1;
		}
		return m_ObjTable[id-1]->GetSize();*/
		return sizeof(void*);break;//it's a pointer
		break;

/*	case dtDouble:
		return 8;break;
*/	default: return -1;
	}
#endif
}

int static log2(int x)
{
	//printf("===>x=%d\n", x);
	int i = 0;
	while (x >= 2)
	{
		i++;
		x >>= 1;
	//	printf("->x=%d", x);
	}
	if (i >=0 && i<=3)
		return i;
	else
		return 0;
}

int static UnitSize(TYPEDES &type)
{	
	if (type.refLevel<=0)
		return typesize(type.type, type.objID);
	else
		return sizeof(long*);

}

// get address mode for command, not for exp stack
// type1 for dest oprand, typ2 for src oprand
int static getAddressMode2(long type1, long type2, TYPEDES& dt1, TYPEDES& dt2){
	int address_mode = (type1<<8)|(short)type2;		// get address mode ( address mode and ref level)
	address_mode |= (log2(UnitSize(dt1))<<14)|(log2(UnitSize(dt2))<<6); // get operand size
	debug("getAddressMode2:: type1=%x, type2=%x, return %x", type1, type2, address_mode);
	return address_mode;
}
// get address mode for command, not for exp stack
int static getAddressMode1(long type1, TYPEDES& dt1){
	int address_mode = (type1<<8);
	address_mode |= (log2(UnitSize(dt1))<<14);
	return address_mode;
}	

// get address mode for exp stack
int static getAddressModeForExpStack(long type, TYPEDES& dt){
	int address_mode = (type);
	address_mode |= (log2(UnitSize(dt))<<6);
	return address_mode;
}	
	
};

#endif
