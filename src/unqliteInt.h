/*
 * Symisc unQLite: 一个嵌入式 NoSQL（后现代）数据库引擎。
 * 版权所有 (C) 2012-2013, Symisc Systems http://unqlite.org/
 * 版本 1.1.6
 * 有关许可协议、再分发和免责声明的详细信息，请联系 Symisc Systems：
 *       legal@symisc.net
 *       licensing@symisc.net
 *       contact@symisc.net
 * 或访问：
 *      http://unqlite.org/licensing.html
 */
 /* $SymiscID: unqliteInt.h v1.7 FreeBSD 2012-11-02 11:25 devel <chm@symisc.net> $ */
#ifndef __UNQLITEINT_H__
#define __UNQLITEINT_H__
/* UnQLite 的内部接口定义。*/
#ifdef UNQLITE_AMALGAMATION
/* Marker for routines not intended for external use */
#define UNQLITE_PRIVATE static
#define JX9_AMALGAMATION
#else
#define UNQLITE_PRIVATE
#include "unqlite.h"
#include "jx9Int.h"
#endif 
/* 前向声明 */
typedef struct unqlite_db unqlite_db;
/*
** 以下值可以作为第二个参数传递给 UnqliteOsLock()。各种锁具有以下语义：
**
** SHARED:    任意数量的进程可以同时持有 SHARED 锁。
** RESERVED:  单个进程可以在任何时候持有 RESERVED 锁。
**            其他进程可以持有并获取新的 SHARED 锁。
** PENDING:   单个进程可以在任何时候持有 PENDING 锁。
**            现有的 SHARED 锁可以存在，但其他进程不能获取新的 SHARED 锁。
** EXCLUSIVE: EXCLUSIVE 锁排除所有其他锁。
**
** PENDING_LOCK 不能直接传递给 UnqliteOsLock()。相反，请求 EXCLUSIVE 锁的进程
** 实际上可能获得 PENDING 锁。这可以通过后续调用 UnqliteOsLock() 升级为 EXCLUSIVE 锁。
*/
#define NO_LOCK         0
#define SHARED_LOCK     1
#define RESERVED_LOCK   2
#define PENDING_LOCK    3
#define EXCLUSIVE_LOCK  4
/*
 * UnQLite 锁定策略（与 SQLite3 相同）
 *
 * 以下 #defines 指定用于锁定的字节范围。
 * SHARED_SIZE 是池中可用字节的数量，从中选择一个随机字节用于共享锁。
 * 共享锁的字节池从 SHARED_FIRST 开始。
 *
 * Unix 和 Windows 使用相同的锁定策略和字节范围。
 * 这留下了让 winNT 和 unix 上的客户端都能正确锁定地与相同共享文件通信的可能性。
 * 要做到这一点，需要 samba（或用于文件共享的任何工具）在 windows 和 unix 之间正确实现锁定。
 * 我猜这不太可能发生，但通过使用相同的锁定范围，我们至少开放了这种可能性。
 *
 * Windows 中的锁定是强制性的。因此，我们不能将实际数据存储在用于锁定的字节中。
 * 因此，pager 从不分配涉及锁定的页面。选择 SHARED_SIZE 是为了使所有锁
 * 即使在最小页面大小下也能容纳在单个页面上。
 * PENDING_BYTE 定义锁的开始。默认情况下，PENDING_BYTE 设置得很高，
 * 以便我们不必为非常大的数据库分配未使用的页面。但是应该通过将 PENDING_BYTE
 * 设置得较低并运行整个回归测试套件来测试页面跳过逻辑。
 *
 * 更改 PENDING_BYTE 的值会导致微妙的不兼容文件格式。根据其更改方式，
 * 您可能不会立即注意到不兼容性，即使运行完整的回归测试。
 * PENDING_BYTE 的默认位置是越过 1GB 边界的第一个字节。
 */
#define PENDING_BYTE     (0x40000000)
#define RESERVED_BYTE    (PENDING_BYTE+1)
#define SHARED_FIRST     (PENDING_BYTE+2)
#define SHARED_SIZE      510
/*
 * 磁盘扇区的默认大小（以字节为单位）。
 */
#ifndef UNQLITE_DEFAULT_SECTOR_SIZE
#define UNQLITE_DEFAULT_SECTOR_SIZE 512
#endif
/*
 * 每个打开的数据库文件由 "Pager" 结构的单独实例管理。
 */
typedef struct Pager Pager;
/*
 * 要由系统访问的每个数据库文件都是以下结构的实例。
 */
