/////////////////////////////////////////////////////////
//
// CIffContainer : generic IFF-format parser,
// detailed parsing must be per file-type (format)
// since IFF is more generic.
//
// (c) Ilkka Prusi, 2011
//


//#include "stdafx.h"
#include "IffContainer.h"


uint16_t CIffContainer::Swap2(const uint16_t val)
{
   return (((val >> 8)) | (val << 8));
}

uint32_t CIffContainer::Swap4(const uint32_t val)
{
	// swap bytes
	return (
			((val & 0x000000FF) << 24) + ((val & 0x0000FF00) <<8) |
			((val & 0x00FF0000) >> 8) + ((val & 0xFF000000) >>24)
			);
}

/* 32-bit format but some may use 64-bit values also?
uint64_t CIffContainer::Swap8(const uint64_t val)
{
}
*/

float CIffContainer::SwapF(const float fval)
{
	// must have value where we can take address (not from function parameter),
	// cast via "bit-array" to another type:
	// avoid implicit "float-int" conversion to avoid rounding errors.
	//
	float fTemp = fval;
	uint32_t tmp = Swap4((*((uint32_t*)(&fTemp))));
	fTemp = (*((float*)(&tmp)));
	return fTemp;
}

/* 32-bit format but some may use 64-bit values also?
double CIffContainer::SwapD(const double dval)
{
	// similar to byteswap on float,
	// avoid int<->float conversion/rounding errors during byteswap
	//
	double dTemp = dval;
	uint64_t tmp = Swap8((*((uint64_t*)(&dTemp))));
	dTemp = (*((double*)(&tmp)));
	return dTemp;
}
*/

// combine individual chars to tag in 32-bit int
uint32_t CIffContainer::MakeTag(const char *buf)
{
	// note: little-endian CPU with big-endian data
	uint32_t tmp = 0;
	tmp |= (((uint32_t)(buf[3])) << 24);
	tmp |= (((uint32_t)(buf[2])) << 16);
	tmp |= (((uint32_t)(buf[1])) << 8);
	tmp |= ((uint32_t)(buf[0]));
	return tmp;

	/*
	// big-endian CPU with big-endian data
	uint32_t tmp = 0;
	tmp |= (((uint32_t)(buf[0])) << 24);
	tmp |= (((uint32_t)(buf[1])) << 16);
	tmp |= (((uint32_t)(buf[2])) << 8);
	tmp |= ((uint32_t)(buf[3]));
	return tmp;
	*/
}

uint32_t CIffContainer::GetValueAtOffset(const int64_t iOffset, CMemoryMappedFile &pFile)
{
	// byte-pointer to given offset, get value as 4-byte int
	uint8_t *pData = (uint8_t*)pFile.GetView();
	pData = pData + iOffset;
	return (*((uint32_t*)pData));

	/* 
	dumb-ass bug below..
	offset might not align to 4-bytes..
	need 1-byte alignment

	locate value, byte-offset to value-index
	uint32_t *pData = (uint32_t*)pFile.GetView();
	return pData[iOffset/sizeof(uint32_t)];
	*/
}

uint8_t *CIffContainer::GetViewByOffset(const int64_t iOffset, CMemoryMappedFile &pFile)
{
	uint8_t *pData = (uint8_t*)pFile.GetView();
	return (pData + iOffset);
}

CIffHeader *CIffContainer::ReadHeader(int64_t &iOffset, CMemoryMappedFile &pFile)
{
	// at least header must exist in file
	if (pFile.GetSize() < 12)
	{
		return nullptr;
	}

	uint32_t *pData = (uint32_t*)pFile.GetView();
	CIffHeader *pHead = new CIffHeader();

	// keep values from header and byteswap (big->little),
	// size before ID in header
	pHead->m_iDataSize = Swap4(pData[1]); // datasize according to header
	pHead->m_iFileID = pData[2]; // actual file type (e.g. ILBM)

	// start after file header
	// (including beginning "FORM" tag)
	iOffset = 12;

	// just check that some chunks exist also after header
	if (pFile.GetSize() > 12)
	{
		pHead->m_pFirst = new CIffChunk();
	}

	return pHead;
}

CIffHeader *CIffContainer::ParseChunks(CMemoryMappedFile &pFile)
{
	int64_t iOffset = 0;
	CIffHeader *pHead = ReadHeader(iOffset, pFile);
	CIffChunk *pCurrent = pHead->m_pFirst;

	while (iOffset < pFile.GetSize())
	{
		// ID followed by size
		pCurrent->m_iChunkID = GetValueAtOffset(iOffset, pFile);
		pCurrent->m_iChunkSize = Swap4(GetValueAtOffset(iOffset+4, pFile));

		// keep chunk "raw" data position in file
		pCurrent->m_iOffset = iOffset +8;

		// offset to next chunk start
		iOffset += (pCurrent->m_iChunkSize +8);

		// create next, link it to this and switch to next
		CIffChunk *pNext = new CIffChunk();
		pNext->m_pPrevious = pCurrent;
		pCurrent->m_pNext = pNext;
		pCurrent = pNext;

		/* 
		// 
		pCurrent->m_pNext = new CIffChunk();
		pCurrent->m_pNext->m_pPrevious = pCurrent;
		pCurrent = pCurrent->m_pNext;
		*/
	}

	return pHead;
}


/////////////// public methods

CIffContainer::CIffContainer(void)
{
}

CIffContainer::~CIffContainer(void)
{
}

CIffHeader *CIffContainer::ParseIffFile(CMemoryMappedFile &pFile)
{
	/*
	if (pFile == nullptr)
	{
		return nullptr;
	}
	*/

	if (pFile.IsCreated() == false)
	{
		return nullptr;
	}

	uint32_t *pData = (uint32_t*)pFile.GetView();

	// must have proper IFF-identifier at start
	//
	// note: also may have LIST or CAT with FORM sub-chunk
	// (e.g. anim-file may include both pics and sound)
	//
	if (pData[0] != MakeTag("FORM")) // TODO: prepare const values for tags?
	{
		// nothing to do, unsupported file
		return nullptr;
	}

	/*
	if (::memcmp(pFile.GetView(), "FORM", 4) != 0)
	{
		// nothing to do, unsupported file
		return nullptr;
	}
	*/

	// invalid size of file
	/*
	if ((pHead->m_iFileSize +12) > pFile->GetSize())
	{
	}
	*/

	return ParseChunks(pFile);
}

