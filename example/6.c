/*
 * 将本文件与 UnQLite 数据库引擎源代码一起编译以生成可执行文件。
 * 例如：
 *  gcc -W -Wall -O6 unqlite_hostapp_info.c unqlite.c -o unqlite_host
*/
/*
 * 这个简单的程序是对如何嵌入和使用 UnQLite 的快速介绍，
 * 无需进行大量繁琐的阅读和配置。
 *
 * 通过 UnQLite 的文档存储接口在宿主应用程序和底层 Jx9 脚本之间共享数据：
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
 *        http://unqlite.org/unqlite.html
 * 有关完整的 C/C++ API 参考指南，请参阅：
 *        http://unqlite.org/c_api.html
 * UnQLite 5 分钟快速入门：
 *        http://unqlite.org/intro.html
 * UnQLite 数据库引擎架构：
 *        http://unqlite.org/arch.html
 */
/* $SymiscID: unqlite_hostapp_info.c v1.0 Win7 2013-05-17 22:37 stable <chm@symisc.net> $ */
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
	"UnQLite Document-Store (Via Jx9) Data Share Intro           \n"
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
/*
 * 以下遍历回调可供 [unqlite_array_walk()] 接口使用，
 * 该接口用于迭代从运行脚本提取的 JSON 对象。
 * （有关更多信息，请参阅下文）。
 */
static int JsonObjectWalker(unqlite_value *pKey,unqlite_value *pData,void *pUserData /* Unused */)
{
	const char *zKey,*zData;
	/* Extract the key and the data field */
	zKey = unqlite_value_to_string(pKey,0);
	zData = unqlite_value_to_string(pData,0);
	/* Dump */
	printf(
		"%s ===> %s\n",
		zKey,
		zData
	);
	return UNQLITE_OK;
}
/* Forward declaration: VM output consumer callback */
static int VmOutputConsumer(const void *pOutput,unsigned int nOutLen,void *pUserData /* Unused */);
/*
 * The following is the Jx9 Program to be executed later by the UnQLite VM:
 *
 * This program demonstrate how data is shared between the host application
 * and the running JX9 script. The main() function defined below creates and install
 * two foreign variables named respectively $my_app and $my_data. The first is a simple
 * scalar value while the last is a complex JSON object. these foreign variables are
 * made available to the running script using the [unqlite_vm_config()] interface with
 * a configuration verb set to UNQLITE_VM_CONFIG_CREATE_VAR.
 * 
 * Jx9 Program:
 *
 * print "Showing foreign variables contents\n";
 * //Scalar foreign variable named $my_app
 * print "\$my_app =",$my_app..JX9_EOL;
 * //Foreign JSON object named $my_data
 * print "\$my_data = ",$my_data;
 * //Dump command line arguments
 * if( count($argv) > 0 ){
 *  print "\nCommand line arguments:\n";
 *  print $argv;
 * }else{
 *  print "\nEmpty command line";
 * }
 * //Return a simple JSON object to the host application
 * $my_config = {
 *        "unqlite_signature" : db_sig(), //UnQLite Unique signature
 *        "time" : __TIME__, //Current time
 *        "date" : __DATE__  //Current date
 * };
 */
#define JX9_PROG \
 "print \"Showing foreign variables contents\n\n\";"\
 " /*Scalar foreign variable named $my_app*/"\
 " print \"\\$my_app = \",$my_app..JX9_EOL;"\
 " /*JSON object foreign variable named $my_data*/"\
 " print \"\n\\$my_data = \",$my_data..JX9_EOL;"\
 " /*Dump command line arguments*/"\
 " if( count($argv) > 0 ){"\
 "  print \"\nCommand line arguments:\n\";"\
 "  print $argv..JX9_EOL;"\
 " }else{"\
 "  print \"\nEmpty command line\";"\
 " }"\
 " /*Return a simple JSON object to the host application*/"\
 " $my_config = {"\
 "        'unqlite_signature' : db_sig(),  /* UnQLite Unique version*/"\
 "        'time' : __TIME__, /*Current time*/"\
 "        'date' : __DATE__  /*Current date*/"\
 " };"
 