struct unqlite_db
{
	Pager *pPager;              /* Pager and Transaction manager */
	jx9 *pJx9;                  /* Jx9 Engine handle */
	unqlite_kv_cursor *pCursor; /* Database cursor for common usage */
};
/*
 * Each database connection is an instance of the following structure.
 */
struct unqlite
{
	SyMemBackend sMem;              /* Memory allocator subsystem */
	SyBlob sErr;                    /* Error log */
	unqlite_db sDB;                 /* Storage backend */
#if defined(UNQLITE_ENABLE_THREADS)
	const SyMutexMethods *pMethods;  /* Mutex methods */
	SyMutex *pMutex;                 /* Per-handle mutex */
#endif
	unqlite_vm *pVms;                /* List of active VM */
	sxi32 iVm;                       /* Total number of active VM */
	sxi32 iFlags;                    /* Control flags (See below)  */
	unqlite *pNext,*pPrev;           /* List of active DB handles */
	sxu32 nMagic;                    /* Sanity check against misuse */
};
#define UNQLITE_FL_DISABLE_AUTO_COMMIT   0x001 /* Disable auto-commit on close */
/*
 * VM control flags (Mostly related to collection handling).
 */
#define UNQLITE_VM_COLLECTION_CREATE     0x001 /* Create a new collection */
#define UNQLITE_VM_COLLECTION_EXISTS     0x002 /* Exists old collection */
#define UNQLITE_VM_AUTO_LOAD             0x004 /* Auto load a collection from the vfs */
/* Forward declaration */
typedef struct unqlite_col_record unqlite_col_record;
typedef struct unqlite_col unqlite_col;
/*
 * Each an in-memory collection record is stored in an instance
 * of the following structure.
 */
struct unqlite_col_record
{
	unqlite_col *pCol;                      /* Collecion this record belong */
	jx9_int64 nId;                          /* Unique record ID */
	jx9_value sValue;                       /* In-memory value of the record */
	unqlite_col_record *pNextCol,*pPrevCol; /* Collision chain */
	unqlite_col_record *pNext,*pPrev;       /* Linked list of records */
};
/* 
 * Magic number to identify a valid collection on disk.
 */
#define UNQLITE_COLLECTION_MAGIC 0x611E /* sizeof(unsigned short) 2 bytes */
/*
 * A loaded collection is identified by an instance of the following structure.
 */
struct unqlite_col
{
	unqlite_vm *pVm;   /* VM that own this instance */
	SyString sName;    /* ID of the collection */
	sxu32 nHash;       /* sName hash */
	jx9_value sSchema; /* Collection schema */
	sxu32 nSchemaOfft; /* Shema offset in sHeader */
	SyBlob sWorker;    /* General purpose working buffer */
	SyBlob sHeader;    /* Collection binary header */
	jx9_int64 nLastid; /* Last collection record ID */
	jx9_int64 nCurid;  /* Current record ID */
	jx9_int64 nTotRec; /* Total number of records in the collection */
	int iFlags;        /* Control flags (see below) */
	unqlite_col_record **apRecord; /* Hashtable of loaded records */
	unqlite_col_record *pList;     /* Linked list of records */
	sxu32 nRec;        /* Total number of records in apRecord[] */     
	sxu32 nRecSize;    /* apRecord[] size */
	Sytm sCreation;    /* Colleation creation time */
	unqlite_kv_cursor *pCursor; /* Cursor pointing to the raw binary data */
	unqlite_col *pNext,*pPrev;  /* Next and previous collection in the chain */
	unqlite_col *pNextCol,*pPrevCol; /* Collision chain */
};
/*
 * Each unQLite Virtual Machine resulting from successful compilation of
 * a Jx9 script is represented by an instance of the following structure.
 */
struct unqlite_vm
{
	unqlite *pDb;              /* Database handle that own this instance */
	SyMemBackend sAlloc;       /* Private memory allocator */
#if defined(UNQLITE_ENABLE_THREADS)
	SyMutex *pMutex;           /* Recursive mutex associated with this VM. */
#endif
	unqlite_col **apCol;       /* Table of loaded collections */
	unqlite_col *pCol;         /* List of loaded collections */
	sxu32 iCol;                /* Total number of loaded collections */
	sxu32 iColSize;            /* apCol[] size  */
	jx9_vm *pJx9Vm;            /* Compiled Jx9 script*/
	unqlite_vm *pNext,*pPrev;  /* Linked list of active unQLite VM */
	sxu32 nMagic;              /* Magic number to avoid misuse */
};
/* 
 * Database signature to identify a valid database image.
 */
