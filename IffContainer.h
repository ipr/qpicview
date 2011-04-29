/////////////////////////////////////////////////////////
//
// CIffContainer : generic IFF-format parser,
// detailed parsing must be per file-type (format)
// since IFF is more generic.
//
// (c) Ilkka Prusi, 2011
//

#ifndef _IFFCONTAINER_H_
#define _IFFCONTAINER_H_

#include <stdint.h>

// I can't be arsed to else,
// simple is best
#include "MemoryMappedFile.h"


class CIffChunk; // fwd decl
class CIffSubChunk; // fwd decl


// not part of general IFF-standard,
// parser must detect:
// used by some fileformats
//
class CIffSubChunk
{
public:
	CIffSubChunk()
		: m_pParent(nullptr)
		, m_pNextSub(nullptr)
		, m_iChunkID(0)
		, m_iSize(0)
		, m_iOffset(0)
	{
	}
	~CIffSubChunk()
	{
		// do nothing,
		// parent must destroy all subchunks
	}

	CIffChunk *m_pParent;
	//CIffSubChunk *m_pChild;
	CIffSubChunk *m_pNextSub;
	//CIffSubChunk *m_pPrevSub;

	// actually CHAR[4]
	uint32_t m_iChunkID;

	// size of sub-chunk (check, 16-bit or 32-bit)
	uint16_t m_iSize;
	//uint32_t m_iSize;

	// offset to start of data in actual file
	// (from start of file): we maintain this
	int64_t m_iOffset;
};

class CIffChunk
{
public:
	CIffChunk() 
		: m_pPrevious(nullptr)
		, m_pNext(nullptr)
		, m_pSubChunk(nullptr)
		, m_iChunkID(0)
		, m_iChunkSize(0)
		, m_iOffset(0)
	{
	}
	~CIffChunk()
	{
		// if any chunks within this chunk..
		DestroySubChunks();
	}

	void DestroySubChunks()
	{
		// use simple loop instead of recursion,
		// otherwise easily we run out of stack-limits
		CIffSubChunk *pCurrent = m_pSubChunk;
		while (pCurrent != nullptr)
		{
			CIffSubChunk *pNext = pCurrent->m_pNextSub;
			delete pCurrent;
			pCurrent = pNext;
		}
		m_pSubChunk = nullptr;
	}

	CIffChunk *m_pPrevious;
	CIffChunk *m_pNext;
	CIffSubChunk *m_pSubChunk;

	// actually CHAR[4]
	uint32_t m_iChunkID;

	// size of data in chunk
	uint32_t m_iChunkSize;

	// offset to start of data in actual file
	// (from start of file): we maintain this
	int64_t m_iOffset;
};

class CIffHeader
{
public:
	CIffHeader() 
		: m_pFirst(nullptr)
		, m_iFileID(0)
		, m_iDataSize(0)
		, m_iFileSize(0)
	{
	}
	~CIffHeader()
	{
		DestroyChunks();
	}

	void DestroyChunks()
	{
		// use simple loop instead of recursion,
		// otherwise easily we run out of stack-limits
		CIffChunk *pCurrent = m_pFirst;
		while (pCurrent != nullptr)
		{
			// keep next before destroying current
			CIffChunk *pNext = pCurrent->m_pNext;
			delete pCurrent;
			pCurrent = pNext;
		}
		m_pFirst = nullptr;
	}


	// start from first chunk in file
	CIffChunk *m_pFirst;

	// actually CHAR[4],
	// ID is type of file
	uint32_t m_iFileID;

	// data size given in file header
	uint32_t m_iDataSize;

	// size of actual file
	int64_t m_iFileSize;
};


// handling of the general IFF-FORM chunks in file
//
class CIffContainer
{
private:

	// should we keep data in this or not?
	//CIffHeader *m_pHeader;

protected:

	// (note, should make inline but also needed in inherited classes..)

	// byteswap methods
	uint16_t Swap2(const uint16_t val);
	uint32_t Swap4(const uint32_t val);
	//uint64_t Swap8(const uint64_t val);
	float SwapF(const float fval);
	//double SwapD(const double dval);

	// make tag from string
	uint32_t MakeTag(const char *buf);

	uint32_t GetValueAtOffset(const int64_t iOffset, CMemoryMappedFile &pFile);
	uint8_t *GetViewByOffset(const int64_t iOffset, CMemoryMappedFile &pFile);

	//bool CheckHeader(CMemoryMappedFile &pFile);
	CIffHeader *ReadHeader(int64_t &iOffset, CMemoryMappedFile &pFile);
	CIffHeader *ParseChunks(CMemoryMappedFile &pFile);

protected:

	// methods which user might want to overload on inheritance

	// simple example: IFF-ILBM picture with chunk CMAP
	// -> create handler for that type of chunk.
	// (chunk handling can depend of what format data is stored in general IFF-file)
	//
	virtual CIffChunk *CreateChunkNode(CIffHeader *pHead, uint32_t iChunkID)
	{
		/*
		if (pHead->m_iFileID == ID_ILBM
			&& iChunkID == ID_CMAP)
		{
			return new CIlbmCmap();
		}
		*/

		// should return default-type if non-supported by inherited implementation?
		return new CIffChunk();
		// ..although it can be skipped if implementation doesn't support such chunk..
		//return nullptr;
	}

	// similar to above but for (optional) sub-chunks
	virtual CIffSubChunk *CreateSubChunkNode(CIffHeader *pHead, CIffChunk *pChunk, uint32_t iSubChunkID)
	{
		/*
		if (pHead->m_iFileID == ID_XXXX
			&& pChunk->m_iChunkID == ID_YYYY
			&& iSubChunkID == ID_ZZZZ)
		{
			return new CXYZ();
		}
		*/

		// should return null when there is no sub-chunk
		return nullptr;
	}


public:
	CIffContainer(void);
	virtual ~CIffContainer(void);

	CIffHeader *ParseIffFile(CMemoryMappedFile &pFile);
};

#endif // ifndef _IFFCONTAINER_H_

