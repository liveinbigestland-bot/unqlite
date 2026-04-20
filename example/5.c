/*
 * 将本文件与 UnQLite 数据库引擎源代码一起编译以生成可执行文件。
 * 例如：
 *  gcc -W -Wall -O6 unqlite_func_intro.c unqlite.c -o unqlite_unqlite_func
*/
/*
 * 这个简单的程序是对如何嵌入和使用 UnQLite 的快速介绍，
 * 无需进行大量繁琐的阅读和配置。
 *
 * UnQLite（通过 Jx9）外部函数机制介绍：
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
 * 外部函数用于从外部环境添加 Jx9 函数或重新定义现有
 * Jx9 函数的行为（见下文）。
 * 这种机制被称为"进程内扩展"。成功调用 [unqlite_create_function()] 后，
 * 安装的函数立即可用，可以从目标 Jx9 代码中调用。
 *
 * 有关 [unqlite_create_function()] 接口和外部函数机制的介绍性教程，
 * 请参阅：
 *        http://unqlite.org/func_intro.html
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
/* $SymiscID: unqlite_func_intro.c v1.7 Linux 2013-05-17 03:17 stable <chm@symisc.net> $ */
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
	"UnQLite (Via Jx9) Foreign Functions                         \n"
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
 * int shift_func(int $num)
 *  将数字右移一位并返回结果。
 * 说明
 *  我们的第一个函数对给定数字执行简单的右移操作
 *  并返回移位后的数字。
 *  此函数期望一个参数，必须是数字类型（整数、浮点数
 *  或看起来像数字的字符串）。
 * 参数
 *  $num
 *   要移位的数字。
 * 返回值
 *   整数：给定数字左移一位（即乘以2）的结果。
 * 用法示例：
 *   print shift_func(150); //输出 300
 *   print shift_func(50);  //输出 100
 */
int shift_func(
	unqlite_context *pCtx, /* 调用上下文 */
	int argc,          /* 传递给函数的参数总数 */
	unqlite_value **argv   /* 函数参数数组 */
	)
{
	int num;
	/* 确保至少有一个参数且类型符合预期 [即：数字]。 */
	if( argc < 1 || !unqlite_value_is_numeric(argv[0]) ){
		/*
		 * 缺少/无效参数，抛出警告并返回 FALSE。
		 * 注意：您不需要记录函数名，UnQLite 会
		 * 自动为您附加函数名。
		 */
		unqlite_context_throw_error(pCtx, UNQLITE_CTX_WARNING, "缺少数字参数");
		/* 返回 false */
		unqlite_result_bool(pCtx, 0);
		return UNQLITE_OK;
	}
	/* 提取数字 */
	num = unqlite_value_to_int(argv[0]);
	/* 左移 1 位（乘以2）*/
	num <<= 1;
	/* 返回新值 */
	unqlite_result_int(pCtx, num);
	/* 完成 */
	return UNQLITE_OK;
}
#include <time.h>
/*
 * string date_func(void)
 *  返回当前系统日期。
 * 说明
 *  我们的第二个函数不接受参数并返回
 *  当前系统日期。
 * 参数
 *  无
 * 返回值
 *   字符串：当前系统日期。
 * 用法示例
 *   print date_func(); //可能输出：2012-23-09 14:53:30
 */