#define UNQLITE_DB_SIG "unqlite"
/*
 * Database magic number (4 bytes).
 */
#define UNQLITE_DB_MAGIC   0xDB7C2712
/*
 * Maximum page size in bytes.
 */
#ifdef UNQLITE_MAX_PAGE_SIZE
# undef UNQLITE_MAX_PAGE_SIZE
#endif
#define UNQLITE_MAX_PAGE_SIZE 65536 /* 65K */
/*
 * Minimum page size in bytes.
 */
#ifdef UNQLITE_MIN_PAGE_SIZE
# undef UNQLITE_MIN_PAGE_SIZE
#endif
#define UNQLITE_MIN_PAGE_SIZE 512
/*
 * The default size of a database page.
 */
#ifndef UNQLITE_DEFAULT_PAGE_SIZE
# undef UNQLITE_DEFAULT_PAGE_SIZE
#endif
# define UNQLITE_DEFAULT_PAGE_SIZE 4096 /* 4K */
/* Forward declaration */
typedef struct Bitvec Bitvec;
/* Private library functions */
/* api.c */
UNQLITE_PRIVATE const SyMemBackend * unqliteExportMemBackend(void);
UNQLITE_PRIVATE int unqliteDataConsumer(
	const void *pOut,   /* Data to consume */
	unsigned int nLen,  /* Data length */
	void *pUserData     /* User private data */
	);
UNQLITE_PRIVATE unqlite_kv_methods * unqliteFindKVStore(
	const char *zName, /* Storage engine name [i.e. Hash, B+tree, LSM, etc.] */
	sxu32 nByte        /* zName length */
	);
UNQLITE_PRIVATE int unqliteGetPageSize(void);
UNQLITE_PRIVATE int unqliteGenError(unqlite *pDb,const char *zErr);
UNQLITE_PRIVATE int unqliteGenErrorFormat(unqlite *pDb,const char *zFmt,...);
UNQLITE_PRIVATE int unqliteGenOutofMem(unqlite *pDb);
/* unql_vm.c */
UNQLITE_PRIVATE int unqliteExistsCollection(unqlite_vm *pVm, SyString *pName);
UNQLITE_PRIVATE int unqliteCreateCollection(unqlite_vm *pVm,SyString *pName);
UNQLITE_PRIVATE jx9_int64 unqliteCollectionLastRecordId(unqlite_col *pCol);
UNQLITE_PRIVATE jx9_int64 unqliteCollectionCurrentRecordId(unqlite_col *pCol);
UNQLITE_PRIVATE int unqliteCollectionCacheRemoveRecord(unqlite_col *pCol,jx9_int64 nId);
UNQLITE_PRIVATE jx9_int64 unqliteCollectionTotalRecords(unqlite_col *pCol);
UNQLITE_PRIVATE void unqliteCollectionResetRecordCursor(unqlite_col *pCol);
UNQLITE_PRIVATE int unqliteCollectionFetchNextRecord(unqlite_col *pCol,jx9_value *pValue);
UNQLITE_PRIVATE int unqliteCollectionFetchRecordById(unqlite_col *pCol,jx9_int64 nId,jx9_value *pValue);
UNQLITE_PRIVATE unqlite_col * unqliteCollectionFetch(unqlite_vm *pVm,SyString *pCol,int iFlag);
UNQLITE_PRIVATE int unqliteCollectionSetSchema(unqlite_col *pCol,jx9_value *pValue);
UNQLITE_PRIVATE int unqliteCollectionPut(unqlite_col *pCol,jx9_value *pValue,int iFlag);
UNQLITE_PRIVATE int unqliteCollectionDropRecord(unqlite_col *pCol,jx9_int64 nId,int wr_header,int log_err);
UNQLITE_PRIVATE int unqliteCollectionUpdateRecord(unqlite_col *pCol,jx9_int64 nId, jx9_value *pValue,int iFlag);
UNQLITE_PRIVATE int unqliteDropCollection(unqlite_col *pCol);
/* unql_jx9.c */
UNQLITE_PRIVATE int unqliteRegisterJx9Functions(unqlite_vm *pVm);
/* fastjson.c */
UNQLITE_PRIVATE sxi32 FastJsonEncode(
	jx9_value *pValue, /* Value to encode */
	SyBlob *pOut,      /* Store encoded value here */
	int iNest          /* Nesting limit */ 
	);
