/*
 * Symisc unQLite: 一个嵌入式 NoSQL（后现代）数据库引擎。
 * 版权所有 (C) 2012-2013, Symisc Systems http://unqlite.org/
 * 版本 1.1.6
 * 有关许可信息、 redistrib免责声明，请通过以下方式联系 Symisc Systems：
 *       legal@symisc.net
 *       licensing@symisc.net
 *       contact@symisc.net
 * 或访问：
 *      http://unqlite.org/licensing.html
 */
/* $SymiscID: fastjson.c v1.1 FreeBSD 2012-12-05 22:52 stable <chm@symisc.net> $ */
#ifndef UNQLITE_AMALGAMATION
#include "unqliteInt.h"
#endif
/* JSON 二进制编码、解码及相关功能 */
#ifndef UNQLITE_FAST_JSON_NEST_LIMIT
#if defined(__WINNT__) || defined(__UNIXES__)
#define UNQLITE_FAST_JSON_NEST_LIMIT 64 /* 嵌套限制 */
#else
#define UNQLITE_FAST_JSON_NEST_LIMIT 32 /* 嵌套限制 */
#endif
#endif /* UNQLITE_FAST_JSON_NEST_LIMIT */
/*
 * 使用 FastJSON 实现将 JSON 转换为二进制（大端序）。
 */
/*
 * FastJSON 实现的二进制标记。
 */
#define FJSON_DOC_START    1 /* { */
#define FJSON_DOC_END      2 /* } */
#define FJSON_ARRAY_START  3 /* [ */
#define FJSON_ARRAY_END    4 /* ] */
#define FJSON_COLON        5 /* : */
#define FJSON_COMMA        6 /* , */
#define FJSON_ID           7 /* ID + 4 Bytes length */
#define FJSON_STRING       8 /* String + 4 bytes length */
#define FJSON_BYTE         9 /* Byte */
#define FJSON_INT64       10 /* Integer 64 + 8 bytes */
#define FJSON_REAL        18 /* Floating point value + 2 bytes */
#define FJSON_NULL        23 /* NULL */
#define FJSON_TRUE        24 /* TRUE */
#define FJSON_FALSE       25 /* FALSE */
/*
 * Encode a Jx9 value to binary JSON.
 */
UNQLITE_PRIVATE sxi32 FastJsonEncode(
	jx9_value *pValue, /* 要编码的值 */
	SyBlob *pOut,      /* 在此存储编码后的值 */
	int iNest          /* 嵌套限制 */
	)
{
	sxi32 iType = pValue ? pValue->iFlags : MEMOBJ_NULL;
	sxi32 rc = SXRET_OK;
	int c;
	if( iNest >= UNQLITE_FAST_JSON_NEST_LIMIT ){
		/* 达到嵌套限制 */
		return SXERR_LIMIT;
	}
	if( iType & (MEMOBJ_NULL|MEMOBJ_RES) ){
		/*
		 * 资源也被编码为 null。
		 */
		c = FJSON_NULL;
		rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
	}else if( iType & MEMOBJ_BOOL ){
		c = pValue->x.iVal ? FJSON_TRUE : FJSON_FALSE;
		rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
	}else if( iType & MEMOBJ_STRING ){
		unsigned char zBuf[sizeof(sxu32)]; /* 字符串长度 */
		c = FJSON_STRING;
		SyBigEndianPack32(zBuf,SyBlobLength(&pValue->sBlob));
		rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
		if( rc == SXRET_OK ){
			rc = SyBlobAppend(pOut,(const void *)zBuf,sizeof(zBuf));
			if( rc == SXRET_OK ){
				rc = SyBlobAppend(pOut,SyBlobData(&pValue->sBlob),SyBlobLength(&pValue->sBlob));
			}
		}
	}else if( iType & MEMOBJ_INT ){
		unsigned char zBuf[8];
		/* 64位大端序整数 */
		c = FJSON_INT64;
		rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
		if( rc == SXRET_OK ){
			SyBigEndianPack64(zBuf,(sxu64)pValue->x.iVal);
			rc = SyBlobAppend(pOut,(const void *)zBuf,sizeof(zBuf));
		}
	}else if( iType & MEMOBJ_REAL ){
		/* 实数 */
		c = FJSON_REAL;
		rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
		if( rc == SXRET_OK ){
			sxu32 iOfft = SyBlobLength(pOut);
			rc = SyBlobAppendBig16(pOut,0);
			if( rc == SXRET_OK ){
				unsigned char *zBlob;
				SyBlobFormat(pOut,"%.15g",pValue->x.rVal);
				zBlob = (unsigned char *)SyBlobDataAt(pOut,iOfft);
				SyBigEndianPack16(zBlob,(sxu16)(SyBlobLength(pOut) - ( 2 + iOfft)));
			}
		}
	}else if( iType & MEMOBJ_HASHMAP ){
		/* JSON 对象或数组 */
		jx9_hashmap *pMap = (jx9_hashmap *)pValue->x.pOther;
		jx9_hashmap_node *pNode;
		jx9_value *pEntry;
		/* 重置哈希表循环游标 */
		jx9HashmapResetLoopCursor(pMap);
		if( pMap->iFlags & HASHMAP_JSON_OBJECT ){
			jx9_value sKey;
			/* JSON 对象 */
			c = FJSON_DOC_START; /* { */
			rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
			if( rc == SXRET_OK ){
				jx9MemObjInit(pMap->pVm,&sKey);
				/* 编码对象条目 */
				while((pNode = jx9HashmapGetNextEntry(pMap)) != 0 ){
					/* 提取键 */
					jx9HashmapExtractNodeKey(pNode,&sKey);
					/* 编码它 */
					rc = FastJsonEncode(&sKey,pOut,iNest+1);
					if( rc != SXRET_OK ){
						break;
					}
					c = FJSON_COLON;
					rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
					if( rc != SXRET_OK ){
						break;
					}
					/* 提取值 */
					pEntry = jx9HashmapGetNodeValue(pNode);
					/* 编码它 */
					rc = FastJsonEncode(pEntry,pOut,iNest+1);
					if( rc != SXRET_OK ){
						break;
					}
					/* 分隔条目 */
					c = FJSON_COMMA;
					rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
					if( rc != SXRET_OK ){
						break;
					}
				}
				jx9MemObjRelease(&sKey);
				if( rc == SXRET_OK ){
					c = FJSON_DOC_END; /* } */
					rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
				}
			}
		}else{
			/* JSON 数组 */
			c = FJSON_ARRAY_START; /* [ */
			rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
			if( rc == SXRET_OK ){
				/* 编码数组条目 */
				while( (pNode = jx9HashmapGetNextEntry(pMap)) != 0 ){
					/* 提取值 */
					pEntry = jx9HashmapGetNodeValue(pNode);
					/* 编码它 */
					rc = FastJsonEncode(pEntry,pOut,iNest+1);
					if( rc != SXRET_OK ){
						break;
					}
					/* 分隔条目 */
					c = FJSON_COMMA;
					rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
					if( rc != SXRET_OK ){
						break;
					}
				}
				if( rc == SXRET_OK ){
					c = FJSON_ARRAY_END; /* ] */
					rc = SyBlobAppend(pOut,(const void *)&c,sizeof(char));
				}
			}
		}
	}
	return rc;
}
/*
 * 解码 FastJSON 二进制 blob。
 */
