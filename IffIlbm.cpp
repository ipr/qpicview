/////////////////////////////////////////////////////////
//
// CIffIlbm : IFF-ILBM image format parser
//
// (c) Ilkka Prusi, 2011
//
// See format specs from: 
// http://www.fine-view.com/jp/labs/doc/ilbm.txt
//

//#include "stdafx.h"
#include "IffIlbm.h"


// TODO: subclass from chunk and do processing there?
//
bool CIffIlbm::ParseBitmapHeader(uint8_t *pData, CIffChunk *pChunk)
{
	// BMHD bitmap header chunk:
	// should be first in file (according to spec)
	// and needed by later processing of data

	// make byteswapping where necessary to a copy (keep original)
	//BitMapHeader SwappedHd;

	BitMapHeader *pBmHd = (BitMapHeader*)pData;
	m_BmHeader.w = Swap2(pBmHd->w);
	m_BmHeader.h = Swap2(pBmHd->h);
	m_BmHeader.x = Swap2(pBmHd->x);
	m_BmHeader.y = Swap2(pBmHd->y);
	m_BmHeader.nPlanes = pBmHd->nPlanes;
	m_BmHeader.masking = pBmHd->masking;
	m_BmHeader.compression = pBmHd->compression;
	m_BmHeader.pad1 = pBmHd->pad1; // unused
	m_BmHeader.transparentColor = Swap2(pBmHd->transparentColor);
	m_BmHeader.xAspect = pBmHd->xAspect;
	m_BmHeader.yAspect = pBmHd->yAspect;
	m_BmHeader.pageWidth = Swap2(pBmHd->pageWidth);
	m_BmHeader.pageHeight = Swap2(pBmHd->pageHeight);

	return true;
}

// TODO: subclass from chunk and do processing there?
//
void CIffIlbm::ParseBody(uint8_t *pData, CIffChunk *pChunk)
{
	//
	// The pixel values in a BODY can be indexes into the palette contained in a CMAP chunk, 
	// or they can be literal RGB values. 
	// If there is no CMAP and if BMHD.BitPlanes is 24, 
	// the ILBM contains a 24-bit image, 
	// and the BODY encodes pixels as literal RGB values.
	//

    
    int64_t ickEnd = (pChunk->m_iOffset + pChunk->m_iChunkSize);
	int64_t iChOffset = pChunk->m_iOffset;
    
    // note: all rows should be same size (and word-aligned?)
    int iRowBytes = ((m_BmHeader.w + 15) >> 4) << 1;
    int iBodySize = m_BmHeader.h*m_BmHeader.nPlanes*iRowBytes;
    
    UBYTE *pBody = new UBYTE[iBodySize];
    if (m_BmHeader.compression == cmpNone)
    {
        // no compression -> copy as-is
        ::memcpy(pBody, pData, iBodySize);
        iChOffset += iBodySize;
        
        // TODO:?
        // extra bitplane per "row" (see height)
        if (m_BmHeader.masking != mskNone)
        {
            // read masking
            /*
            UBYTE *pLine = new UBYTE[iRowBytes];
            ::memcpy(pLine, pData, iRowBytes);
            iChOffset += iRowBytes;
            */
        }
    }
    else if (m_BmHeader.compression == cmpByteRun1)
    {
        // decompress scanline
        DecompressByteRun1(pData, ickEnd, iChOffset, pBody);
    }

/*    
	while (iChOffset < ickEnd)
	{
		// read each scanline for each plane
		// also handle compression (if any)
		// and masking-scanline (if any).

		// note: all rows should be same size (and word-aligned?)
		int iRowBytes = ((m_BmHeader.w + 15) >> 4) << 1;

		// for each plane..
		// with or without compression?
		//if (m_BmHeader.compression == cmpNone)

		for (int j = 0; j < m_BmHeader.h; j++)
		{
			for (int i = 0; i < m_BmHeader.nPlanes; i++)
			{
				// read scanline:
				// output-width should be known already (in header)

				// actually, arrays of ColorRegister ?
				// (depends if CMAP exists..)
				UBYTE *pLine = new UBYTE[iRowBytes];
				//size_t nBytes = sizeof(int16_t)*m_BmHeader.w;

				if (m_BmHeader.compression == cmpNone)
				{
					// no compression -> copy as-is
					::memcpy(pLine, pData, iRowBytes);
					iChOffset += iRowBytes;
				}
				else if (m_BmHeader.compression == cmpByteRun1)
				{
					// decompress scanline
					DecompressByteRun1(pData, ickEnd, iChOffset, pLine);
				}
			}

			// extra bitplane per "row" (see height)
			if (m_BmHeader.masking != mskNone)
			{
				// read masking
				/*
				UBYTE *pLine = new UBYTE[iRowBytes];
				::memcpy(pLine, pData, iRowBytes);
				iChOffset += iRowBytes;
				*/
			}
		}
	}
*/

}


