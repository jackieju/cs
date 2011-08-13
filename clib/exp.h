#ifndef __EXP_H__
#define __EXP_H__


#include <exception>

#define MSGSIZE 201		// message buffer size

#define err_basic_base -1000	// base of error code value
#define err_user_base  -100000	// base of sub class error code value

namespace clib{

/**
	对象名称	: CExp
	对象描述	: 通用异常类, 封装了系统出现的异常, 每个系统或模块可以派生自己的异常类
	重要函数	: 
				CExp(long lCode = 0, char* sMsg = "", char* file = NULL, long nLine = -1, long lState = CExp::stat_null);
				virtual	~CExp();
				virtual const char* what();
				char*	GetExp(long& lExpCode, long &lState);
				long	GetExpCode();
				char*	GetExpMsg();
				long	GetState();
				char*	GetPos();
				long	GetLine();
				void	SetLow(CExp* e);
				static void throwup(CExp* e, long lCode = 0, char* sMsg = "", char* file = NULL, long nLine = -1, long lState = CExp::stat_null);
				char* Show();

	编写人		: 居卫华
	完成日期	: 2003 - 4 - 23
**/
class CExp : public std::exception
{
private:
	char m_sMsg[MSGSIZE];		// message
	long m_lExpCode;			// exception code
	long m_lState;				// status code
	char m_sFile[_MAX_PATH];	// file in which exception is raised
	long m_nLine;				// line number of file where exception is raised
	CExp* pLow;					// error born in the low tie.
	long m_lLevel;				// error level

public:
	// constructor and desctor
	CExp(long lCode = 0, char* sMsg = "", char* file = NULL, long nLine = -1, long lState = CExp::stat_null, long lLevel = 0);
	virtual	~CExp() throw();

	virtual const char* what(){
		return m_sMsg;
	}

	// get exception cotent
	char* GetExp(long& lExpCode, long &lState){
		lExpCode = m_lExpCode; lState = m_lState; return m_sMsg;
	}

	// get exception code
	long GetExpCode(){
		return m_lExpCode;		
	};

	// get exception msg
	char* GetExpMsg(){return m_sMsg;};

	// get status code
	long  GetState(){
		return m_lState;
	};
	
	// Get file
	char* GetPos()
	{
		return m_sFile;
	};

	// get line
	long GetLine()
	{
		return m_nLine;
	};

	// set low level exception
	void SetLow(CExp* e)
	{
		pLow = e;
	}

	// method to throw up exception with the lower exception
	static void throwup(CExp* e, long lCode = 0, char* sMsg = "", char* file = NULL, long nLine = -1, long lState = CExp::stat_null)
	{
		CExp* exp = new CExp(lCode, sMsg, file, nLine, lState);
		if (exp && e)
		{
			exp->SetLow(e);
			throw exp;
		}
	}
	
	// output all mag to string
	char* Show();

public:
	
	// return code
static		const long ret_ok;
static		const long err_unknown;
static		const long err_invalid_param;
static		const long err_allocate_memory;
static		const long err_unexpected_status;
static		const long err_createdbconnectpool_failed;
static		const long err_getdbstmt_failed;
static		const long err_dbop_failed;
static		const long err_createevent_failed;

//	const static	long err_user_defined;

	// status
static		const long stat_null;

static		const long stat_uninitialized;
static		const long stat_init_ok;
static		const long stat_starting_no_err;
static		const long stat_starting_with_err;
static		const long stat_stoping;
static		const long stat_stopped_ok;
static		const long stat_stopped_with_err;
static		const long stat_user_defined;
};

}

#endif //__VMEXCEPTION_H__
