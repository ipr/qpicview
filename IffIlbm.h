/////////////////////////////////////////////////////////
//
// CIffIlbm : IFF-ILBM image format parser
// (ILBM == InterLeaved BitMap)
//
// (c) Ilkka Prusi, 2011
//
// See format specs from: 
// http://www.fine-view.com/jp/labs/doc/ilbm.txt
//

#ifndef _IFFILBM_H_
#define _IFFILBM_H_

#include <stdint.h>

#include "MemoryMappedFile.h"
#include "IffContainer.h"


// support for old-style decl
//
typedef uint8_t UBYTE;
//typedef int16_t WORD;
typedef uint16_t UWORD;

/* Choice of masking technique. */
typedef UBYTE Masking; 	
#define mskNone	0
#define mskHasMask	1
#define mskHasTransparentColor	2
#define mskLasso	3

/* Choice of compression algorithm applied to the rows of all 
	* source and mask planes. "cmpByteRun1" is the byte run encoding 
	* described in Appendix C. Do not compress across rows! */
typedef UBYTE Compression;	
#define cmpNone	0
#define cmpByteRun1	1

// from format specifications,
// note one-byte alignment in struct

#pragma pack(push, 1)

//// BMHD
typedef struct {
	UWORD w, h;	/* raster width & height in pixels	*/
	WORD  x, y;	/* pixel position for this image	*/
	UBYTE nPlanes;	/* # source bitplanes	*/
	Masking masking; /* Choice of masking technique. */
	Compression compression;
	UBYTE pad1;	/* unused; for consistency, put 0 here	*/
	UWORD transparentColor;	/* transparent "color number" (sort of)	*/
	UBYTE xAspect, yAspect;	/* pixel aspect, a ratio width : height	*/
	WORD  pageWidth, pageHeight;	/* source "page" size in pixels	*/
	} BitMapHeader;

//// CMAP
typedef struct {
	UBYTE red, green, blue;	/* color intensities 0..255 */
	} ColorRegister;	/* size = 3 bytes */

typedef ColorRegister ColorMap[3];	/* size = 3n bytes */

typedef struct {
	unsigned pad1 :4, red :4, green :4, blue :4;
	} Color4;	/* Amiga RAM format. Not filed. */

//// GRAB
typedef struct {
	WORD x, y;	/* relative coordinates (pixels) */
	} Point2D;

//// DEST
typedef struct {
	UBYTE depth;	/* # bitplanes in the original source	*/
	UBYTE pad1;	/* unused; for consistency put 0 here	*/
	UWORD planePick; /* how to scatter source bitplanes into destination */
	UWORD planeOnOff;	/* default bitplane data for planePick	*/
	UWORD planeMask;	/* selects which bitplanes to store into */
	} DestMerge;

//// SPRT
typedef UWORD SpritePrecedence; /* relative precedence, 0 is the highest */

//// CRNG
typedef struct {
	WORD  pad1;	/* reserved for future use; store 0 here	*/
	WORD  rate;	/* color cycle rate	*/
	WORD  active;	/* nonzero means cycle the colors	*/
	UBYTE low, high; /* lower and upper color registers selected	*/
	} CRange;

//// CCRT
typedef struct {
	WORD  direction; /* 0 = don't cycle. 1 = cycle forwards (1, 2, 3).
		 	  * -1 = cycle backwards (3, 2, 1)	*/
	UBYTE start, end;  /* lower and upper color registers selected	*/
	LONG  seconds;	   /* # seconds between changing colors	*/
	LONG  microseconds;	/* # microseconds between changing colors */ 
	WORD  pad;		/* reserved for future use; store 0 here  */ 
	} CycleInfo;

#pragma pack(pop)


class CIffIlbm : public CIffContainer
{
private:
	CMemoryMappedFile m_File;
	CIffHeader *m_pHead;

protected:

	// bitmap header required to parse actual data in BODY-chunk
	BitMapHeader m_BmHeader;
	ColorRegister *m_pCmap;

	// virtual overload to generate IFF-ILBM chunk handlers
	//virtual CIffChunk *CreateChunkDesc(CIffHeader *pHead, uint32_t iChunkID)

	bool ParseBitmapHeader(uint8_t *pData, CIffChunk *pChunk);
	void ParseBody(uint8_t *pData, CIffChunk *pChunk);

	inline void DecompressByteRun1(uint8_t *pData, const int64_t ickEnd, int64_t &iChOffset, UBYTE *pLine);

public:
	CIffIlbm(void);
	virtual ~CIffIlbm(void);

	bool ParseFile(LPCTSTR szPathName);
	
	// TODO: if already loaded/mapped to memory
	//bool ParseMappedFile(uint8_t *pData, const uint64_t u64Size);
};

#endif // ifndef _IFFILBM_H_

