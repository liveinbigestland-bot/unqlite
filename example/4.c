/*
 * 将本文件与 UnQLite 数据库引擎源代码一起编译以生成可执行文件。
 * 例如：
 *  gcc -W -Wall -O6 unqlite_doc_intro.c unqlite.c -o unqlite_doc
*/
/*
 * 这个简单的程序是对如何嵌入和使用 UnQLite 的快速介绍，
 * 无需进行大量繁琐的阅读和配置。
 *
 * UnQLite 文档存储接口介绍：
 *
 * UnQLite 的文档存储用于在数据库中存储 JSON 文档（即对象、数组、字符串等），
 * 由 Jx9 编程语言提供支持。
 *
 * Jx9 是一个嵌入式脚本语言，也称为扩展语言，旨在支持
 * 具有数据描述功能的一般过程式编程。
 * Jx9 是一个基于 JSON 的图灵完备、动态类型编程语言，
 * 作为库在 UnQLite 核心中实现。
 *
 * Jx9 构建了大量的功能，具有类似于 C 和 Javascript 的简洁熟悉的语法。
 * 作为扩展语言，Jx9 没有主程序的概念，只能在宿主应用程序中运行。
 * 宿主程序（在本例中为 UnQLite）可以写入和读取 Jx9 变量，
 * 还可以注册由 Jx9 代码调用的 C/C++ 函数。
 *
 * 有关 UnQLite C/C++ 接口的介绍，请参阅：
 *        http://unqlite.org/api_intro.html
 * 有关 Jx9 的介绍，请参阅：
 *        http://unqlite.org/jx9.html
 * 有关完整的 C/C++ API 参考指南，请参阅：
 *        http://unqlite.org/c_api.html
 * UnQLite 5 分钟快速入门：
 *        http://unqlite.org/intro.html
 * UnQLite 数据库引擎架构：
 *        http://unqlite.org/arch.html
 */
/* $SymiscID: unqlite_doc_intro.c v1.0 FreeBSD 2013-05-17 15:56 stable <chm@symisc.net> $ */
/*
 * 请确保使用最新版本的 UnQLite：
 *  http://unqlite.org/downloads.html
 */
#include <stdio.h>  /* puts() */
#include <stdlib.h> /* exit() */
/* 确保此头文件可用。*/
#include "unqlite.h"
/*
 * 横幅。
 */
static const char zBanner[] = {
	"============================================================\n"
	"UnQLite Document-Store (Via Jx9) Intro                      \n"
	"                                         http://unqlite.org/\n"
	"============================================================\n"
};
/*
 * 提取数据库错误日志并退出。
 */
static void Fatal(unqlite *pDb,const char *zMsg)
{
	if( pDb ){
		const char *zErr;
		int iLen = 0; /* 愚蠢的 cc 警告 */

		/* 提取数据库错误日志 */
		unqlite_config(pDb,UNQLITE_CONFIG_ERR_LOG,&zErr,&iLen);
		if( iLen > 0 ){
			/* 输出数据库错误日志 */
			puts(zErr); /* 始终以 null 终止 */
		}
	}else{
		if( zMsg ){
			puts(zMsg);
		}
	}
	/* 手动关闭库 */
	unqlite_lib_shutdown();
	/* 立即退出 */
	exit(0);
}
/* 前向声明：VM 输出消费者回调 */
static int VmOutputConsumer(const void *pOutput,unsigned int nOutLen,void *pUserData /* 未使用 */);
/*
 * 以下是由 UnQLite VM 稍后执行的 Jx9 程序：
 * 此程序将一些 JSON 对象（虚拟用户集合）存储到
 * 存储在数据库中的 'users' 集合中。
 * // 创建 'users' 集合
 * if( !db_exists('users') ){
 *   // 尝试创建它
 *  $rc = db_create('users');
 *  if ( !$rc ){
 *    // 处理错误
 *     print db_errlog();
 *	  return;
 *  }
 * }
 * // 以下是即将存储在我们集合中的记录
 * $zRec = [
 * {
 *  name : 'james',
 *  age  : 27,
 *  mail : 'dude@example.com'
 * },
 * {
 *  name : 'robert',
 *  age  : 35,
 *  mail : 'rob@example.com'
 * },
 *
 * {
 *  name : 'monji',
 *  age  : 47,
 *  mail : 'monji@example.com'
 * },
 * {
 * name : 'barzini',
 * age  : 52,
 * mail : 'barz@mobster.com'
 * }
 * ];
 *
 * // 存储我们的记录
 * $rc = db_store('users',$zRec);
 * if( !$rc ){
 *  // 处理错误
 *  print db_errlog();
 *  return;
 * }
 * // 创建我们的过滤回调
 * $zCallback = function($rec){
 *   // 只允许年龄 >= 30 岁的用户。
 *   if( $rec.age < 30 ){
 *       // 丢弃此记录
 *       return FALSE;
 *   }
 *   // 记录符合我们的条件
 *   return TRUE;
 * }; // 别忘记这里的分号
 *
 * // 检索集合记录并应用我们的过滤回调
 * $data = db_fetch_all('users',$zCallback);
 *
 * // 遍历提取的元素
 * foreach($data as $value){ // 包含过滤记录的 JSON 数组
 * print $value..JX9_EOL;
 * }
 */
