/*
 * 将此文件与 UnQLite 数据库引擎源代码一起编译以生成可执行文件。例如：
 *  gcc -W -Wall -O6 unqlite_const_intro.c unqlite.c -o unqlite_jx9_const
*/
/*
 * 这个简单的程序是对如何嵌入和开始使用 UnQLite 进行实验的快速介绍，
 * 而无需进行大量繁琐的阅读和配置。
 *
 * UnQLite（通过 Jx9）常量扩展机制简介：
 *
 * UnQLite 的文档存储用于在数据库中存储 JSON 文档（即对象、数组、字符串等），
 * 由 Jx9 编程语言提供支持。
 *
 * Jx9 是一种嵌入式脚本语言，也称为扩展语言，旨在支持具有数据描述功能的
 * 通用过程编程。Jx9 是一种基于 JSON 的图灵完备、动态类型的编程语言，
 * 在 UnQLite 核心中作为库实现。
 *
 * Jx9 拥有大量功能，具有类似于 C 和 Javascript 的简洁熟悉的语法。
 * 作为扩展语言，Jx9 没有主程序的概念，它只能作为主机应用程序的一部分运行。
 * 主机程序（我们的例子中是 UnQLite）可以写入和读取 Jx9 变量，
 * 还可以注册要由 Jx9 代码调用的 C/C++ 函数。
 *
 * Jx9 下的常量扩展机制非常强大但简单，工作原理如下：
 * 每个注册的常量都有一个关联的 C 过程。这个过程称为常量扩展回调，
 * 负责将调用的常量扩展为所需的值，例如：
 * 与 "__PI__" 常量关联的 C 过程扩展为 3.14（PI 的值）。
 * "__OS__" 常量过程扩展为主机操作系统的名称（Windows、Linux、...），
 * "__TIME__" 常量扩展为当前系统时间，等等。
 * 
 * 有关常量扩展机制的入门课程，您可以参阅以下教程：
 *        http://unqlite.org/const_intro.html
 * 有关 UnQLite C/C++ 接口的介绍，请参阅：
 *        http://unqlite.org/api_intro.html
 * 有关 Jx9 的介绍，请参阅：
 *        http://unqlite.org/jx9.html
 * 有关完整的 C/C++ API 参考指南，请参阅：
 *        http://unqlite.org/c_api.html
 * 5 分钟或更短时间内了解 UnQLite：
 *        http://unqlite.org/intro.html
 * UnQLite 数据库引擎的架构：
 *        http://unqlite.org/arch.html
 */
/* $SymiscID: unqlite_const_intro.c v1.5 Unix 2013-05-17 00:17 stable <chm@symisc.net> $ */
/* 
 * 确保您有 UnQLite 的最新版本：
 *  http://unqlite.org/downloads.html
 */
#include <stdio.h>  /* puts() */
#include <stdlib.h> /* exit() */
/* 确保此头文件可用。*/
#include "unqlite.h"
/*
 * Banner.
 */
static const char zBanner[] = {
	"============================================================\n"
	"UnQLite (Via Jx9) Constant Expansion Mechanism              \n"
	"                                         http://unqlite.org/\n"
	"============================================================\n"
};
/*
 * Extract the database error log and exit.
 */
static void Fatal(unqlite *pDb,const char *zMsg)
{
	if( pDb ){
		const char *zErr;
		int iLen = 0; /* Stupid cc warning */

		/* Extract the database error log */
		unqlite_config(pDb,UNQLITE_CONFIG_ERR_LOG,&zErr,&iLen);
		if( iLen > 0 ){
			/* Output the DB error log */
			puts(zErr); /* Always null terminated */
		}
	}else{
		if( zMsg ){
			puts(zMsg);
		}
	}
	/* Manually shutdown the library */
	unqlite_lib_shutdown();
	/* Exit immediately */
	exit(0);
}
/*
 * __PI__: expand the value of PI (3.14...)
 * Our first constant is the __PI__ constant. The following procedure
 * is the callback associated with the __PI__ identifier. That is, when
 * the __PI__ identifier is seen in the running script, this procedure
 * gets called by the underlying Jx9 virtual machine.
 * This procedure is responsible of expanding the constant identifier to
 * the desired value (3.14 in our case).
 */
static void PI_Constant(
	unqlite_value *pValue, /* Store expanded value here */
	void *pUserData    /* User private data (unused in our case) */
	){
		/* Expand the value of PI */
		unqlite_value_double(pValue, 3.1415926535898);
}
/*
 * __TIME__: expand the current local time.
 * Our second constant is the __TIME__ constant.
 * When the __TIME__ identifier is seen in the running script, this procedure
 * gets called by the underlying Jx9 virtual machine.
 * This procedure is responsible of expanding the constant identifier to
 * the desired value (local time in our case).
 */
#include <time.h>
static void TIME_Constant(unqlite_value *pValue, void *pUserData /* Unused */)
{
	struct tm *pLocal;
#if defined(_MSC_VER) || defined(__MINGW32__)
    /* errno_t err; */
    __time64_t long_time;
    struct tm newtime;
    _time64( &long_time );
    /* err = */ _localtime64_s(&newtime, &long_time);
    /* if (err != 0) return UNQLITE_INVALID; */
    pLocal = &newtime;
#else
	time_t tt;
	/* Get the current local time */
	time(&tt);
	pLocal = localtime(&tt);
#endif
	/* Expand the current time now */
	unqlite_value_string_format(pValue, "%02d:%02d:%02d", 
		pLocal->tm_hour, 
		pLocal->tm_min, 
		pLocal->tm_sec
		);
}
/*
 * __OS__: expand the name of the Host Operating System.
 * Our last constant is the __OS__ constant.
 * When the __OS__ identifier is seen in the running script, this procedure
 * gets called by the underlying Jx9 virtual machine.
 * This procedure is responsible of expanding the constant identifier to
 * the desired value (OS name in our case).
 */