void CIffIlbm::DecompressByteRun1(uint8_t *pData, const int64_t ickEnd, int64_t &iChOffset, UBYTE *pLine)
{
	//for (UWORD i = 0; i < m_BmHeader.w; i++)

	UWORD i = 0;
	while (i < m_BmHeader.w && iChOffset < ickEnd)
	{
		int8_t bMark = (int8_t)pData[iChOffset]; // compression flag
		uint8_t bNext = pData[iChOffset+1]; // byte to replicate or not

		if (bMark >= 0)
		{
			// copy next (bMark +1) bytes as-is
			int j = 0;
			while (j < bMark)
			{
				pLine[i] = pData[iChOffset+1];
				iChOffset++;
				i++;
				j++;
			}
		}
		else if (bMark < 0 && bMark >= -127)
		{
			// replicate bNext (-bMark +1) times

			int j = 0;
			while (j < (bMark*-1))
			{
				pLine[i] = bNext;
				//iChOffset++; // just replicate
				i++;
				j++;
			}

		}
		else if (bMark == -128)
		{
			// noop
			iChOffset++;
		}
	}
}

//////////////////// public methods

CIffIlbm::CIffIlbm(void)
	: CIffContainer()
	, m_File()
	, m_pHead(nullptr)
	, m_pCmap(nullptr)
{
}

CIffIlbm::~CIffIlbm(void)
{
	if (m_pCmap != nullptr)
	{
		delete m_pCmap;
		m_pCmap = nullptr;
	}
	if (m_pHead != nullptr)
	{
		delete m_pHead;
		m_pHead = nullptr;
	}
	m_File.Destroy();
}

bool CIffIlbm::ParseFile(LPCTSTR szPathName)
{
	if (m_File.Create(szPathName) == false)
	{
		return false;
	}

	m_pHead = ParseIffFile(m_File);
	if (m_pHead == nullptr)
	{
		return false;
	}

	// we expect ILBM-type IFF-file
	if (m_pHead->m_iFileID != MakeTag("ILBM"))
	{
		return false;
	}

	// TODO: make virtual methods in base
	// so that processing can be done on first-pass
	// through file instead of second here?

	CIffChunk *pChunk = m_pHead->m_pFirst;
	while (pChunk != nullptr)
	{
		// "raw" data of the chunk,
		// locate by offset
		uint8_t *pData = CIffContainer::GetViewByOffset(pChunk->m_iOffset, m_File);

		// suitable handling for chunk data..
		if (pChunk->m_iChunkID == MakeTag("BMHD"))
		{
			// BMHD bitmap header chunk:
			// should be first in file (according to spec)
			// and needed by later processing of data
			ParseBitmapHeader(pData, pChunk);
		}
		else if (pChunk->m_iChunkID == MakeTag("CMAP"))
		{
			// CMAP color map chunk

			int iCount = (pChunk->m_iChunkSize/sizeof(ColorRegister));
			m_pCmap = new ColorRegister[iCount];

			// bytes only, copy as-is: no need for byteswap
			::memcpy(m_pCmap, pData, pChunk->m_iChunkSize);
		}
		else if (pChunk->m_iChunkID == MakeTag("GRAB"))
		{
			// GRAB "hotspot" position (optional)
			// (e.g. brush center)
			Point2D Pt;
			Point2D *pPt = (Point2D*)pData;
			Pt.x = Swap2(pPt->x);
			Pt.y = Swap2(pPt->y);
		}
		else if (pChunk->m_iChunkID == MakeTag("DEST"))
		{
			// DEST (optional)
			DestMerge Dst;
			DestMerge *pDst = (DestMerge*)pData;
			Dst.depth = pDst->depth;
			Dst.pad1 = pDst->pad1;
			Dst.planePick = Swap2(pDst->planePick);
			Dst.planeOnOff = Swap2(pDst->planeOnOff);
			Dst.planeMask = Swap2(pDst->planeMask);
		}
		else if (pChunk->m_iChunkID == MakeTag("CRNG"))
		{
			// CRNG (optional), "nonstandard"
			// used by e.g. EA Deluxe Paint
		}
		else if (pChunk->m_iChunkID == MakeTag("CCRT"))
		{
			// CCRT (optional)
		}
		else if (pChunk->m_iChunkID == MakeTag("SPRT"))
		{
			// SPRT (optional),
			// when image intended as a sprite
			SpritePrecedence Sprt = Swap2((*(SpritePrecedence*)pData));
		}
		else if (pChunk->m_iChunkID == MakeTag("CAMG"))
		{
			// CAMG (optional),
			// Amiga "viewport mode"
			// e.g. "dual playfield", "hold and modify"
		}
		else if (pChunk->m_iChunkID == MakeTag("BODY"))
		{
			// BODY raster body chunk
			// content is concatenation of scan lines

			ParseBody(pData, pChunk);
		}

		pChunk = pChunk->m_pNext;
	}

	return true;
}

/* // TODO: if already loaded/mapped to memory
bool CIffIlbm::ParseMappedFile(uint8_t *pData, const uint64_t u64Size)
{
}
*/