UNQLITE_PRIVATE sxi32 FastJsonDecode(
	const void *pIn,  /* 二进制 JSON */
	sxu32 nByte,      /* 块分隔符 */
	jx9_value *pOut,  /* 解码后的值 */
	const unsigned char **pzPtr,
	int iNest /* 嵌套限制 */
	)
{
	const unsigned char *zIn = (const unsigned char *)pIn;
	const unsigned char *zEnd = &zIn[nByte];
	sxi32 rc = SXRET_OK;
	int c;
	if( iNest >= UNQLITE_FAST_JSON_NEST_LIMIT ){
		/* 达到嵌套限制 */
		return SXERR_LIMIT;
	}
	c = zIn[0];
	/* 前进流游标 */
	zIn++;
	/* 处理二进制标记 */
	switch(c){
	case FJSON_NULL:
		/* null */
		jx9_value_null(pOut);
		break;
	case FJSON_FALSE:
		/* 布尔值 FALSE */
		jx9_value_bool(pOut,0);
		break;
	case FJSON_TRUE:
		/* 布尔值 TRUE */
		jx9_value_bool(pOut,1);
		break;
	case FJSON_INT64: {
		/* 64位整数 */
		sxu64 iVal;
		/* 完整性检查 */
		if( &zIn[8] >= zEnd ){
			/* 损坏的块 */
			rc = SXERR_CORRUPT;
			break;
		}
		SyBigEndianUnpack64(zIn,&iVal);
		/* 前进指针 */
		zIn += 8;
		jx9_value_int64(pOut,(jx9_int64)iVal);
		break;
					  }
	case FJSON_REAL: {
		/* 实数 */
		double iVal = 0; /* cc 警告 */
		sxu16 iLen;
		/* 完整性检查 */
		if( &zIn[2] >= zEnd ){
			/* 损坏的块 */
			rc = SXERR_CORRUPT;
			break;
		}
		SyBigEndianUnpack16(zIn,&iLen);
		if( &zIn[iLen] >= zEnd ){
			/* 损坏的块 */
			rc = SXERR_CORRUPT;
			break;
		}
		zIn += 2;
		SyStrToReal((const char *)zIn,(sxu32)iLen,&iVal,0);
		/* 前进指针 */
		zIn += iLen;
		jx9_value_double(pOut,iVal);
		break;
					 }
	case FJSON_STRING: {
		/* UTF-8/二进制块 */
		sxu32 iLength;
		/* 完整性检查 */
		if( &zIn[4] >= zEnd ){
			/* 损坏的块 */
			rc = SXERR_CORRUPT;
			break;
		}
		SyBigEndianUnpack32(zIn,&iLength);
		if( &zIn[iLength] >= zEnd ){
			/* 损坏的块 */
			rc = SXERR_CORRUPT;
			break;
		}
		zIn += 4;
		/* 使任何先前的表示无效 */
		if( pOut->iFlags & MEMOBJ_STRING ){
			/* 重置字符串游标 */
			SyBlobReset(&pOut->sBlob);
		}
		rc = jx9MemObjStringAppend(pOut,(const char *)zIn,iLength);
		/* 更新指针 */
		zIn += iLength;
		break;
					   }
	case FJSON_ARRAY_START: {
		/* 二进制 JSON 数组 */
		jx9_hashmap *pMap;
		jx9_value sVal;
		/* 分配新的哈希表 */
		pMap = (jx9_hashmap *)jx9NewHashmap(pOut->pVm,0,0);
		if( pMap == 0 ){
			rc = SXERR_MEM;
			break;
		}
		jx9MemObjInit(pOut->pVm,&sVal);
		jx9MemObjRelease(pOut);
		MemObjSetType(pOut,MEMOBJ_HASHMAP);
		pOut->x.pOther = pMap;
		rc = SXRET_OK;
		for(;;){
			/* 跳过前导二进制逗号 */
			while (zIn < zEnd && zIn[0] == FJSON_COMMA ){
				zIn++;
			}
			if( zIn >= zEnd || zIn[0] == FJSON_ARRAY_END ){
				if( zIn < zEnd ){
					zIn++; /* 跳过尾随二进制 ] */
				}
				break;
			}
			/* 解码值 */
			rc = FastJsonDecode((const void *)zIn,(sxu32)(zEnd-zIn),&sVal,&zIn,iNest+1);
			if( rc != SXRET_OK ){
				break;
			}
			/* 插入解码后的值 */
			rc = jx9HashmapInsert(pMap,0,&sVal);
			if( rc != UNQLITE_OK ){
				break;
			}
		}
		if( rc != SXRET_OK ){
			jx9MemObjRelease(pOut);
		}
		jx9MemObjRelease(&sVal);
		break;
							}
	case FJSON_DOC_START: {
		/* 二进制 JSON 对象 */
		jx9_value sVal,sKey;
		jx9_hashmap *pMap;
		/* 分配新的哈希表 */
		pMap = (jx9_hashmap *)jx9NewHashmap(pOut->pVm,0,0);
		if( pMap == 0 ){
			rc = SXERR_MEM;
			break;
		}
		jx9MemObjInit(pOut->pVm,&sVal);
		jx9MemObjInit(pOut->pVm,&sKey);
		jx9MemObjRelease(pOut);
		MemObjSetType(pOut,MEMOBJ_HASHMAP);
		pOut->x.pOther = pMap;
		rc = SXRET_OK;
		for(;;){
			/* 跳过前导二进制逗号 */
			while (zIn < zEnd && zIn[0] == FJSON_COMMA ){
				zIn++;
			}
			if( zIn >= zEnd || zIn[0] == FJSON_DOC_END ){
				if( zIn < zEnd ){
					zIn++; /* 跳过尾随二进制 } */
				}
				break;
			}
			/* 提取键 */
			rc = FastJsonDecode((const void *)zIn,(sxu32)(zEnd-zIn),&sKey,&zIn,iNest+1);
			if( rc != UNQLITE_OK ){
				break;
			}
			if( zIn >= zEnd || zIn[0] != FJSON_COLON ){
				rc = UNQLITE_CORRUPT;
				break;
			}
			zIn++; /* 跳过二进制冒号 ':' */
			if( zIn >= zEnd ){
				rc = UNQLITE_CORRUPT;
				break;
			}
			/* 解码值 */
			rc = FastJsonDecode((const void *)zIn,(sxu32)(zEnd-zIn),&sVal,&zIn,iNest+1);
			if( rc != SXRET_OK ){
				break;
			}
			/* 插入键及其关联的值 */
			rc = jx9HashmapInsert(pMap,&sKey,&sVal);
			if( rc != UNQLITE_OK ){
				break;
			}
		}
		if( rc != SXRET_OK ){
			jx9MemObjRelease(pOut);
		}
		jx9MemObjRelease(&sVal);
		jx9MemObjRelease(&sKey);
		break;
						  }
	default:
		/* 损坏的数据 */
		rc = SXERR_CORRUPT;
		break;
	}
	if( pzPtr ){
		*pzPtr = zIn;
	}
	return rc;
}
