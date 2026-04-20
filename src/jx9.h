/* 本文件由自动生成。请勿修改（编译时指令除外）！*/
#ifndef _JX9H_
#define _JX9H_
/*
 * Symisc Jx9: 一个基于 JSON 的高效嵌入式脚本引擎。
 * 版权所有 (C) 2012-2013, Symisc Systems http://jx9.symisc.net/
 * 版本 1.7.2
 * 有关许可协议、再分发和免责声明的详细信息，请联系 Symisc Systems：
 *       legal@symisc.net
 *       licensing@symisc.net
 *       contact@symisc.net
 * 或访问：
 *      http://jx9.symisc.net/
 */
/*
 * 版权所有 (C) 2012, 2013 Symisc Systems。保留所有权利。
 *
 * 在满足以下条件的前提下，允许以源代码和二进制形式重新分发和使用：
 * 1. 源代码的分发必须保留上述版权声明、本条件列表和以下免责声明。
 * 2. 二进制形式的分发必须在提供的文档和/或其他材料中重现上述版权
 *    声明、本条件列表和以下免责声明。
 * 3. 以任何形式分发必须随附获取 JX9 引擎及相关软件完整源代码的信息。
 *    源代码必须包含在分发中或可获取，且获取费用不超过分发成本加上
 *    象征性费用，且必须在合理条件下可自由分发。对于可执行文件，完整
 *    源代码指其包含的所有模块的源代码，不包括与可执行文件运行所在的
 *    操作系统主要组件一起提供的典型模块的源代码。
 *
 * 本软件按"原样"提供，SYMISC SYSTEMS 不做任何明示或暗示的保证，
 * 包括但不限于对适销性、特定用途适用性和非侵权性的保证。在任何情况下，
 * SYMISC SYSTEMS 均不对任何直接、间接、偶然、特殊、惩戒性或后果性
 * 损害（包括但不限于替代商品或服务的采购、使用、数据丢失、利润损失
 * 或业务中断）承担责任，无论何种原因导致，即使已被告知可能发生此类损害。
 */
 /* $SymiscID: jx9.h v2.1 UNIX|WIN32/64 2012-09-15 09:43 stable <chm@symisc.net> $ */
#include "unqlite.h"
/*
 * 编译时引擎版本、签名、Symisc 源码树中的标识和版权声明。
 * 每个宏都有对应的 C 接口函数，提供相同的信息，但与库关联而非头文件。
 * 请参阅 [jx9_lib_version()]、[jx9_lib_signature()]、[jx9_lib_ident()] 和
 * [jx9_lib_copyright()] 了解更多信息。
 */
/*
 * JX9_VERSION 预处理器宏计算为一个字符串字面量，
 * 格式为 "X.Y.Z"，其中 X 为主版本号，Y 为次版本号，Z 为发布版本号。
 */
#define JX9_VERSION "1.7.2"
/*
 * JX9_VERSION_NUMBER 预处理器宏解析为一个整数值 (X*1000000 + Y*1000 + Z)，
 * 其中 X、Y 和 Z 与 [JX9_VERSION] 中使用的数字相同。
 */
#define JX9_VERSION_NUMBER 1007002
/*
 * JX9_SIG 预处理器宏计算为字符串字面量，表示 jx9 引擎的公共签名。
 * 此签名可包含在主机应用程序生成的 Server MIME 头中，如下所示：
 *   Server: YourWebServer/x.x Jx9/x.x.x \r\n
 */
#define JX9_SIG "Jx9/1.7.2"
/*
 * JX9 在 Symisc 源码树中的标识：
 * Symisc Systems 发布的每个软件版本都有唯一的标识符。
 * 此宏保存与 jx9 关联的标识符。
 */
#define JX9_IDENT "jx9:d217a6e8c7f10fb35a8becb2793101fd2036aeb7"
/*
 * 版权声明。
 * 如有关于许可协议的问题，请访问 http://jx9.symisc.net/licensing.html
 * 或联系 Symisc Systems：
 *   legal@symisc.net
 *   licensing@symisc.net
 *   contact@symisc.net
 */