int date_func(
	unqlite_context *pCtx, /* Call Context */
	int argc,          /* Total number of arguments passed to the function */
	unqlite_value **argv   /* Array of function arguments*/
	){
        struct tm *pNow;
#if defined(_MSC_VER) || defined(__MINGW32__)
        errno_t err;
        __time64_t long_time;
        struct tm newtime;
        _time64( &long_time );
        err = _localtime64_s(&newtime, &long_time);
        if (err != 0) return UNQLITE_INVALID;
        pNow = &newtime;
#else
        time_t tt;
        /* Get the current time */
        time(&tt);
		pNow = localtime(&tt);
#endif
        /*
         * Return the current date.
         */
        unqlite_result_string_format(pCtx,
                                     "%04d-%02d-%02d %02d:%02d:%02d", /* printf() style format */
                                     pNow->tm_year + 1900, /* Year */
                                     pNow->tm_mday,        /* Day of the month */
                                     pNow->tm_mon + 1,     /* Month number */
                                     pNow->tm_hour, /* Hour */
                                     pNow->tm_min,  /* Minutes */
                                     pNow->tm_sec   /* Seconds */
        );

		/* All done */
		return UNQLITE_OK;
}
/*
 * int64 sum_func(int $arg1, int $arg2, int $arg3, ...)
 *  返回给定参数的总和。
 * 说明
 *  此函数期望可变数量的参数，参数必须是数字类型
 *  （整数、浮点数或看起来像数字的字符串），
 *  并返回给定数字的总和。
 * 参数
 *   int $n1, n2, ...（可变数量的参数）
 * 返回值
 *   Integer64：给定数字的总和。
 * 用法示例
 *   print sum_func(7, 8, 9, 10); //可能输出 34
 */
int sum_func(unqlite_context *pCtx, int argc, unqlite_value **argv)
{
	unqlite_int64 iTotal = 0; /* Counter */
	int i;
	if( argc < 1 ){
		/*
		 * Missing arguments, throw a notice and return NULL.
		 * Note that you do not need to log the function name, UnQLite will
		 * automatically append the function name for you.
		 */
		unqlite_context_throw_error(pCtx, UNQLITE_CTX_NOTICE, "Missing function arguments $arg1, $arg2, ..");
		/* Return null */
		unqlite_result_null(pCtx);
		return UNQLITE_OK;
	}
	/* Sum the arguments */
	for( i = 0; i < argc ; i++ ){
		unqlite_value *pVal = argv[i];
		unqlite_int64 n;
		/* Make sure we are dealing with a numeric argument */
		if( !unqlite_value_is_numeric(pVal) ){
			/* Throw a notice and continue */
			unqlite_context_throw_error_format(pCtx, UNQLITE_CTX_NOTICE, 
				"Arg[%d]: Expecting a numeric value", /* printf() style format */
				i
				);
			/* Ignore */
			continue;
		}
		/* Get a 64-bit integer representation and increment the counter */
		n = unqlite_value_to_int64(pVal);
		iTotal += n;
	}
	/* Return the count  */
	unqlite_result_int64(pCtx, iTotal);
	/* All done */
	return UNQLITE_OK;
}
/*
 * array array_time_func(void)
 *  以 JSON 数组形式返回当前系统时间。
 * 说明
 *  此函数不接受参数并以数组形式返回
 *  当前系统时间。
 * 参数
 *  无
 * 返回值
 *   包含当前系统时间的数组。
 * 用法示例
 *
 *   print array_time_func() ;
 *
 * 运行时应看到类似以下内容：
 * JSON array(3) [14,53,30]
 */