static void OS_Constant(unqlite_value *pValue, void *pUserData /* Unused */ )
{
#ifdef __WINNT__
	unqlite_value_string(pValue, "Windows", -1 /*Compute input length automatically */);
#else
	/* Assume UNIX */
	unqlite_value_string(pValue, "UNIX", -1 /*Compute input length automatically */);
#endif /* __WINNT__ */
}
/* Forward declaration: VM output consumer callback */
static int VmOutputConsumer(const void *pOutput,unsigned int nOutLen,void *pUserData /* Unused */);
/*
 * The following is the Jx9 Program to be executed later by the UnQLite VM:
 * 
 * //Test the constant expansion mechanism
 *    print '__PI__   value: ' .. __PI__ .. JX9_EOL;
 *    print '__TIME__ value: ' .. __TIME__  .. JX9_EOL;
 *    print '__OS__   value: ' .. __OS__ .. JX9_EOL;
 * When running, you should see something like that:
 *	__PI__   value: 3.1415926535898
 *  __TIME__ value: 15:02:27
 *  __OS__   value: UNIX
 * 
 */
#define JX9_PROG \
 "print '__PI__   value: ' .. __PI__ ..   JX9_EOL;"\
 "print '__TIME__ value: ' .. __TIME__ .. JX9_EOL;"\
 "print '__OS__   value: ' .. __OS__ ..   JX9_EOL;"

 /* No need for command line arguments, everything is stored in-memory */
int main(void)
{
	unqlite *pDb;       /* Database handle */
	unqlite_vm *pVm;    /* UnQLite VM resulting from successful compilation of the target Jx9 script */
	int rc;

	puts(zBanner);

	/* Open our database */
	rc = unqlite_open(&pDb,":mem:" /* In-mem DB */,UNQLITE_OPEN_CREATE);
	if( rc != UNQLITE_OK ){
		Fatal(0,"Out of memory");
	}
	
	/* Compile our Jx9 script defined above */
	rc = unqlite_compile(pDb,JX9_PROG,sizeof(JX9_PROG)-1,&pVm);
	if( rc != UNQLITE_OK ){
		/* Compile error, extract the compiler error log */
		const char *zBuf;
		int iLen;
		/* Extract error log */
		unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
		if( iLen > 0 ){
			puts(zBuf);
		}
		Fatal(0,"Jx9 compile error");
	}

	/* Now we have our program compiled, it's time to register our constants
	 * and their associated C procedure.
	 */
	rc = unqlite_create_constant(pVm, "__PI__", PI_Constant, 0);
	if( rc != UNQLITE_OK ){
		Fatal(0,"Error while installing the __PI__ constant");
	}
	
	rc = unqlite_create_constant(pVm, "__TIME__", TIME_Constant, 0);
	if( rc != UNQLITE_OK ){
		Fatal(0,"Error while installing the __TIME__ constant");
	}
	
	rc = unqlite_create_constant(pVm, "__OS__", OS_Constant, 0);
	if( rc != UNQLITE_OK ){
		Fatal(0,"Error while installing the __OS__ constant");
	}

	/* Install a VM output consumer callback */
	rc = unqlite_vm_config(pVm,UNQLITE_VM_CONFIG_OUTPUT,VmOutputConsumer,0);
	if( rc != UNQLITE_OK ){
		Fatal(pDb,0);
	}
	
	/* Execute our script */
	rc = unqlite_vm_exec(pVm);
	if( rc != UNQLITE_OK ){
		Fatal(pDb,0);
	}

	/* Release our VM */
	unqlite_vm_release(pVm);
	
	/* Auto-commit the transaction and close our database */
	unqlite_close(pDb);
	return 0;
}

#ifdef __WINNT__
#include <Windows.h>
#else
/* Assume UNIX */
#include <unistd.h>
#endif
/*
 * The following define is used by the UNIX build process and have
 * no particular meaning on windows.
 */
#ifndef STDOUT_FILENO
#define STDOUT_FILENO	1
#endif
/*
 * VM output consumer callback.
 * Each time the UnQLite VM generates some outputs, the following
 * function gets called by the underlying virtual machine to consume
 * the generated output.
 *
 * All this function does is redirecting the VM output to STDOUT.
 * This function is registered via a call to [unqlite_vm_config()]
 * with a configuration verb set to: UNQLITE_VM_CONFIG_OUTPUT.
 */
static int VmOutputConsumer(const void *pOutput,unsigned int nOutLen,void *pUserData /* Unused */)
{
#ifdef __WINNT__
	BOOL rc;
	rc = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),pOutput,(DWORD)nOutLen,0,0);
	if( !rc ){
		/* Abort processing */
		return UNQLITE_ABORT;
	}
#else
	ssize_t nWr;
	nWr = write(STDOUT_FILENO,pOutput,nOutLen);
	if( nWr < 0 ){
		/* Abort processing */
		return UNQLITE_ABORT;
	}
#endif /* __WINT__ */
	
	/* All done, data was redirected to STDOUT */
	return UNQLITE_OK;
}