#define JX9_COPYRIGHT "版权所有 (C) Symisc Systems 2012-2013, http://jx9.symisc.net/"

/* 公共对象的前向声明 */
typedef struct jx9_io_stream jx9_io_stream;
typedef struct jx9_context jx9_context;
typedef struct jx9_value jx9_value;
typedef struct jx9_vfs jx9_vfs;
typedef struct jx9_vm jx9_vm;
typedef struct jx9 jx9;

#include "unqlite.h"

#if !defined( UNQLITE_ENABLE_JX9_HASH_FUNC )
#define JX9_DISABLE_HASH_FUNC
#endif /* UNQLITE_ENABLE_JX9_HASH_FUNC */
#ifdef UNQLITE_ENABLE_THREADS
#define JX9_ENABLE_THREADS
#endif /* UNQLITE_ENABLE_THREADS */
/* 标准 JX9 返回值 */
#define JX9_OK      SXRET_OK      /* 成功结果 */
/* 错误码开始 */
#define JX9_NOMEM   UNQLITE_NOMEM     /* 内存不足 */
#define JX9_ABORT   UNQLITE_ABORT   /* 外部函数请求中止操作/另一线程已释放此实例 */
#define JX9_IO_ERR  UNQLITE_IOERR      /* IO 错误 */
#define JX9_CORRUPT UNQLITE_CORRUPT /* 损坏的指针/未知配置选项 */
#define JX9_LOOKED  UNQLITE_LOCKED  /* 禁止操作 */ 
#define JX9_COMPILE_ERR UNQLITE_COMPILE_ERR /* 编译错误 */
#define JX9_VM_ERR      UNQLITE_VM_ERR      /* 虚拟机错误 */
/* 错误码结束 */
/*
 * 如果为缺乏浮点支持的处理器编译，
 * 则用整数替代浮点数。
 */
#ifdef JX9_OMIT_FLOATING_POINT
typedef sxi64 jx9_real;
#else
typedef double jx9_real;
#endif
typedef sxi64 jx9_int64;
/*
 * 引擎配置命令。
 *
 * 以下常量集是可用于主机应用程序配置 JX9 引擎的配置动词。
 * 这些常量必须作为第二个参数传递给 [jx9_config()] 接口。
 * 每个选项需要可变数量的参数。
 * [jx9_config()] 接口成功时返回 JX9_OK，任何其他返回值表示失败。
 * 有关配置动词及其预期参数的完整讨论，请参阅此页面：
 *      http://jx9.symisc.net/c_api_func.html#jx9_config
 */
#define JX9_CONFIG_ERR_ABORT     1  /* RESERVED FOR FUTURE USE */
#define JX9_CONFIG_ERR_LOG       2  /* TWO ARGUMENTS: const char **pzBuf, int *pLen */
/*
 * 虚拟机配置命令。
 *
 * 以下常量集是可用于主机应用程序配置 JX9 虚拟机的配置动词。
 * 这些常量必须作为第二个参数传递给 [jx9_vm_config()] 接口。
 * 每个选项需要可变数量的参数。
 * [jx9_vm_config()] 接口成功时返回 JX9_OK，任何其他返回值表示失败。
 * 有很多选项，但最重要的是：JX9_VM_CONFIG_OUTPUT 用于安装 VM 输出消费者回调，
 * JX9_VM_CONFIG_HTTP_REQUEST 用于解析和注册 HTTP 请求，
 * JX9_VM_CONFIG_ARGV_ENTRY 用于填充 $argv 数组。
 * 有关配置动词及其预期参数的完整讨论，请参阅此页面：
 *      http://jx9.symisc.net/c_api_func.html#jx9_vm_config
 */
