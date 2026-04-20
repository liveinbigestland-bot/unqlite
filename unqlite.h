/* 本文件由自动生成。请勿修改（编译时指令除外）！*/
#ifndef _UNQLITE_H_
#define _UNQLITE_H_
/* 确保可以从 C++ 调用这些内容 */
#ifdef __cplusplus
 extern "C" {
#endif 
/*
 * Symisc UnQLite: 一个嵌入式 NoSQL（后现代）数据库引擎。
 * 版权所有 (C) 2012-2019, Symisc Systems http://unqlite.org/
 * 版本 1.1.9
 * 有关许可协议、再分发和免责声明的详细信息，请联系 Symisc Systems：
 *       legal@symisc.net
 *       licensing@symisc.net
 *       contact@symisc.net
 * 或访问：
 *      http://unqlite.org/licensing.html
 */
/*
 * 版权所有 (C) 2012, 2019 Symisc Systems, S.U.A.R.L [M.I.A.G Mrad Chems Eddine <chm@symisc.net>]。
 * 保留所有权利。
 *
 * 在满足以下条件的前提下，允许以源代码和二进制形式重新分发和使用：
 * 1. 源代码的分发必须保留上述版权声明、本条件列表和以下免责声明。
 * 2. 二进制形式的分发必须在提供的文档和/或其他材料中重现上述版权
 *    声明、本条件列表和以下免责声明。
 *
 * 本软件按"原样"提供，SYMISC SYSTEMS 不做任何明示或暗示的保证，
 * 包括但不限于对适销性、特定用途适用性和非侵权性的保证。在任何情况下，
 * SYMISC SYSTEMS 均不对任何直接、间接、偶然、特殊、惩戒性或后果性
 * 损害（包括但不限于替代商品或服务的采购、使用、数据丢失、利润损失
 * 或业务中断）承担责任，无论何种原因导致，即使已被告知可能发生此类损害。
 */
 /* $SymiscID: unqlite.h v1.3 Win10 2108-04-27 02:35:11 stable <chm@symisc.net>  $ */
#include <stdarg.h> /* va_list 定义所需 */
/*
 * 编译时引擎版本、签名、Symisc 源码树中的标识和版权声明。
 * 每个宏都有对应的 C 接口函数，提供相同的信息，但与库关联而非头文件。
 * 请参阅 [unqlite_lib_version()]、[unqlite_lib_signature()]、[unqlite_lib_ident()] 和
 * [unqlite_lib_copyright()] 了解更多信息。
 */
/*
 * UNQLITE_VERSION 预处理器宏计算为一个字符串字面量，
 * 格式为 "X.Y.Z"，其中 X 为主版本号，Y 为次版本号，Z 为发布版本号。
 */
#define UNQLITE_VERSION "1.1.9"
/*
 * UNQLITE_VERSION_NUMBER 预处理器宏解析为一个整数值 (X*1000000 + Y*1000 + Z)，
 * 其中 X、Y 和 Z 与 [UNQLITE_VERSION] 中使用的数字相同。
 */
#define UNQLITE_VERSION_NUMBER 1001009
/*
 * UNQLITE_SIG 预处理器宏计算为字符串字面量，表示 unqlite 引擎的公共签名。
 * 此签名可包含在主机应用程序生成的 Server MIME 头中，如下所示：
 *   Server: YourWebServer/x.x unqlite/x.x.x \r\n
 */
#define UNQLITE_SIG "unqlite/1.1.9"
/*
 * UnQLite 在 Symisc 源码树中的标识：
 * Symisc Systems 发布的每个软件版本都有唯一的标识符。
 * 此宏保存与 unqlite 关联的标识符。
 */
#define UNQLITE_IDENT "unqlite:29c173b1-ac2c-4b49-93ba-e600619e304e"
/*
 * 版权声明。
 * 如有关于许可协议的问题，请访问 http://unqlite.org/licensing.html
 * 或联系 Symisc Systems：
 *   legal@symisc.net
 *   licensing@symisc.net
 *   contact@symisc.net
 */
#define UNQLITE_COPYRIGHT "版权所有 (C) Symisc Systems, S.U.A.R.L [Mrad Chems Eddine <chm@symisc.net>] 2012-2019, http://unqlite.org/"

/* 公共对象的前向声明 */
typedef struct unqlite_io_methods unqlite_io_methods;
typedef struct unqlite_kv_methods unqlite_kv_methods;
typedef struct unqlite_kv_engine unqlite_kv_engine;
typedef struct jx9_io_stream unqlite_io_stream;
typedef struct jx9_context unqlite_context;
typedef struct jx9_value unqlite_value;
typedef struct unqlite_vfs unqlite_vfs;
typedef struct unqlite_vm unqlite_vm;
typedef struct unqlite unqlite;
/*
 * ------------------------------
 * 编译时指令
 * ------------------------------
 * 在大多数情况下，UnQLite 可以使用默认编译选项正常构建。
 * 但是，如果需要，可以使用以下文档化的编译时选项来省略 UnQLite
 * 的某些功能（从而减小编译后的库大小）或更改某些参数的默认值。
 * 已尽一切努力确保各种编译选项组合能够协调工作并产生可用的库。
 *
 * UNQLITE_ENABLE_THREADS
 *  此选项控制是否在 UnQLite 中包含代码以使其能够在线程安全的环境中运行。
 *  默认是不包含。所有互斥代码都被省略，在多线程程序中使用 UnQLite 是不安全的。
 *  当使用 UNQLITE_ENABLE_THREADS 指令启用编译时，UnQLite 可以用于多线程程序，
 *  并且可以安全地在两个或更多线程之间共享相同的虚拟机和引擎句柄。
 *  UNQLITE_ENABLE_THREADS 的值可以使用 unqlite_lib_is_threadsafe() 接口在运行时确定。
 *  当 UnQLite 已使用线程支持编译时，可以使用 unqlite_lib_config() 接口以及以下动词之一
 *  来更改线程模式：
 *    UNQLITE_LIB_CONFIG_THREAD_LEVEL_SINGLE
 *    UNQLITE_LIB_CONFIG_THREAD_LEVEL_MULTI
 *  Windows 和 UNIX 系统以外的平台必须通过 unqlite_lib_config() 并将配置动词设置为
 *  UNQLITE_LIB_CONFIG_USER_MUTEX 来安装自己的互斥子系统。否则库不是线程安全的。
 *  注意，在 UNIX 系统下必须将 UnQLite 与 POSIX 线程库链接（即：-lpthread）。
 *
 * 功能省略/启用选项
 *
 * 以下选项可用于通过省略可选功能来减小编译库的大小。这可能仅在空间特别紧张的
 * 嵌入式系统中有用，因为即使包含所有功能，UnQLite 库也相对较小。
 * 别忘了告诉编译器优化二进制大小！（如果使用 GCC，则为 -Os 选项）。
 * 告诉编译器优化大小通常比使用这些编译时选项对库占用空间的影响更大。
 *
 * JX9_DISABLE_BUILTIN_FUNC
 *  Jx9 附带了 300 多个内置函数，适用于大多数用途，如字符串和 INI 处理、
 *  ZIP 提取、Base64 编码/解码、JSON 编码/解码等。
 *  如果启用此指令，则所有内置 Jx9 函数都将从构建中省略。
 *  请注意，db_create()、db_store()、db_fetch() 等特殊函数不会从构建中省略，
 *  也不受此指令的影响。
 *
 * JX9_ENABLE_MATH_FUNC
 *  如果启用此指令，则内置数学函数（如 sqrt()、abs()、log()、ceil() 等）
 *  将包含在构建中。注意，您可能需要在相同的 Linux/BSD 版本中将 UnQLite 与数学库链接
 * （即：-lm）。
 *
 * JX9_DISABLE_DISK_IO
 *  如果启用此指令，则内置 VFS 函数（如 chdir()、mkdir()、chroot()、unlink()、sleep() 等）
 *  将从构建中省略。
 *
 * UNQLITE_ENABLE_JX9_HASH_IO
 * 如果启用此指令，则内置哈希函数（如 md5()、sha1()、md5_file()、crc32() 等）
 * 将包含在构建中。
 */
/* Symisc 公共定义 */
#if !defined(SYMISC_STANDARD_DEFS)
#define SYMISC_STANDARD_DEFS
#if defined (_WIN32) || defined (WIN32) || defined(__MINGW32__) || defined (_MSC_VER) || defined (_WIN32_WCE)
/* Windows 系统 */
#if !defined(__WINNT__)
#define __WINNT__
#endif 
/*
 * 确定我们是否正在处理 WindowsCE——它有一个大幅精简的 API。
 */
#if defined(_WIN32_WCE)
#ifndef __WIN_CE__
#define __WIN_CE__
#endif /* __WIN_CE__ */
#endif /* _WIN32_WCE */
#else
/*
 * 默认情况下，我们将假设我们正在 UNIX 系统上编译。
 * 否则必须定义 OS_OTHER 指令。
 */
#if !defined(OS_OTHER)
#if !defined(__UNIXES__)
#define __UNIXES__
#endif /* __UNIXES__ */
#else
#endif /* OS_OTHER */
#endif /* __WINNT__/__UNIXES__ */
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef signed __int64     sxi64; /* 64 bits(8 bytes) signed int64 */
typedef unsigned __int64   sxu64; /* 64 bits(8 bytes) unsigned int64 */
#else
typedef signed long long int   sxi64; /* 64 bits(8 bytes) signed int64 */
typedef unsigned long long int sxu64; /* 64 bits(8 bytes) unsigned int64 */
#endif /* _MSC_VER */
/* 消费者例程的签名 */
typedef int (*ProcConsumer)(const void *, unsigned int, void *);
/* 前向引用 */
typedef struct SyMutexMethods SyMutexMethods;
typedef struct SyMemMethods SyMemMethods;
typedef struct SyString SyString;
typedef struct syiovec syiovec;
typedef struct SyMutex SyMutex;
typedef struct Sytm Sytm;
/* Scatter and gather array. */
struct syiovec
{
#if defined (__WINNT__)
	/* Same fields type and offset as WSABUF structure defined one winsock2 header */
	unsigned long nLen;
	char *pBase;
#else
	void *pBase;
	unsigned long nLen;
#endif
};
struct SyString
{
	const char *zString;  /* Raw string (may not be null terminated) */
	unsigned int nByte;   /* Raw string length */
};
/* 时间结构。*/
struct Sytm
{
  int tm_sec;     /* 秒 (0 - 60) */
  int tm_min;     /* 分钟 (0 - 59) */
  int tm_hour;    /* 小时 (0 - 23) */
  int tm_mday;    /* 月中的某天 (1 - 31) */
  int tm_mon;     /* 年中的月份 (0 - 11) */
  int tm_year;    /* 年 + 1900 */
  int tm_wday;    /* 星期几 (周日 = 0) */
  int tm_yday;    /* 年中的某天 (0 - 365) */
  int tm_isdst;   /* 是否夏令时生效？*/
  char *tm_zone;  /* 时区名称缩写 */
  long tm_gmtoff; /* 与 UTC 的偏移量（秒）*/
};
/* 将 <time.h> 中的 tm 结构 (struct tm *) 转换为 Sytm 结构 */
#define STRUCT_TM_TO_SYTM(pTM, pSYTM) \
	(pSYTM)->tm_hour = (pTM)->tm_hour;\
	(pSYTM)->tm_min	 = (pTM)->tm_min;\
	(pSYTM)->tm_sec	 = (pTM)->tm_sec;\
	(pSYTM)->tm_mon	 = (pTM)->tm_mon;\
	(pSYTM)->tm_mday = (pTM)->tm_mday;\
	(pSYTM)->tm_year = (pTM)->tm_year + 1900;\
	(pSYTM)->tm_yday = (pTM)->tm_yday;\
	(pSYTM)->tm_wday = (pTM)->tm_wday;\
	(pSYTM)->tm_isdst = (pTM)->tm_isdst;\
	(pSYTM)->tm_gmtoff = 0;\
	(pSYTM)->tm_zone = 0;

/* Convert a SYSTEMTIME structure (LPSYSTEMTIME: Windows Systems only ) to a Sytm structure */
#define SYSTEMTIME_TO_SYTM(pSYSTIME, pSYTM) \
	 (pSYTM)->tm_hour = (pSYSTIME)->wHour;\
	 (pSYTM)->tm_min  = (pSYSTIME)->wMinute;\
	 (pSYTM)->tm_sec  = (pSYSTIME)->wSecond;\
	 (pSYTM)->tm_mon  = (pSYSTIME)->wMonth - 1;\
	 (pSYTM)->tm_mday = (pSYSTIME)->wDay;\
	 (pSYTM)->tm_year = (pSYSTIME)->wYear;\
	 (pSYTM)->tm_yday = 0;\
	 (pSYTM)->tm_wday = (pSYSTIME)->wDayOfWeek;\
	 (pSYTM)->tm_gmtoff = 0;\
	 (pSYTM)->tm_isdst = -1;\
	 (pSYTM)->tm_zone = 0;

/* 动态内存分配方法。*/
struct SyMemMethods 
{
	void * (*xAlloc)(unsigned int);          /* [必需:] 分配内存块 */
	void * (*xRealloc)(void *, unsigned int); /* [必需:] 重新分配内存块 */
	void   (*xFree)(void *);                 /* [必需:] 释放内存块 */
	unsigned int  (*xChunkSize)(void *);     /* [可选:] 返回块大小 */
	int    (*xInit)(void *);                 /* [可选:] 初始化回调 */
	void   (*xRelease)(void *);              /* [可选:] 释放回调 */
	void  *pUserData;                        /* [可选:] xInit() 和 xRelease() 的第一个参数 */
};
/* 内存不足回调签名。*/
typedef int (*ProcMemError)(void *);
/* 互斥方法。*/
struct SyMutexMethods 
{
	int (*xGlobalInit)(void);		/* [可选:] 全局互斥初始化 */
	void  (*xGlobalRelease)(void);	/* [可选:] 全局释放回调 () */
	SyMutex * (*xNew)(int);	        /* [必需:] 请求新的互斥 */
	void  (*xRelease)(SyMutex *);	/* [可选:] 释放互斥  */
	void  (*xEnter)(SyMutex *);	    /* [必需:] 进入互斥 */
	int (*xTryEnter)(SyMutex *);    /* [可选:] 尝试进入互斥 */
	void  (*xLeave)(SyMutex *);	    /* [必需:] 离开锁定的互斥 */
};
#if defined (_MSC_VER) || defined (__MINGW32__) ||  defined (__GNUC__) && defined (__declspec)
#define SX_APIIMPORT	__declspec(dllimport)
#define SX_APIEXPORT	__declspec(dllexport)
#else
#define	SX_APIIMPORT
#define	SX_APIEXPORT
#endif
/* Standard return values from Symisc public interfaces */
#define SXRET_OK       0      /* Not an error */	
#define SXERR_MEM      (-1)   /* Out of memory */
#define SXERR_IO       (-2)   /* IO error */
#define SXERR_EMPTY    (-3)   /* Empty field */
#define SXERR_LOCKED   (-4)   /* Locked operation */
#define SXERR_ORANGE   (-5)   /* Out of range value */
#define SXERR_NOTFOUND (-6)   /* Item not found */
#define SXERR_LIMIT    (-7)   /* Limit reached */
#define SXERR_MORE     (-8)   /* Need more input */
#define SXERR_INVALID  (-9)   /* Invalid parameter */
#define SXERR_ABORT    (-10)  /* User callback request an operation abort */
#define SXERR_EXISTS   (-11)  /* Item exists */
#define SXERR_SYNTAX   (-12)  /* Syntax error */
#define SXERR_UNKNOWN  (-13)  /* Unknown error */
#define SXERR_BUSY     (-14)  /* Busy operation */
#define SXERR_OVERFLOW (-15)  /* Stack or buffer overflow */
#define SXERR_WILLBLOCK (-16) /* Operation will block */
#define SXERR_NOTIMPLEMENTED  (-17) /* Operation not implemented */
#define SXERR_EOF      (-18) /* End of input */
#define SXERR_PERM     (-19) /* Permission error */
#define SXERR_NOOP     (-20) /* No-op */	
#define SXERR_FORMAT   (-21) /* Invalid format */
#define SXERR_NEXT     (-22) /* Not an error */
#define SXERR_OS       (-23) /* System call return an error */
#define SXERR_CORRUPT  (-24) /* Corrupted pointer */
#define SXERR_CONTINUE (-25) /* Not an error: Operation in progress */
#define SXERR_NOMATCH  (-26) /* No match */
#define SXERR_RESET    (-27) /* Operation reset */
#define SXERR_DONE     (-28) /* Not an error */
#define SXERR_SHORT    (-29) /* Buffer too short */
#define SXERR_PATH     (-30) /* Path error */
#define SXERR_TIMEOUT  (-31) /* Timeout */
#define SXERR_BIG      (-32) /* Too big for processing */
#define SXERR_RETRY    (-33) /* Retry your call */
#define SXERR_IGNORE   (-63) /* Ignore */
#endif /* SYMISC_PUBLIC_DEFS */
/* 
 * Marker for exported interfaces. 
 */
#define UNQLITE_APIEXPORT SX_APIEXPORT
/*
 * If compiling for a processor that lacks floating point
 * support, substitute integer for floating-point.
 */
#ifdef UNQLITE_OMIT_FLOATING_POINT
typedef sxi64 uqlite_real;
#else
typedef double unqlite_real;
#endif
typedef sxi64 unqlite_int64;
/* 标准 UnQLite 返回值 */
#define UNQLITE_OK      SXRET_OK      /* 成功结果 */
/* 错误码开始 */
#define UNQLITE_NOMEM    SXERR_MEM     /* Out of memory */
#define UNQLITE_ABORT    SXERR_ABORT   /* Another thread have released this instance */
#define UNQLITE_IOERR    SXERR_IO      /* IO error */
#define UNQLITE_CORRUPT  SXERR_CORRUPT /* Corrupt pointer */
#define UNQLITE_LOCKED   SXERR_LOCKED  /* Forbidden Operation */ 
#define UNQLITE_BUSY	 SXERR_BUSY    /* The database file is locked */
#define UNQLITE_DONE	 SXERR_DONE    /* Operation done */
#define UNQLITE_PERM     SXERR_PERM    /* Permission error */
#define UNQLITE_NOTIMPLEMENTED SXERR_NOTIMPLEMENTED /* Method not implemented by the underlying Key/Value storage engine */
#define UNQLITE_NOTFOUND SXERR_NOTFOUND /* No such record */
#define UNQLITE_NOOP     SXERR_NOOP     /* No such method */
#define UNQLITE_INVALID  SXERR_INVALID  /* Invalid parameter */
#define UNQLITE_EOF      SXERR_EOF      /* End Of Input */
#define UNQLITE_UNKNOWN  SXERR_UNKNOWN  /* Unknown configuration option */
#define UNQLITE_LIMIT    SXERR_LIMIT    /* Database limit reached */
#define UNQLITE_EXISTS   SXERR_EXISTS   /* Record exists */
#define UNQLITE_EMPTY    SXERR_EMPTY    /* Empty record */
#define UNQLITE_COMPILE_ERR (-70)       /* Compilation error */
#define UNQLITE_VM_ERR      (-71)       /* Virtual machine error */
#define UNQLITE_FULL        (-73)       /* Full database (unlikely) */
#define UNQLITE_CANTOPEN    (-74)       /* Unable to open the database file */
#define UNQLITE_READ_ONLY   (-75)       /* Read only Key/Value storage engine */
#define UNQLITE_LOCKERR     (-76)       /* Locking protocol error */
/* 错误码结束 */
/*
 * 数据库句柄配置命令。
 *
 * 以下常量集是可用于主机应用程序配置 UnQLite 数据库句柄的配置动词。
 * 这些常量必须作为第二个参数传递给 [unqlite_config()]。
 *
 * 每个选项需要可变数量的参数。
 * [unqlite_config()] 接口成功时返回 UNQLITE_OK，任何其他返回值表示失败。
 * 有关配置动词及其预期参数的完整讨论，请参阅此页面：
 *      http://unqlite.org/c_api/unqlite_config.html
 */
#define UNQLITE_CONFIG_JX9_ERR_LOG         1  /* TWO ARGUMENTS: const char **pzBuf, int *pLen */
#define UNQLITE_CONFIG_MAX_PAGE_CACHE      2  /* ONE ARGUMENT: int nMaxPage */
#define UNQLITE_CONFIG_ERR_LOG             3  /* TWO ARGUMENTS: const char **pzBuf, int *pLen */
#define UNQLITE_CONFIG_KV_ENGINE           4  /* ONE ARGUMENT: const char *zKvName */
#define UNQLITE_CONFIG_DISABLE_AUTO_COMMIT 5  /* NO ARGUMENTS */
#define UNQLITE_CONFIG_GET_KV_NAME         6  /* ONE ARGUMENT: const char **pzPtr */
/*
 * UnQLite/Jx9 虚拟机配置命令。
 *
 * 以下常量集是可用于主机应用程序配置 Jx9（通过 UnQLite）虚拟机的配置动词。
 * 这些常量必须作为第二个参数传递给 [unqlite_vm_config()] 接口。
 * 每个选项需要可变数量的参数。
 * [unqlite_vm_config()] 接口成功时返回 UNQLITE_OK，任何其他返回值表示失败。
 * 有很多选项，但最重要的是：UNQLITE_VM_CONFIG_OUTPUT 用于安装 VM 输出消费者回调，
 * UNQLITE_VM_CONFIG_HTTP_REQUEST 用于解析和注册 HTTP 请求，
 * UNQLITE_VM_CONFIG_ARGV_ENTRY 用于填充 $argv 数组。
 * 有关配置动词及其预期参数的完整讨论，请参阅此页面：
 *      http://unqlite.org/c_api/unqlite_vm_config.html
 */
#define UNQLITE_VM_CONFIG_OUTPUT           1  /* TWO ARGUMENTS: int (*xConsumer)(const void *pOut, unsigned int nLen, void *pUserData), void *pUserData */
#define UNQLITE_VM_CONFIG_IMPORT_PATH      2  /* ONE ARGUMENT: const char *zIncludePath */
#define UNQLITE_VM_CONFIG_ERR_REPORT       3  /* NO ARGUMENTS: Report all run-time errors in the VM output */
#define UNQLITE_VM_CONFIG_RECURSION_DEPTH  4  /* ONE ARGUMENT: int nMaxDepth */
#define UNQLITE_VM_OUTPUT_LENGTH           5  /* ONE ARGUMENT: unsigned int *pLength */
#define UNQLITE_VM_CONFIG_CREATE_VAR       6  /* TWO ARGUMENTS: const char *zName, unqlite_value *pValue */
#define UNQLITE_VM_CONFIG_HTTP_REQUEST     7  /* TWO ARGUMENTS: const char *zRawRequest, int nRequestLength */
#define UNQLITE_VM_CONFIG_SERVER_ATTR      8  /* THREE ARGUMENTS: const char *zKey, const char *zValue, int nLen */
#define UNQLITE_VM_CONFIG_ENV_ATTR         9  /* THREE ARGUMENTS: const char *zKey, const char *zValue, int nLen */
#define UNQLITE_VM_CONFIG_EXEC_VALUE      10  /* ONE ARGUMENT: unqlite_value **ppValue */
#define UNQLITE_VM_CONFIG_IO_STREAM       11  /* ONE ARGUMENT: const unqlite_io_stream *pStream */
#define UNQLITE_VM_CONFIG_ARGV_ENTRY      12  /* ONE ARGUMENT: const char *zValue */
#define UNQLITE_VM_CONFIG_EXTRACT_OUTPUT  13  /* TWO ARGUMENTS: const void **ppOut, unsigned int *pOutputLen */
/*
 * 存储引擎配置命令。
 *
 * 以下常量集是可用于主机应用程序配置底层存储引擎（即 Hash、B+tree、R+tree）的配置动词。
 * 这些常量必须作为第一个参数传递给 [unqlite_kv_config()]。
 * 每个选项需要可变数量的参数。
 * [unqlite_kv_config()] 接口成功时返回 UNQLITE_OK，任何其他返回值表示失败。
 * 有关配置动词及其预期参数的完整讨论，请参阅此页面：
 *      http://unqlite.org/c_api/unqlite_kv_config.html
 */
#define UNQLITE_KV_CONFIG_HASH_FUNC  1 /* ONE ARGUMENT: unsigned int (*xHash)(const void *,unsigned int) */
#define UNQLITE_KV_CONFIG_CMP_FUNC   2 /* ONE ARGUMENT: int (*xCmp)(const void *,const void *,unsigned int) */
/*
 * Global Library Configuration Commands.
 *
 * The following set of constants are the available configuration verbs that can
 * be used by the host-application to configure the whole library.
 * These constants must be passed as the first argument to [unqlite_lib_config()].
 *
 * Each options require a variable number of arguments.
 * The [unqlite_lib_config()] interface will return UNQLITE_OK on success, any other return
 * value indicates failure.
 * Notes:
 * The default configuration is recommended for most applications and so the call to
 * [unqlite_lib_config()] is usually not necessary. It is provided to support rare 
 * applications with unusual needs. 
 * The [unqlite_lib_config()] interface is not threadsafe. The application must insure that
 * no other [unqlite_*()] interfaces are invoked by other threads while [unqlite_lib_config()]
 * is running. Furthermore, [unqlite_lib_config()] may only be invoked prior to library
 * initialization using [unqlite_lib_init()] or [unqlite_init()] or after shutdown
 * by [unqlite_lib_shutdown()]. If [unqlite_lib_config()] is called after [unqlite_lib_init()]
 * or [unqlite_init()] and before [unqlite_lib_shutdown()] then it will return UNQLITE_LOCKED.
 * For a full discussion on the configuration verbs and their expected parameters, please
 * refer to this page:
 *      http://unqlite.org/c_api/unqlite_lib.html
 */
#define UNQLITE_LIB_CONFIG_USER_MALLOC            1 /* ONE ARGUMENT: const SyMemMethods *pMemMethods */ 
#define UNQLITE_LIB_CONFIG_MEM_ERR_CALLBACK       2 /* TWO ARGUMENTS: int (*xMemError)(void *), void *pUserData */
#define UNQLITE_LIB_CONFIG_USER_MUTEX             3 /* ONE ARGUMENT: const SyMutexMethods *pMutexMethods */ 
#define UNQLITE_LIB_CONFIG_THREAD_LEVEL_SINGLE    4 /* NO ARGUMENTS */ 
#define UNQLITE_LIB_CONFIG_THREAD_LEVEL_MULTI     5 /* NO ARGUMENTS */ 
#define UNQLITE_LIB_CONFIG_VFS                    6 /* ONE ARGUMENT: const unqlite_vfs *pVfs */
#define UNQLITE_LIB_CONFIG_STORAGE_ENGINE         7 /* ONE ARGUMENT: unqlite_kv_methods *pStorage */
#define UNQLITE_LIB_CONFIG_PAGE_SIZE              8 /* ONE ARGUMENT: int iPageSize */
/*
 * These bit values are intended for use in the 3rd parameter to the [unqlite_open()] interface
 * and in the 4th parameter to the xOpen method of the [unqlite_vfs] object.
 */
#define UNQLITE_OPEN_READONLY         0x00000001  /* Read only mode. Ok for [unqlite_open] */
#define UNQLITE_OPEN_READWRITE        0x00000002  /* Ok for [unqlite_open] */
#define UNQLITE_OPEN_CREATE           0x00000004  /* Ok for [unqlite_open] */
#define UNQLITE_OPEN_EXCLUSIVE        0x00000008  /* VFS only */
#define UNQLITE_OPEN_TEMP_DB          0x00000010  /* VFS only */
#define UNQLITE_OPEN_NOMUTEX          0x00000020  /* Ok for [unqlite_open] */
#define UNQLITE_OPEN_OMIT_JOURNALING  0x00000040  /* Omit journaling for this database. Ok for [unqlite_open] */
#define UNQLITE_OPEN_IN_MEMORY        0x00000080  /* An in memory database. Ok for [unqlite_open]*/
#define UNQLITE_OPEN_MMAP             0x00000100  /* Obtain a memory view of the whole file. Ok for [unqlite_open] */
/*
 * Synchronization Type Flags
 *
 * When UnQLite invokes the xSync() method of an [unqlite_io_methods] object it uses
 * a combination of these integer values as the second argument.
 *
 * When the UNQLITE_SYNC_DATAONLY flag is used, it means that the sync operation only
 * needs to flush data to mass storage.  Inode information need not be flushed.
 * If the lower four bits of the flag equal UNQLITE_SYNC_NORMAL, that means to use normal
 * fsync() semantics. If the lower four bits equal UNQLITE_SYNC_FULL, that means to use
 * Mac OS X style fullsync instead of fsync().
 */
#define UNQLITE_SYNC_NORMAL        0x00002
#define UNQLITE_SYNC_FULL          0x00003
#define UNQLITE_SYNC_DATAONLY      0x00010
/*
 * 文件锁定级别
 *
 * UnQLite 使用这些整数值之一作为对 [unqlite_io_methods] 对象的
 * xLock() 和 xUnlock() 方法调用的第二个参数。
 */
#define UNQLITE_LOCK_NONE          0
#define UNQLITE_LOCK_SHARED        1
#define UNQLITE_LOCK_RESERVED      2
#define UNQLITE_LOCK_PENDING       3
#define UNQLITE_LOCK_EXCLUSIVE     4
/*
 * CAPIREF: OS 接口：打开文件句柄
 *
 * [unqlite_file] 对象表示 [unqlite_vfs] OS 接口层中打开的文件。
 * 单独的 OS 接口实现可能希望将此对象子类化，追加自己的字段。
 * pMethods 条目是指向 [unqlite_io_methods] 对象的指针，该对象定义了对
 * [unqlite_file] 对象表示的打开文件执行各种 I/O 操作的方法。
*/
typedef struct unqlite_file unqlite_file;
struct unqlite_file {
  const unqlite_io_methods *pMethods;  /* 打开文件的方法。必须是第一个 */
};
/*
 * CAPIREF: OS 接口：文件方法对象
 *
 * [unqlite_vfs] xOpen 方法打开的每个文件都会用指向此对象实例的指针填充
 * [unqlite_file] 对象（或更常见的 [unqlite_file] 对象的子类）。
 * 此对象定义了对 [unqlite_file] 对象表示的打开文件执行各种操作的方法。
 *
 * 如果 xOpen 方法将 unqlite_file.pMethods 元素设置为非 NULL 指针，
 * 则即使 xOpen 报告失败，也可能调用 unqlite_io_methods.xClose 方法。
 * 防止在 xOpen 失败后调用 xClose 的唯一方法是让 xOpen 将
 * unqlite_file.pMethods 元素设置为 NULL。
 *
 * xSync 的 flags 参数可以是 [UNQLITE_SYNC_NORMAL] 或 [UNQLITE_SYNC_FULL] 之一。
 * 第一种选择是正常的 fsync()。第二种选择是 Mac OS X 风格的 fullsync。
 * [UNQLITE_SYNC_DATAONLY] 标志可以通过 OR 运算添加，以表示只需要同步文件的数据，
 * 而不需要同步其 inode。
 *
 * xLock() 和 xUnlock() 的整数值是以下之一
 *
 * UNQLITE_LOCK_NONE
 * UNQLITE_LOCK_SHARED
 * UNQLITE_LOCK_RESERVED
 * UNQLITE_LOCK_PENDING
 * UNQLITE_LOCK_EXCLUSIVE
 * 
 * xLock() increases the lock. xUnlock() decreases the lock.
 * The xCheckReservedLock() method checks whether any database connection,
 * either in this process or in some other process, is holding a RESERVED,
 * PENDING, or EXCLUSIVE lock on the file. It returns true if such a lock exists
 * and false otherwise.
 * 
 * The xSectorSize() method returns the sector size of the device that underlies
 * the file. The sector size is the minimum write that can be performed without
 * disturbing other bytes in the file.
 *
 */
struct unqlite_io_methods {
  int iVersion;                 /* Structure version number (currently 1) */
  int (*xClose)(unqlite_file*);
  int (*xRead)(unqlite_file*, void*, unqlite_int64 iAmt, unqlite_int64 iOfst);
  int (*xWrite)(unqlite_file*, const void*, unqlite_int64 iAmt, unqlite_int64 iOfst);
  int (*xTruncate)(unqlite_file*, unqlite_int64 size);
  int (*xSync)(unqlite_file*, int flags);
  int (*xFileSize)(unqlite_file*, unqlite_int64 *pSize);
  int (*xLock)(unqlite_file*, int);
  int (*xUnlock)(unqlite_file*, int);
  int (*xCheckReservedLock)(unqlite_file*, int *pResOut);
  int (*xSectorSize)(unqlite_file*);
};
/*
 * CAPIREF: OS Interface Object
 *
 * An instance of the unqlite_vfs object defines the interface between
 * the UnQLite core and the underlying operating system.  The "vfs"
 * in the name of the object stands for "Virtual File System".
 *
 * Only a single vfs can be registered within the UnQLite core.
 * Vfs registration is done using the [unqlite_lib_config()] interface
 * with a configuration verb set to UNQLITE_LIB_CONFIG_VFS.
 * Note that Windows and UNIX (Linux, FreeBSD, Solaris, Mac OS X, etc.) users
 * does not have to worry about registering and installing a vfs since UnQLite
 * come with a built-in vfs for these platforms that implements most the methods
 * defined below.
 *
 * Clients running on exotic systems (ie: Other than Windows and UNIX systems)
 * must register their own vfs in order to be able to use the UnQLite library.
 *
 * The value of the iVersion field is initially 1 but may be larger in
 * future versions of UnQLite. 
 *
 * The szOsFile field is the size of the subclassed [unqlite_file] structure
 * used by this VFS. mxPathname is the maximum length of a pathname in this VFS.
 * 
 * At least szOsFile bytes of memory are allocated by UnQLite to hold the [unqlite_file]
 * structure passed as the third argument to xOpen. The xOpen method does not have to
 * allocate the structure; it should just fill it in. Note that the xOpen method must
 * set the unqlite_file.pMethods to either a valid [unqlite_io_methods] object or to NULL.
 * xOpen must do this even if the open fails. UnQLite expects that the unqlite_file.pMethods
 * element will be valid after xOpen returns regardless of the success or failure of the
 * xOpen call.
 *
 */
struct unqlite_vfs {
  const char *zName;       /* Name of this virtual file system [i.e: Windows, UNIX, etc.] */
  int iVersion;            /* Structure version number (currently 1) */
  int szOsFile;            /* Size of subclassed unqlite_file */
  int mxPathname;          /* Maximum file pathname length */
  int (*xOpen)(unqlite_vfs*, const char *zName, unqlite_file*,unsigned int flags);
  int (*xDelete)(unqlite_vfs*, const char *zName, int syncDir);
  int (*xAccess)(unqlite_vfs*, const char *zName, int flags, int *pResOut);
  int (*xFullPathname)(unqlite_vfs*, const char *zName,int buf_len,char *zBuf);
  int (*xTmpDir)(unqlite_vfs*,char *zBuf,int buf_len);
  int (*xSleep)(unqlite_vfs*, int microseconds);
  int (*xCurrentTime)(unqlite_vfs*,Sytm *pOut);
  int (*xGetLastError)(unqlite_vfs*, int, char *);
};
/*
 * Flags for the xAccess VFS method
 *
 * These integer constants can be used as the third parameter to
 * the xAccess method of an [unqlite_vfs] object.  They determine
 * what kind of permissions the xAccess method is looking for.
 * With UNQLITE_ACCESS_EXISTS, the xAccess method
 * simply checks whether the file exists.
 * With UNQLITE_ACCESS_READWRITE, the xAccess method
 * checks whether the named directory is both readable and writable
 * (in other words, if files can be added, removed, and renamed within
 * the directory).
 * The UNQLITE_ACCESS_READWRITE constant is currently used only by the
 * [temp_store_directory pragma], though this could change in a future
 * release of UnQLite.
 * With UNQLITE_ACCESS_READ, the xAccess method
 * checks whether the file is readable.  The UNQLITE_ACCESS_READ constant is
 * currently unused, though it might be used in a future release of
 * UnQLite.
 */
#define UNQLITE_ACCESS_EXISTS    0
#define UNQLITE_ACCESS_READWRITE 1   
#define UNQLITE_ACCESS_READ      2 
/*
 * The type used to represent a page number.  The first page in a file
 * is called page 1.  0 is used to represent "not a page".
 * A page number is an unsigned 64-bit integer.
 */
typedef sxu64 pgno;
/*
 * A database disk page is represented by an instance
 * of the following structure.
 */
typedef struct unqlite_page unqlite_page;
struct unqlite_page
{
  unsigned char *zData;       /* Content of this page */
  void *pUserData;            /* Extra content */
  pgno iPage;                 /* Page number for this page */
};
/*
 * UnQLite handle to the underlying Key/Value Storage Engine (See below).
 */
typedef void * unqlite_kv_handle;
/*
 * UnQLite pager IO methods.
 *
 * An instance of the following structure define the exported methods of the UnQLite pager
 * to the underlying Key/Value storage engine.
 */
typedef struct unqlite_kv_io unqlite_kv_io;
struct unqlite_kv_io
{
	unqlite_kv_handle  pHandle;     /* UnQLite handle passed as the first parameter to the
									 * method defined below.
									 */
	unqlite_kv_methods *pMethods;   /* Underlying storage engine */
	/* Pager methods */
	int (*xGet)(unqlite_kv_handle,pgno,unqlite_page **);
	int (*xLookup)(unqlite_kv_handle,pgno,unqlite_page **);
	int (*xNew)(unqlite_kv_handle,unqlite_page **);
	int (*xWrite)(unqlite_page *);
	int (*xDontWrite)(unqlite_page *);
	int (*xDontJournal)(unqlite_page *);
	int (*xDontMkHot)(unqlite_page *);
	int (*xPageRef)(unqlite_page *);
	int (*xPageUnref)(unqlite_page *);
	int (*xPageSize)(unqlite_kv_handle);
	int (*xReadOnly)(unqlite_kv_handle);
	unsigned char * (*xTmpPage)(unqlite_kv_handle);
	void (*xSetUnpin)(unqlite_kv_handle,void (*xPageUnpin)(void *)); 
	void (*xSetReload)(unqlite_kv_handle,void (*xPageReload)(void *));
	void (*xErr)(unqlite_kv_handle,const char *);
};
/*
 * Key/Value Storage Engine Cursor Object
 *
 * An instance of a subclass of the following object defines a cursor
 * used to scan through a key-value storage engine.
 */
typedef struct unqlite_kv_cursor unqlite_kv_cursor;
struct unqlite_kv_cursor
{
  unqlite_kv_engine *pStore; /* Must be first */
  /* Subclasses will typically add additional fields */
};
/*
 * Possible seek positions.
 */
#define UNQLITE_CURSOR_MATCH_EXACT  1
#define UNQLITE_CURSOR_MATCH_LE     2
#define UNQLITE_CURSOR_MATCH_GE     3
/*
 * Key/Value Storage Engine.
 *
 * A Key-Value storage engine is defined by an instance of the following
 * object.
 * UnQLite works with run-time interchangeable storage engines (i.e. Hash, B+Tree, R+Tree, LSM, etc.).
 * The storage engine works with key/value pairs where both the key
 * and the value are byte arrays of arbitrary length and with no restrictions on content.
 * UnQLite come with two built-in KV storage engine: A Virtual Linear Hash (VLH) storage
 * engine is used for persistent on-disk databases with O(1) lookup time and an in-memory
 * hash-table or Red-black tree storage engine is used for in-memory databases.
 * Future versions of UnQLite might add other built-in storage engines (i.e. LSM). 
 * Registration of a Key/Value storage engine at run-time is done via [unqlite_lib_config()]
 * with a configuration verb set to UNQLITE_LIB_CONFIG_STORAGE_ENGINE.
 */
struct unqlite_kv_engine
{
  const unqlite_kv_io *pIo; /* IO methods: MUST be first */
   /* Subclasses will typically add additional fields */
};
/*
 * Key/Value Storage Engine Virtual Method Table.
 *
 * Key/Value storage engine methods is defined by an instance of the following
 * object.
 * Registration of a Key/Value storage engine at run-time is done via [unqlite_lib_config()]
 * with a configuration verb set to UNQLITE_LIB_CONFIG_STORAGE_ENGINE.
 */
struct unqlite_kv_methods
{
  const char *zName; /* Storage engine name [i.e. Hash, B+tree, LSM, R-tree, Mem, etc.]*/
  int szKv;          /* 'unqlite_kv_engine' subclass size */
  int szCursor;      /* 'unqlite_kv_cursor' subclass size */
  int iVersion;      /* Structure version, currently 1 */
  /* Storage engine methods */
  int (*xInit)(unqlite_kv_engine *,int iPageSize);
  void (*xRelease)(unqlite_kv_engine *);
  int (*xConfig)(unqlite_kv_engine *,int op,va_list ap);
  int (*xOpen)(unqlite_kv_engine *,pgno);
  int (*xReplace)(
	  unqlite_kv_engine *,
	  const void *pKey,int nKeyLen,
	  const void *pData,unqlite_int64 nDataLen
	  ); 
    int (*xAppend)(
	  unqlite_kv_engine *,
	  const void *pKey,int nKeyLen,
	  const void *pData,unqlite_int64 nDataLen
	  );
  void (*xCursorInit)(unqlite_kv_cursor *);
  int (*xSeek)(unqlite_kv_cursor *,const void *pKey,int nByte,int iPos); /* Mandatory */
  int (*xFirst)(unqlite_kv_cursor *);
  int (*xLast)(unqlite_kv_cursor *);
  int (*xValid)(unqlite_kv_cursor *);
  int (*xNext)(unqlite_kv_cursor *);
  int (*xPrev)(unqlite_kv_cursor *);
  int (*xDelete)(unqlite_kv_cursor *);
  int (*xKeyLength)(unqlite_kv_cursor *,int *);
  int (*xKey)(unqlite_kv_cursor *,int (*xConsumer)(const void *,unsigned int,void *),void *pUserData);
  int (*xDataLength)(unqlite_kv_cursor *,unqlite_int64 *);
  int (*xData)(unqlite_kv_cursor *,int (*xConsumer)(const void *,unsigned int,void *),void *pUserData);
  void (*xReset)(unqlite_kv_cursor *);
  void (*xCursorRelease)(unqlite_kv_cursor *);
};
/*
 * UnQLite journal file suffix.
 */
#ifndef UNQLITE_JOURNAL_FILE_SUFFIX
#define UNQLITE_JOURNAL_FILE_SUFFIX "_unqlite_journal"
#endif
/*
 * Call Context - Error Message Severity Level.
 *
 * The following constants are the allowed severity level that can
 * passed as the second argument to the [unqlite_context_throw_error()] or
 * [unqlite_context_throw_error_format()] interfaces.
 * Refer to the official documentation for additional information.
 */
#define UNQLITE_CTX_ERR       1 /* Call context error such as unexpected number of arguments, invalid types and so on. */
#define UNQLITE_CTX_WARNING   2 /* Call context Warning */
#define UNQLITE_CTX_NOTICE    3 /* Call context Notice */
/* 
 * C-API-REF: Please refer to the official documentation for interfaces
 * purpose and expected parameters. 
 */ 

/* Database Engine Handle */
UNQLITE_APIEXPORT int unqlite_open(unqlite **ppDB,const char *zFilename,unsigned int iMode);
UNQLITE_APIEXPORT int unqlite_config(unqlite *pDb,int nOp,...);
UNQLITE_APIEXPORT int unqlite_close(unqlite *pDb);


/* Key/Value (KV) Store Interfaces */
UNQLITE_APIEXPORT int unqlite_kv_store(unqlite *pDb,const void *pKey,int nKeyLen,const void *pData,unqlite_int64 nDataLen);
UNQLITE_APIEXPORT int unqlite_kv_append(unqlite *pDb,const void *pKey,int nKeyLen,const void *pData,unqlite_int64 nDataLen);
UNQLITE_APIEXPORT int unqlite_kv_store_fmt(unqlite *pDb,const void *pKey,int nKeyLen,const char *zFormat,...);
UNQLITE_APIEXPORT int unqlite_kv_append_fmt(unqlite *pDb,const void *pKey,int nKeyLen,const char *zFormat,...);
UNQLITE_APIEXPORT int unqlite_kv_fetch(unqlite *pDb,const void *pKey,int nKeyLen,void *pBuf,unqlite_int64 /* in|out */*pBufLen);
UNQLITE_APIEXPORT int unqlite_kv_fetch_callback(unqlite *pDb,const void *pKey,
	                    int nKeyLen,int (*xConsumer)(const void *,unsigned int,void *),void *pUserData);
UNQLITE_APIEXPORT int unqlite_kv_delete(unqlite *pDb,const void *pKey,int nKeyLen);
UNQLITE_APIEXPORT int unqlite_kv_config(unqlite *pDb,int iOp,...);

/* Document (JSON) Store Interfaces powered by the Jx9 Scripting Language */
UNQLITE_APIEXPORT int unqlite_compile(unqlite *pDb,const char *zJx9,int nByte,unqlite_vm **ppOut);
UNQLITE_APIEXPORT int unqlite_compile_file(unqlite *pDb,const char *zPath,unqlite_vm **ppOut);
UNQLITE_APIEXPORT int unqlite_vm_config(unqlite_vm *pVm,int iOp,...);
UNQLITE_APIEXPORT int unqlite_vm_exec(unqlite_vm *pVm);
UNQLITE_APIEXPORT int unqlite_vm_reset(unqlite_vm *pVm);
UNQLITE_APIEXPORT int unqlite_vm_release(unqlite_vm *pVm);
UNQLITE_APIEXPORT int unqlite_vm_dump(unqlite_vm *pVm, int (*xConsumer)(const void *, unsigned int, void *), void *pUserData);
UNQLITE_APIEXPORT unqlite_value * unqlite_vm_extract_variable(unqlite_vm *pVm,const char *zVarname);

/*  Cursor Iterator Interfaces */
UNQLITE_APIEXPORT int unqlite_kv_cursor_init(unqlite *pDb,unqlite_kv_cursor **ppOut);
UNQLITE_APIEXPORT int unqlite_kv_cursor_release(unqlite *pDb,unqlite_kv_cursor *pCur);
UNQLITE_APIEXPORT int unqlite_kv_cursor_seek(unqlite_kv_cursor *pCursor,const void *pKey,int nKeyLen,int iPos);
UNQLITE_APIEXPORT int unqlite_kv_cursor_first_entry(unqlite_kv_cursor *pCursor);
UNQLITE_APIEXPORT int unqlite_kv_cursor_last_entry(unqlite_kv_cursor *pCursor);
UNQLITE_APIEXPORT int unqlite_kv_cursor_valid_entry(unqlite_kv_cursor *pCursor);
UNQLITE_APIEXPORT int unqlite_kv_cursor_next_entry(unqlite_kv_cursor *pCursor);
UNQLITE_APIEXPORT int unqlite_kv_cursor_prev_entry(unqlite_kv_cursor *pCursor);
UNQLITE_APIEXPORT int unqlite_kv_cursor_key(unqlite_kv_cursor *pCursor,void *pBuf,int *pnByte);
UNQLITE_APIEXPORT int unqlite_kv_cursor_key_callback(unqlite_kv_cursor *pCursor,int (*xConsumer)(const void *,unsigned int,void *),void *pUserData);
UNQLITE_APIEXPORT int unqlite_kv_cursor_data(unqlite_kv_cursor *pCursor,void *pBuf,unqlite_int64 *pnData);
UNQLITE_APIEXPORT int unqlite_kv_cursor_data_callback(unqlite_kv_cursor *pCursor,int (*xConsumer)(const void *,unsigned int,void *),void *pUserData);
UNQLITE_APIEXPORT int unqlite_kv_cursor_delete_entry(unqlite_kv_cursor *pCursor);
UNQLITE_APIEXPORT int unqlite_kv_cursor_reset(unqlite_kv_cursor *pCursor);

/* Manual Transaction Manager */
UNQLITE_APIEXPORT int unqlite_begin(unqlite *pDb);
UNQLITE_APIEXPORT int unqlite_commit(unqlite *pDb);
UNQLITE_APIEXPORT int unqlite_rollback(unqlite *pDb);

/* Utility interfaces */
UNQLITE_APIEXPORT int unqlite_util_load_mmaped_file(const char *zFile,void **ppMap,unqlite_int64 *pFileSize);
UNQLITE_APIEXPORT int unqlite_util_release_mmaped_file(void *pMap,unqlite_int64 iFileSize);
UNQLITE_APIEXPORT int unqlite_util_random_string(unqlite *pDb,char *zBuf,unsigned int buf_size);
UNQLITE_APIEXPORT unsigned int unqlite_util_random_num(unqlite *pDb);

/* In-process extending interfaces */
UNQLITE_APIEXPORT int unqlite_create_function(unqlite_vm *pVm,const char *zName,int (*xFunc)(unqlite_context *,int,unqlite_value **),void *pUserData);
UNQLITE_APIEXPORT int unqlite_delete_function(unqlite_vm *pVm, const char *zName);
UNQLITE_APIEXPORT int unqlite_create_constant(unqlite_vm *pVm,const char *zName,void (*xExpand)(unqlite_value *, void *),void *pUserData);
UNQLITE_APIEXPORT int unqlite_delete_constant(unqlite_vm *pVm, const char *zName);

/* On Demand Object allocation interfaces */
UNQLITE_APIEXPORT unqlite_value * unqlite_vm_new_scalar(unqlite_vm *pVm);
UNQLITE_APIEXPORT unqlite_value * unqlite_vm_new_array(unqlite_vm *pVm);
UNQLITE_APIEXPORT int unqlite_vm_release_value(unqlite_vm *pVm,unqlite_value *pValue);
UNQLITE_APIEXPORT unqlite_value * unqlite_context_new_scalar(unqlite_context *pCtx);
UNQLITE_APIEXPORT unqlite_value * unqlite_context_new_array(unqlite_context *pCtx);
UNQLITE_APIEXPORT void unqlite_context_release_value(unqlite_context *pCtx,unqlite_value *pValue);

/* Dynamically Typed Value Object Management Interfaces */
UNQLITE_APIEXPORT int unqlite_value_int(unqlite_value *pVal, int iValue);
UNQLITE_APIEXPORT int unqlite_value_int64(unqlite_value *pVal, unqlite_int64 iValue);
UNQLITE_APIEXPORT int unqlite_value_bool(unqlite_value *pVal, int iBool);
UNQLITE_APIEXPORT int unqlite_value_null(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_double(unqlite_value *pVal, double Value);
UNQLITE_APIEXPORT int unqlite_value_string(unqlite_value *pVal, const char *zString, int nLen);
UNQLITE_APIEXPORT int unqlite_value_string_format(unqlite_value *pVal, const char *zFormat,...);
UNQLITE_APIEXPORT int unqlite_value_reset_string_cursor(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_resource(unqlite_value *pVal, void *pUserData);
UNQLITE_APIEXPORT int unqlite_value_release(unqlite_value *pVal);

/* Foreign Function Parameter Values */
UNQLITE_APIEXPORT int unqlite_value_to_int(unqlite_value *pValue);
UNQLITE_APIEXPORT int unqlite_value_to_bool(unqlite_value *pValue);
UNQLITE_APIEXPORT unqlite_int64 unqlite_value_to_int64(unqlite_value *pValue);
UNQLITE_APIEXPORT double unqlite_value_to_double(unqlite_value *pValue);
UNQLITE_APIEXPORT const char * unqlite_value_to_string(unqlite_value *pValue, int *pLen);
UNQLITE_APIEXPORT void * unqlite_value_to_resource(unqlite_value *pValue);
UNQLITE_APIEXPORT int unqlite_value_compare(unqlite_value *pLeft, unqlite_value *pRight, int bStrict);

/* Setting The Result Of A Foreign Function */
UNQLITE_APIEXPORT int unqlite_result_int(unqlite_context *pCtx, int iValue);
UNQLITE_APIEXPORT int unqlite_result_int64(unqlite_context *pCtx, unqlite_int64 iValue);
UNQLITE_APIEXPORT int unqlite_result_bool(unqlite_context *pCtx, int iBool);
UNQLITE_APIEXPORT int unqlite_result_double(unqlite_context *pCtx, double Value);
UNQLITE_APIEXPORT int unqlite_result_null(unqlite_context *pCtx);
UNQLITE_APIEXPORT int unqlite_result_string(unqlite_context *pCtx, const char *zString, int nLen);
UNQLITE_APIEXPORT int unqlite_result_string_format(unqlite_context *pCtx, const char *zFormat, ...);
UNQLITE_APIEXPORT int unqlite_result_value(unqlite_context *pCtx, unqlite_value *pValue);
UNQLITE_APIEXPORT int unqlite_result_resource(unqlite_context *pCtx, void *pUserData);

/* Dynamically Typed Value Object Query Interfaces */
UNQLITE_APIEXPORT int unqlite_value_is_int(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_float(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_bool(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_string(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_null(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_numeric(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_callable(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_scalar(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_json_array(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_json_object(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_resource(unqlite_value *pVal);
UNQLITE_APIEXPORT int unqlite_value_is_empty(unqlite_value *pVal);

/* JSON Array/Object Management Interfaces */
UNQLITE_APIEXPORT unqlite_value * unqlite_array_fetch(unqlite_value *pArray, const char *zKey, int nByte);
UNQLITE_APIEXPORT int unqlite_array_walk(unqlite_value *pArray, int (*xWalk)(unqlite_value *, unqlite_value *, void *), void *pUserData);
UNQLITE_APIEXPORT int unqlite_array_add_elem(unqlite_value *pArray, unqlite_value *pKey, unqlite_value *pValue);
UNQLITE_APIEXPORT int unqlite_array_add_strkey_elem(unqlite_value *pArray, const char *zKey, unqlite_value *pValue);
UNQLITE_APIEXPORT int unqlite_array_count(unqlite_value *pArray);

/* Call Context Handling Interfaces */
UNQLITE_APIEXPORT int unqlite_context_output(unqlite_context *pCtx, const char *zString, int nLen);
UNQLITE_APIEXPORT int unqlite_context_output_format(unqlite_context *pCtx,const char *zFormat, ...);
UNQLITE_APIEXPORT int unqlite_context_throw_error(unqlite_context *pCtx, int iErr, const char *zErr);
UNQLITE_APIEXPORT int unqlite_context_throw_error_format(unqlite_context *pCtx, int iErr, const char *zFormat, ...);
UNQLITE_APIEXPORT unsigned int unqlite_context_random_num(unqlite_context *pCtx);
UNQLITE_APIEXPORT int unqlite_context_random_string(unqlite_context *pCtx, char *zBuf, int nBuflen);
UNQLITE_APIEXPORT void * unqlite_context_user_data(unqlite_context *pCtx);
UNQLITE_APIEXPORT int unqlite_context_push_aux_data(unqlite_context *pCtx, void *pUserData);
UNQLITE_APIEXPORT void * unqlite_context_peek_aux_data(unqlite_context *pCtx);
UNQLITE_APIEXPORT unsigned int unqlite_context_result_buf_length(unqlite_context *pCtx);
UNQLITE_APIEXPORT const char * unqlite_function_name(unqlite_context *pCtx);

/* Call Context Memory Management Interfaces */
UNQLITE_APIEXPORT void * unqlite_context_alloc_chunk(unqlite_context *pCtx,unsigned int nByte,int ZeroChunk,int AutoRelease);
UNQLITE_APIEXPORT void * unqlite_context_realloc_chunk(unqlite_context *pCtx,void *pChunk,unsigned int nByte);
UNQLITE_APIEXPORT void unqlite_context_free_chunk(unqlite_context *pCtx,void *pChunk);

/* Global Library Management Interfaces */
UNQLITE_APIEXPORT int unqlite_lib_config(int nConfigOp,...);
UNQLITE_APIEXPORT int unqlite_lib_init(void);
UNQLITE_APIEXPORT int unqlite_lib_shutdown(void);
UNQLITE_APIEXPORT int unqlite_lib_is_threadsafe(void);
UNQLITE_APIEXPORT const char * unqlite_lib_version(void);
UNQLITE_APIEXPORT const char * unqlite_lib_signature(void);
UNQLITE_APIEXPORT const char * unqlite_lib_ident(void);
UNQLITE_APIEXPORT const char * unqlite_lib_copyright(void);
#ifdef __cplusplus
 }
#endif
#endif /* _UNQLITE_H_ */