UNQLITE_PRIVATE sxi32 FastJsonDecode(
	const void *pIn, /* Binary JSON  */
	sxu32 nByte,     /* Chunk delimiter */
	jx9_value *pOut, /* Decoded value */
	const unsigned char **pzPtr,
	int iNest /* Nesting limit */
	);
/* vfs.c [io_win.c, io_unix.c ] */
UNQLITE_PRIVATE const unqlite_vfs * unqliteExportBuiltinVfs(void);
/* mem_kv.c */
UNQLITE_PRIVATE const unqlite_kv_methods * unqliteExportMemKvStorage(void);
/* lhash_kv.c */
UNQLITE_PRIVATE const unqlite_kv_methods * unqliteExportDiskKvStorage(void);
/* os.c */
UNQLITE_PRIVATE int unqliteOsRead(unqlite_file *id, void *pBuf, unqlite_int64 amt, unqlite_int64 offset);
UNQLITE_PRIVATE int unqliteOsWrite(unqlite_file *id, const void *pBuf, unqlite_int64 amt, unqlite_int64 offset);
UNQLITE_PRIVATE int unqliteOsTruncate(unqlite_file *id, unqlite_int64 size);
UNQLITE_PRIVATE int unqliteOsSync(unqlite_file *id, int flags);
UNQLITE_PRIVATE int unqliteOsFileSize(unqlite_file *id, unqlite_int64 *pSize);
UNQLITE_PRIVATE int unqliteOsLock(unqlite_file *id, int lockType);
UNQLITE_PRIVATE int unqliteOsUnlock(unqlite_file *id, int lockType);
UNQLITE_PRIVATE int unqliteOsCheckReservedLock(unqlite_file *id, int *pResOut);
UNQLITE_PRIVATE int unqliteOsSectorSize(unqlite_file *id);
UNQLITE_PRIVATE int unqliteOsOpen(
  unqlite_vfs *pVfs,
  SyMemBackend *pAlloc,
  const char *zPath, 
  unqlite_file **ppOut, 
  unsigned int flags
);
UNQLITE_PRIVATE int unqliteOsCloseFree(SyMemBackend *pAlloc,unqlite_file *pId);
UNQLITE_PRIVATE int unqliteOsDelete(unqlite_vfs *pVfs, const char *zPath, int dirSync);
UNQLITE_PRIVATE int unqliteOsAccess(unqlite_vfs *pVfs,const char *zPath,int flags,int *pResOut);
/* bitmap.c */
UNQLITE_PRIVATE Bitvec *unqliteBitvecCreate(SyMemBackend *pAlloc,pgno iSize);
UNQLITE_PRIVATE int unqliteBitvecTest(Bitvec *p,pgno i);
UNQLITE_PRIVATE int unqliteBitvecSet(Bitvec *p,pgno i);
UNQLITE_PRIVATE void unqliteBitvecDestroy(Bitvec *p);
/* pager.c */
UNQLITE_PRIVATE int unqliteInitCursor(unqlite *pDb,unqlite_kv_cursor **ppOut);
UNQLITE_PRIVATE int unqliteReleaseCursor(unqlite *pDb,unqlite_kv_cursor *pCur);
UNQLITE_PRIVATE int unqlitePagerSetCachesize(Pager *pPager,int mxPage);
UNQLITE_PRIVATE int unqlitePagerClose(Pager *pPager);
UNQLITE_PRIVATE int unqlitePagerOpen(
  unqlite_vfs *pVfs,       /* The virtual file system to use */
  unqlite *pDb,            /* Database handle */
  const char *zFilename,   /* Name of the database file to open */
  unsigned int iFlags      /* flags controlling this file */
  );
UNQLITE_PRIVATE int unqlitePagerRegisterKvEngine(Pager *pPager,unqlite_kv_methods *pMethods);
UNQLITE_PRIVATE unqlite_kv_engine * unqlitePagerGetKvEngine(unqlite *pDb);
UNQLITE_PRIVATE int unqlitePagerBegin(Pager *pPager);
UNQLITE_PRIVATE int unqlitePagerCommit(Pager *pPager);
UNQLITE_PRIVATE int unqlitePagerRollback(Pager *pPager,int bResetKvEngine);
UNQLITE_PRIVATE void unqlitePagerRandomString(Pager *pPager,char *zBuf,sxu32 nLen);
UNQLITE_PRIVATE sxu32 unqlitePagerRandomNum(Pager *pPager);
#endif /* __UNQLITEINT_H__ */