#define JX9_VM_CONFIG_OUTPUT           UNQLITE_VM_CONFIG_OUTPUT  /* TWO ARGUMENTS: int (*xConsumer)(const void *pOut, unsigned int nLen, void *pUserData), void *pUserData */
#define JX9_VM_CONFIG_IMPORT_PATH      UNQLITE_VM_CONFIG_IMPORT_PATH  /* ONE ARGUMENT: const char *zIncludePath */
#define JX9_VM_CONFIG_ERR_REPORT       UNQLITE_VM_CONFIG_ERR_REPORT  /* NO ARGUMENTS: Report all run-time errors in the VM output */
#define JX9_VM_CONFIG_RECURSION_DEPTH  UNQLITE_VM_CONFIG_RECURSION_DEPTH  /* ONE ARGUMENT: int nMaxDepth */
#define JX9_VM_OUTPUT_LENGTH           UNQLITE_VM_OUTPUT_LENGTH  /* ONE ARGUMENT: unsigned int *pLength */
#define JX9_VM_CONFIG_CREATE_VAR       UNQLITE_VM_CONFIG_CREATE_VAR  /* TWO ARGUMENTS: const char *zName, jx9_value *pValue */
#define JX9_VM_CONFIG_HTTP_REQUEST     UNQLITE_VM_CONFIG_HTTP_REQUEST  /* TWO ARGUMENTS: const char *zRawRequest, int nRequestLength */
#define JX9_VM_CONFIG_SERVER_ATTR      UNQLITE_VM_CONFIG_SERVER_ATTR  /* THREE ARGUMENTS: const char *zKey, const char *zValue, int nLen */
#define JX9_VM_CONFIG_ENV_ATTR         UNQLITE_VM_CONFIG_ENV_ATTR  /* THREE ARGUMENTS: const char *zKey, const char *zValue, int nLen */
#define JX9_VM_CONFIG_EXEC_VALUE       UNQLITE_VM_CONFIG_EXEC_VALUE  /* ONE ARGUMENT: jx9_value **ppValue */
#define JX9_VM_CONFIG_IO_STREAM        UNQLITE_VM_CONFIG_IO_STREAM  /* ONE ARGUMENT: const jx9_io_stream *pStream */
#define JX9_VM_CONFIG_ARGV_ENTRY       UNQLITE_VM_CONFIG_ARGV_ENTRY  /* ONE ARGUMENT: const char *zValue */
#define JX9_VM_CONFIG_EXTRACT_OUTPUT   UNQLITE_VM_CONFIG_EXTRACT_OUTPUT  /* TWO ARGUMENTS: const void **ppOut, unsigned int *pOutputLen */
/*
 * 全局库配置命令。
 *
 * 以下常量集是可用于主机应用程序配置整个库的配置动词。
 * 这些常量必须作为第一个参数传递给 [jx9_lib_config()] 接口。
 * 每个选项需要可变数量的参数。
 * [jx9_lib_config()] 接口成功时返回 JX9_OK，任何其他返回值表示失败。
 * 注意：
 * 默认配置推荐用于大多数应用程序，因此通常不需要调用 [jx9_lib_config()]。
 * 提供此接口是为了支持有特殊需求的少数应用程序。
 * [jx9_lib_config()] 接口不是线程安全的。应用程序必须确保在 [jx9_lib_config()]
 * 运行期间，其他线程不会调用任何 [jx9_*()] 接口。此外，[jx9_lib_config()] 只能
 * 在使用 [jx9_lib_init()] 或 [jx9_init()] 进行库初始化之前或使用 [jx9_lib_shutdown()]
 * 关闭之后调用。如果在 [jx9_lib_init()] 或 [jx9_init()] 之后且在 [jx9_lib_shutdown()] 
 * 之前调用 [jx9_lib_config()]，则将返回 jx9LOCKED。
 * 有关配置动词及其预期参数的完整讨论，请参阅此页面：
 *      http://jx9.symisc.net/c_api_func.html#Global_Library_Management_Interfaces
 */
