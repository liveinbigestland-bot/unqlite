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
 /* $SymiscID: os.c v1.0 FreeBSD 2012-11-12 21:27 devel <chm@symisc.net> $ */
#ifndef UNQLITE_AMALGAMATION
#include "unqliteInt.h"
#endif
/* OS 接口抽象层：主要来自 SQLite3 源码树 */
/*
** 以下例程是 unqlite_file 对象方法的便捷包装器。这主要只是语法糖。
** 如果 UnQLite 使用 C++ 而不是普通的 C 编写，所有这些都将是完全自动的。
*/
UNQLITE_PRIVATE int unqliteOsRead(unqlite_file *id, void *pBuf, unqlite_int64 amt, unqlite_int64 offset)
{
  return id->pMethods->xRead(id, pBuf, amt, offset);
}
UNQLITE_PRIVATE int unqliteOsWrite(unqlite_file *id, const void *pBuf, unqlite_int64 amt, unqlite_int64 offset)
{
  return id->pMethods->xWrite(id, pBuf, amt, offset);
}
UNQLITE_PRIVATE int unqliteOsTruncate(unqlite_file *id, unqlite_int64 size)
{
  return id->pMethods->xTruncate(id, size);
}
UNQLITE_PRIVATE int unqliteOsSync(unqlite_file *id, int flags)
{
  return id->pMethods->xSync(id, flags);
}
UNQLITE_PRIVATE int unqliteOsFileSize(unqlite_file *id, unqlite_int64 *pSize)
{
  return id->pMethods->xFileSize(id, pSize);
}
UNQLITE_PRIVATE int unqliteOsLock(unqlite_file *id, int lockType)
{
  return id->pMethods->xLock(id, lockType);
}
UNQLITE_PRIVATE int unqliteOsUnlock(unqlite_file *id, int lockType)
{
  return id->pMethods->xUnlock(id, lockType);
}
UNQLITE_PRIVATE int unqliteOsCheckReservedLock(unqlite_file *id, int *pResOut)
{
  return id->pMethods->xCheckReservedLock(id, pResOut);
}
UNQLITE_PRIVATE int unqliteOsSectorSize(unqlite_file *id)
{
  if( id->pMethods->xSectorSize ){
	  return id->pMethods->xSectorSize(id);
  }
  return  UNQLITE_DEFAULT_SECTOR_SIZE;
}
/*
** The next group of routines are convenience wrappers around the
** VFS methods.
*/
UNQLITE_PRIVATE int unqliteOsOpen(
  unqlite_vfs *pVfs,
  SyMemBackend *pAlloc,
  const char *zPath, 
  unqlite_file **ppOut, 
  unsigned int flags 
)
{
	unqlite_file *pFile;
	int rc;
	*ppOut = 0;
	if( zPath == 0 ){
		/* May happen if dealing with an in-memory database */
		return SXERR_EMPTY;
	}
	/* Allocate a new instance */
	pFile = (unqlite_file *)SyMemBackendAlloc(pAlloc,sizeof(unqlite_file)+pVfs->szOsFile);
	if( pFile == 0 ){
		return UNQLITE_NOMEM;
	}
	/* Zero the structure */
	SyZero(pFile,sizeof(unqlite_file)+pVfs->szOsFile);
	/* Invoke the xOpen method of the underlying VFS */
	rc = pVfs->xOpen(pVfs, zPath, pFile, flags);
	if( rc != UNQLITE_OK ){
		SyMemBackendFree(pAlloc,pFile);
		pFile = 0;
	}
	*ppOut = pFile;
	return rc;
}
UNQLITE_PRIVATE int unqliteOsCloseFree(SyMemBackend *pAlloc,unqlite_file *pId)
{
	int rc = UNQLITE_OK;
	if( pId ){
		rc = pId->pMethods->xClose(pId);
		SyMemBackendFree(pAlloc,pId);
	}
	return rc;
}
UNQLITE_PRIVATE int unqliteOsDelete(unqlite_vfs *pVfs, const char *zPath, int dirSync){
  return pVfs->xDelete(pVfs, zPath, dirSync);
}
UNQLITE_PRIVATE int unqliteOsAccess(
  unqlite_vfs *pVfs, 
  const char *zPath, 
  int flags, 
  int *pResOut
){
  return pVfs->xAccess(pVfs, zPath, flags, pResOut);
}
