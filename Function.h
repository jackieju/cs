// Function.h: interface for the CFunction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FUNCTION_H__521D16E5_8E52_45FD_98E2_6A6DD4F00749__INCLUDED_)
#define AFX_FUNCTION_H__521D16E5_8E52_45FD_98E2_6A6DD4F00749__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "types.h"
#include "COMMONDEF.H"
#include "SymbolTable.h"

/*typedef struct _tagData
{
	long size_t;              //使用的长度
	long MaxSize_t;           //总长度
	unsigned char *m_pData;   //数据空间
}DATA;
*/
typedef struct _tagExpressionDigit
{
	long digit;				// operand
	int  type;            //  address mode, 0: 常量 1: 变量地址 2:寄存器
	TYPEDES dtType;			// variable description
	std::string name;		// variable
	_tagExpressionDigit* pPrev;
}EXPRESSIONDIGIT;//表达式使用的操作数栈的节点

#define CMDTABLEINITSIZE 20                //指令数组的初始长度
#define DATASEGMENTINITSIZE 30             //数据段的初始长度

class CSymbolTable;
class CFunction  
{
public:
	static long OutCmd(COMMAND* pcmd, char *buffer);
	long OutupCmd(long index, char* buffer);           //输出指令内容
	bool AddCommand(COMMAND& cmd);                     //添加指令
	int GetSymbolAddress(char* szName);                //根据名称取符号地址 
	void Output(char* szName);                         //输出函数
	bool AddCommand(long opcode, long address_mode, long opnum, long *opTable, long lineNum);//添加指令
	bool AddVal(char* szName, TYPEDES type);           //向符号表添加变量
	int AddStaticData(int size_t, unsigned char* pData);       //向符号表添加常量
	CFunction();
	virtual ~CFunction();
	char* name(){
		return m_szName;
	};
	void setName(char* n){
		if (strlen(n) > 1000)
		{
			nLOG("member name too long", 300);
			return;
		}
		if (n == NULL){
			printf("name is NULL");
			return;
		}
		if (n != NULL)
			strncpy(m_szName, n, 1000);
	}

//	void SetAX(long val);
//	BOOL MoveCmd(unsigned int uFrom, unsigned int uTo);//不能使用该功能， 因为她会导致跳转语句出错
	//long AX, BX, CX, DX;                    //寄存器
	
#define VM_MODE_NORMAL 0
#define VM_MODE_DEBUG  1
#define VM_MODE_STEPDEBUG 2
	int m_nWorkMode;
	
	int m_iParamNum;					//参数个数
	int m_iParamTotalSize;				//参数空间长度
	char m_szName[1024];					//函数名称

	PCOMMAND m_pCmdTable;				   //指令表
	int m_nCurrentCmdNum;                  //当前的命令数
	int m_nCmdTableSize;                   //指令数组的当前最大长度
	
	CSymbolTable m_SymbolTable;           //数据段符号表

//	CSymbolTable m_pointTable;            //指针变量符号表

	unsigned char* m_staticSegment;                 //常量段
	int   m_nSSUsedSize;                     //以使用的长度
	int   m_nSSCurMaxSize;                   //目前的数据块大小
#define STATICSEGINCSIZE  1024       //常量段的增量

//	DATA m_dataSegment;                   //数据段
	
//	DATA m_StackSegment;                  //堆栈段

	CFunction *parent; // parent , can be a class or function
	
	void setParent(CFunction* p){
		parent = p;
	}
	CFunction* getParent(){
		return this->parent;
	}
protected:

public:
			//运算数栈
	EXPRESSIONDIGIT m_ExpDigit;
   	EXPRESSIONDIGIT* m_pExpDigitPt;//栈指针
		/*
	   函数名称     : cParser::ClearExpStack
	   函数功能	    : 清空表达式栈
	   变量说明     : 
	   返回值       : 
	   编写人       : 居卫华
	   完成日期     : 2001 - 4 - 17
	*/
	void ClearExpStack()
	{
		while (m_pExpDigitPt->pPrev != NULL)
		{
			EXPRESSIONDIGIT *pTemp = m_pExpDigitPt;
			this->m_pExpDigitPt = this->m_pExpDigitPt->pPrev; 		
			delete pTemp;
		}
		m_pExpDigitPt = &m_ExpDigit;
	}
	BOOL PopDigit(long *digit, long *type, TYPEDES *dtType)
	{
		if (digit == NULL || type == NULL)
			return FALSE;
		*digit = 0;
		*type = -1;
	//	printf("===>33331, %x,", this);
		memset(dtType, 0, sizeof(TYPEDES));
	//	printf("===>3333, %x,", this);
		if (this->m_pExpDigitPt->pPrev == NULL)
			return FALSE;
		*digit = m_pExpDigitPt->digit;
		*type = m_pExpDigitPt->type;
		memcpy(dtType, &(m_pExpDigitPt->dtType), sizeof(TYPEDES));
		EXPRESSIONDIGIT *pTemp = m_pExpDigitPt;
		m_pExpDigitPt = m_pExpDigitPt->pPrev;
		delete pTemp;

			printf("pop address=%04x, type=%04x, line:%04d\r\n", *digit, *type, __LINE__);

		return TRUE;
	}
	BOOL PopDigit(long *digit, long *type, TYPEDES *dtType, char* name)
	{
		if (digit == NULL || type == NULL)
			return FALSE;
		*digit = 0;
		*type = -1;
	//	printf("===>33331, %x,", this);
		memset(dtType, 0, sizeof(TYPEDES));
	//	printf("===>3333, %x,", this);
		if (this->m_pExpDigitPt->pPrev == NULL)
			return FALSE;
		*digit = m_pExpDigitPt->digit;
		*type = m_pExpDigitPt->type;
		memcpy(dtType, &(m_pExpDigitPt->dtType), sizeof(TYPEDES));
		strcpy(name, m_pExpDigitPt->name.c_str());
		EXPRESSIONDIGIT *pTemp = m_pExpDigitPt;
		m_pExpDigitPt = m_pExpDigitPt->pPrev;
		delete pTemp;

		printf("pop name=%s, address=%04x, type=%04x, line:%04d\r\n", name, *digit, *type, __LINE__);
		return TRUE;
	}
	void PushDigit(long digit, long type, TYPEDES dtType, std::string name = "")
	{

		if (dtType.type < dtFirstType || dtType.type > dtLastType)
		{
		//	GenError(125);
			return;
		}
		EXPRESSIONDIGIT *pNew = new EXPRESSIONDIGIT;	
		if (pNew == NULL)
		{
			REPORT_MEM_ERROR("memory allocation error");
			return;
		}
		pNew->digit = digit;
		pNew->type = type;
		pNew->name = name;
		memcpy(&pNew->dtType, &dtType, sizeof(TYPEDES));
		pNew->pPrev = this->m_pExpDigitPt;
		printf("push name=%s, address=%04x, type=%04x, line:%04d\r\n", (char*)name.c_str(), digit, type, __LINE__);
		m_pExpDigitPt = pNew;
		
	}
};

#endif // !defined(AFX_FUNCTION_H__521D16E5_8E52_45FD_98E2_6A6DD4F00749__INCLUDED_)

