/*
 * 将此文件与 UnQLite 数据库引擎源代码一起编译以生成可执行文件。例如：
 *  gcc -W -Wall -O6 unqlite_kv_intro.c unqlite.c -o unqlite_kv
*/
/*
 * 这个简单的程序是对如何嵌入和开始使用 UnQLite 进行实验的快速介绍，
 * 而无需进行大量繁琐的阅读和配置。
 *
 * 键/值存储接口简介：
 *
 * UnQLite 是一个标准的键/值存储，类似于 BerkeleyDB、Tokyo Cabinet、LevelDB 等。
 * 但是，具有丰富的功能集，包括对事务（ACID）、并发读取器等的支持。
 * 在 KV 存储下，键和值都被视为简单的字节数组，因此内容可以是
 * ASCII 字符串、二进制 blob 甚至磁盘文件。
 * KV 存储层通过一组接口向主机应用程序提供，包括：
 * unqlite_kv_store()、unqlite_kv_append()、unqlite_kv_fetch_callback()、unqlite_kv_append_fmt()、
 * unqlite_kv_delete()、unqlite_kv_fetch() 等。
 *
 * 有关 UnQLite C/C++ 接口的介绍，请参阅：
 *        http://unqlite.org/api_intro.html
 * 有关完整的 C/C++ API 参考指南，请参阅：
 *        http://unqlite.org/c_api.html
 * 5 分钟或更短时间内了解 UnQLite：
 *        http://unqlite.org/intro.html
 * UnQLite 数据库引擎的架构：
 *        http://unqlite.org/arch.html
 * 有关 UnQLite 游标接口的介绍，请参阅：
 *        http://unqlite.org/c_api/unqlite_kv_cursor.html
 * 有关 Jx9（为 UnQLite 的文档存储接口提供动力的脚本语言）的介绍，请参阅：
 *        http://unqlite.org/jx9.html
 */
/* $SymiscID: unqlite_kv_intro.c v1.0 FreeBSD 2013-05-14 10:17 stable <chm@symisc.net> $ */
/* 
 * 确保您有 UnQLite 的最新版本：
 *  http://unqlite.org/downloads.html
 */
#include <stdio.h>  /* puts() */
#include <stdlib.h> /* exit() */
/* 确保此头文件可用。*/
#include "unqlite.h"
/*
 * 横标。
 */
static const char zBanner[] = {
	"============================================================\n"
	"UnQLite Key/Value Store Intro                              \n"
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

int main(int argc,char *argv[])
{
	unqlite *pDb;               /* Database handle */
	unqlite_kv_cursor *pCur;    /* Cursor handle */
	int i,rc;

	puts(zBanner);

	/* Open our database */
	rc = unqlite_open(&pDb,argc > 1 ? argv[1] /* On-disk DB */ : ":mem:" /* In-mem DB */,UNQLITE_OPEN_CREATE);
	if( rc != UNQLITE_OK ){
		Fatal(0,"Out of memory");
	}
	
	/* Store some records */
	rc = unqlite_kv_store(pDb,"test",-1,"Hello World",11); /* test => 'Hello World' */
	if( rc != UNQLITE_OK ){
		/* Insertion fail, extract database error log and exit */
		Fatal(pDb,0);
	}
	/* A small formatted string */
	rc = unqlite_kv_store_fmt(pDb,"date",-1,"dummy date: %d:%d:%d",2013,06,07); /* Dummy date */
	if( rc != UNQLITE_OK ){
		/* Insertion fail, extract database error log and exit */
		Fatal(pDb,0);
	}
	
	/* Switch to the append interface */
	rc = unqlite_kv_append(pDb,"msg",-1,"Hello, ",7); //msg => 'Hello, '
	if( rc == UNQLITE_OK ){
		/* The second chunk */
		rc = unqlite_kv_append(pDb,"msg",-1,"dummy time is: ",17); /* msg => 'Hello, Current time is: '*/
		if( rc == UNQLITE_OK ){
			/* The last formatted chunk */
			rc = unqlite_kv_append_fmt(pDb,"msg",-1,"%d:%d:%d",10,16,53); /* msg => 'Hello, Current time is: 10:16:53' */
		}
	}
	/* Store 20 random records.*/
	for(i = 0 ; i < 20 ; ++i ){
		char zKey[12]; /* Random generated key */
		char zData[34]; /* Dummy data */
		
		/* Generate the random key */
		unqlite_util_random_string(pDb,zKey,sizeof(zKey));
		
		/* Perform the insertion */
		rc = unqlite_kv_store(pDb,zKey,sizeof(zKey),zData,sizeof(zData));
		if( rc != UNQLITE_OK ){
			break;
		}
	}
	if( rc != UNQLITE_OK ){
		/* Insertion fail, rollback the transaction  */
		rc = unqlite_rollback(pDb);
		if( rc != UNQLITE_OK ){
			/* Extract database error log and exit */
			Fatal(pDb,0);
		}
	}

	/* Delete a record */
	unqlite_kv_delete(pDb,"test",-1);

	puts("Done...Starting the iteration process");

	/* Allocate a new cursor instance */
	rc = unqlite_kv_cursor_init(pDb,&pCur);
	if( rc != UNQLITE_OK ){
		Fatal(0,"Out of memory");
	}
	/* Point to the first record */
	unqlite_kv_cursor_first_entry(pCur);
	
		
	/* Iterate over the entries */
	while( unqlite_kv_cursor_valid_entry(pCur) ){
		int nKeyLen;
		/*unqlite_int64 nDataLen;*/
		
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