#define JX9_LIB_CONFIG_USER_MALLOC            1 /* ONE ARGUMENT: const SyMemMethods *pMemMethods */ 
#define JX9_LIB_CONFIG_MEM_ERR_CALLBACK       2 /* TWO ARGUMENTS: int (*xMemError)(void *), void *pUserData */
#define JX9_LIB_CONFIG_USER_MUTEX             3 /* ONE ARGUMENT: const SyMutexMethods *pMutexMethods */ 
#define JX9_LIB_CONFIG_THREAD_LEVEL_SINGLE    4 /* NO ARGUMENTS */ 
#define JX9_LIB_CONFIG_THREAD_LEVEL_MULTI     5 /* NO ARGUMENTS */ 
#define JX9_LIB_CONFIG_VFS                    6 /* ONE ARGUMENT: const jx9_vfs *pVfs */
/*
 * 调用上下文 - 错误消息严重级别。
 */
#define JX9_CTX_ERR      UNQLITE_CTX_ERR      /* 调用上下文错误，如参数数量无效、类型无效等。*/
#define JX9_CTX_WARNING  UNQLITE_CTX_WARNING  /* 调用上下文警告 */
#define JX9_CTX_NOTICE   UNQLITE_CTX_NOTICE   /* 调用上下文通知 */
/* 当前 VFS 结构版本*/
#define JX9_VFS_VERSION 2 
/* 
 * JX9 虚拟文件系统 (VFS)。
 *
 * jx9_vfs 对象的实例定义了 JX9 核心与底层操作系统之间的接口。
 * 对象名中的 "vfs" 代表"虚拟文件系统"。vfs 用于实现 JX9 系统函数，
 * 如 mkdir()、chdir()、stat()、get_user_name() 等。
 * iVersion 字段的值初始为 2，但在未来版本的 JX9 中可能更大。
 * 当 iVersion 值增加时，可能会向此对象追加额外字段。
 * 只能在 JX9 核心中注册单个 vfs。vfs 注册使用 jx9_lib_config() 接口，
 * 配置动词设置为 JX9_LIB_CONFIG_VFS。
 * 注意，Windows 和 UNIX（Linux、FreeBSD、Solaris、Mac OS X 等）用户不必担心
 * vfs 的注册和安装，因为 JX9 为这些平台提供了内置 vfs，实现了以下定义的大多数方法。
 * 在非主流系统（即 Windows 和 UNIX 系统以外的系统）上运行的主机应用程序必须
 * 注册自己的 vfs 才能使用和调用 JX9 系统函数。
 * 还要注意，jx9_compile_file() 接口依赖于底层 vfs 的 xMmap() 方法，
 * 因此此方法必须可用（使用内置 VFS 时始终如此）才能使用此接口。
 * 希望实现自己的 vfs 的开发者请联系 symisc systems 获取 JX9 VFS C/C++ 规范手册。
 */