int array_time_func(unqlite_context *pCtx, int argc, unqlite_value **argv)
{
	unqlite_value *pArray;    /* Our JSON Array */
	unqlite_value *pValue;    /* Array entries value */
	struct tm *pNow;
	/* Get the current time first */
#if defined(_MSC_VER) || defined(__MINGW32__)
    errno_t err;
    __time64_t long_time;
    struct tm newtime;
    _time64( &long_time );
    err = _localtime64_s(&newtime, &long_time);
    if (err != 0) return UNQLITE_INVALID;
    pNow = &newtime;
#else
    time_t tt;
    time(&tt);
	pNow = localtime(&tt);
#endif
	/* Create a new array */
	pArray = unqlite_context_new_array(pCtx);
	/* Create a worker scalar value */
	pValue = unqlite_context_new_scalar(pCtx);
	if( pArray == 0 || pValue == 0 ){
		/*
		 * If the supplied memory subsystem is so sick that we are unable
		 * to allocate a tiny chunk of memory, there is no much we can do here.
		 * Abort immediately.
		 */
		unqlite_context_throw_error(pCtx, UNQLITE_CTX_ERR, "Fatal, out of memory");
		/* emulate the die() construct */
		return UNQLITE_ABORT; 
	}
	/* Populate the array.
	 * Note that we will use the same worker scalar value (pValue) here rather than
	 * allocating a new value for each array entry. This is due to the fact
	 * that the populated array will make it's own private copy of the inserted
	 * key(if available) and it's associated value.
	 */
	
	unqlite_value_int(pValue, pNow->tm_hour); /* Hour */
	/* Insert the hour at the first available index */
	unqlite_array_add_elem(pArray, 0/* NULL: Assign an automatic index*/, pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	unqlite_value_int(pValue, pNow->tm_min); /* Minutes */
	/* Insert minutes */
	unqlite_array_add_elem(pArray, 0/* NULL: Assign an automatic index*/, pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	unqlite_value_int(pValue, pNow->tm_sec); /* Seconds */
	/* Insert seconds */
	unqlite_array_add_elem(pArray, 0/* NULL: Assign an automatic index*/, pValue /* Will make it's own copy */);

	/* Return the array as the function return value */
	unqlite_result_value(pCtx, pArray);

	/* All done. Don't worry about freeing memory here, every
	 * allocated resource will be released automatically by the engine
	 * as soon we return from this foreign function.
	 */
	return UNQLITE_OK;
}
/*
 * object object_date_func(void)
 *  以 JSON 数组形式返回 'struct tm' 结构的副本。
 * 说明
 *  此函数不接受参数并返回
 *  'time.h' 头文件中 'struct tm' 结构的副本。
 *  此结构保存当前系统日期和时间。
 * 参数
 *  无
 * 返回值
 *   包含 'struct tm' 结构副本的关联数组。
 * 用法示例
 *
 *   print object_date_func();
 *
 * 运行时应看到类似以下内容：
 * JSON Object(6 {
 *  "tm_year":2012,
 *  "tm_mon":12,
 *  "tm_mday":29,
 *  "tm_hour":1,
 *  "tm_min":13,
 *  "tm_sec":58
 *  }
 * )
 */
int object_date_func(unqlite_context *pCtx, int argc /* unused */, unqlite_value **argv /* unused */)
{
	unqlite_value *pObject;    /* Our JSON object */
	unqlite_value *pValue;    /* Objecr entries value */
	struct tm *pNow;
	/* Get the current time first */
#if defined(_MSC_VER) || defined(__MINGW32__)
    errno_t err;
    __time64_t long_time;
    struct tm newtime;
    _time64( &long_time );
    err = _localtime64_s(&newtime, &long_time);
    if (err != 0) return UNQLITE_INVALID;
    pNow = &newtime;
#else
    time_t tt;
    time(&tt);
    pNow = localtime(&tt);
#endif
	/* Create a new JSON object */
	pObject = unqlite_context_new_array(pCtx);
	/* Create a worker scalar value */
	pValue = unqlite_context_new_scalar(pCtx);
	if( pObject == 0 || pValue == 0 ){
		/*
		 * If the supplied memory subsystem is so sick that we are unable
		 * to allocate a tiny chunk of memory, there is no much we can do here.
		 * Abort immediately.
		 */
		unqlite_context_throw_error(pCtx, UNQLITE_CTX_ERR, "Fatal, out of memory");
		/* emulate the die() construct */
		return UNQLITE_ABORT;
	}
	/* Populate the array.
	 * Note that we will use the same worker scalar value (pValue) here rather than
	 * allocating a new value for each array entry. This is due to the fact
	 * that the populated array will make it's own private copy of the inserted
	 * key(if available) and it's associated value.
	 */
	
	unqlite_value_int(pValue, pNow->tm_year + 1900); /* Year */
	/* Insert Year */
	unqlite_array_add_strkey_elem(pObject, "tm_year", pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	unqlite_value_int(pValue, pNow->tm_mon + 1); /* Month [1-12]*/
	/* Insert month number */
	unqlite_array_add_strkey_elem(pObject, "tm_mon", pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	unqlite_value_int(pValue, pNow->tm_mday); /* Day of the month [1-31] */
	/* Insert the day of the month */
	unqlite_array_add_strkey_elem(pObject, "tm_mday", pValue /* Will make it's own copy */);

	unqlite_value_int(pValue, pNow->tm_hour); /* Hour */
	/* Insert the hour */
	unqlite_array_add_strkey_elem(pObject, "tm_hour", pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	unqlite_value_int(pValue, pNow->tm_min); /* Minutes */
	/* Insert minutes */
	unqlite_array_add_strkey_elem(pObject, "tm_min", pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	unqlite_value_int(pValue, pNow->tm_sec); /* Seconds */
	/* Insert seconds */
	unqlite_array_add_strkey_elem(pObject, "tm_sec", pValue /* Will make it's own copy */);

	/* Return the JSON object as the function return value */
	unqlite_result_value(pCtx, pObject);
	/* All done. Don't worry about freeing memory here, every
	 * allocated resource will be released automatically by the engine
	 * as soon we return from this foreign function.
	 */
	return UNQLITE_OK;
}
/*
 * array array_string_split(string $str)
 *  返回字符串每个字符组成的数组副本。
 * 说明
 *  此函数将给定字符串分割为
 *  各个字符并以数组形式返回结果。
 * 参数
 *  $str
 *     要分割的目标字符串。
 * 返回值
 *   包含字符串字符的数组。
 * 用法示例
 *
 *   print array_str_split('Hello');
 *
 * 运行时应看到类似以下内容：
 *   JSON Array(5 ["H","e","l","l","o"])
 */
int array_string_split_func(unqlite_context *pCtx, int argc, unqlite_value **argv)
{
	unqlite_value *pArray;    /* Our JSON Array */
	unqlite_value *pValue;    /* Array entries value */
	const char *zString, *zEnd;  /* String to split */
	int nByte;            /* String length */
	/* Make sure there is at least one argument and is of the
	 * expected type [i.e: string].
	 */
	if( argc < 1 || !unqlite_value_is_string(argv[0]) ){
		/*
		 * Missing/Invalid argument, throw a warning and return FALSE.
		 * Note that you do not need to log the function name, UnQLite will
		 * automatically append the function name for you.
		 */
		unqlite_context_throw_error(pCtx, UNQLITE_CTX_WARNING, "Missing string to split");
		/* Return false */
		unqlite_result_bool(pCtx, 0);
		return UNQLITE_OK;
	}
	/* Extract the target string.
	 * Note that zString is null terminated and unqlite_value_to_string() never
	 * fail and always return a pointer to a null terminated string.
	 */
	zString = unqlite_value_to_string(argv[0], &nByte /* String length */);
	if( nByte < 1 /* Empty string [i.e: '' or ""] */ ){
		unqlite_context_throw_error(pCtx, UNQLITE_CTX_NOTICE, "Empty string");
		/* Return false */
		unqlite_result_bool(pCtx, 0);
		return UNQLITE_OK;
	}
	/* Create our array */
	pArray = unqlite_context_new_array(pCtx);
	/* Create a scalar worker value */
	pValue = unqlite_context_new_scalar(pCtx);
	/* Split the target string */
	zEnd = &zString[nByte]; /* Delimit the string */
	while( zString < zEnd ){
		int c = zString[0];
		/* Prepare the character for insertion */
		unqlite_value_string(pValue, (const char *)&c, (int)sizeof(char));
		/* Insert the character */
		unqlite_array_add_elem(pArray, 0/* NULL: Assign an automatic index */, pValue /* Will make it's own copy*/);
		/* Erase the previous data from the worker variable */
		unqlite_value_reset_string_cursor(pValue);
		/* Next character */
		zString++;
	}
	/* Return our array as the function return value */
	unqlite_result_value(pCtx, pArray);
	/* All done. Don't worry about freeing memory here, every
	 * allocated resource will be released automatically by the engine
	 * as soon we return from this foreign function.
	 */
	return UNQLITE_OK;
}
/*
 * 上面定义的外部函数的容器。
 * 这些函数稍后将使用调用
 * [unqlite_create_function()] 注册。
 */
static const struct foreign_func {
	const char *zName; /* Name of the foreign function*/
	int (*xProc)(unqlite_context *, int, unqlite_value **); /* Pointer to the C function performing the computation*/
}aFunc[] = {
	{"shift_func", shift_func}, 
	{"date_func", date_func}, 
	{"sum_func",  sum_func  }, 
	{"array_time_func", array_time_func}, 
	{"array_str_split", array_string_split_func}, 
	{"object_date_func", object_date_func}
};
/* Forward declaration: VM output consumer callback */
static int VmOutputConsumer(const void *pOutput,unsigned int nOutLen,void *pUserData /* Unused */);
/*
 * 以下是由 UnQLite VM 稍后执行的 Jx9 程序：
 *
 * //测试外部函数机制
 *  print 'shift_func(150) = ' .. shift_func(150) .. JX9_EOL;
 *  print 'sum_func(7,8,9,10) = ' .. sum_func(7,8,9,10) .. JX9_EOL;
 *  print 'date_func(5) = ' .. date_func() .. JX9_EOL;
 *  print 'array_time_func() =' .. array_time_func() .. JX9_EOL;
 *  print 'object_date_func() =' ..  JX9_EOL;
 *  dump(object_date_func());
 *  print 'array_str_split('Hello') ='  .. JX9_EOL;
 *  dump(array_str_split('Hello'))
 *
 * 运行时应看到类似以下内容：
 *
 * shift_func(150) = 300
 * sum_func(7,8,9,10) = 34
 * date_func(5) = 2013-06-12 01:13:58
 * array_time_func() = [1,13,58]
 * object_date_func() =
 * JSON Object(6 {
 *  "tm_year":2012,
 *  "tm_mon":12,
 *  "tm_mday":29,
 *  "tm_hour":1,
 *  "tm_min":13,
 *  "tm_sec":58
 *  }
 * )
 * array_str_split('Hello') =
 *  JSON Array(5 ["H","e","l","l","o"])
 *
 * 注意：'..'（两个点）是连接运算符（即相当于 Javascript 中的 '+'）
 */
#define JX9_PROG \
  "print 'shift_func(150) = ' .. shift_func(150) .. JX9_EOL;"\
  "print 'sum_func(7,8,9,10) = ' .. sum_func(7,8,9,10) .. JX9_EOL;"\
  "print 'date_func(5) = ' .. date_func() .. JX9_EOL;"\
  "print 'array_time_func() =' .. array_time_func() .. JX9_EOL;"\
  "print 'object_date_func() =' ..JX9_EOL;"\
  "dump(object_date_func());"\
  "print 'array_str_split(\\'Hello\\') =' .. JX9_EOL;"\
  "dump(array_str_split('Hello'));"

 /* 无需命令行参数，所有内容都存储在内存中 */
int main(void)
{
	unqlite *pDb;       /* 数据库句柄 */
	unqlite_vm *pVm;    /* 成功编译目标 Jx9 脚本产生的 UnQLite VM */
	int i,rc;

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

	/* 现在我们的程序已编译，是时候注册
	 * 我们的外部函数了。
	 */
	for( i = 0 ; i < (int)sizeof(aFunc)/sizeof(aFunc[0]) ;  ++i ){
		/* 安装外部函数 */
		rc = unqlite_create_function(pVm, aFunc[i].zName, aFunc[i].xProc, 0 /* NULL：无私有数据 */);
		if( rc != UNQLITE_OK ){
			Fatal(pDb,"注册外部函数时出错");
		}
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
