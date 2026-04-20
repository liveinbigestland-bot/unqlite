/*
 * 将此文件与 UnQLite 数据库引擎源代码一起编译以生成可执行文件。例如：
 *  gcc -W -Wall -O6 unqlite_csr_intro.c unqlite.c -o unqlite_csr
*/
/*
 * 这个简单的程序是对如何嵌入和开始使用 UnQLite 进行实验的快速介绍，
 * 而无需进行大量繁琐的阅读和配置。
 *
 * 使用数据库游标：
 *
 * 游标提供了一种机制，通过它可以遍历数据库中的记录。
 * 使用游标，您可以查找、获取、移动和删除数据库记录。
 *
 * 在使用游标之前，您必须首先使用 unqlite_kv_cursor_init() 分配一个新的游标句柄。
 * 这通常是应用程序进行的第一个 UnQLite 游标 API 调用，是使用游标的先决条件。
 * 使用完毕后，您必须调用 unqlite_kv_cursor_release() 来释放游标分配的任何资源，
 * 从而避免内存泄漏。
 *
 * 要遍历数据库记录（从第一条到最后一条），只需调用 unqlite_kv_cursor_first_entry()，
 * 然后反复调用 unqlite_kv_cursor_next_entry()，直到返回 UNQLITE_OK 以外的值（见下文示例）。
 * 请注意，您可以调用 unqlite_kv_cursor_valid_entry() 来检查游标是否指向有效记录
 *（有效时返回 1，否则返回 0）。
 *
 * 您还可以使用游标搜索记录并从那里开始迭代过程。
 * 为此，只需使用目标记录键和查找方向（最后一个参数）调用 unqlite_kv_cursor_seek()...
 *
 * 有关 UnQLite 游标接口的介绍，请参阅：
 *        http://unqlite.org/c_api/unqlite_kv_cursor.html
 * 有关 UnQLite C/C++ 接口的介绍，请参阅：
 *        http://unqlite.org/api_intro.html
 * 有关完整的 C/C++ API 参考指南，请参阅：
 *        http://unqlite.org/c_api.html
 * 5 分钟或更短时间内了解 UnQLite：
 *        http://unqlite.org/intro.html
 * UnQLite 数据库引擎的架构：
 *        http://unqlite.org/arch.html
 * 有关 Jx9（为 UnQLite 的文档存储接口提供动力的脚本语言）的介绍，请参阅：
 *        http://unqlite.org/jx9.html
 */
/* $SymiscID: unqlite_csr_intro.c v1.0 FreeBSD 2013-05-17 00:02 stable <chm@symisc.net> $ */
/* 
 * 确保您有 UnQLite 的最新版本：
 *  http://unqlite.org/downloads.html
 */
#include <stdio.h>  /* puts() */
#include <stdlib.h> /* exit() */
/* Make sure this header file is available.*/
#include "unqlite.h"
/*
 * Banner.
 */
static const char zBanner[] = {
	"============================================================\n"
	"UnQLite Cursors Intro                                       \n"
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
/* Forward declaration: Data consumer callback */
static int DataConsumerCallback(const void *pData,unsigned int nDatalen,void *pUserData /* Unused */);
/*
 * Maximum random records to be inserted in our database.
 */
#define MAX_RECORDS 20

int main(int argc,char *argv[])
{
	unqlite *pDb;               /* Database handle */
	unqlite_kv_cursor *pCur;    /* Cursor handle */
	char zKey[14];              /* Random generated key */
	char zData[32];             /* Dummy data */
	int i,rc;

	puts(zBanner);

	/* Open our database */
	rc = unqlite_open(&pDb,argc > 1 ? argv[1] /* On-disk DB */ : ":mem:" /* In-mem DB */,UNQLITE_OPEN_CREATE);
	if( rc != UNQLITE_OK ){
		Fatal(0,"Out of memory");
	}
	
	printf("Starting insertions of %d random records...\n",MAX_RECORDS);
	
	/* Start the random insertions */
	for( i = 0 ; i < MAX_RECORDS; ++i ){
		
		/* Genearte the random key first */
		unqlite_util_random_string(pDb,zKey,sizeof(zKey));

		/* Perform the insertion */
		rc = unqlite_kv_store(pDb,zKey,sizeof(zKey),zData,sizeof(zData));
		if( rc != UNQLITE_OK ){
			/* Something goes wrong */
			break;
		}
	}
	if( rc != UNQLITE_OK ){
		/* Something goes wrong, extract the database error log and exit */
		Fatal(pDb,0);
	}
	puts("Done...Starting the iteration process");

	/* Allocate a new cursor instance */
	rc = unqlite_kv_cursor_init(pDb,&pCur);
	if( rc != UNQLITE_OK ){
		Fatal(0,"Out of memory");
	}
	/* Point to the first record */
	unqlite_kv_cursor_first_entry(pCur);
	/* To point to the last record instead of the first, simply call [unqlite_kv_cursor_last_entry()] as follows */
	
	/* unqlite_kv_cursor_last_entry(pCur); */
		
	/* Iterate over the entries */
	while( unqlite_kv_cursor_valid_entry(pCur) ){
		int nKeyLen;
		/* unqlite_int64 nDataLen; */
		
		/* Consume the key */
		unqlite_kv_cursor_key(pCur,0,&nKeyLen); /* Extract key length */
		printf("\nKey ==> %u\n\t",nKeyLen);
		unqlite_kv_cursor_key_callback(pCur,DataConsumerCallback,0);
			
		/* Consume the data */
		/*
		unqlite_kv_cursor_data(pCur,0,&nDataLen);
		printf("\nData ==> %lld\n\t",nDataLen);
		unqlite_kv_cursor_data_callback(pCur,DataConsumerCallback,0);
		*/


		/* Point to the next entry */
		unqlite_kv_cursor_next_entry(pCur);

		/*unqlite_kv_cursor_prev_entry(pCur); //If [unqlite_kv_cursor_last_entry(pCur)] instead of [unqlite_kv_cursor_first_entry(pCur)] */
	}
	/* Finally, Release our cursor */
	unqlite_kv_cursor_release(pDb,pCur);
	
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
 * Data consumer callback [unqlite_kv_fetch_callback(), unqlite_kv_cursor_key_callback(), etc.).
 * 
 * Rather than allocating a static or dynamic buffer (Inefficient scenario for large data).
 * The caller simply need to supply a consumer callback which is responsible of consuming
 * the record data perhaps redirecting it (i.e. Record data) to its standard output (STDOUT),
 * disk file, connected peer and so forth.
 * Depending on how large the extracted data, the callback may be invoked more than once.
 */
static int DataConsumerCallback(const void *pData,unsigned int nDatalen,void *pUserData /* Unused */)
{
#ifdef __WINNT__
	BOOL rc;
	rc = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),pData,(DWORD)nDatalen,0,0);
	if( !rc ){
		/* Abort processing */
		return UNQLITE_ABORT;
	}
#else
	ssize_t nWr;
	nWr = write(STDOUT_FILENO,pData,nDatalen);
	if( nWr < 0 ){
		/* Abort processing */
		return UNQLITE_ABORT;
	}
#endif /* __WINT__ */
	
	/* All done, data was redirected to STDOUT */
	return UNQLITE_OK;
}
