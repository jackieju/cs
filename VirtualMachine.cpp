

/************************************
  REVISION LOG ENTRY
  Revision By: Weihua Ju
  Revised on 2003-5-15 15:36:18
  Comments: 
  fix a bug in GetStatus(), should use pCurCall, not m_pCurCall
 ************************************/


/************************************
  REVISION LOG ENTRY
  Revision By: Weihua Ju
  Revised on 2003-5-6 11:26:02
  Comments: 
  m_ToStop should be reset in Reset() function, or the machine will not run after the first run.
 ************************************/


/************************************
  REVISION LOG ENTRY
  Revision By: Weihua Ju
  Revised on 2003-4-21 12:26:16
  Comments: 
  In Run(), add try catch for call of _endcallpub;
  In _endcallpub, add code to safely exit befor throw up exception
 ************************************/


/************************************
  REVISION LOG ENTRY
  Revision By: Weihua Ju
  Revised on 2003-3-27 16:30:00
  Comments: 
  modify the AttachParam()
  when check size, the size_t can be greater than function specified size,
  but can not be less.
 ************************************/


/************************************
  REVISION LOG ENTRY
  Revision By: Weihua Ju
  Revised on 2003-3-11 17:49:25
  Comments: 
  chang the return type of CVirtualMachine::LoadFunction() and CVirtualMachine::_LoadFunc
  and AddExternalSpace from BOOL to void,
  and change the return type of CVirtualMachine::AttachParam() from BOOL to long
 ************************************/


/************************************
  REVISION LOG ENTRY
  Revision By: Weihua Ju
  Revised on 2003-3-11 17:49:25
  Comments: 
  modify the AttachParam()
  when check size, the size_t must be equel to function specification
 ************************************/




/************************************
  REVISION LOG ENTRY
  Revision By: Weihua Ju
  Revised on 2003-3-5 19:17:35
  Comments: 
  //	1. modify conctructor of CVirtualMachine, create m_Stopped with
  //  initial status false. If initial status is true, the HandleRequest
  //  thread will hang up on the CVirtualMachine::Reset() call in case 
  //  it before run(). 
************************************/


#include "clib.h"
#include "CSS_LOCKEX.h"
#include <valarray>
#include <stack>
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
//#include "io.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stdlib.h"
#include "Exp.h"
#include "Function.h"
#include "ScriptFuncTable.h"
#include "debug.h"
#include "VMException.h"
//#include "DOCommon.h"
#include "se_interface.h"
#include "ClassDes.h"
#include "ClassTable.h"
#include "ObjectInst.h"
#include "ObjTable.h"
#include "VirtualMachine.h"
#include "VMManager.h"


#include "compiler.h"
//#include "sym_engine.h"

long CVirtualMachine::m_lDbgCmdID = 0;
long CVirtualMachine::m_lDbgCmdP1 = 0;
long CVirtualMachine::m_lDbgCmdP2 = 0;
CSS_MUTEX CVirtualMachine::m_DgbCmdLock;
char CVirtualMachine::m_szDbgCmdP3[1024];
std::string CVirtualMachine::m_sDbgOutMsg;	// return msg of debug command execution
long CVirtualMachine::m_lDbgOutCode = 0;	// return code of debug command execution

#ifdef WIN32
/*
*	Use asm to call a function
*	_HGDispatchCall make call to pfn. pArgs is prealloced, which size
*  is (nSizeArgs + _SCRATCH_SIZE)
*
*/
extern "C"
{
	__declspec(naked) void __stdcall
		_HGDispatchCall(long* /*ret*/, void* /*pfn*/, void* /*pArgs*/, unsigned long /*nSizeArgs*/ )
	{
		_asm
		{
			pop     edx         // edx = return address
				//				pop		eax			// BE "return to" address, but not here actually(Instead, in (long*)pStack[0])
				pop     ebx         // edx = ret
				pop     eax         // eax = pfn
				pop     ecx         // ecx = pArgs
				add     ecx,[esp]   // ecx += nSizeArgs (=scratch area)
				mov     [ecx],edx   // scratch[0] = return address
				sub     ecx,[esp]   // ecx = pArgs (again)
				mov     esp,ecx     // esp = pArgs (usually already correct)
				call    eax         // call member function
				mov     [ebx], eax  //mov return value
				ret                 // esp[0] should = scratch[0] = return address
		}
	}
}
#else
#ifdef _MACOS
#ifdef _64
extern "C"
{
	
	 static	__inline__ __attribute__ ((naked)) void 
		_HGDispatchCall(long* /*ret*/, void* /*pfn*/, void* /*pArgs*/, unsigned long /*nSizeArgs*/ )
		  {
			  __asm__ __volatile__(
			  "popq %rdx\n\t"
			  "popq %rbx\n\t"
			  "popq %rax\n\t"
			  "popq %rcx\n\t"
			  "addq (%rsp),%rcx\n\t"
			 "movq %rdx, (%rcx)\n\t"
			  "subq (%rsp),%rcx\n\t"
				"movq %rcx,%rsp\n\t"			
				"call %rax\n\t"			
				"movq %rax,(%rbx)\n\t"
				"ret"
				);
	}
}
#else
extern "C"
{
	
	 static	__inline__ __attribute__ ((naked)) void __stdcall
		_HGDispatchCall(long* /*ret*/, void* /*pfn*/, void* /*pArgs*/, unsigned long /*nSizeArgs*/ )
		  {
			  __asm__ __volatile__(
			  "popl %edx\n\t"
			  "popl %ebx\n\t"
			  "popl %eax\n\t"
			  "popl %ecx\n\t"
			  "addl (%esp),%ecx\n\t"
			  "movl %edx, (%ecx)\n\t"
			  "subl (%esp),%ecx\n\t"
				"movl %ecx,%esp\n\t"			
				"call %eax\n\t"			
				"movl %eax,(%ebx)\n\t"
				"ret"
				);
	}
}
#endif //#ifdef _64
#else

extern "C"
{
	
	 static	__inline__ __attribute__ ((naked)) void __stdcall
		_HGDispatchCall(long* /*ret*/, void* /*pfn*/, void* /*pArgs*/, unsigned long /*nSizeArgs*/ )
		  {
			  __asm__ __volatile__(
			  "popl %edx\n\t"
			  "popl %ebx\n\t"
			  "popl %eax\n\t"
			  "popl %ecx\n\t"
			  "addl (%esp),%ecx\n\t"
			  "movl %edx, (%ecx)\n\t"
			  "subl (%esp),%ecx\n\t"
				"movl %ecx,%esp\n\t"			
				"call %eax\n\t"			
				"movl %eax,(%ebx)\n\t"
				"ret"
				);
	}
}
#endif //#ifdef _MACOS
#endif
void test1(long l){
	printf("-------------\n");
	printf("--->%d\n", l);
		printf("-------------\n");
}
BOOL __stdcall CallPubFunc(long lParamSize, void* pfn, void* pParam, long* pRet, int paramNum)
{
	LOG2p("====>call pub function %s(%lx) ...\n", CCompiler::m_PubFuncTable.GetFunction(CCompiler::m_PubFuncTable.FindFuncByAddress((long)pfn))->szName, CCompiler::m_PubFuncTable.FindFuncByAddress((long)pfn));

//	pfn = (void*)test1;
	//printf("==>pub functon lParamSize=%ld, pfn=%lx, pParam=%lx, pRet=%lx\n", lParamSize,pfn,pParam,pRet);
	// debug code
	//printf("CallPub: ParamBlockSize = %d, pData[0]: %x\n", lParamSize, *(long*)pParam);	
	int n = lParamSize % 16;

		void* p = alloca(lParamSize+n/*_SCRATCH_SIZE*/);
	//		printf("==>p=%lx\n", p);
	memset(p, 0, lParamSize+n);	
	
	memcpy(p, (void*)pParam, lParamSize);
printf("==>pub functon lParamSize=%ld, pfn=%lx, pParam=%lx, pRet=%lx, paramNum=%d\n", lParamSize,pfn,p,pRet,paramNum);

#ifdef WIN32
	_HGDispatchCall(pRet, (void*)pfn, p, lParamSize);
#else
#ifdef _MACOS
#ifdef _64
	long p1 = 0;
	long p2 = 0;
	long p3 = 0;
	long p4 = 0;
	long p5 = 0;
	long p6 = 0;
	long* pp = (long*)p;
	if (paramNum>=1)
	  p1 = *(long*)pp;
	if (paramNum>=2)
	  p2 = *(long*)(++pp);
	if (paramNum>=3)
	  p3 = *(long*)(++pp);
	if (paramNum>=4)
	  p4 = *(long*)(++pp);
	if (paramNum>=5)
	  p5 = *(long*)(++pp);
	if (paramNum>=6)
	  p6 = *(long*)(++pp);
	if (paramNum>=7){ // not allowed
	//	printf("paramNum=%d\n", paramNum);
	//	p = ++pp;
	//	lParamSize -= 48; // 64 bit OS 
	}
	else{
	//	lParamSize = 0;
		lParamSize+=n;
	}
lParamSize = 0;
	printf("===>CallPubFunc %d %x %d %d %d %d %d\n", lParamSize, p1, p2, p3, p4, p5, p6);
	//_HGDispatchCall(pRet, (void*)pfn, p, lParamSize);
	
			__asm__ __volatile__ (
			//"pushl	%0;"
			//"pushl	%1;"
			//"pushl	%2;"
			//"pushl	%3;"		
			
		//	"pushq %3;"
	//		"movq %%rsp, %%r12\n\t"
		// calling convention of am64 arch is at http://www.x86-64.org/documentation/abi.pdf
			"movq %4, %%rdi\n\t"	// p1					=>mov    %rsi,%rdi // %4 实际是rcx
			"movq %5, %%rsi\n\t"	// p2					=>mov    %r11,%rsi // %5 实际是r11
			"movq %2, %%r11\n\t"	// r11 = pfn			=>mov    %rdx,%r11 // 提到前面，因为%2就是rdx, rdx后面会被用掉	
			"movq %6, %%rdx\n\t"	// p3					=>mov    %r10,%rdx // %6 实际是r10
			"movq %3, %%r10\n\t"	// r10 = pRet			=>mov    %rcx,%r10 // 提前到最前，因为%3就是rcx, rcx后面会被用掉
			"movq %7, %%rcx\n\t"	// p4					=>mov    %r14,%rcx
			"movq %8, %%r8\n\t"		// p5					=>mov    %r12,%r8
			"movq %9, %%r9\n\t"		// p6					=>mov    %rbx,%r9		
		//	"movq %1, %%rcx\n\t"	// ecx = p 				=>mov    %r13,%rcx
		//	"movq %0, %%rdx\n\t"	// edx = lParamSize  	=>mov    %r15,%rdx
		
		//	"addq %0, %1;"			// ecx = p + lParamsize = Scrach erea	=>add    %rax,%r13
		//	"movq %%r10,(%1);"		// scrach =  address of return value;
		//	"subq %0, %1;"			// ecx = ecx - lParamsize = p;
		//	"movq %1, %%rsp\n\t"		// move stack point to p
			"pushq %%r10\n\t"
			"call %%r11\n\t"			// call function
		
			"movq (%%rsp), %%r10\n\t"
	//			"popq %%rsp;"
	//		"movq %%r12, %%rsp\n\t"
			"movq %%rax,(%%r10)\n\t"   //rax contains result, mov result value to address which is the value of r10
		//	"popq %%r10\n\t"
				: 	:"r"(lParamSize) , "r"(p) , "r"(pfn) , "r"(pRet), "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5), "r"(p6)
			);	
#endif // _64
#else
			__asm__ __volatile__ (
			//"pushl	%0;"
			//"pushl	%1;"
			//"pushl	%2;"
			//"pushl	%3;"			
			"movl %3, %%ebx;"	// edx = pRet
			"movl %2, %%eax;"	// eax = pfn
			"movl %1, %%ecx;"	// ecx = p
			"movl %0, %%edx;"	// edx = lParamSize
	//		"addl %%edx, %%ecx;"	// ecx = p + lParamsize = Scrach erea
	//		"movl %%edx,(%%ecx);"	// scrach = return address;
	//		"subl %%edx, %%ecx;"	// ecx = ecx - lParasize = p;
			"movl %%ecx,%%esp;"		// move stack point to p
			"call %%eax;"			// call function
			"movl %%eax,(%%ebx);"	: 	:"r"(lParamSize) , "r"(p) , "r"(pfn) , "r"(pRet)
			);	
#endif	// _MACOS
#endif  // WIN32

	LOG2p("====>call pub function %s(%lx) ok\n", CCompiler::m_PubFuncTable.GetFunction(CCompiler::m_PubFuncTable.FindFuncByAddress((long)pfn))->szName, CCompiler::m_PubFuncTable.FindFuncByAddress((long)pfn));
	return TRUE;
}
#include "opcode.h"

//////////////////////////////////////////////////////////////////////
// Consuction/Destruction
//////////////////////////////////////////////////////////////////////

CVirtualMachine::CVirtualMachine()
{
//	m_BreakPoint = NULL;
//	m_BpList.size() = 0;
	__AX = __BX = __CX = __DX = __IP = __PSW = 0;
//	IsRunning = FALSE;
	m_pCurCall = NULL;
	m_lExternalSpaceListSize = 0;
	m_lExternalSpaceNum = 0;

//	m_pInputFn = NULL;
//	m_debugOut = stdout;
	m_nWorkMode = VM_MODE_NORMAL;
	m_pExternalSpaceList = NULL;
	
	m_bIsDebugBreak = FALSE;
	if (m_ToStop.Create(NULL, false, false, NULL) != LOCKEX_ERR_OK)
		throw new CVMMException("create stop event failed");
	// 20030305 modify by Weihua Ju
	// original code:
	//if (m_Stopped.Create(NULL, true, false, NULL) != LOCKEX_ERR_OK)
	//	throw new CVMMException("create stop event failed");
	// new code
	if (m_Stopped.Create(NULL, true, true, NULL) != LOCKEX_ERR_OK)
		throw new CVMMException("create stop event failed");
	// modification end

	if (m_StatusLock.Create(NULL) != LOCKEX_ERR_OK)
		throw new CVMMException("create stop event failed");
//	Reset();
}