struct jx9_vfs
{
	const char *zName;  /* Underlying VFS name [i.e: FreeBSD/Linux/Windows...] */
	int iVersion;       /* Current VFS structure version [default 2] */
	/* Directory functions */
	int (*xChdir)(const char *);                     /* Change directory */
	int (*xChroot)(const char *);                    /* Change the root directory */
	int (*xGetcwd)(jx9_context *);                   /* Get the current working directory */
	int (*xMkdir)(const char *, int, int);             /* Make directory */
	int (*xRmdir)(const char *);                     /* Remove directory */
	int (*xIsdir)(const char *);                     /* Tells whether the filename is a directory */
	int (*xRename)(const char *, const char *);       /* Renames a file or directory */
	int (*xRealpath)(const char *, jx9_context *);    /* Return canonicalized absolute pathname*/
	/* Systems functions */
	int (*xSleep)(unsigned int);                     /* Delay execution in microseconds */
	int (*xUnlink)(const char *);                    /* Deletes a file */
	int (*xFileExists)(const char *);                /* Checks whether a file or directory exists */
	int (*xChmod)(const char *, int);                 /* Changes file mode */
	int (*xChown)(const char *, const char *);        /* Changes file owner */
	int (*xChgrp)(const char *, const char *);        /* Changes file group */
	jx9_int64 (*xFreeSpace)(const char *);           /* Available space on filesystem or disk partition */
	jx9_int64 (*xTotalSpace)(const char *);          /* Total space on filesystem or disk partition */
	jx9_int64 (*xFileSize)(const char *);            /* Gets file size */
	jx9_int64 (*xFileAtime)(const char *);           /* Gets last access time of file */
	jx9_int64 (*xFileMtime)(const char *);           /* Gets file modification time */
	jx9_int64 (*xFileCtime)(const char *);           /* Gets inode change time of file */
	int (*xStat)(const char *, jx9_value *, jx9_value *);   /* Gives information about a file */
	int (*xlStat)(const char *, jx9_value *, jx9_value *);  /* Gives information about a file */
	int (*xIsfile)(const char *);                    /* Tells whether the filename is a regular file */
	int (*xIslink)(const char *);                    /* Tells whether the filename is a symbolic link */
	int (*xReadable)(const char *);                  /* Tells whether a file exists and is readable */
	int (*xWritable)(const char *);                  /* Tells whether the filename is writable */
	int (*xExecutable)(const char *);                /* Tells whether the filename is executable */
	int (*xFiletype)(const char *, jx9_context *);    /* Gets file type [i.e: fifo, dir, file..] */
	int (*xGetenv)(const char *, jx9_context *);      /* Gets the value of an environment variable */
	int (*xSetenv)(const char *, const char *);       /* Sets the value of an environment variable */
	int (*xTouch)(const char *, jx9_int64, jx9_int64); /* Sets access and modification time of file */
	int (*xMmap)(const char *, void **, jx9_int64 *);  /* Read-only memory map of the whole file */
	void (*xUnmap)(void *, jx9_int64);                /* Unmap a memory view */
	int (*xLink)(const char *, const char *, int);     /* Create hard or symbolic link */
	int (*xUmask)(int);                              /* Change the current umask */
	void (*xTempDir)(jx9_context *);                 /* Get path of the temporary directory */
	unsigned int (*xProcessId)(void);                /* Get running process ID */
	int (*xUid)(void);                               /* user ID of the process */
	int (*xGid)(void);                               /* group ID of the process */
	void (*xUsername)(jx9_context *);                /* Running username */
	int (*xExec)(const char *, jx9_context *);        /* Execute an external program */
};
/* 当前 JX9 IO 流结构版本。*/
#define JX9_IO_STREAM_VERSION 1 
/* 
 * 可以传递给底层 IO 流设备的 xOpen() 例程的可能打开模式标志。
 * 有关其他信息，请参阅 JX9 IO 流 C/C++ 规范手册 (http://jx9.symisc.net/io_stream_spec.html)。
 */