#define JX9_PROG \
"/* Create the collection 'users'  */"\
 "if( !db_exists('users') ){"\
 "   /* Try to create it */"\
 "  $rc = db_create('users');"\
 "  if ( !$rc ){"\
 "    /*Handle error*/"\
 "    print db_errlog();"\
 "	  return;"\
 "  }else{"\
 "     print \"Collection 'users' successfuly created\\n\";"\
 "   }"\
 " }"\
 "/*The following is the records to be stored shortly in our collection*/ "\
 "$zRec = ["\
 "{"\
 "  name : 'james',"\
 "  age  : 27,"\
 "  mail : 'dude@example.com'"\
 "},"\
 "{"\
 "  name : 'robert',"\
 "  age  : 35,"\
 "  mail : 'rob@example.com'"\
 "},"\
 "{"\
 "  name : 'monji',"\
 "  age  : 47,"\
 "  mail : 'monji@example.com'"\
 "},"\
 "{"\
 " name : 'barzini',"\
 " age  : 52,"\
 " mail : 'barz@mobster.com'"\
 "}"\
 "];"\
 "/*Store our records*/"\
 "$rc = db_store('users',$zRec);"\
 "if( !$rc ){"\
 " /*Handle error*/"\
 " print db_errlog();"\
 " return;"\
 "}"\
 "/*Create our filter callback*/"\
 "$zCallback = function($rec){"\
 "   /*Allow only users >= 30 years old.*/"\
 "   if( $rec.age < 30 ){"\
 "       /* Discard this record*/"\
 "       return FALSE;"\
 "   }"\
 "   /* Record correspond to our criteria*/"\
 "   return TRUE;"\
 "}; /* Don't forget the semi-colon here*/"\
 "/* Retrieve collection records and apply our filter callback*/"\
 "$data = db_fetch_all('users',$zCallback);"\
 "print \"Filtered records\\n\";"\
 "/*Iterate over the extracted elements*/"\
 "foreach($data as $value){ /*JSON array holding the filtered records*/"\
 " print $value..JX9_EOL;"\
 "}"

int main(int argc,char *argv[])
{
	unqlite *pDb;       /* 数据库句柄 */
	unqlite_vm *pVm;    /* 成功编译目标 Jx9 脚本产生的 UnQLite VM */
	int rc;

	puts(zBanner);

	/* 打开我们的数据库 */
	rc = unqlite_open(&pDb,argc > 1 ? argv[1] /* 磁盘数据库 */ : ":mem:" /* 内存数据库 */,UNQLITE_OPEN_CREATE);
	if( rc != UNQLITE_OK ){
		Fatal(0,"内存不足");
	}

	/* 编译上面定义的 Jx9 脚本 */
	rc = unqlite_compile(pDb,JX9_PROG,sizeof(JX9_PROG)-1,&pVm);
	if( rc != UNQLITE_OK ){
		/* 编译错误，提取编译器错误日志 */
		const char *zBuf;
		int iLen;
		/* 提取错误日志 */
		unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
		if( iLen > 0 ){
			puts(zBuf);
		}
		Fatal(0,"Jx9 编译错误");
	}

	/* 安装 VM 输出消费者回调 */
	rc = unqlite_vm_config(pVm,UNQLITE_VM_CONFIG_OUTPUT,VmOutputConsumer,0);
	if( rc != UNQLITE_OK ){
		Fatal(pDb,0);
	}

	/* 执行我们的脚本 */
	rc = unqlite_vm_exec(pVm);
	if( rc != UNQLITE_OK ){
		Fatal(pDb,0);
	}

	/* 释放我们的 VM */
	unqlite_vm_release(pVm);

	/* 自动提交事务并关闭我们的数据库 */
	unqlite_close(pDb);
	return 0;
}

#ifdef __WINNT__
#include <Windows.h>
#else
/* 假定 UNIX */
#include <unistd.h>
#endif
/*
 * 以下定义由 UNIX 构建过程使用，
 * 在 Windows 上没有特殊含义。
 */
#ifndef STDOUT_FILENO
#define STDOUT_FILENO	1
#endif
/*
 * VM 输出消费者回调。
 * 每次 UnQLite VM 生成一些输出时，
 * 下面的函数会被底层虚拟机调用以消费生成的输出。
 *
 * 此函数所做的只是将 VM 输出重定向到 STDOUT。
 * 此函数通过调用 [unqlite_vm_config()]
 * 注册，配置动词设置为：UNQLITE_VM_CONFIG_OUTPUT。
 */
static int VmOutputConsumer(const void *pOutput,unsigned int nOutLen,void *pUserData /* 未使用 */)
{
#ifdef __WINNT__
	BOOL rc;
	rc = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),pOutput,(DWORD)nOutLen,0,0);
	if( !rc ){
		/* 中止处理 */
		return UNQLITE_ABORT;
	}
#else
	ssize_t nWr;
	nWr = write(STDOUT_FILENO,pOutput,nOutLen);
	if( nWr < 0 ){
		/* 中止处理 */
		return UNQLITE_ABORT;
	}
#endif /* __WINT__ */

	/* 完成，数据已重定向到 STDOUT */
	return UNQLITE_OK;
}
