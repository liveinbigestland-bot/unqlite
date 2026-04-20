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
/* $SymiscID: bitvec.c v1.0 Win7 2013-02-27 15:16 stable <chm@symisc.net> $ */
#ifndef UNQLITE_AMALGAMATION
#include "unqliteInt.h"
#endif

/**
 * 本文件实现一个动态位图的代表。
 *
 * 位图用于记录数据库文件在事务期间已写入的页面，
 * 或者具有"不写入"属性的页面。通常只有少数页面满足任一条件。
 * 因此位图通常是稀疏的，基数较低。
 */
/*
 * 实际上，这不是真正的位图，而是一个简单的哈希表，
 * 其中页面号（64位无符号整数）用作查找键。
 */
typedef struct bitvec_rec bitvec_rec;
struct bitvec_rec
{
	pgno iPage;                  /* 页号 */
	bitvec_rec *pNext,*pNextCol; /* 冲突链接 */
};
struct Bitvec
{
	SyMemBackend *pAlloc; /* 内存分配器 */
	sxu32 nRec;           /* 记录总数 */
	sxu32 nSize;          /* 表大小 */
	bitvec_rec **apRec;   /* 记录表 */
	bitvec_rec *pList;    /* 记录链表 */
};
/*
 * 分配一个新的 bitvec 实例。
*/
UNQLITE_PRIVATE Bitvec * unqliteBitvecCreate(SyMemBackend *pAlloc,pgno iSize)
{
	bitvec_rec **apNew;
	Bitvec *p;
	
	p = (Bitvec *)SyMemBackendAlloc(pAlloc,sizeof(*p) );
	if( p == 0 ){
		SXUNUSED(iSize); /* cc 警告 */
		return 0;
	}
	/* 清零结构体 */
	SyZero(p,sizeof(Bitvec));
	/* 分配新表 */
	p->nSize = 64; /* 必须是 2 的幂 */
	apNew = (bitvec_rec **)SyMemBackendAlloc(pAlloc,p->nSize * sizeof(bitvec_rec *));
	if( apNew == 0 ){
		SyMemBackendFree(pAlloc,p);
		return 0;
	}
	/* 清零新表 */
	SyZero((void *)apNew,p->nSize * sizeof(bitvec_rec *));
	/* 填充数据 */
	p->apRec = apNew;
	p->pAlloc = pAlloc;
	return p;
}
/*
 * 检查给定页号是否已存在于表中。
 * 如果存在返回 true，否则返回 false。
 */
UNQLITE_PRIVATE int unqliteBitvecTest(Bitvec *p,pgno i)
{  
	bitvec_rec *pRec;
	/* 指向目标桶 */
	pRec = p->apRec[i & (p->nSize - 1)];
	for(;;){
		if( pRec == 0 ){ break; }
		if( pRec->iPage == i ){
			/* 找到页 */
			return 1;
		}
		/* 指向下一个条目 */
		pRec = pRec->pNextCol;

		if( pRec == 0 ){ break; }
		if( pRec->iPage == i ){
			/* 找到页 */
			return 1;
		}
		/* 指向下一个条目 */
		pRec = pRec->pNextCol;


		if( pRec == 0 ){ break; }
		if( pRec->iPage == i ){
			/* 找到页 */
			return 1;
		}
		/* 指向下一个条目 */
		pRec = pRec->pNextCol;


		if( pRec == 0 ){ break; }
		if( pRec->iPage == i ){
			/* 找到页 */
			return 1;
		}
		/* 指向下一个条目 */
		pRec = pRec->pNextCol;
	}
	/* 没有该条目 */
	return 0;
}
/*
 * 在位图（实际上是哈希表）中安装给定页号。
 */
UNQLITE_PRIVATE int unqliteBitvecSet(Bitvec *p,pgno i)
{
	bitvec_rec *pRec;
	sxi32 iBuck;
	/* 分配新实例 */
	pRec = (bitvec_rec *)SyMemBackendPoolAlloc(p->pAlloc,sizeof(bitvec_rec));
	if( pRec == 0 ){
		return UNQLITE_NOMEM;
	}
	/* 清零结构体 */
	SyZero(pRec,sizeof(bitvec_rec));
	/* 填充数据 */
	pRec->iPage = i;
	iBuck = i & (p->nSize - 1);
	pRec->pNextCol = p->apRec[iBuck];
	p->apRec[iBuck] = pRec;
	pRec->pNext = p->pList;
	p->pList = pRec;
	p->nRec++;
	if( p->nRec >= (p->nSize * 3) && p->nRec < 100000 ){
		/* 扩展哈希表 */
		sxu32 nNewSize = p->nSize << 1;
		bitvec_rec *pEntry,**apNew;
		sxu32 n;
		apNew = (bitvec_rec **)SyMemBackendAlloc(p->pAlloc, nNewSize * sizeof(bitvec_rec *));
		if( apNew ){
			sxu32 iBucket;
			/* 清零新表 */
			SyZero((void *)apNew, nNewSize * sizeof(bitvec_rec *));
			/* 重新哈希所有条目 */
			n = 0;
			pEntry = p->pList;
			for(;;){
				/* 循环一次 */
				if( n >= p->nRec ){
					break;
				}
				pEntry->pNextCol = 0;
				/* 安装到新桶 */
				iBucket = pEntry->iPage & (nNewSize - 1);
				pEntry->pNextCol = apNew[iBucket];
				apNew[iBucket] = pEntry;
				/* 指向下一个条目 */
				pEntry = pEntry->pNext;
				n++;
			}
			/* 释放旧表并反映变化 */
			SyMemBackendFree(p->pAlloc,(void *)p->apRec);
			p->apRec = apNew;
			p->nSize  = nNewSize;
		}
	}
	return UNQLITE_OK;
}
/*
 * 销毁 bitvec 实例。回收所有已用内存。
 */
UNQLITE_PRIVATE void unqliteBitvecDestroy(Bitvec *p)
{
	bitvec_rec *pNext,*pRec = p->pList;
	SyMemBackend *pAlloc = p->pAlloc;
	
	for(;;){
		if( p->nRec < 1 ){
			break;
		}
		pNext = pRec->pNext;
		SyMemBackendPoolFree(pAlloc,(void *)pRec);
		pRec = pNext;
		p->nRec--;

		if( p->nRec < 1 ){
			break;
		}
		pNext = pRec->pNext;
		SyMemBackendPoolFree(pAlloc,(void *)pRec);
		pRec = pNext;
		p->nRec--;


		if( p->nRec < 1 ){
			break;
		}
		pNext = pRec->pNext;
		SyMemBackendPoolFree(pAlloc,(void *)pRec);
		pRec = pNext;
		p->nRec--;


		if( p->nRec < 1 ){
			break;
		}
		pNext = pRec->pNext;
		SyMemBackendPoolFree(pAlloc,(void *)pRec);
		pRec = pNext;
		p->nRec--;
	}
	SyMemBackendFree(pAlloc,(void *)p->apRec);
	SyMemBackendFree(pAlloc,p);
}