#define JX9_IO_OPEN_RDONLY   0x001  /* Read-only open */
#define JX9_IO_OPEN_WRONLY   0x002  /* Write-only open */
#define JX9_IO_OPEN_RDWR     0x004  /* Read-write open. */
#define JX9_IO_OPEN_CREATE   0x008  /* If the file does not exist it will be created */
#define JX9_IO_OPEN_TRUNC    0x010  /* Truncate the file to zero length */
#define JX9_IO_OPEN_APPEND   0x020  /* Append mode.The file offset is positioned at the end of the file */
#define JX9_IO_OPEN_EXCL     0x040  /* Ensure that this call creates the file, the file must not exist before */
#define JX9_IO_OPEN_BINARY   0x080  /* Simple hint: Data is binary */
#define JX9_IO_OPEN_TEMP     0x100  /* Simple hint: Temporary file */
#define JX9_IO_OPEN_TEXT     0x200  /* Simple hint: Data is textual */
/*
 * JX9 IO 流设备。
 *
 * jx9_io_stream 对象的实例定义了 JX9 核心与底层流设备之间的接口。
 * 流是一种智能机制，用于概括文件、网络、数据压缩和其他 IO 操作，
 * 这些操作使用抽象的统一接口共享一组通用函数。
 * 流设备是告诉流如何处理特定协议/编码的附加代码。例如，
 * http 设备知道如何将 URL 转换为远程服务器上文件的 HTTP/1.1 请求。
 * JX9 附带两个内置 IO 流设备：
 * 执行高效磁盘 IO 的 file:// 流和允许访问各种 I/O 流的特殊 jx9:// 流
 * （有关此流的更多信息，请参阅 JX9 官方文档）。
 * 流的引用格式为：scheme://target
 * scheme(字符串) - 要使用的包装器名称。示例包括：file、http、https、ftp、
 * ftps、compress.zlib、compress.bz2 和 jx9。如果未指定包装器，
 * 则使用函数默认值（通常为 file://）。
 * target - 取决于使用的设备。对于文件系统相关的流，这通常是所需文件的路径和文件名。
 * 对于网络相关的流，这通常是主机名，通常还附有路径。
 * IO 流设备使用调用 jx9_vm_config() 并将配置动词设置为 JX9_VM_CONFIG_IO_STREAM 来注册。
 * 目前 JX9 开发团队正在努力实现 http:// 和 ftp:// IO 流协议。这些设备将在
 * JX9 引擎的下一个主要版本中提供。
 * 希望实现自己的 IO 流设备的开发者必须理解并遵循
 * JX9 IO 流 C/C++ 规范手册 (http://jx9.symisc.net/io_stream_spec.html)。
 */
struct jx9_io_stream
{
	const char *zName;                                     /* Underlying stream name [i.e: file/http/zip/jx9, ..] */
	int iVersion;                                          /* IO stream structure version [default 1]*/
	int  (*xOpen)(const char *, int, jx9_value *, void **);   /* Open handle*/
	int  (*xOpenDir)(const char *, jx9_value *, void **);    /* Open directory handle */
	void (*xClose)(void *);                                /* Close file handle */
	void (*xCloseDir)(void *);                             /* Close directory handle */
	jx9_int64 (*xRead)(void *, void *, jx9_int64);           /* Read from the open stream */         
	int (*xReadDir)(void *, jx9_context *);                 /* Read entry from directory handle */
	jx9_int64 (*xWrite)(void *, const void *, jx9_int64);    /* Write to the open stream */
	int (*xSeek)(void *, jx9_int64, int);                    /* Seek on the open stream */
	int (*xLock)(void *, int);                              /* Lock/Unlock the open stream */
	void (*xRewindDir)(void *);                            /* Rewind directory handle */
	jx9_int64 (*xTell)(void *);                            /* Current position of the stream  read/write pointer */
	int (*xTrunc)(void *, jx9_int64);                       /* Truncates the open stream to a given length */
	int (*xSync)(void *);                                  /* Flush open stream data */
	int (*xStat)(void *, jx9_value *, jx9_value *);          /* Stat an open stream handle */
};
/* 
 * C-API-REF：请参阅官方文档以了解接口用途和预期参数。
 */