CVirtualMachine::~CVirtualMachine()
{
	Reset();
}

void CVirtualMachine::Reset()
{
//	if (m_BreakPoint)
//		delete m_BreakPoint;
//	m_BreakPoint = NULL;
//	m_BpList.size() = 0;
//	IsRunning = FALSE;
	m_ToStop.Set();		// stop the machine
	m_Stopped.Wait();	// wait machine stop
	m_ToStop.Reset();	


	__AX = __BX = __CX = __DX = __IP = __PSW = 0;
	/*	if (m_PointSeg)
	{
	delete m_PointSeg;
	m_PointSeg = NULL;
	}
	*/

	m_StatusLock.Lock();
	
	while (m_CallStack.empty() == false)		
	{
		m_pCurCall = &(m_CallStack.back());
		if (m_pCurCall->VMemory)
		{
			delete m_pCurCall->VMemory;	
			for (int i = 0; i< m_pCurCall->refs.size();i++){
				if (m_pCurCall->refs[i])
					delete m_pCurCall->refs[i];
			}		
		}
		m_CallStack.pop_back();		
	}	
	memset(&m_StartTime, 0, sizeof(SYSTEMTIME));
	m_StatusLock.Unlock();

	m_pCurCall = NULL;
	FUNCTIONSTACKELE *pCallInfo;
	while( m_FuncStack.empty() == false)
	{
		pCallInfo = m_FuncStack.top();
		PUBFUNCPARAM* param = &pCallInfo->paramhdr;
		PUBFUNCPARAM* paramsparent = NULL;
		while (pCallInfo->paramPt != &pCallInfo->paramhdr)
		{
			while (param != pCallInfo->paramPt)
			{
				//get last param
				paramsparent = param;
				param = param->pNext;
			}
			delete param;
			//go up;
			pCallInfo->paramPt = paramsparent;
			param = &pCallInfo->paramhdr;
		}
//		pCallInfo->paramPt = &pCallInfo->paramhdr;
		delete pCallInfo;
		m_FuncStack.pop();
	}
	if (m_pExternalSpaceList)
	{
		delete m_pExternalSpaceList;
		m_pExternalSpaceList = NULL;
	}
	m_lExternalSpaceListSize = 0;
	m_lExternalSpaceNum = 0;

//	m_pInputFn = NULL;
//	m_debugOut = stdout;
	m_nWorkMode = VM_MODE_NORMAL;
}


/*
函数声明：	BOOL CVirtualMachine::LoadFunction(CFunction *pFunc)
函数功能：	加载脚本函数
参数说明：	
			[IN]CFunction *pFunc	-	函数指针
返 回 值：	BOOL  - 成功或失败
编 写 人：	居卫华
完成日期：	2001-6-26
*/
void CVirtualMachine::LoadFunction(CFunction *pFunc)
{
//	BOOL bRet;
	m_StatusLock.Lock();
//	bRet = _LoadFunc(pFunc);
	_LoadFunc(pFunc);
	m_StatusLock.Unlock();

//	return bRet;
}

/*
	load new function
	1. create new stack element for new call
	2. push new call to call stack
	3. set current call to new call, set IP to 0, 
*/
void CVirtualMachine::_LoadFunc(CFunction *pFunc)
{
	printf("load virtual function %s(%lx)\n", pFunc->name(), pFunc);
	if (pFunc == NULL)
		throw new CVMMException("input CFuntion is null");	
	
	//	m_nWorkMode = pFunc->m_nWorkMode;
	if (m_pCurCall != NULL) // 保存现场
	{
		m_pCurCall->IP = __IP;
	}
	

	// create new stack element for new function call
	//分配虚拟内存
	int TotalSize = pFunc->m_SymbolTable.m_nTotalSize + pFunc->m_nSSUsedSize;
	CALLSTACKELE newStackEle;
	memset(&newStackEle, sizeof(CALLSTACKELE), 0);
	CALLSTACKELE* pStackEle = &newStackEle;
	if (pStackEle == NULL)
	{
	//	REPORT_MEM_ERROR("not enough memory, when virtual machine create CALLSTACK ELEMENT\r\n");
		throw new CVMMException("not enough memory, when virtual machine create CALLSTACK ELEMENT");
	//	return FALSE;
	}
	pStackEle->pFunc = pFunc;
	pStackEle->IP = 0;
	pStackEle->VMemory = new unsigned char[TotalSize];
	if (pStackEle->VMemory == NULL)
	{
		//REPORT_MEM_ERROR("not enough memory, when virtual machine create virtual memory\r\n")
		throw new CVMMException("not enough memory, when virtual machine create virtual memory");	
	//	return FALSE;
	}
	//	clear virtual memory
	memset(pStackEle->VMemory, 0, TotalSize);
	pStackEle->ulVMemSize = TotalSize;
	pStackEle->ulDataSegSize = pFunc->m_SymbolTable.m_nTotalSize;
	pStackEle->ulSSize = pFunc->m_nSSUsedSize;	
	pStackEle->DataSeg = pStackEle->VMemory + pFunc->m_nSSUsedSize;
	pStackEle->StaticSeg = pStackEle->VMemory;

	// push new call to call stack
	m_CallStack.push_back(*pStackEle);
	m_pCurCall = &(m_CallStack.back());
	
	if (pFunc->m_staticSegment)
		memcpy(pStackEle->StaticSeg, pFunc->m_staticSegment, pFunc->m_nSSUsedSize);
		

	__IP = 0;
//	return TRUE;
}

#define CMD_PREPROCESS2 \
	if (cmd == NULL)\
{\
	nLOG("SE:: commond is NUll", 50);\
	return FALSE;\
}\
	int op1mode, op2mode, op2reflvl, op1reflvl;\
	unsigned char *src, *dest;\
	if (!Preprocess2(cmd, op1mode, op1reflvl, op2mode, op2reflvl, dest, src))\
return FALSE;

#define CMD_PREPROCESS1 \
	if (cmd == NULL)\
{\
	nLOG("SE:: commond is NUll", 50);\
	printf("command is NULL");\
	return FALSE;\
}\
	int op1mode, op1reflvl;\
	unsigned char * dest;\
	if (!Preprocess1(cmd, op1mode, op1reflvl, dest))  {printf("Preprocess1 failed\n");return FALSE;}