int main(int argc,char *argv[])
{
	unqlite_value *pScalar,*pObject; /* 稍后安装的外部 Jx9 变量 */
	unqlite *pDb;       /* 数据库句柄 */
	unqlite_vm *pVm;    /* 成功编译目标 Jx9 脚本产生的 UnQLite VM */
	int n,rc;

	puts(zBanner);

	/* 打开我们的数据库 */
	rc = unqlite_open(&pDb,":mem:" /* 内存数据库 */,UNQLITE_OPEN_CREATE);
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

	/* 注册脚本参数，以便我们稍后可以从已编译的 Jx9 程序
	 * 使用 $argv[] 数组访问它们。
	 */
	for( n = 1; n < argc ; ++n ){
		unqlite_vm_config(pVm, UNQLITE_VM_CONFIG_ARGV_ENTRY, argv[n]/* 参数值 */);
	}

	/* 安装 VM 输出消费者回调 */
	rc = unqlite_vm_config(pVm,UNQLITE_VM_CONFIG_OUTPUT,VmOutputConsumer,0);
	if( rc != UNQLITE_OK ){
		Fatal(pDb,0);
	}

	/*
	 * 创建一个简单的标量变量。
	 */
	pScalar = unqlite_vm_new_scalar(pVm);
	if( pScalar == 0 ){
		Fatal(0,"无法创建外部变量 $my_app");
	}
	/* 用所需信息填充变量 */
	unqlite_value_string(pScalar,"My Host Application/1.2.5",-1/*自动计算长度*/); /* 虚拟签名*/
	/*
	 * 安装变量 ($my_app)。
	 */
	rc = unqlite_vm_config(
		  pVm,
		  UNQLITE_VM_CONFIG_CREATE_VAR, /* 创建变量命令 */
		  "my_app", /* 变量名（不带美元符号）*/
		  pScalar   /* 值 */
		);
	if( rc != UNQLITE_OK ){
		Fatal(0,"安装 $my_app 时出错");
	}
	/* 要从运行脚本访问此外部变量，只需按如下方式调用它：
	 *  print $my_app;
	 * 或
	 *  dump($my_app);
	 */

	/*
	 * 现在是时候创建并安装一个更复杂的变量了，
	 * 这是一个名为 $my_data 的 JSON 对象。
	 * JSON 对象看起来像这样：
	 *  {
	 *     "path" : "/usr/local/etc",
	 *     "port" : 8082,
	 *     "fork" : true
	 *  };
	 */
	pObject = unqlite_vm_new_array(pVm); /* JSON 对象和数组的统一接口 */
	/* 用上面定义的字段填充对象。
	*/
	unqlite_value_reset_string_cursor(pScalar);

	/* 添加 "path" : "/usr/local/etc" 条目 */
	unqlite_value_string(pScalar,"/usr/local/etc",-1);
	unqlite_array_add_strkey_elem(pObject,"path",pScalar); /* 会创建自己的 pScalar 副本 */

	/* 添加 "port" : 8080 条目 */
	unqlite_value_int(pScalar,8080);
	unqlite_array_add_strkey_elem(pObject,"port",pScalar); /* 会创建自己的 pScalar 副本 */

	/* 添加 "fork": true 条目 */
	unqlite_value_bool(pScalar,1 /* TRUE */);
	unqlite_array_add_strkey_elem(pObject,"fork",pScalar); /* 会创建自己的 pScalar 副本 */

	/* 现在安装变量并将 JSON 对象与之关联 */
	rc = unqlite_vm_config(
		  pVm,
		  UNQLITE_VM_CONFIG_CREATE_VAR, /* 创建变量命令 */
		  "my_data", /* 变量名（不带美元符号）*/
		  pObject    /*值 */
		);
	if( rc != UNQLITE_OK ){
		Fatal(0,"安装 $my_data 时出错");
	}

	/* 释放这两个值 */
	unqlite_vm_release_value(pVm,pScalar);
	unqlite_vm_release_value(pVm,pObject);

	/* 执行我们的脚本 */
	unqlite_vm_exec(pVm);

	/* 提取运行脚本中定义的名为 $my_config 的变量的内容，
	 * 该变量保存一个简单的 JSON 对象。
	 */
	pObject = unqlite_vm_extract_variable(pVm,"my_config");
	if( pObject && unqlite_value_is_json_object(pObject) ){
		/* 迭代对象字段 */
		printf("\n\nTotal fields in $my_config = %u\n",unqlite_array_count(pObject));
		unqlite_array_walk(pObject,JsonObjectWalker,0);
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