/* 引擎处理接口 */
JX9_PRIVATE int jx9_init(jx9 **ppEngine);
/*JX9_PRIVATE int jx9_config(jx9 *pEngine, int nConfigOp, ...);*/
JX9_PRIVATE int jx9_release(jx9 *pEngine);
/* 编译接口 */
JX9_PRIVATE int jx9_compile(jx9 *pEngine, const char *zSource, int nLen, jx9_vm **ppOutVm);
JX9_PRIVATE int jx9_compile_file(jx9 *pEngine, const char *zFilePath, jx9_vm **ppOutVm);
/* 虚拟机处理接口 */
JX9_PRIVATE int jx9_vm_config(jx9_vm *pVm, int iConfigOp, ...);
/*JX9_PRIVATE int jx9_vm_exec(jx9_vm *pVm, int *pExitStatus);*/
/*JX9_PRIVATE jx9_value * jx9_vm_extract_variable(jx9_vm *pVm,const char *zVarname);*/
/*JX9_PRIVATE int jx9_vm_reset(jx9_vm *pVm);*/
JX9_PRIVATE int jx9_vm_release(jx9_vm *pVm);
/*JX9_PRIVATE int jx9_vm_dump_v2(jx9_vm *pVm, int (*xConsumer)(const void *, unsigned int, void *), void *pUserData);*/
/* 进程内扩展接口 */
JX9_PRIVATE int jx9_create_function(jx9_vm *pVm, const char *zName, int (*xFunc)(jx9_context *, int, jx9_value **), void *pUserData);
/*JX9_PRIVATE int jx9_delete_function(jx9_vm *pVm, const char *zName);*/
JX9_PRIVATE int jx9_create_constant(jx9_vm *pVm, const char *zName, void (*xExpand)(jx9_value *, void *), void *pUserData);
/*JX9_PRIVATE int jx9_delete_constant(jx9_vm *pVm, const char *zName);*/
/* 外部函数参数值 */
JX9_PRIVATE int jx9_value_to_int(jx9_value *pValue);
JX9_PRIVATE int jx9_value_to_bool(jx9_value *pValue);
JX9_PRIVATE jx9_int64 jx9_value_to_int64(jx9_value *pValue);
JX9_PRIVATE double jx9_value_to_double(jx9_value *pValue);
JX9_PRIVATE const char * jx9_value_to_string(jx9_value *pValue, int *pLen);
JX9_PRIVATE void * jx9_value_to_resource(jx9_value *pValue);
JX9_PRIVATE int jx9_value_compare(jx9_value *pLeft, jx9_value *pRight, int bStrict);
/* 设置外部函数的结果 */
JX9_PRIVATE int jx9_result_int(jx9_context *pCtx, int iValue);
JX9_PRIVATE int jx9_result_int64(jx9_context *pCtx, jx9_int64 iValue);
JX9_PRIVATE int jx9_result_bool(jx9_context *pCtx, int iBool);
JX9_PRIVATE int jx9_result_double(jx9_context *pCtx, double Value);
JX9_PRIVATE int jx9_result_null(jx9_context *pCtx);
JX9_PRIVATE int jx9_result_string(jx9_context *pCtx, const char *zString, int nLen);
JX9_PRIVATE int jx9_result_string_format(jx9_context *pCtx, const char *zFormat, ...);
JX9_PRIVATE int jx9_result_value(jx9_context *pCtx, jx9_value *pValue);
JX9_PRIVATE int jx9_result_resource(jx9_context *pCtx, void *pUserData);
/* 调用上下文处理接口 */
JX9_PRIVATE int jx9_context_output(jx9_context *pCtx, const char *zString, int nLen);
/*JX9_PRIVATE int jx9_context_output_format(jx9_context *pCtx, const char *zFormat, ...);*/
JX9_PRIVATE int jx9_context_throw_error(jx9_context *pCtx, int iErr, const char *zErr);
JX9_PRIVATE int jx9_context_throw_error_format(jx9_context *pCtx, int iErr, const char *zFormat, ...);
JX9_PRIVATE unsigned int jx9_context_random_num(jx9_context *pCtx);
JX9_PRIVATE int jx9_context_random_string(jx9_context *pCtx, char *zBuf, int nBuflen);
JX9_PRIVATE void * jx9_context_user_data(jx9_context *pCtx);
JX9_PRIVATE int    jx9_context_push_aux_data(jx9_context *pCtx, void *pUserData);
JX9_PRIVATE void * jx9_context_peek_aux_data(jx9_context *pCtx);
JX9_PRIVATE void * jx9_context_pop_aux_data(jx9_context *pCtx);
JX9_PRIVATE unsigned int jx9_context_result_buf_length(jx9_context *pCtx);
JX9_PRIVATE const char * jx9_function_name(jx9_context *pCtx);
/* 调用上下文内存管理接口 */
JX9_PRIVATE void * jx9_context_alloc_chunk(jx9_context *pCtx, unsigned int nByte, int ZeroChunk, int AutoRelease);
JX9_PRIVATE void * jx9_context_realloc_chunk(jx9_context *pCtx, void *pChunk, unsigned int nByte);
JX9_PRIVATE void jx9_context_free_chunk(jx9_context *pCtx, void *pChunk);
/* 按需动态类型值对象分配接口 */
JX9_PRIVATE jx9_value * jx9_new_scalar(jx9_vm *pVm);
JX9_PRIVATE jx9_value * jx9_new_array(jx9_vm *pVm);
JX9_PRIVATE int jx9_release_value(jx9_vm *pVm, jx9_value *pValue);
JX9_PRIVATE jx9_value * jx9_context_new_scalar(jx9_context *pCtx);
JX9_PRIVATE jx9_value * jx9_context_new_array(jx9_context *pCtx);
JX9_PRIVATE void jx9_context_release_value(jx9_context *pCtx, jx9_value *pValue);
/* 动态类型值对象管理接口 */
JX9_PRIVATE int jx9_value_int(jx9_value *pVal, int iValue);
JX9_PRIVATE int jx9_value_int64(jx9_value *pVal, jx9_int64 iValue);
JX9_PRIVATE int jx9_value_bool(jx9_value *pVal, int iBool);
JX9_PRIVATE int jx9_value_null(jx9_value *pVal);
JX9_PRIVATE int jx9_value_double(jx9_value *pVal, double Value);
JX9_PRIVATE int jx9_value_string(jx9_value *pVal, const char *zString, int nLen);
JX9_PRIVATE int jx9_value_string_format(jx9_value *pVal, const char *zFormat, ...);
JX9_PRIVATE int jx9_value_reset_string_cursor(jx9_value *pVal);
JX9_PRIVATE int jx9_value_resource(jx9_value *pVal, void *pUserData);
JX9_PRIVATE int jx9_value_release(jx9_value *pVal);
/* JSON 数组/对象管理接口 */
JX9_PRIVATE jx9_value * jx9_array_fetch(jx9_value *pArray, const char *zKey, int nByte);
JX9_PRIVATE int jx9_array_walk(jx9_value *pArray, int (*xWalk)(jx9_value *, jx9_value *, void *), void *pUserData);
JX9_PRIVATE int jx9_array_add_elem(jx9_value *pArray, jx9_value *pKey, jx9_value *pValue);
JX9_PRIVATE int jx9_array_add_strkey_elem(jx9_value *pArray, const char *zKey, jx9_value *pValue);
JX9_PRIVATE unsigned int jx9_array_count(jx9_value *pArray);
/* 动态类型值对象查询接口 */
JX9_PRIVATE int jx9_value_is_int(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_float(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_bool(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_string(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_null(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_numeric(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_callable(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_scalar(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_json_array(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_json_object(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_resource(jx9_value *pVal);
JX9_PRIVATE int jx9_value_is_empty(jx9_value *pVal);
/* 全局库管理接口 */
/*JX9_PRIVATE int jx9_lib_init(void);*/
JX9_PRIVATE int jx9_lib_config(int nConfigOp, ...);
JX9_PRIVATE int jx9_lib_shutdown(void);
/*JX9_PRIVATE int jx9_lib_is_threadsafe(void);*/
/*JX9_PRIVATE const char * jx9_lib_version(void);*/
JX9_PRIVATE const char * jx9_lib_signature(void);
/*JX9_PRIVATE const char * jx9_lib_ident(void);*/
/*JX9_PRIVATE const char * jx9_lib_copyright(void);*/

#endif /* _JX9H_ */