BOOL CVirtualMachine::_mov(PCOMMAND cmd)
{
	CMD_PREPROCESS2	
		short opsize;//是字节操作, 字操作, 还双字操作
//	opsize = __min((cmd->address_mode >> 14)&0x3 ,(cmd->address_mode >> 6)&0x3);
    if (((cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
        opsize = (cmd->address_mode >> 6)&0x3;
    else
        opsize = (cmd->address_mode >> 14)&0x3;
	switch (opsize)
	{
	case 0:	memcpy(dest, src, 1);break;
	case 1:	memcpy(dest, src, 2);break;
	case 2:	memcpy(dest, src, 4);break;
	case 3:	memcpy(dest, src, 8);break;
	}
	printf("*dest=%lx\n", *(long*)dest);
	__IP++;
	return TRUE;
}

BOOL CVirtualMachine::_add(PCOMMAND cmd)
{	
	CMD_PREPROCESS2
		short opsize;//是字节操作, 字操作, 还双字操作
	//opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
	if ( ((cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
	    opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	switch (opsize)
	{
	case 0:	__AX = *((char*)dest) + *((char*)src);break;
	case 1:	__AX = *((short*)dest) + *((short*)src);break;
	case 2:	__AX = *((int*)dest) + *((int*)src);break;
	case 3:	__AX = *((long*)dest) + *((long*)src);break;
	}
	__IP++;
	return TRUE;
}

BOOL CVirtualMachine::_mul(PCOMMAND cmd)
{
	CMD_PREPROCESS2
		short opsize;//是字节操作, 字操作, 还双字操作
	//opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
	if ( ((cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
	    opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	switch (opsize)
	{
	case 0:	__AX = *((char*)dest) * *((char*)src);break;
	case 1:	__AX = *((short*)dest) * *((short*)src);break;
	case 2:	__AX = *((int*)dest) * *((int*)src);break;
	case 3:	__AX = *((long*)dest) * *((long*)src);break;
	}
	
	__IP++;
	return TRUE;
}



BOOL CVirtualMachine::_not(PCOMMAND cmd)
{
	CMD_PREPROCESS1
		short opsize = (cmd->address_mode >> 14)&0x3;
	switch (opsize)
	{
	case 0: *(char*)dest = ~(*(char*)dest);break;
	case 1: *(short*)dest = ~(*(short*)dest);break;
	case 2: *(int*)dest = ~(*(int*)dest);break;
	case 3: *(long*)dest = ~(*(long*)dest);break;
		
	}
	/*	int temp = ~(*((int*)dest));
	memcpy(dest, &temp, (cmd->address_mode >> 14)&0x3);
	*/	__IP++;
	return TRUE;
}

BOOL CVirtualMachine::_notr(PCOMMAND cmd)
{
	CMD_PREPROCESS1
		short opsize = (cmd->address_mode >> 14)&0x3;
	switch (opsize)
	{
	case 0: 
		*(char*)dest = !(*(char*)dest);
/*		if (*(char*)dest)
			__PSW |= 0x1;
		else
			__PSW &= 0xfffe;
*/		break;
	case 1: 
		*(short*)dest = !(*(short*)dest);
/*		if (*(short*)dest)
			__PSW |= 0x1;
		else
			__PSW &= 0xfffe;
*/		break;
	case 2: 
		*(int*)dest = !(*(int*)dest);
/*		if (*(int*)dest)
			__PSW |= 0x1;
		else
			__PSW &= 0xfffe;
*/		break;
	case 3:
		*(long*)dest = !(*(long*)dest);
/*		if (*(long*)dest)
			__PSW |= 0x1;
		else
			__PSW &= 0xfffe;
*/		break;
	}

	/*	int temp = ~(*((int*)dest));
	memcpy(dest, &temp, (cmd->address_mode >> 14)&0x3);
	*/	__IP++;
	return TRUE;
}

BOOL CVirtualMachine::_sub(PCOMMAND cmd)
{
	CMD_PREPROCESS2
		short opsize;//是字节操作, 字操作, 还双字操作
//	opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
    if (( (cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
        opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	switch (opsize)
	{
	case 0:	__AX = *((char*)dest) - *((char*)src);break;
	case 1:	__AX = *((short*)dest) - *((short*)src);break;
	case 2:	__AX = *((int*)dest) - *((int*)src);break;
	case 3:	__AX = *((long*)dest) - *((long*)src);break;
	}
	__IP++;
	
	return TRUE;
}

BOOL CVirtualMachine::_div(PCOMMAND cmd)
{
	char szMsg[201] = "";
	CMD_PREPROCESS2
		short opsize;//是字节操作, 字操作, 还双字操作
	//opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
    if (( (cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
        opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	switch (opsize)
	{
	case 0:	
		if (*((char*)src) ==0)
		{
			snprintf(szMsg, 200, "VM:: devide zero error, function='%s', line=%d, IP=%d\r\n", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
			REPORT_ERROR(szMsg, 20);
			return FALSE;
		}
		__AX = *((char*)dest) / *((char*)src);
		break;
	case 1:	
		if (*((short*)src) ==0)
		{
			snprintf(szMsg, 200, "VM:: devide zero error, function='%s', line=%d, IP=%d\r\n", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
			REPORT_ERROR(szMsg, 20);
			return FALSE;
		}
		__AX = *((short*)dest) / *((short*)src);
		break;
	case 2:	
		if (*((int*)src) ==0)
		{
			snprintf(szMsg, 200, "VM:: devide zero error, function='%s', line=%d, IP=%d\r\n", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
			REPORT_ERROR(szMsg, 20);
			return FALSE;
		}
		__AX = *((int*)dest) / *((int*)src);
		break;
	case 3:	
		if (*((long*)src) ==0)
		{
			snprintf(szMsg, 200, "VM:: devide zero error, function='%s', line=%d, IP=%d\r\n", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
			REPORT_ERROR(szMsg, 20);
			return FALSE;
		}
		__AX = *((long*)dest) / *((long*)src);
		break;
	}
	
	__IP++;
	
	return TRUE;
}

BOOL CVirtualMachine::_mod(PCOMMAND cmd)
{
	char szMsg[201] = "";
	CMD_PREPROCESS2
		short opsize;//是字节操作, 字操作, 还双字操作
	//  opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
    if (( (cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
        opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	switch (opsize)
	{
	case 0:	
		if (*((char*)src) ==0)
		{
			snprintf(szMsg, 200, "VM:: devide zero error, function='%s', line=%d, IP=%d\r\n", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
			REPORT_ERROR(szMsg, 20);
			return FALSE;
		}
		__AX = *((char*)dest) % *((char*)src);
		break;
	case 1:	
		if (*((short*)src) ==0)
		{
			snprintf(szMsg, 200, "VM:: devide zero error, function='%s', line=%d, IP=%d\r\n", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
			REPORT_ERROR(szMsg, 20);
			return FALSE;
		}
		__AX = *((short*)dest) % *((short*)src);
		break;
	case 2:	
		if (*((int*)src) ==0)
		{
			snprintf(szMsg, 200, "VM:: devide zero error, function='%s', line=%d, IP=%d\r\n", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
			REPORT_ERROR(szMsg, 20);
			return FALSE;
		}
		__AX = *((int*)dest) % *((int*)src);
		break;
	case 3:	
		if (*((long*)src) ==0)
		{
			snprintf(szMsg, 200, "VM:: devide zero error, function='%s', line=%d, IP=%d\r\n", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
			REPORT_ERROR(szMsg, 20);
			return FALSE;
		}
		__AX = *((long*)dest) % *((long*)src);
		break;
	}
	
	__IP++;
	
	return TRUE;
}

/*
函数名称     : CVirtualMachine::_jz
函数功能	    : 如果__PSW的第零位是1, 就跳转
变量说明     : 
返回值       : 
编写人       : 居卫华
完成日期     : 2001 - 5 - 11
*/
BOOL CVirtualMachine::_jz(PCOMMAND cmd)
{
	CMD_PREPROCESS1
		if (__PSW&0x1 != 0)
			__IP = *((int*)dest);
		else
			__IP++;
		return TRUE;
}


/*
函数名称     : CVirtualMachine::_jnz
函数功能	    : 如果__PSW的第零位是0, 就跳转
变量说明     : 
返回值       : 
编写人       : 居卫华
完成日期     : 2001 - 5 - 11
*/
BOOL CVirtualMachine::_jnz(PCOMMAND cmd)
{
	CMD_PREPROCESS1
		if ((__PSW&0x1) == 0)
			__IP = *((int*)dest);
		else
			__IP++;
		return TRUE;
}

BOOL CVirtualMachine::_jmpz(PCOMMAND cmd)
{
	CMD_PREPROCESS2
		if (*((int*)dest) != 0)
			__IP = *((int*)src);
		else
			__IP++;
		return TRUE;
}

BOOL CVirtualMachine::_jmp(PCOMMAND cmd)
{
	CMD_PREPROCESS1
		__IP = *((int*)dest);
	return TRUE;
}

// get physical address of one variable, and put into dest 
BOOL CVirtualMachine::_ea(PCOMMAND cmd)
{
	long* EA;
	CMD_PREPROCESS2
	EA = (long*)src;

	memcpy(dest, &EA, sizeof(long*));
	DEBUG2p("-->EA: src=%lx, dest=%lx\n", src, dest);
	__IP++;
	return TRUE;
}

// get address of an object member,  the src must be a string
BOOL CVirtualMachine::_eaobj(PCOMMAND cmd)
{
	
	CMD_PREPROCESS2
	long EA=NULL;
	
	CRef *dest_ref = *(CRef**)dest;
	if (dest_ref == NULL || dest_ref->getRef() == NULL){
		ERR("_eaobj:object not initialzed");
		return FALSE;
	}
	if (dest_ref == NULL){
		dest_ref = createRef();
		*(CRef**)dest = dest_ref;
	}
	if (dest_ref->getRef() == NULL){
		dest_ref->setRef(createObject(NULL));
	}
 
//	unsigned char* dest = (unsigned char*)&(m_pCurCall->DataSeg[cmd->op[0]]);
	CObjectInst* obj = (CObjectInst*)(dest_ref->getRef());
	char* member = *(char**)src;
	debug("member name=%s", member);
//	CAttribute* o =  obj->getMemberAddress(member);
	CRef* r = obj->getMemberRef(member);
		printf("==>eaobj: obj=%x, dest ref=%x, member ref=%x, member name=%s\n", obj, *(CRef**)dest, r, member);
	r = obj->getMemberRef(member);
		printf("==>eaobj: obj=%x, dest ref=%x, member ref=%x, member name=%s\n", obj, *(CRef**)dest, r, member);
	debug("member object = %x", r->getRef());
	EA = (long)r;
/*	if (cmd->opnum > 2){
		if (obj == NULL)
			return FALSE;
		char* member = (char*)(&(m_pCurCall->StaticSeg[cmd->op[2]]));
		CObjectInst* o =  obj->getMemberAddress(member);
		if (o != NULL)
			EA = (INT)o;
	}else
		EA = (long)obj;*/
	__AX= EA;
	__IP++;
	return TRUE;
}
/*
// get address of an object member,  the src must be a string
BOOL CVirtualMachine::_evalobj(PCOMMAND cmd)
{
	CMD_PREPROCESS2
	
 
//	unsigned char* dest = (unsigned char*)&(m_pCurCall->DataSeg[cmd->op[0]]);
	CObjectInst* obj = *(CObjectInst**)src;
//	std::string s = obj->to_s();
//	*(char**)dest = s.c_str();


	__IP++;
	return TRUE;
}*/
#if 0
// get address of an object member,  the src must be a string
BOOL CVirtualMachine::_eaobj(PCOMMAND cmd)
{
	CMD_PREPROCESS2
	long EA=NULL;
 
//	unsigned char* dest = (unsigned char*)&(m_pCurCall->DataSeg[cmd->op[0]]);
	CObjectInst* obj = *(CObjectInst**)dest;
	char* member = src;
	CObjectInst* o =  obj->getMemberAddress(member);
	EA = (long)o;
/*	if (cmd->opnum > 2){
		if (obj == NULL)
			return FALSE;
		char* member = (char*)(&(m_pCurCall->StaticSeg[cmd->op[2]]));
		CObjectInst* o =  obj->getMemberAddress(member);
		if (o != NULL)
			EA = (INT)o;
	}else
		EA = (long)obj;*/
	__AX= EA;
	__IP++;
	return TRUE;
}
#endif

BOOL CVirtualMachine::_newobj(PCOMMAND cmd)
{
		
	CRef * r = NULL;
	if (cmd->op[0] == -1) {
							r =  createRef();
		r->setRef(createObject(NULL));


	}else	if (cmd->op[0] == -2){
			r =  new CRef(); // tmp ref, prevent it from clean by function return
		r->setRef(createObject(NULL));
	} else{

		CMD_PREPROCESS1
		printf("create object for %s\n", (char*)dest);
	
	/*	CClassDes*	pc = CCompiler::classDesTable.getClass((char*)dest);
		if (pc == NULL){
			fprintf(stderr, "Load Object for %s failed!", (char*)dest);
			return FALSE;
		}*/
			r =  createRef();
		r->setRef(createObject((char*)dest));

	}
	printf("newobj:new ref=%x, new object %lx\n", r, r->getRef());
		__AX = (long)r;
	//__AX = LoadObject((char*)dest);

	__IP++;
	return TRUE;
}

BOOL CVirtualMachine::_rmref(PCOMMAND cmd)
{
		CMD_PREPROCESS1
		printf("rm cref %lx at adddress %lx\n", *dest, dest);
	CRef * r = *(CRef**)dest;
		if (r)
			delete r;
		*(CRef**)dest = NULL;
	__IP++;
	return TRUE;
}
/*

int exception_filter(EXCEPTION_POINTERS * pex)
{
	std::ofstream of("exception_report.txt", ios_base::out | ios_base::app);
	SYSTEMTIME time;
	GetLocalTime(&time);
	of << std::endl << "---------------------------------------------------";
	of << std::endl <<"Exception occured on " << time.wYear << time.wMonth << time.wDay << time.wHour << time.wMinute << time.wSecond << time.wMilliseconds << std::endl;
	sym_engine::stack_trace(of, pex->ContextRecord);		
	of.flush();
	return EXCEPTION_EXECUTE_HANDLER ;
}
*/
BOOL CVirtualMachine::_endcallpub(PCOMMAND cmd)
{
	// get current function call info
	FUNCTIONSTACKELE *pCallInfo = NULL;
	if (m_FuncStack.empty() == true)
		return FALSE;
	pCallInfo = m_FuncStack.top();/* line = %x,*/
//	printf("&pCallInfo =%x, IP = (%x)", &pCallInfo,  /*cmd->line,*/ &__IP);

	//generate parametre block
	PUBFUNCPARAM* param, *paramsparent;
	param = &pCallInfo->paramhdr;

	void* pData = (void*) new char[pCallInfo->lParamBlockSize];
	memset(pData, 0, pCallInfo->lParamBlockSize);
	unsigned char* pDataPt = (unsigned char*)pData;
	int paramNum = 0;
	while (param != pCallInfo->paramPt)
	{
	
		if ((BYTE*)pDataPt + param->size - (BYTE*)pData > pCallInfo->lParamBlockSize)
		{
			
			pCallInfo->lParamBlockSize = 0;
			delete pData;
			param = &pCallInfo->paramhdr;
			
			while (pCallInfo->paramPt != &pCallInfo->paramhdr)
			{
				while (param != pCallInfo->paramPt)
				{
					//get last param
					paramsparent = param;
					param = param->pNext;
				}
				delete param;
				//go up;
				pCallInfo->paramPt = paramsparent;
				param = &pCallInfo->paramhdr;
			}
			pCallInfo->paramPt = &pCallInfo->paramhdr;
			return FALSE;
		}
		memcpy(pDataPt, param->pData, param->size);
		pDataPt += param->size;	
		paramNum++;
		
		// 字节对齐
#ifdef _64	
		if (param->size < 8)
		{
			pDataPt += 8- param->size;
		}
#else
		if (param->size < 4)
		{
			pDataPt += 4- param->size;
		}
#endif
		/*		//只考虑一重指针, 不允许有指针的指针
		if (param->reflvl > 0)
		*((int*)(param->pData)) += (int)memory;
		*/
		param = param->pNext;
	}
#if 0		
	//call function
	{
		void* p = alloca(pCallInfo->lParamBlockSize+16/*_SCRATCH_SIZE*/);
		memset(p, 0, pCallInfo->lParamBlockSize+16);	
		memcpy(p, (void*)pData, pCallInfo->lParamBlockSize);
//	printf("2:param = %x, param->size = %x, param->next = %x, &pCallInfo =%x\n", param, param->size, param->pNext, &pCallInfo);
		_HGDispatchCall(&__AX, (void*)pCallInfo->fn, p, pCallInfo->lParamBlockSize);		
//	printf("2:param = %x, param->size = %x, param->next = %x, &pCallInfo =%x\n", param, param->size, param->pNext, &pCallInfo);
	}
#endif

	BOOL ret = TRUE;
	
	char msg[201] = "";

/*
	try
	{
		CallPubFunc(pCallInfo->lParamBlockSize, (void*)pCallInfo->fn, (void*)pData, &__AX);		
	}
	catch(CVMException *e)
	{
		long lCode;
		char* sMsg = NULL;
		sMsg = e->GetExp(lCode);
		snprintf(msg, 200, "SE:: fatal error - external function call cause VM exception. script: %s, line: %d, Exception Error(%d): %s", m_pCurCall->pFunc->m_szName, cmd->line, lCode, sMsg);
		LOG(sMsg, 5);
		ret = FALSE;
		delete e;
	}

 */

/*	cactch all exception, project must not use the /GX link option, and "CallPubFunc" must be a __stdcall function
*/

	try
	{
		CallPubFunc(pCallInfo->lParamBlockSize, (void*)pCallInfo->fn, (void*)pData, &__AX, paramNum);		
	}
	catch(CVMException *e)
	{
		long lCode;
		char* sMsg = NULL;
		sMsg = e->GetExp(lCode);
		snprintf(msg, 200, "SE:: fatal error - external function call cause VM exception. script: %s, line: %d, Exception Error(%d): %s", m_pCurCall->pFunc->m_szName, cmd->line, lCode, sMsg);
		nLOG(msg, 0);		
		ret = FALSE;
		delete e;

		// dump memroy to file
		DumpToFile();
	}
	catch (CExp* e)
	{
		snprintf(msg, 200, "SE::  Got exception out of virtual machine. script: %s, line: %d", m_pCurCall->pFunc->m_szName, cmd->line);
		nLOG(msg, 0);
		ret = FALSE;
		
		// dump memroy to file
		//DumpToFile();
		
		// before throw, shuld exit safely
		//clear
		pCallInfo->lParamBlockSize = 0;
		delete pData;
		param = &pCallInfo->paramhdr;
		
		while (pCallInfo->paramPt != &pCallInfo->paramhdr)
		{
			while (param != pCallInfo->paramPt)
			{
				//get last param
				paramsparent = param;
				param = param->pNext;
			}
			delete param;
			paramsparent->pNext = NULL;
			//go up;
			pCallInfo->paramPt = paramsparent;
			param = &pCallInfo->paramhdr;
		}
		delete pCallInfo;
		m_FuncStack.pop();
		
		__IP++;
		//	printf("&pCallInfo =%x, line = %x, IP = %x(%x)", &pCallInfo, cmd->line, __IP, &__IP);
		
		//	return ret;		// throw to system and let system dump memory
		CExp * exp = new CExp(0, msg, __FILE__, __LINE__);
		exp->SetLow(e);
		throw exp;
	}
	catch (...)
	{
		snprintf(msg, 200, "SE:: fatal error - external function call cause unknown exception. script: %s, line: %d", m_pCurCall->pFunc->m_szName, cmd->line);
		nLOG(msg, 0);
		ret = FALSE;
		
		// dump memroy to file
		DumpToFile();

		// before throw, shuld exit safely
		//clear
		pCallInfo->lParamBlockSize = 0;
		delete pData;
		param = &pCallInfo->paramhdr;
		
		while (pCallInfo->paramPt != &pCallInfo->paramhdr)
		{
			while (param != pCallInfo->paramPt)
			{
				//get last param
				paramsparent = param;
				param = param->pNext;
			}
			delete param;
			paramsparent->pNext = NULL;
			//go up;
			pCallInfo->paramPt = paramsparent;
			param = &pCallInfo->paramhdr;
		}
		delete pCallInfo;
		m_FuncStack.pop();
		
		__IP++;
		//	printf("&pCallInfo =%x, line = %x, IP = %x(%x)", &pCallInfo, cmd->line, __IP, &__IP);
		
	//	return ret;		// throw to system and let system dump memory
		throw;
	}
		
/* use dbghelp api to get exception position, must not use the /O2 compilation option
	__try
	{
			CallPubFunc(pCallInfo->lParamBlockSize, (void*)pCallInfo->fn, (void*)pData, &__AX);		
	}
	__except (exception_filter(GetExceptionInformation()))
	{
		snprintf(msg, 200, "SE:: fatal error - external function call cause unknown exception. script: %s, line: %d", m_pCurCall->pFunc->m_szName, cmd->line);
		nLOG((msg, 5);
		ret = FALSE;
	}
*/
	//clear
	pCallInfo->lParamBlockSize = 0;
	delete pData;
	param = &pCallInfo->paramhdr;
	
//	printf("3:param = %x, param->size = %x, param->next = %x\n", param, param->size, param->pNext);
	while (pCallInfo->paramPt != &pCallInfo->paramhdr)
	{
		while (param != pCallInfo->paramPt)
		{
			//get last param
			paramsparent = param;
			param = param->pNext;
		}
		delete param;
		paramsparent->pNext = NULL;
		//go up;
		pCallInfo->paramPt = paramsparent;
		param = &pCallInfo->paramhdr;
	}
	//pCallInfo->paramPt = &pCallInfo->paramhdr;
	delete pCallInfo;
	m_FuncStack.pop();

	__IP++;
//	printf("&pCallInfo =%x, line = %x, IP = %x(%x)", &pCallInfo, cmd->line, __IP, &__IP);

	return ret;
}

BOOL CVirtualMachine::_callpub(PCOMMAND cmd)
{
	if (cmd == NULL)
	{
		nLOG("SE:: commond is NUll", 50);
		return FALSE;
	}
	int op1mode, op1reflvl;
	unsigned char * dest;
	if (!Preprocess1(cmd, op1mode, op1reflvl, dest, FALSE)) return FALSE;
	
	FUNCTIONSTACKELE *newCall = new FUNCTIONSTACKELE;
	if (newCall == NULL)
	{
		nLOG("SE:: allocate memory failed", 5);
		return FALSE;
	}
	memset(newCall, 0, sizeof(FUNCTIONSTACKELE));
	newCall->paramPt = &newCall->paramhdr;
	newCall->fn = *((long*)dest);
	m_FuncStack.push(newCall);

	__IP++;
	return TRUE;
}

BOOL CVirtualMachine::_parampub(PCOMMAND cmd)
{
	CMD_PREPROCESS1
	short opsize;// 是字节操作, 字操作, 还双字操作
	
	// get opsize
	
	// get first 2 bit by shifting and masking with 0x000000011
	opsize= (cmd->address_mode >> 14)&0x3; 

	int size = 1;
	// 0=>1Btype, 1=>2Byte, 2=>4Bytpe, 3=>8Btype ...
	for (int i = 0; i< opsize; i++)
	{
		size *= 2;
	}	
	printf("--->parampub: address mode=0x%x, size=%d, dest=%lx\n", cmd->address_mode, size, dest);
	
	// get current function call info
	FUNCTIONSTACKELE *pCallInfo = NULL;
	if (m_FuncStack.empty() == true)
		return FALSE;
	pCallInfo = m_FuncStack.top();

	int dt = (cmd->address_mode >> 8) & 0x0f;
	// if allow pass object as param to pub function, should add datat type into pCallInfo->paramPt
/*	if (dt == AMODE_OBJ){
		debug("fdfda===>%x", *(CObjectInst**) dest);
		BYTE* vt = ( *(CObjectInst**) dest)->getValueAddress();
		pCallInfo->paramPt->pData = vt;
		debug("====>3333%s\n", *(char**)vt);
	}
	else*/
		pCallInfo->paramPt->pData = (unsigned char*)dest;
	pCallInfo->paramPt->size = size;
	pCallInfo->paramPt->reflvl = op1reflvl;
	pCallInfo->lParamBlockSize += pCallInfo->paramPt->size;
	// 字节对齐
	#ifdef _64
	 if (pCallInfo->paramPt->size < 8)
	 {
	 	pCallInfo->lParamBlockSize+= 8-pCallInfo->paramPt->size;
	 }
	#else
		 if (pCallInfo->paramPt->size < 4)
	 {
	 	pCallInfo->lParamBlockSize+= 4-pCallInfo->paramPt->size;
	 }
	#endif
	pCallInfo->paramPt->pNext = new PUBFUNCPARAM;
	pCallInfo->paramPt = pCallInfo->paramPt->pNext;
	pCallInfo->paramPt->pData = NULL;
	pCallInfo->paramPt->pNext = NULL;
	pCallInfo->paramPt->size = 0;
	
	// debug code
	//printf("param: pData[0] = %x, size = %d, reflvl = %d\n", *(unsigned long*)dest, size, op1reflvl);
//	printf("param: address_mode = %x, opcode\n", cmd->address_mode, cmd->opcode[0]);
	__IP++;
	return TRUE;
}

BOOL CVirtualMachine::_callv(PCOMMAND cmd)
{
	if (cmd == NULL)
	{
		nLOG("SE:: commond is NUll", 50);
		return FALSE;	
	}
	

	int op1mode, op1reflvl;
	unsigned char * dest;
	if (!Preprocess1(cmd, op1mode, op1reflvl, dest, FALSE)) return FALSE;



	FUNCTIONSTACKELE *newCall = new FUNCTIONSTACKELE;
	if (newCall == NULL)
	{
		nLOG("SE:: allocate memory failed", 5);
		return FALSE;
	}

	
	memset(newCall, 0, sizeof(FUNCTIONSTACKELE));
	newCall->paramPt = &newCall->paramhdr;
	newCall->fn = *((long*)dest);
	m_FuncStack.push(newCall);

	__IP++;

	
	return TRUE;
}
BOOL CVirtualMachine::_paramv(PCOMMAND cmd)
{
	CMD_PREPROCESS1
	short opsize;// 是字节操作, 字操作, 还双字操作

	// get opsize
	opsize= (cmd->address_mode >> 14)&0x3;
	int size = 1;
	for (int i = 0; i< opsize; i++)
	{
		size *= 2;
	}	
	FUNCTIONSTACKELE *pCallInfo = NULL;

	if (m_FuncStack.empty() == true)
		return FALSE;
	pCallInfo = m_FuncStack.top();


	debug("paramv obj %x", *(long **)dest);
	pCallInfo->paramPt->pData = (unsigned char*)dest;
	pCallInfo->paramPt->size = size;
	pCallInfo->paramPt->reflvl = op1reflvl;
	pCallInfo->lParamBlockSize += pCallInfo->paramPt->size;

/*	if (pCallInfo->paramPt->size < 4)
	{
		pCallInfo->paramhdr+= 4-pCallInfo->paramPt->size;
	}
*/
	pCallInfo->paramPt->pNext = new PUBFUNCPARAM;
	pCallInfo->paramPt = pCallInfo->paramPt->pNext;
	pCallInfo->paramPt->pData = NULL;
	pCallInfo->paramPt->pNext = NULL;
	pCallInfo->paramPt->size = 0;

	__IP++;
	return TRUE;
}
BOOL CVirtualMachine::_endcallv(PCOMMAND cmd)
{
	//generate parametre block
	PUBFUNCPARAM* param, *paramsparent;
	
	// get current function call info
	FUNCTIONSTACKELE *pCallInfo = NULL;
	if (m_FuncStack.empty() == true)
		return FALSE;
	pCallInfo = m_FuncStack.top();
	param = &pCallInfo->paramhdr;

	void* pData = (void*) new char[pCallInfo->lParamBlockSize];
	if (pData == NULL)
	{
		nLOG("SE:: allocate memory failed", 5);
		return FALSE;
	}
	memset(pData, 0, pCallInfo->lParamBlockSize);
	unsigned char* pDataPt = (unsigned char*)pData;
	while (param != pCallInfo->paramPt)
	{
		if ((BYTE*)pDataPt + param->size - (BYTE*)pData > pCallInfo->lParamBlockSize)
		{// has error			
			delete pData;
			param = &pCallInfo->paramhdr;
			
			while (pCallInfo->paramPt != &pCallInfo->paramhdr)
			{
				while (param != pCallInfo->paramPt)
				{
					//get last param
					paramsparent = param;
					param = param->pNext;
				}
				delete param;
				//go up;
				pCallInfo->paramPt = paramsparent;
				param = &pCallInfo->paramhdr;
			}
			pCallInfo->paramPt = &pCallInfo->paramhdr;
			return FALSE;			
		}
		// copy data
		memcpy(pDataPt, param->pData, param->size);
		pDataPt += param->size;		
		param = param->pNext;
	}
	
	// load function
	__IP++;
	_LoadFunc((CFunction*)pCallInfo->fn);
	AttachParam((BYTE*)pData, pCallInfo->lParamBlockSize);
	
	//clear
	if (pData) delete pData;
	param = &pCallInfo->paramhdr;
	
	// release link table of param info
	while (pCallInfo->paramPt != &pCallInfo->paramhdr)
	{
		while (param != pCallInfo->paramPt)
		{
		//	printf("param = %x, param->size = %x\n", param, param->size);
			//get last param
			paramsparent = param;
			param = param->pNext;
		}
		delete param;
		paramsparent->pNext = NULL;
		//go up;
		pCallInfo->paramPt = paramsparent;
		param = &pCallInfo->paramhdr;
	}
	//pCallInfo->paramPt = &pCallInfo->paramhdr;
	delete pCallInfo;
	m_FuncStack.pop();

	__IP = 0;
	return TRUE;
}


BOOL CVirtualMachine::_test(PCOMMAND cmd)
{
	CMD_PREPROCESS2
		short opsize;//是字节操作, 字操作, 还双字操作
	//	opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
    if (( (cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
        opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	
	
	int op3 = cmd->op[2];//条件
	//0: == 1: != 2: >= 3:<= 4:> 5:< 6:&& 7:||
    //如成立则置标志位1, 不成立置零
	char msg[201] = "";
	switch (opsize)
	{
	case 0:
		switch (op3)
		{
		case 0: 
			if (*((char*)src) == *((char*)dest))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
		case 1: 
			if (*((char*)src) != *((char*)dest))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 2:	
			if (*((char*)dest) >= *((char*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 3:	
			if (*((char*)dest) <= *((char*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 4: 		
			if (*((char*)dest) > *((char*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 5: 		
			if (*((char*)dest) < *((char*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 6: 		//'&&'
			if (*((char*)dest) &&  *((char*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 7:         //'||'
			if (*((char*)dest) || *((char*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		default:
			{
				snprintf(msg, 200, "SE:: invalid test command condition: %d, (line = %d))", op3, cmd->line);
				nLOG(msg, 9);
				return FALSE;
			}
		};
		break;
	case 1:
		switch (op3)
		{
		case 0: 
			if (*((short*)src) == *((short*)dest))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
		case 1: 
			if (*((short*)src) != *((short*)dest))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 2:	
			if (*((short*)dest) >= *((short*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 3:	
			if (*((short*)dest) <= *((short*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 4: 		
			if (*((short*)dest) > *((short*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 5: 		
			if (*((short*)dest) < *((short*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 6: 		//'&&'
			if (*((short*)dest) &&  *((short*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 7:         //'||'
			if (*((short*)dest) || *((short*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		default:
			{
				snprintf(msg, 200, "SE:: invalid test command condition: %d, (line = %d))", op3, cmd->line);				
				nLOG(msg, 9);
				return FALSE;
			}
		};
		break;
	case 2:
		switch (op3)
		{
		case 0: 
			if (*((int*)src) == *((int*)dest))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
		case 1: 
			if (*((int*)src) != *((int*)dest))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 2:	
			if (*((int*)dest) >= *((int*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 3:	
			if (*((int*)dest) <= *((int*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 4: 		
			if (*((int*)dest) > *((int*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 5: 		
			if (*((int*)dest) < *((int*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 6: 		//'&&'
			if (*((int*)dest) &&  *((int*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 7:         //'||'
			if (*((int*)dest) || *((int*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		default:
			{
				snprintf(msg, 200, "SE:: invalid test command condition: %d, (line = %d))", op3, cmd->line);				
				nLOG(msg, 9);
				return FALSE;
			}
		};
		break;
	case 3:
		switch (op3)
		{
		case 0: 
			if (*((long*)src) == *((long*)dest))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
		case 1: 
			if (*((long*)src) != *((long*)dest))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 2:	
			if (*((long*)dest) >= *((long*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 3:	
			if (*((long*)dest) <= *((long*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 4: 		
			if (*((long*)dest) > *((long*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 5: 		
			if (*((long*)dest) < *((long*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 6: 		//'&&'
			if (*((long*)dest) &&  *((long*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;
			
		case 7:         //'||'
			if (*((long*)dest) || *((long*)src))
				__PSW |= 0x1;
			else
				__PSW &= 0xfffe;
			break;						
		default:
			{
				snprintf(msg, 200, "SE:: invalid test command condition: %d, (line = %d))", op3, cmd->line);			
				nLOG(msg, 9);
				return FALSE;
			}
		};
		break;
	}
	__IP++;
	return TRUE;
}

BOOL CVirtualMachine::_ret(PCOMMAND cmd)
{
	char msg[201] = "";
	snprintf(msg, 200, "SE:: script '%s' exit at line %04d, IP %04d", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
	DEBUG(msg);
//	if (m_pCurCall)
//		debug("return from call to virtual function %s", ((CFunction*)m_pCurCall->fn)->name());
	//m_CallStack.back();
	if (m_pCurCall->VMemory){
		delete m_pCurCall->VMemory;
		for (int i = 0; i< m_pCurCall->refs.size();i++){
				if (m_pCurCall->refs[i])
					delete m_pCurCall->refs[i];
		}
	}
	m_CallStack.pop_back();
	if (m_CallStack.empty() == false)
		m_pCurCall = &m_CallStack.back();
	else
	{
		nLOG(msg, 85);
		m_pCurCall = NULL;
	//	IsRunning = FALSE;
		m_ToStop.Set();

		return TRUE;
	}


	__IP = m_pCurCall->IP;
	
	return TRUE;	
}

std::string CVirtualMachine::getCodeName(int code){
	switch(code){
		case 0x1010: return "__mov		  ";  						//mov (变量, 变量)
		case 0x10f4: return "__ea		  ";  						//取有效地址(只能用AMODE_MEM寻址方式)
		case 0x1021: return "__add		  ";  		            //add (变量, 变量)-> ax
		case 0x1050: return "__sub		  ";  					//sub (变量, 变量)
		case 0x1070: return "__mul		  ";  					//mul (变量, 常量)
		case 0x1090: return "__div		  ";  					//div (变量, 常量)
		case 0x10f0: return "__mod		  ";  					//%
		case 0x10d0: return "__not		  ";  					//not (变量), 相当于~运算
		case 0x10f2: return "__test		  ";  					//比较test(a, b, 条件)
		case 0x10f5: return "__notr		  ";  					//(__notr(变量) !运算 	
		case 0x1022: return "__fadd		  "; 
		case 0x1051: return "__fsub		  "; 
		case 0x1071: return "__fmul		  "; 
		case 0x1091: return "__fdiv		  "; 
		case 0x10a0: return "__jmp		  ";                     //jmp (常量)
		case 0x10a1: return "__jz		  ";  					//jz (语句)
		case 0x10a2: return "__jnz		  ";  					//jnz (constant)
		case 0x10f3: return "__ret		  ";                      
		case 0x10a3: return "__callpub	  ";  				//call (function entry)
		case 0x10a4: return "__parampub	  ";  				//param (address)
		case 0x10a5: return "__endcallpub  ";  				//endcall
		case 0x10a6: return "__callv		  "; 
		case 0x10a7: return "__paramv	  "; 
		case 0x10a8: return "__endcallv	  "; 
		case 0x10a9: return "__loadlib	  ";  			// LoadLib xxx
		case 0x10aa: return "__eaobj		  ";  			// get address of object
		case 0x10ab: return "__newobj	  ";  			// create new object or array
		case 0x10ac: return "__movobj	  ";  			// assignment for object
		case 0x10ad: return "__i_evalstring";  			// eval string including #{}  
		case 0x10ae: return "__rmref";            
		case 0x10f6: return "__cast		  ";  
		default: 
			return "";
	}
	return "";
}
/**
函数声明：	BOOL CVirtualMachine::Run()
函数功能：	开始运行vm
参数说明：	
返 回 值：	BOOL  - 成功返回TRUE， 失败返回FALSE
编 写 人：	居卫华
完成日期：	2002-3-19
**/
BOOL CVirtualMachine::Run()
{
	printf("--->run\n");
	m_Stopped.Reset();

	m_nRemainCmdNum = 0;
	__IP = 0;
	//IsRunning = TRUE;
	m_lDbgCmdID = DBGCMD_NULL;
	char msg[1001] = "";	
	
	int opcode;
	BOOL bError = FALSE;
	int nodebug = 0;
	PCOMMAND pcmd = NULL;
//	vector kk;
	//分配断点列表
//	m_BreakPoint = new long[m_pCurCall->pFunc->m_nCurrentCmdNum];
//	memset(m_BreakPoint, -1, sizeof(long)*m_pCurCall->pFunc->m_nCurrentCmdNum);
	m_BpList.clear();


	// start time
	m_StatusLock.Lock();
	GetLocalTime(&m_StartTime);	
	m_StatusLock.Unlock();

	while (m_ToStop.Wait(0) == LOCKEX_ERR_TIMEOUT && !bError && m_pCurCall->pFunc->m_pCmdTable != NULL)
	{
		JUJU::CLog::debug("--->__IP=%d, mode=%d\n", __IP, m_nWorkMode);
		JUJU::CLog::debug("--->line %d, code 0x%x(%s)\n", m_pCurCall->pFunc->m_pCmdTable[__IP].line, m_pCurCall->pFunc->m_pCmdTable[__IP].opcode, getCodeName(m_pCurCall->pFunc->m_pCmdTable[__IP].opcode).c_str());
		m_StatusLock.Lock();
		opcode = m_pCurCall->pFunc->m_pCmdTable[__IP].opcode;
		pcmd = &(m_pCurCall->pFunc->m_pCmdTable[__IP]);
		if (m_nWorkMode >= VM_MODE_DEBUG)
		{

			if (__IP == 0 || (__IP > 0 && pcmd->line != m_pCurCall->pFunc->m_pCmdTable[__IP - 1].line ) )// 避免打印相同的行
			{
				snprintf(msg, 200, "SE::%s IP: %d line: %d", m_pCurCall->pFunc->m_szName, __IP, pcmd->line);
				nLOG(msg, 500);
				printf("%s\n", msg);
			}

			if (m_nWorkMode >= VM_MODE_STEPDEBUG)
			{
				m_pCurCall->pFunc->OutupCmd(__IP, msg);
				nLOG(msg, 500);
			}
		}
		switch (opcode)
		{
		case __mov:		
			if (!_mov(pcmd))	bError = TRUE;
			break;
		case __ret:
			if (!_ret(pcmd))	bError = TRUE;
			break;
		case __add:
			if (!_add(pcmd))	bError = TRUE;
			break;
		case __sub:
			if (!_sub(pcmd))	bError = TRUE;
			break;
		case __mul:
			if (!_mul(pcmd))	bError = TRUE;
			break;
		case __div:
			if (!_div(pcmd))	bError = TRUE;
			break;
		case __mod:
			if (!_mod(pcmd))	bError = TRUE;
			break;			
		case __fadd:
			if (!_fadd(pcmd))	bError = TRUE;
			break;
		case __fsub:
			if (!_fsub(pcmd))	bError = TRUE;
			break;
		case __fmul:
			if (!_fmul(pcmd))	bError = TRUE;
			break;
		case __fdiv:
			if (!_fdiv(pcmd))	bError = TRUE;
			break;
		case __not:
			if (!_not(pcmd))	bError = TRUE;
			break;
		case __notr:
			if (!_notr(pcmd)) bError = TRUE;
			break;
		case __jz:
			if (!_jz(pcmd))	bError = TRUE;
			break;
		case __jmp:
			if (!_jmp(pcmd))	bError = TRUE;
			break;
		case __test:
			if (!_test(pcmd))	bError = TRUE;
			break;
		case __jnz:
			if (!_jnz(pcmd))    bError = TRUE;
			break;
		case __callpub:
			if (!_callpub(pcmd))    bError = TRUE;
			break;
		case __parampub:
			if (!_parampub(pcmd))    bError = TRUE;
			break;
		case __endcallpub:
			try {			
				if (!_endcallpub(pcmd))    bError = TRUE;
			}
			catch(...)
			{
				// exit saftly befor throw
				m_Stopped.Set();
				throw;
			}
			break;
		case __callv:
			if (!_callv(pcmd))    bError = TRUE;
	//printf("SE:: debug info flag, %d@%s\n", __LINE__, __FILE__);
			break;
		case __paramv:
			if (!_paramv(pcmd))    bError = TRUE;
			break;
		case __endcallv:
			if (!_endcallv(pcmd))    bError = TRUE;
			break;
		case __ea:
			if (!_ea(pcmd)) bError = TRUE;
			break;
		case __cast:
			if (!_cast(pcmd)) bError = TRUE;
			break;
		case __movobj:
			if (!_movobj(pcmd)) bError = TRUE;
			break;
		case __eaobj:
			if (!_eaobj(pcmd)) bError = TRUE;
			break;
		case __newobj:
			if (!_newobj(pcmd)) bError = TRUE;
			break;
		case __rmref:
			if (!_rmref(pcmd)) bError = TRUE;
			break;
		default:
			snprintf(msg, 200, "SE:: can not find the implement of this command %xH (line: %d)", opcode, pcmd->line);
			nLOG(msg, 9);
			bError = TRUE;
			break;
		}

		m_StatusLock.Unlock();

		if (m_nWorkMode < VM_MODE_STEPDEBUG)
			continue;

		//--------- debug ---------//

		if (m_nRemainCmdNum > 0)
			m_nRemainCmdNum--;

		//是否是断点
		int index;
		for (index = 0; index < m_BpList.size(); index++)
		{
			if (m_BpList[index].lIP == __IP && strcmp(m_pCurCall->pFunc->m_szName, m_BpList[index].m_szName) == 0)
			{
				m_nRemainCmdNum = 0;
				break;
			}
		}
		if (index >= m_BpList.size())
		{
			// not a break point and remain command number > 0
			if (m_nRemainCmdNum > 0)
				continue;
		}
		//是断点或不是断点但m_nRemainCmdNum == 0
		//      ||
		//      \/
		//检测并运行调试命令

		m_bIsDebugBreak = TRUE;

		// clear all dbg input and output variable
		CVirtualMachine::m_DgbCmdLock.Lock();
		m_lDbgCmdID = DBGCMD_NULL;
		m_lDbgCmdP1 = m_lDbgCmdP2 = 0;
		memset(m_szDbgCmdP3, 0, 1024);
		CVirtualMachine::m_sDbgOutMsg = "";
		CVirtualMachine::m_lDbgOutCode = 0;

		CVirtualMachine::m_DgbCmdLock.Unlock();

		while (m_nRemainCmdNum == 0)
		{

			if (m_lDbgCmdID == DBGCMD_NULL)
			{
				Sleep(100);
				continue;
			}
			else
			{
				CVirtualMachine::m_DgbCmdLock.Lock();
				memset(msg, 0, 1001);
				long lDbgRet = 0;
		
				switch (m_lDbgCmdID)
				{
				case DBGCMD_NULL:	//没有命令	
					break;
				case DBGCMD_FORWARD:   	//向前执行n条语
					if (m_lDbgCmdP1 == 0)
						m_lDbgCmdP1 = 1;
					m_nRemainCmdNum = m_lDbgCmdP1;
					break;
				case DBGCMD_HALT: 	//停止运行
				//	IsRunning = FALSE;
					m_ToStop.Set();
					m_nRemainCmdNum = 1;
					break;
				case DBGCMD_MEM:  	//显示内存
					{
						long bytenum;//要显示的字节数
						long bytestart;//显示的开始字节
						char sTempMsg[201] = "";
//						int j;
						
						if (m_lDbgCmdP1 >= m_pCurCall->ulDataSegSize)
						{
							snprintf(msg, 1000, "start address %xH is too large\n", m_lDbgCmdP1);
							lDbgRet = -1;
							break;
						}
						if (m_lDbgCmdP1 <4) 
							m_lDbgCmdP1 = 4;

						bytestart = m_lDbgCmdP1;

						if (bytestart + m_lDbgCmdP2 >= m_pCurCall->ulDataSegSize)
						{
							bytenum = m_pCurCall->ulDataSegSize - bytestart;
						}
						else
							bytenum = m_lDbgCmdP2;
						for (int i = 0; i< bytenum; i++)
						{
							snprintf(sTempMsg, 200, " %02x", m_pCurCall->DataSeg[bytestart+i]);
							strcat(msg, sTempMsg);

							if (i%16 == 0 && i != 0) 
							{
								strcat(msg, "\r\n");
							}
						}
						break;////////break可以在里面吗?
					}
				case DBGCMD_REG:				//显示寄存器
					{					
						strlwr(m_szDbgCmdP3);
						if (strlen(m_szDbgCmdP3) == 0)
						{//show all registers
							snprintf(msg, 1000, "PSW = %xH AX = %xH BX = %xH CX = %xH DX = %xH IP = %xH XX = %xH YX = %xH ZX = %xH\n", 
								m_registers[0], m_registers[1], m_registers[2], 
								m_registers[3], m_registers[4], m_registers[5], 
								m_registers[6], m_registers[6], m_registers[7]);
							break;							
						}
						switch(m_szDbgCmdP3[0])
						{
						case 'p':
								snprintf(msg, 1000, "PSW = %xH\n", m_registers[0]);break;
						case 'a':
								snprintf(msg, 1000, "AX = %xH\n", m_registers[1]);break;
						case 'b':
								snprintf(msg, 1000, "BX = %xH\n", m_registers[2]);break;
						case 'c': 	snprintf(msg, 1000, "CX = %xH\n", m_registers[3]);break;
						case 'd': 	snprintf(msg, 1000, "DX = %xH\n", m_registers[4]);break;						
						case 'i': 	snprintf(msg, 1000, "IP = %xH\n", m_registers[5]);break;
						case 'x': 	snprintf(msg, 1000, "XX = %xH\n", m_registers[6]);break;
						case 'y': 	snprintf(msg, 1000, "YX = %xH\n", m_registers[7]);break;
						case 'z': 	snprintf(msg, 1000, "ZX = %xH\n", m_registers[8]);break;
						default:	snprintf(msg, 1000, "no this register\n"); lDbgRet = -1;
						}
						break;
					}
				case DBGCMD_BP:   	//显示所有断点
					{
						int i;
//						int count;
						char sTemp[201] = "";
					

						for (i = 0; i< m_BpList.size(); i++)
						{
							snprintf(sTemp, 200, "IP:%d LINE: %d\n", m_BpList[i].lIP, m_pCurCall->pFunc->m_pCmdTable[m_BpList[i].lIP].line);
							strcat(msg, sTemp);
						}
						break;
					}
				case DBGCMD_SP:   	//设置断点
					if (SetBreakPoint(m_lDbgCmdP1))
					{
						//snprintf(msg, 1000,"set break point success.\n");
					}
					else
					{
						//snprintf(msg, 1000, "set break point failed.\n");
						lDbgRet = -1;
					}
					break;
				case DBGCMD_DP:   //删除断点
					if (DelBreakPoint(m_lDbgCmdP1))
					{
						//	snprintf(msg, 1000, "delete break point success.\n");
					}
					else
					{
						snprintf(msg, 1000, "delete break point failed.\n");
						lDbgRet = -1;
					}
					break;
				case DBGCMD_GO://取消单步
					m_nRemainCmdNum = 0xffffffff;
					break;
				default:
					{
						snprintf(msg, 1000, "invalid debug command: %d\n", m_lDbgCmdID);
						lDbgRet = -2;
					}
				}

				//清除命令
				m_lDbgCmdID = m_lDbgCmdP1 = m_lDbgCmdP1 = 0;
				memset(m_szDbgCmdP3, 0, 1024);
				CVirtualMachine::m_sDbgOutMsg = msg;
				CVirtualMachine::m_lDbgOutCode = lDbgRet;

				CVirtualMachine::m_DgbCmdLock.Unlock();

			}
		}
		m_bIsDebugBreak = FALSE;
/*		//set debug input		
		long dest;
		char input[1024];
		char incmd[20];
		BOOL bContinue = FALSE;
		if (m_nWorkMode >= VM_MODE_STEPDEBUG && !nodebug)
		{
			while (!bContinue)
			{
				memset(input, 0, 1024);
				memset(incmd, 0, 20);
				scanf("%s", input);
				int a;
				if (( a = sscanf(input, "%c %x", incmd, &dest)) < 2)
					sscanf(input, "%s", incmd);
				if (!strcmp(incmd, "r"))
				{
					switch(dest)
					{
					case 0: printf("PSW = %x\n", m_registers[dest]);break;
					case 1: printf("AX = %x\n", m_registers[dest]);break;
					case 2: printf("BX = %x\n", m_registers[dest]);break;
					case 3: printf("CX = %x\n", m_registers[dest]);break;
					case 4: printf("DX = %x\n", m_registers[dest]);break;
					case 5: printf("PSW = %x\n", m_registers[dest]);break;
					case 6: printf("IP = %x\n", m_registers[dest]);break;
					case 7: printf("XX = %x\n", m_registers[dest]);break;
					case 8: printf("YX = %x\n", m_registers[dest]);break;
					case 9: printf("ZX = %x\n", m_registers[dest]);break;
					default:printf("no this register\n");
					}
				}
				else if (!strcmp(incmd, "d"))
				{
					printf("%04x:", dest);
					for (int i = 0; i< 16; i++)
					{
						printf(" %02x", m_pCurCall->DataSeg[dest+i]);
					}
					printf("\r\n");
				}
				else if (!strcmp(incmd, "s"))
				{
					printf("%04x:", dest);
					for (int i = 0; i< 16; i++)
					{
						printf(" %02x", m_pCurCall->StaticSeg[dest+i]);
					}
					printf("\r\n");
				}
				else if (!strcmp(incmd, "g"))
				{
					bContinue = TRUE;
				}
				else if (!strcmp(incmd, "G"))
				{
					nodebug = 1;
					bContinue = TRUE;
				}
				else if (!strcmp(incmd, "m"))
				{
					printf("%04x:", dest);
					char temp;
					for (int i = 0; i< 16; i++)
					{
						
						_asm
						{
							mov EAX, [dest+i]
								mov temp, AH
						}
						
						printf(" %02x", temp);
					}
					printf("\r\n");
				}
				else
					printf("invalid command\r\n");
			}
			
		}*/
	}	

	m_Stopped.Set();
	return TRUE;
}



void CVirtualMachine::OutputMemToFile(FILE* file)
{
	if (file == NULL)
		return;
	
	if (m_pCurCall->pFunc == NULL)
		return;
	
	int i;
	char msg[1024]="";
	SYSTEMTIME time;
	GetLocalTime(&time);
	snprintf(msg, 1000, "%04d-%02d-%02d %02d:%02d:%0d\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	fwrite(msg, 1, strlen(msg), file);
	snprintf(msg, 1000, "Function Name: %s\r\n"
	//	"Current Line: %d\r\n"
		"Current IP: %d\r\n"
		"virtual memory segment: %x\r\n"
		"virtual memory segment size: %d\r\n"
		"static segment: %x\r\n"
		"static segment size: %x\r\n"
		"Data segment: %x\r\n"
		"Data segment size: %x\r\n"
		"AX = %x(%d)\r\n"
		"BX = %x(%d)\r\n"
		"CX = %x(%d)\r\n"
		"DX = %x(%d)\r\n"
		"PSW = %x(%d)\r\n",
		m_pCurCall->pFunc->m_szName,
		//m_pCurCall->pFunc->m_pCmdTable[m_pCurCall->IP].line,
//		m_pCurCall->IP,
		__IP,
		m_pCurCall->VMemory,
		m_pCurCall->ulVMemSize,
		m_pCurCall->StaticSeg,
		m_pCurCall->ulSSize,
		m_pCurCall->DataSeg,
		m_pCurCall->ulDataSegSize,
		__AX, __AX, __BX, __BX, __CX, __CX, __DX, __DX, __PSW, __PSW
		);

	fwrite(msg, 1, strlen(msg), file);
	snprintf(msg, 1000, "************************variable result*********************\r\n");
	fwrite(msg, 1, strlen(msg), file);
	
	//output command table
	int offset;
	for (i = 0; i < m_pCurCall->pFunc->m_SymbolTable.m_nSymbolCount; i++)
	{
		offset = m_pCurCall->pFunc->m_SymbolTable.tableEntry[i].address;
		snprintf(msg, 1000, "%s:   %04x\r\n",m_pCurCall->pFunc->m_SymbolTable.tableEntry[i].szName, *(int*)(m_pCurCall->DataSeg+offset));
		fwrite(msg, 1, strlen(msg), file);
		
	}
	snprintf(msg, 1000, "************************static segment*********************\r\n");
	fwrite(msg, 1, strlen(msg), file);
	
	int j;
	for (offset = 0; offset < m_pCurCall->pFunc->m_nSSUsedSize; )
	{
		snprintf(msg, 1000,  "%04x:", offset);
		fwrite(msg, 1, strlen(msg), file);
		
		for (j = 0;j <8; j++)
		{
			snprintf(msg, 1000, " %02x", (BYTE)(m_pCurCall->VMemory[offset]));
			fwrite(msg, 1, strlen(msg), file);
			offset++;
		}
		snprintf(msg, 1000, " - ", offset);
		fwrite(msg, 1, strlen(msg), file);
		for (j = 0;j <8; j++)
		{
			snprintf(msg, 1000, " %02x", (BYTE)(m_pCurCall->VMemory[offset]));
			fwrite(msg, 1, strlen(msg), file);
			offset++;
		}
		snprintf(msg, 1000, "\r\n", offset);
		fwrite(msg, 1, strlen(msg), file);
		
	}
	snprintf(msg, 1000, "************************data segment*********************\r\n");
	fwrite(msg, 1, strlen(msg), file);
	
	for (offset = 0; offset < m_pCurCall->pFunc->m_SymbolTable.m_nTotalSize; )
	{
		snprintf(msg, 1000, "%04x:", offset);
		fwrite(msg, 1, strlen(msg), file);
		for (j = 0;j <8; j++)
		{
			snprintf(msg, 1000,  " %02x", (BYTE)(m_pCurCall->DataSeg[offset]));
			fwrite( msg, 1, strlen(msg), file);
			offset++;
		}

		snprintf(msg, 1000, " - ", offset);
		fwrite(msg, 1, strlen(msg), file);

		for (j = 0;j <8; j++)
		{
			snprintf(msg, 1000,  " %02x", (BYTE)(m_pCurCall->DataSeg[offset]));
			fwrite(msg, 1, strlen(msg), file);
			offset++;
		}
		snprintf(msg, 1000, "\r\n", offset);
		fwrite(msg, 1, strlen(msg), file);
	}	
}

BOOL CVirtualMachine::OutputMemToFile(char* szFileName)
{

	//BOOL bRet;
	FILE* file = NULL;
	file = fopen(szFileName, "wb");
	if (file == NULL)
		return FALSE;
	OutputMemToFile(file);	
	fclose(file);

	return TRUE;
}



/*
函数名称     : CVirtualMachine::AttachParam
函数功能	    : attach parameters for script function call
BYTE *pParam:  参数的地址, 注意不是参数的内容
int size_t  :  参数内容的长度
变量说明     : 
返回值       : 
编写人       : 居卫华
完成日期     : 2001 - 4 - 27
*/
long CVirtualMachine::AttachParam(BYTE *pParam, int size_t)
{
	char sMsg[201] = "";
	if (pParam == NULL)
	{
		snprintf(sMsg, 200, "SE::AttachParam: input parameter block pointer is NULL, script: %s", m_pCurCall->pFunc->m_szName);
		nLOG(sMsg, 9);
		throw new CVMMException("SE::AttachParam: input parameter block pointer is NULL");
	//	return FALSE;
	}

	// modified on 20030331 by weihua ju
	// modified on 20030327 by weihua ju
	// moddified on 20110510 by weihua ju (jackie.ju@gmail.com)
	// doosn't check params size for virtual function call
	if (size_t != m_pCurCall->pFunc->m_iParamTotalSize)
	{
		snprintf(sMsg, 200, "SE::AttachParam: input parameter block size(%d) is not equal to function total parameters size(%d), script: %s", size_t, m_pCurCall->pFunc->m_iParamTotalSize, m_pCurCall->pFunc->m_szName);
		nLOG(sMsg, 9);
	//	return REQERR_PARAMNUMERROR;

	}
/*	if (size_t > m_pCurCall->pFunc->m_iParamTotalSize)
	{
		memcpy(m_pCurCall->DataSeg, pParam, m_pCurCall->pFunc->m_iParamTotalSize);
	}
	else*/// not check param size, this give flexibility to function call, but also bring risk.
		memcpy(m_pCurCall->DataSeg, pParam, size_t);
	return REQERR_NOERROR;
}


/*
函数声明：	BOOL CVirtualMachine::Preprocess1(PCOMMAND cmd, int &op1mode, int &op1reflvl, unsigned char* &dest, BOOL bValidate)
函数功能：	指令预处理（单操作数指令）
参数说明：	
			[IN]PCOMMAND cmd	-	指令内容
			[OUT]int &op1mode	-	操作数1的寻址方式字
			[OUT]int &op1reflvl	-	操作数1的间接级别
			[OUT]unsigned char* &dest	-	实际地址	
			[IN]BOOL bValidate	-	是否进行地址验证
返 回 值：	BOOL  - 成功或失败
编 写 人：	居卫华
完成日期：	2001-6-26
*/
BOOL CVirtualMachine::Preprocess1(PCOMMAND cmd, int &op1mode, int &op1reflvl, unsigned char* &dest, BOOL bValidate)
{
	char msg[201] = "";
	int i;
	op1mode = (cmd->address_mode>>8)&3;
	op1reflvl = (cmd->address_mode>>12)&0x3;
	switch (op1mode)
	{
	case AMODE_DIRECT:
		dest = (unsigned char*)&(cmd->op[0]);break;
	case AMODE_MEM:
		if (cmd->op[0]>= m_pCurCall->ulDataSegSize || cmd->op[0]<0)
		{
			REPORT_ERROR("virtual machine error: access violation\r\n", 90);
			return FALSE;
		}
		dest = (unsigned char*)&(m_pCurCall->DataSeg[cmd->op[0]]);break;
	case AMODE_REG:
		if (cmd->op[0]> 8 || cmd->op[0]<0)
		{
			REPORT_ERROR("virtual machine error: access violation\r\n", 90);
			return FALSE;
		}
		dest = (unsigned char*)&(m_registers[cmd->op[0]]);break;
	case AMODE_STATIC:	
		if (cmd->op[0]>= m_pCurCall->ulSSize || cmd->op[0]<0)
		{
			REPORT_ERROR("virtual machine error: access violation\r\n", 90);
			return FALSE;
		}
		
		dest = (unsigned char*)&(m_pCurCall->StaticSeg[cmd->op[0]]);break;
//	case AMODE_ARRAY: 
	//	dest = (unsigned char*)&(cmd->op[0]);break;
		
		//	dest = (unsigned char*)(m_pCurCall->DataSeg[__BX+ m_pCurCall->DataSeg[cmd->op[0]]]);break;
		
	// case AMODE_OBJ:
		// dest = (unsigned char*)&(m_pCurCall->DataSeg[cmd->op[0]]) ;
		// break;
		
	default:
		{
			snprintf(msg, 200, "invalid address mode, line %d, op %xH, addressmode %xH", cmd->line, cmd->op, cmd->address_mode);
			nLOG(msg, 50);
			return FALSE;
		}
	}
	
	if (bValidate&& op1mode != AMODE_DIRECT && !ValidateAddress(dest))
	{
		char cmdmsg[128];
		CFunction::OutCmd(cmd, cmdmsg);
		 snprintf(msg, 200, "access violation %s", cmdmsg);
		 nLOG(msg, 20);

		return FALSE;
	}

	for (i = 0; i< op1reflvl; i++)
	{
		//dest = &m_pCurCall->DataSeg[*dest];
		dest = (unsigned char*)(*((int*)dest));
		if (bValidate&&!ValidateAddress(dest))
		{
			char cmdmsg[128];
			CFunction::OutCmd(cmd, cmdmsg);

			snprintf(msg, 200, "access violation %s", cmdmsg);
			nLOG(msg, 20);
			return FALSE;
		}
	}

	return TRUE;
	
}

BOOL CVirtualMachine::Preprocess2(PCOMMAND cmd, int &op1mode, int &op2mode, int &op1reflvl, int &op2reflvl, unsigned char * &dest, unsigned char* &src)
{
	char msg[201] = "";
	int i;
	op1mode = (cmd->address_mode>>8)&0x3;
	op2mode = cmd->address_mode&0x0003;
	op2reflvl = (cmd->address_mode>>4)&0x3;
	op1reflvl = (cmd->address_mode>>12)&0x3;
	
	switch (op1mode)
	{
	case AMODE_DIRECT:
		dest = (unsigned char*)&(cmd->op[0]);break;
	case AMODE_MEM:
		if (cmd->op[0]>= m_pCurCall->ulDataSegSize || cmd->op[0]<0)
		{
			REPORT_ERROR("virtual machine error: access violation\r\n", 90);
			return FALSE;
		}
		dest = (unsigned char*)&(m_pCurCall->DataSeg[cmd->op[0]]);break;
	case AMODE_REG:
		if (cmd->op[0]> 8 || cmd->op[0]<0)
		{
			REPORT_ERROR("virtual machine error: access violation\r\n", 90);
			return FALSE;
		}
		dest = (unsigned char*)&(m_registers[cmd->op[0]]);break;
	case AMODE_STATIC:		
		if (cmd->op[0]>= m_pCurCall->ulSSize || cmd->op[0]<0)
		{
			REPORT_ERROR("virtual machine error: access violation", 90);
			return FALSE;
		}
		dest = &(m_pCurCall->StaticSeg[cmd->op[0]]);break;
/*	case AMODE_ARRAY: 
		dest = (unsigned char*)&(cmd->op[0]);break;
		//		dest = (unsigned char*)(m_pCurCall->DataSeg[__BX+ m_pCurCall->DataSeg[cmd->op[0]]]);break;
	*/	

	// case AMODE_OBJ:
	// 	dest = (unsigned char*)&(m_pCurCall->DataSeg[cmd->op[0]]) ;
	// 	break;
	default:
		{			
			snprintf(msg, 200, "SE:: invalid address mode, line %d, op %xH, addressmode %xH", cmd->line, cmd->op, cmd->address_mode);
			nLOG(msg, 9);
			return FALSE;
		}
	}
	if (op1mode != AMODE_DIRECT&&!ValidateAddress(dest))
	{
		char cmdmsg[128];
		CFunction::OutCmd(cmd, cmdmsg);

		snprintf(msg, 200, "access violation (line: %d) %s", cmd->line, cmdmsg);
		nLOG(msg, 9);

		return FALSE;
	}

	for ( i = 0; i< op1reflvl; i++)
	{
		//	dest = &m_pCurCall->DataSeg[*((int*)dest)];
		dest = (unsigned char *)(*((int*)dest));
		if (!ValidateAddress(dest))
		{	
			char cmdmsg[128];
			CFunction::OutCmd(cmd, cmdmsg);
			
			snprintf(msg, 200, "SE:: access violation (line: %d) ", cmd->line, cmdmsg);
			nLOG(msg, 9);

			return FALSE;
		}
		
	}

	switch (op2mode)
	{
	case AMODE_DIRECT:
		src = (unsigned char*)&(cmd->op[1]);break;
	case AMODE_MEM:
		if (cmd->op[1]>= m_pCurCall->ulDataSegSize || cmd->op[1]<0)
		{
			REPORT_ERROR("virtual machine error: access violation\r\n", 90);
			return FALSE;
		}
		
		src = (unsigned char*)&(m_pCurCall->DataSeg[cmd->op[1]]);break;
	case AMODE_REG:
		if (cmd->op[1]> 8 || cmd->op[1]<0)
		{
			REPORT_ERROR("virtual machine error: access violation\r\n", 90);
			return FALSE;
		}
		
		src = (unsigned char*)&(m_registers[cmd->op[1]]);break;
	case AMODE_STATIC:
		if (cmd->op[1]>= m_pCurCall->ulSSize || cmd->op[1]<0)
		{
			REPORT_ERROR("virtual machine error: access violation\r\n", 90);
			return FALSE;
		}
		
		src = (unsigned char*)&(m_pCurCall->StaticSeg[cmd->op[1]]);break;
//	case AMODE_ARRAY: 
	//	src = (unsigned char*)&(cmd->op[1]);break;
		
		//		src = (unsigned char*)(m_pCurCall->DataSeg[__BX+ m_pCurCall->DataSeg[cmd->op[1]]]);break;
	// case AMODE_OBJ:
	// 	dest = (unsigned char*)&(m_pCurCall->DataSeg[cmd->op[0]]) ;
	// 	break;
	default:
		{			
			snprintf(msg, 200, "SE:: invalid address mode, line %d, op %xH, addressmode %xH", cmd->line, cmd->op, cmd->address_mode);
			nLOG(msg, 9);
			return FALSE;
		}
	}
	
	if (op2mode != AMODE_DIRECT && !ValidateAddress(src))
	{
		char cmdmsg[128];
		CFunction::OutCmd(cmd, cmdmsg);
		
		snprintf(msg, 200, "SE:: access violation (line: %d) %s", cmd->line, cmdmsg);
		nLOG(msg, 9);
		return FALSE;
	}
	for ( i = 0; i< op2reflvl; i++)
	{
		//src = &m_pCurCall->DataSeg[*((int*)src)];
		src = (unsigned char*)(*((int*)src));
		if (!ValidateAddress(src))
		{
			char cmdmsg[128];
			CFunction::OutCmd(cmd, cmdmsg);
			
			snprintf(msg, 200, "SE:: access violation (line: %d) ", cmd->line, cmdmsg);
			nLOG(msg, 9);
			return FALSE;
		}
	}
	return TRUE;
}


void CVirtualMachine::SetWorkMode(int mode)
{
	m_nWorkMode = mode;
}


/*
函数声明：	BOOL CVirtualMachine::ValidateAddress(unsigned char* address)
函数功能：	验证地址是否合法
参数说明：	
			[IN]unsigned char* address	-	地址
返 回 值：	BOOL  - 成功或失败
编 写 人：	居卫华
完成日期：	2001-6-26
*/
BOOL CVirtualMachine::ValidateAddress(unsigned char* address)
{
	CALLSTACKELE* pCall = NULL;
	// CALL_STACK  stack = m_CallStack;
	long s = m_CallStack.size();
	int i = 0;
	for (; i < s; i++ )
	{
		pCall = &(m_CallStack[i]);
		if (address>= pCall->VMemory && address < pCall->VMemory + pCall->ulVMemSize)
			return TRUE;
	}

	if (address >= (unsigned char*)&m_registers[0] && address < (unsigned char*)&m_registers[LASTREGISTER])
		return TRUE;
	
	for ( i = 0; i< m_lExternalSpaceNum; i++)
	{
		if (address >= (unsigned char*)m_pExternalSpaceList[i].ulExternalSpaceAddress && address < (unsigned char*)m_pExternalSpaceList[i].ulExternalSpaceAddress + m_pExternalSpaceList[i].ulExternalSpaceSize)
			return TRUE;
	}
	
	return FALSE;
}

void CVirtualMachine::AddExternalSpace(unsigned char *start, unsigned long size)
{
	if (m_lExternalSpaceListSize == m_lExternalSpaceNum)
	{
		EXTERNALSPACE* pTemp = new EXTERNALSPACE[m_lExternalSpaceListSize + 10];
		if (pTemp == NULL)
		{
			//REPORT_MEM_ERROR("not enough memory when virtual machine alloc external space description list\r\n")
			throw new CVMMException("not enough memory when virtual machine alloc external space description list");
		//	return FALSE;
		}
		memcpy(pTemp, m_pExternalSpaceList, sizeof(EXTERNALSPACE)*  m_lExternalSpaceNum);
		delete m_pExternalSpaceList;
		m_pExternalSpaceList = pTemp;
		m_lExternalSpaceListSize += 10;
	}
	m_pExternalSpaceList[m_lExternalSpaceNum].ulExternalSpaceAddress = (long)start;
	m_pExternalSpaceList[m_lExternalSpaceNum].ulExternalSpaceSize = size;
	m_lExternalSpaceNum++;
//	return TRUE;
}

BOOL CVirtualMachine::_cast(PCOMMAND cmd)
{
	char msg[201] = "";
	CMD_PREPROCESS2
	long op3 = cmd->op[2];//转换操作的类型
	switch (op3)
	{
		//integer -> float
	case __CAST_L2F:
	case __CAST_I2F:
		*(float*)dest = (float)(*(long*)src);
		break;
	case __CAST_C2F:
		*(float*)dest = (float)(*(char*)src);		
		break;
	case __CAST_S2F:
		*(float*)dest = (float)(*(short*)src);		
		break;

		//* -> char
	case __CAST_S2C:
		*(char*)dest = (char)(*(short*)src);
		break;
	case __CAST_L2C:
	case __CAST_I2C:
		*(char*)dest = (char)(*(long*)src);
		break;
	
		//* -> short
	case __CAST_C2S:
		*(short*)dest = (short)(*(char*)src);
		break;
	case __CAST_I2S:
	case __CAST_L2S:
		*(short*)dest = (short)(*(long*)src);
		break;

		//* -> long||int
	case __CAST_C2L:
	case __CAST_C2I:
		*(long*)dest = (long)(*(char*)src);
		break;
	case __CAST_S2L:
	case __CAST_S2I:
		*(long*)dest = (long)(*(short*)src);
		break;
		//long -> int
	case __CAST_L2I:
	case __CAST_I2L:
		*(long*)dest = (*(long*)src);
		break;

	default:
		
		snprintf(msg, 200, "SE:: cast command with invalid cast type code (line = %d)", cmd->line);
		nLOG(msg, 9);
		return  FALSE;
		break;
	}
	__IP++;
	return TRUE;	
}

/**
 *  move value of src to dest
 * op[0] - dest:  
 * op[1] - src: 
 * op[2] - dest type:  
 * op[3] - src type: 
 * NOTIC: cRef* should never be assigned to another variable, use dest_ref->setRef() instead
 */

BOOL CVirtualMachine::_movobj(PCOMMAND cmd)
{
	char msg[201] = "";
	CMD_PREPROCESS2
	//long op3 = cmd->op[2];// dest type 
	long dest_type = (cmd->op[2] & 0xf0 ) >> 4;	
	long dest_reflevel = cmd->op[2] & 0x0f;
	//long op4 = cmd->op[3];// src type
	long src_type = (cmd->op[3] & 0xf0 ) >> 4;	
	long src_reflevel = cmd->op[3] & 0x0f;
	long src_mode = op2mode;
	long dest_mode = op1mode;
	printf("cast %d(%d) to %d(%d)\n", src_type, src_reflevel, dest_type, dest_reflevel);
	
	if (dest_type == dtGeneral){ // if dest is object
		// TODO find and call "=" operator method first, if failed then do address copy or create new object
			CRef *dest_ref = *(CRef**)dest;
			printf("dest_ref=%x\n", dest_ref);
		if (src_type == dtGeneral ){ // object => object
			CRef* src_ref = *(CRef**)src;
			if (src_ref)
				printf("src=%lx, src_ref=%lx, src_obj=%lx\n", src, src_ref, src_ref->getRef());
			else
				printf("src=%lx, src_ref=%lx, src_obj=N/A\n", src, src_ref);
			if (src_ref == NULL || src_ref->getRef() == NULL){
				ERR("object not initialzed");
				return FALSE;
			}
		//	if (dest_ref)
			//	removeRef(dest_ref);
		//	*(CRef**)dest = src_ref;
			if (dest_ref == NULL){
				dest_ref = createRef();
				*(CRef**)dest = dest_ref;
						
				printf("new dest_ref=%lx\n", dest_ref);
			}else{
				dest_ref->release();
			}
		printf("set content of address %lx to ref %lx to object %lx...\n", dest, dest_ref, src_ref->getRef());
			dest_ref->setRef(  src_ref->getRef() ); // copy address of object
			printf("set content of address %lx to ref %lx to object %lx\n", dest, dest_ref, dest_ref->getRef());

	
		}else{ // primitive => object, 
		
		
			CObjectInst* obj = NULL;
				// release current object instance
			if (dest_ref == NULL){ //if src is primitive and dest is not initialized object reference
				CRef* r = createRef();
				*(CRef**)dest = r;
				dest_ref = *(CRef**)dest;
			}else {
				dest_ref->release();			
			}
			
			// create new object instance
				obj = CObjectInst::createObject(NULL);
				dest_ref->setRef(obj);
			
				debug("new object %x, ref %x", obj, dest_ref);
				
			if ( (src_reflevel == 1 && src_type == dtChar) || src_type == dtStr){
				if (src_mode != AMODE_STATIC){
					printf("obj1=%x ", obj);
					printf("src=%s\n", *(char**)src);
					obj->setValue(dtStr, src);
				}else{
					printf("obj=%x ", obj);
					printf("src=%s(%x)\n", (char*)src, src);
					obj->setValue(dtStr, &src);
				}
			}else if (src_reflevel > 0 )
				obj->setValue(dtLong, src);
			else{
				debug("set value %x(%x)", *(long*)src, src);
				obj->setValue(src_type, src);
			}
	   }
   }else { // dest is primitive type
		if (src_type == dtGeneral ){  // object => primitive
			CRef * src_ref = *(CRef**)src;
			CObjectInst *obj = NULL;
			printf("src_ref=%lx\n", src_ref);
			if (src_ref == NULL || src_ref->getRef() == NULL){
				ERR("object not initialzed");
				return FALSE;
			}
			obj = (CObjectInst *)src_ref->getRef();
			if (dest_reflevel>0){ // object => char*
				if (dest_type == dtChar  || dest_type == dtStr){
					printf("obj=%x\n", obj);
				//	printf("src=%s\n", (char*)obj->getSValue().c_str());
				//	printf("src=%s\n", *(char**)obj->getValueAddress());
					*(char**)dest = (char*)obj->getSValue();
				}else
				*(long*)dest = obj->getValue().l;
	    	}else{ // object -> int, long, short ...
				switch (dest_type){
					case dtInt:
					*(int*)dest = obj->getValue().i;
					break;
					case dtUInt:
					*(int*)dest = obj->getValue().ui;
					break;
					case dtShort:
					*(int*)dest = obj->getValue().st;
					break;
					case dtUShort:
					*(int*)dest = obj->getValue().ust;
					break;
					case dtLong:
					*(int*)dest = obj->getValue().l;
					break;
					case dtULong:
					*(int*)dest = obj->getValue().ul;
					break;
					case dtChar:
					*(int*)dest = obj->getValue().c;
					break;
					case dtUChar:
					*(int*)dest = obj->getValue().uc;
					break;
					case dtFloat:
					*(int*)dest = obj->getValue().f;
					break;
					default:
					fprintf(stderr, "Execute __movobj failed, dest type %d not support when move a object to primitive", src_type);
				}
			}
		}else{ // primitive => primitive
			short opsize;//是字节操作, 字操作, 还双字操作
			//opsize = __min((cmd->address_mode >> 14)&0x3 ,(cmd->address_mode >> 6)&0x3);
			   if (((cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
			       opsize = (cmd->address_mode >> 6)&0x3;
			   else
			       opsize = (cmd->address_mode >> 14)&0x3;
			switch (opsize)
			{
			case 0:	memcpy(dest, src, 1);break;
			case 1:	memcpy(dest, src, 2);break;
			case 2:	memcpy(dest, src, 4);break;
			case 3:	memcpy(dest, src, 8);break;
			}
		}
	}

	__IP++;
	return TRUE;	
}
BOOL CVirtualMachine::_fadd(PCOMMAND cmd)
{
	char msg[201] = "";
	CMD_PREPROCESS2
	short opsize;//是字节操作, 字操作, 还双字操作
	//	opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
    if (( (cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
        opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	float temp;
	switch (opsize)
	{
		case 2:	
			temp = *((float*)dest) + *((float*)src);
			__AX = *(long*)(&temp);
			break;
		default:			
			snprintf(msg, 200, "invalid operation size(line = %d)", cmd->line);
			nLOG(msg,  9);
			return FALSE;
	}
	__IP++;
	return TRUE;
}
BOOL CVirtualMachine::_fsub(PCOMMAND cmd)
{
	CMD_PREPROCESS2
	short opsize;//是字节操作, 字操作, 还双字操作
//	opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
    if (( (cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
        opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	float temp;
	switch (opsize)
	{
		case 2:	
			temp = *((float*)dest) - *((float*)src);
			__AX = *(long*)&temp;
			break;
		default:
			char msg[201] = "";
			snprintf(msg, 200, "SE:: invalid operation size(line = %d)", cmd->line);
			nLOG(msg,  9);
			return FALSE;
	}

	__IP++;
	return TRUE;
}
BOOL CVirtualMachine::_fmul(PCOMMAND cmd)
{
	CMD_PREPROCESS2
	short opsize;//是字节操作, 字操作, 还双字操作
//	opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
    if (( (cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
        opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	float temp;
	switch (opsize)
	{
		case 2:
			temp = *((float*)dest) * *((float*)src);
			__AX = *(long*)&temp;break;
		default:
			char msg[201] = "";
			snprintf(msg, 200, "SE:: invalid operation size(line = %d)", cmd->line);
			nLOG(msg,  9);
			return FALSE;
	}

	__IP++;
	return TRUE;
}
BOOL CVirtualMachine::_fdiv(PCOMMAND cmd)
{
	char szMsg[201] = "";
	CMD_PREPROCESS2
	short opsize;//是字节操作, 字操作, 还双字操作
//	opsize= __max( (cmd->address_mode >> 14)&0x3, (cmd->address_mode >> 6)&0x3);
    if (( (cmd->address_mode >> 14)&0x3)>((cmd->address_mode >> 6)&0x3))
        opsize = (cmd->address_mode >> 14)&0x3;
    else
        opsize = (cmd->address_mode >> 6)&0x3;
	float temp;
	switch (opsize)
	{
		case 2:
			if (*((float*)src) ==0)
			{
			snprintf(szMsg, 200, "VM:: devide zero error, function='%s', line=%d, IP=%d\r\n", m_pCurCall->pFunc->m_szName, cmd->line, __IP);
			REPORT_ERROR(szMsg, 20);
				return FALSE;
			}
			temp = *((float*)dest) / *((float*)src);
			__AX = *(long*)&temp;break;
		default:

			
			snprintf(szMsg, 200, "invalid operation size(line = %d)", cmd->line);
			nLOG(szMsg,  9);
			return FALSE;
	}
	__IP++;
	return TRUE;
}


BOOL CVirtualMachine::_loadlib(PCOMMAND cmd){
	char szMsg[201] = "";
	CMD_PREPROCESS1
	
	char szLibFile[_MAX_PATH] = "";
	strcpy(szLibFile, (const char*)dest);
#ifdef WIN32
	strcat(szLibFile, ".dll");
#else
	strcat(szLibFile, ".so");
#endif
	// TODO load library in pubfunction table

	__IP++;
	return TRUE;
}

/*
   函数名称     : CVirtualMachine::SetDbgCmd
   函数功能	    : 传入调试命令
   变量说明     : 
   long lCmdID  : debug命令的ID
   long p1      : 参数1
   long p2      : 参数2
   char* p3		: 参数3(长度不超过1024byte, 包括\0)
   返回值       : 无
   编写人       : 居卫华
   完成日期     : 2001 - 6 - 12
*/
void CVirtualMachine::SetDbgCmd(long lCmdID, long p1, long p2, char* p3)
{
	m_lDbgCmdID = lCmdID;
	m_lDbgCmdP1 = p1;
	m_lDbgCmdP2 = p2;
	strcpy(m_szDbgCmdP3, p3);
}





/*
   函数名称     : CVirtualMachine::SetBreakPoint
   函数功能	    : 
   变量说明     : 
   返回值       : 
   编写人       : 居卫华
   完成日期     : 2001 - 6 - 13
*/
BOOL CVirtualMachine::SetBreakPoint(long line)
{
//	if (m_BreakPoint == NULL)
//		return FALSE;

	long IP;
	long i;

	//得到改行第一条指令的IP
	for (i = 0; i< m_pCurCall->pFunc->m_nCurrentCmdNum; i++)
	{
		if (m_pCurCall->pFunc->m_pCmdTable[i].line > line)
			return FALSE;
		if (m_pCurCall->pFunc->m_pCmdTable[i].line == line)
		{
			IP = i;
			break;
		}
	}

	//IP是否在指令数范围内
	if (IP >= m_pCurCall->pFunc->m_nCurrentCmdNum)
		return FALSE;

	//查找是否重复
	for (i = 0; i< m_BpList.size(); i++)
	{
		if (m_BpList[i].lIP == IP)
			return FALSE;
	}
	
	BP bp;
	bp.lIP = IP;
	strcpy(bp.m_szName, m_pCurCall->pFunc->m_szName);
	//	m_BpList[i].lIP = IP;
	//	m_BpList.size() = i + 1;
	m_BpList.push_back(bp);

	return TRUE;
}

/*
   函数名称     : CVirtualMachine::DelBreakPoint
   函数功能	    : 
   变量说明     : 
   返回值       : 
   编写人       : 居卫华
   完成日期     : 2001 - 6 - 13
*/
BOOL CVirtualMachine::DelBreakPoint(long line)
{
//	if (m_BreakPoint == NULL)
//		return	FALSE;
	long i;
	for (i = 0; i< m_BpList.size(); i++)
	{
		if (m_pCurCall->pFunc->m_pCmdTable[m_BpList[i].lIP].line == line)
		{
		//	memcpy(&m_BpList[i].lIP, &m_BreakPoint[i+1], sizeof(long)*(m_BpList.size() - i -1));
		//	m_BreakPoint[m_BpList.size() - 1] = -1;
		//	m_BpList.size() --;
			m_BpList.erase(m_BpList.begin() + i);
			return TRUE;
		}
	}	
	return FALSE;
}

void CVirtualMachine::DumpToFile()
{
	char sMsg[201] = "";
	///////////////////////// dmp memory to file /////////////////////////
	char sDmpFile[_MAX_PATH]  = "";
	time_t tm;
	time(&tm);
	struct tm* local_time = localtime(&tm);
	snprintf(sDmpFile, 200, 
		"/SEDMP%04d%02d%02d.%02d%02d%02d", 
		local_time->tm_year, 
		local_time->tm_mon,
		local_time->tm_mday, 
		local_time->tm_hour,
		local_time->tm_min,
		local_time->tm_sec);
	
	if (!OutputMemToFile(sDmpFile))
	{
		snprintf(sMsg, 200, "SE:: dump VM memroy to file %s failed", sDmpFile);
		nLOG(sMsg, 0);		
	}
	else
	{
		snprintf(sMsg, 200, "SE:: dump VM memroy to file %s ok", sDmpFile);
		nLOG(sMsg, 0);		
	}
	///////////////////////////////////////////////////////////////////////////
}

void CVirtualMachine::GetStatus(VMSTATUS* status)
{
		if (status == NULL)
			return;

		m_StatusLock.Lock();
		
		CALLSTACKELE* pCurCall = NULL;
		int i = 0;
		for (i = 0 ; i< m_CallStack.size(); i++)
		{
			pCurCall = &(m_CallStack[i]);
			status->sCallStack.push_back(pCurCall->pFunc->m_szName);
			if (i==0)
				status->lines.push_back(m_pCurCall->pFunc->m_pCmdTable[m_pCurCall->IP].line);
			else
				status->lines.push_back(pCurCall->pFunc->m_pCmdTable[pCurCall->IP].line);
			
		}
		m_StatusLock.Unlock();
}


CClassDes* getClassDes(char* name){
	return CCompiler::classDesTable.getClass((char*)(name));
}

// load object and exectue constructor
CObjectInst* CVirtualMachine::LoadObject(char* name){
	printf("==>LoadObject %s\n", name);
	CClassDes* pc = NULL;
	pc = getClassDes(name);
	if (pc == NULL){
		fprintf(stderr, "Load Object for %s failed!", name);
		return NULL;
	}
	return LoadObject(pc);
}

void CVirtualMachine::execFunction(char* name, void*pParam){
	char szName[256] = "";
	strcpy(szName, name);
	char* p = strchr(szName, ':');
	
	char* className = "_global_";
	if (p){
			fprintf(stderr, "p=%s\n", p);
		*p = 0;
		p++;
		className = szName;
	}
	CClassDes* pc = NULL;
	fprintf(stderr, "class=%s\n", className);
	pc = getClassDes(className);
	if (pc == NULL){
		fprintf(stderr, "Load class for %s failed!", className);
		return;
	}	
	fprintf(stderr, "Load class for %s OK!", className);
	char* functionName = szName;
	if (p){
		while (*p == ':')
			p++;
		functionName = p;
	}
	fprintf(stderr, "method=%s\n", functionName);
	CFunction* pFunc = pc->getMethod(functionName);
	if (pFunc == NULL){
		fprintf(stderr, "Load function for %s failed!", functionName);
		return;
	}
	LoadFunction(pFunc);
	if (p){
		long param = (long)p;
		AttachParam((BYTE*)&param, sizeof(long));
	}
	Run();
		
}

CClass* CVirtualMachine::loadClass(char* name){
	// lookup regestered class instance, create new class instance if not foudn
	CClass* pClass = NULL;
	pClass = m_classTable.getClass(name);
	if (pClass == NULL){
		CClassDes *c = getClassDes(name);
		if (c == NULL)
			return NULL;
		pClass = m_classTable.createClassInst(c);
	}
	printf("==>class %s found\n", name);
	return pClass;
}

CObjectInst* CVirtualMachine::createObject(char* className){
	CClass* pClass = NULL;
	if (className !=  NULL){
		// load class instance
		pClass = loadClass(className);
	}

	// create object according to class instance
	CObjectInst* obj = m_objTable.createObjectInstance(pClass);
	return obj;
}
	
// create new object and execute it's constructor
CObjectInst* CVirtualMachine::LoadObject(CClassDes* c, void* p){
	printf("==>LoadObject %x, %x \n", c, p);
	
	CObjectInst* obj = createObject(c->GetFullName());

	printf("==>create instance %x OK\n", obj);
	
	// not need to load "create" method, because class is also a function which will call its "create" method
	
	//long index = 1;
	this->Reset();
	
/*	//CFunction* pfn = c;
//	LoadFunction(pfn);
//	printf("==>LoadFunction OK\n");
//	void* pthis = obj;
//	CRef* ref = new CRef("this");
//	ref->setRef(obj);
	
	CRef* ref_p = new CRef("p");
	CAttribute *attr = new CAttribute();
	attr->setValue(dtLong, &p);
	ref_p->setRef(attr);
	long** buf_param = new long*[2];
	memset(buf_param, 0, 2*sizeof(long*));
	buf_param[0] = (long*)&ref;
	buf_param[1] = (long*)&ref_p;
//	AttachParam((BYTE*)&ref, sizeof(long*));
	AttachParam((BYTE*)buf_param, sizeof(long*)*2);
	printf("==>AttachParam OK\n");
	this->Run();
	delete buf_param;
*/
	c->getFuncTable()->dump();
	CFunction* pfn = c->getMethod("create");
		printf("==>LoadFunction... %lx'%s'\n", pfn, pfn->name());
	if (pfn != NULL){
		LoadFunction(pfn);
		printf("==>LoadFunction '%s' OK\n", pfn->name());
		void* pthis = obj;
		CRef* ref = createRef("this");
		ref->setRef(obj);
	
	//	AttachParam((BYTE*)&ref, sizeof(long*));
		CRef* ref_p = createRef("p");
		/*CAttribute *attr = new CAttribute();
		attr->setValue(dtLong, &p);
		ref_p->setRef(attr);*/
		CObjectInst* oi = createObject(NULL);
		oi->setValue(dtLong, &p);
		ref_p->setRef(oi);
		
		long** buf_param = new long*[2];
		memset(buf_param, 0, 2*sizeof(long*));
		buf_param[0] = (long*)ref_p;
		buf_param[1] = (long*)ref_p;
	//	AttachParam((BYTE*)&ref, sizeof(long*));
		AttachParam((BYTE*)buf_param, sizeof(long*)*2);
		printf("==>AttachParam for '%s' OK\n", pfn->name());
	
		this->Run();
		delete buf_param;
	}
	return obj;

}