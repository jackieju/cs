// psstdlib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "exp.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "baselib_version.h"
#include "baselib.h"

#ifdef _MACOS
#define __stdcall 
#endif
extern "C"{
	
	void __stdcall test(){
		printf("=== baselib test ===\n");
	}
	void __stdcall test0(){
	
	}
	void __stdcall test1(long l){
		printf("=== baselib test ===\n");
		printf("%d\n", l);
	    printf("====================\n");	
	}
void __stdcall put_str(char* msg)
{
	if (msg == NULL)
		return;
	printf("*******[%s]******\n", msg);
	printf("\n");
}


void __stdcall putl(long l)
{
	printf("*******[%ld]*******\n", l);
	return;
}

void __stdcall putf(float f)
{
	printf("%f", f);
	return;
}
void __stdcall put(char* format, long data)
{
	printf(format, data);
	return;
}
/*
// string manipulation
char* __stdcall strcpy(char* dest, char* src)
{
	if (dest == NULL || src == NULL)
		throw new CExp(CExp::err_invalid_param, "invalid param", CExp::stat_null);
	return strcpy(dest, src);
}*/


float __stdcall AToF(char* string)
{
	float ret = 0;
	if (string == NULL || strlen(string) == 0)
	{
		throw new CExp(CExp::err_invalid_param, "invalid param", CExp::stat_null);
		//return ret;
	}
	ret = (float)atof(string);
	return ret;
}

long __stdcall testre(){
	return 92;
}
testa tt;
long __stdcall creatett(){
	return (long)new testa();
}
long __stdcall testtt(testa* tt){
	tt->go();
}
long __stdcall test3p(char* p1, char* p2, char * p3){
	printf("********** p1=%s, p2=%s, p3=%s *******", p1, p2, p3);
	tt.go();
	
}
}

