//////////////////////////////////
//
// CFileType : file type detection
//
// Ilkka Prusi, 2010
//

#include "FileType.h"

/*
tHeaderType CFileType::SubTypeFromHeader(const tHeaderType enType, const uint8_t *pBuffer, const uint32_t ulLength)
{
	if (enType == HEADERTYPE_XPK_GENERIC)
	{
		// determine compression type
	}
	
	// give generic type if sub-type is not known
	return enType;
}
*/

tHeaderType CFileType::FileTypeFromHeader(const uint8_t *pBuffer, const uint32_t ulLength) const
{
	// invalid/small buffer/file
	// (file should have at least 16 bytes to have any meaningful data..)
	tHeaderType enFileType = HEADERTYPE_UNKNOWN;
	if (pBuffer == NULL
		|| ulLength < 16)
	{
		return HEADERTYPE_UNKNOWN;
	}
	
	// note: start with types which have least possibility of conflict
	// such as IFF "FORM", 
	// add ambigious types to end (such as only two-byte identifiers).

	// try to determine by string at start..
	if (::memcmp(pBuffer, "FORM", 4) == 0)
	{
		// EA-IFF format file header:
		// handle additional info for actual type,
		// values in file in big-endian format
	    enFileType = HEADERTYPE_IFF_GENERIC;

		const uint8_t *pTmp = (pBuffer +8);
		if (::memcmp(pTmp, "LWOB", 4) == 0)
		{
			// Lightwave, older (<6.5)
			return HEADERTYPE_LWOB;
		}
		else if (::memcmp(pTmp, "LWLO", 4) == 0)
		{
			// Lightwave, older (<6.5) with layers
			return HEADERTYPE_LWLO;
		}
		else if (::memcmp(pTmp, "LWSC", 4) == 0)
		{
			// Lightwave, older (<6.5) scene-format
			return HEADERTYPE_LWSC;
		}
		else if (::memcmp(pTmp, "LWO2", 4) == 0)
		{
			// Lightwave, newer (>=6.5)
			return HEADERTYPE_LWO2;
		}
		else if (::memcmp(pTmp, "TDDD", 4) == 0)
		{
			// Imagine
			return HEADERTYPE_TDDD;
		}
		else if (::memcmp(pTmp, "ILBM", 4) == 0)
		{
			return HEADERTYPE_ILBM;
		}
		else if (::memcmp(pTmp, "8SVX", 4) == 0)
		{
			// 8SVX-audio
			return HEADERTYPE_8SVX;
		}
		else if (::memcmp(pTmp, "ANIM", 4) == 0)
		{
			// Amiga ANIM
			return HEADERTYPE_ANIM;
		}
		return enFileType;
	}
	else if (::memcmp(pBuffer, "XPKF", 4) == 0)
	{
		// XPK-packer (generic packer),
		// determine actual packer..
	    enFileType = HEADERTYPE_XPK_GENERIC;

		const uint8_t *pTmp = (pBuffer +8);
		if (::memcmp(pTmp, "SQSH", 4) == 0)
		{
			return HEADERTYPE_XPK_SQSH;
		}
		else if (::memcmp(pTmp, "NUKE", 4) == 0)
		{
			return HEADERTYPE_XPK_NUKE;
		}
		return enFileType;
	}
	else if (::memcmp(pBuffer, "PP20", 4) == 0)
	{
		// powerpacker
		return HEADERTYPE_PP20;
	}
	else if (::memcmp(pBuffer, "LZX", 3) == 0)
	{
		// LZX archive
		return HEADERTYPE_LZX;
	}
	else if (::memcmp(pBuffer, "Rar", 3) == 0)
	{
		//0x52 0x61 0x72 0x21 0x1a 0x07 0x00
		// Rar-archive
		return HEADERTYPE_RAR;
	}
	else if (::memcmp(pBuffer, "MMD", 3) == 0
			 || ::memcmp(pBuffer, "MED", 3) == 0)
	{
		// OctaMED
		// MMD0..3
		// MED2..4
		return HEADERTYPE_OCTAMED;
	}
	else if (::memcmp(pBuffer, "DIGI", 4) == 0)
	{
		// digibooster module
		return HEADERTYPE_DBM;
	}
	else if (::memcmp(pBuffer, "DBM0", 4) == 0)
	{
		// digibooster pro module
		return HEADERTYPE_DBPRO;
	}
	else if (::memcmp(pBuffer, "SymM", 4) == 0)
	{
		// symphonie module
		return HEADERTYPE_SYMMOD;
	}
	else if (::memcmp(pBuffer, "RIFF", 4) == 0)
	{
		// Microsoft/IBM counterpart to EA-IFF format:
		// values in file in little-endian format
	    enFileType = HEADERTYPE_RIFF_GENERIC;
	    
	    const uint8_t *pTmp = (pBuffer +8);
	    if (::memcmp(pTmp, "WAVE", 4) == 0)
	    {
			enFileType = HEADERTYPE_WAVE;
	    }
	    else if (::memcmp(pTmp, "AVI ", 4) == 0)
		{
			enFileType = HEADERTYPE_AVI;
		}
		/*
	    else if (::memcmp(pTmp, "CDDA", 4) == 0)
		{}
		*/
		return enFileType;
	}
	/*
	else if (::memcmp(pBuffer, "AIFF", 4) == 0)
	{
		// Apple counterpart to EA-IFF-format
		// note: has "FORM" identifier at start?
		return HEADERTYPE_AIFF;
	}
	*/
	else if (::memcmp(pBuffer, "DOS", 3) == 0)
	{
		// AmigaDOS-disk?
		enFileType = HEADERTYPE_ADFDOS_DISK;
		/*
		if (pBuffer[3] == 0)
		{
			// OFS
		}
		else if (pBuffer[3] == 1)
		{
			// FFS
		}
		*/
		return enFileType;
	}

	if (pBuffer[0] == 0x42
		&& pBuffer[1] == 0x5A)
	{
		// "BZ"
		// variations: "BZ0", "BZh1".."BZh9"
		return HEADERTYPE_BZIP2;
	}
	else if (pBuffer[0] == 0x1F
		&& pBuffer[1] == 0x8B
		&& pBuffer[2] == 0x08)
	{
		return HEADERTYPE_GZIP;
	}
	else if (pBuffer[0] == 0x1F
		&& pBuffer[1] == 0x9D)
	{
		return HEADERTYPE_Z;
	}
	/*
	else if (pBuffer[0] == 0x1F
		&& pBuffer[1] == 0x0A)
	{
		// LHA-compressed TAR?
		//enFileType = ;
	}
	*/
	else if (pBuffer[0] == 0x4D
		&& pBuffer[1] == 0x5A)
	{
		// executable
		return HEADERTYPE_MSDOSWINDOWS;
	}
	else if (pBuffer[0] == 0xFF
		 && pBuffer[1] == 0xD8
		 && pBuffer[2] == 0xFF)
	{
		// next byte should be one of: E0, E1, E8 ?
		// also, should follow with "JFIF" ?
		return HEADERTYPE_JPEG;
	}
	else if (pBuffer[0] == '7'
			 && pBuffer[1] == 'z'
			 && pBuffer[2] == 0xBC
			 && pBuffer[3] == 0xAF
			 && pBuffer[4] == 0x27
			 && pBuffer[5] == 0x1C)
	{
		return HEADERTYPE_7Z;
	}
	else if (pBuffer[0] ==  0xFD
			 && pBuffer[1] == '7'
			 && pBuffer[2] == 'z'
			 && pBuffer[3] == 'X'
			 && pBuffer[4] == 'Z'
			 && pBuffer[5] == 0x00)
	{
		return HEADERTYPE_XZ;
	}
	
	// try to determine from first four bytes.. (in case "magic number" as non-ASCII chars)
	// note: needs byteswap..
	//
	uint32_t ulFirstFour = Swap4((*((uint32_t*)pBuffer)));
	if (ulFirstFour == 0x444D5321)
	{
		// note: big-endian
		// 0x444D5321 // DMS! (big-endian)
		return HEADERTYPE_DMS_DISK;
	}
	else if (ulFirstFour == 0x504B0304)
	{
		// "PK"
		return HEADERTYPE_ZIP;
	}
	else if (ulFirstFour == 0x89504E47)
	{
		return HEADERTYPE_PNG;
	}
	else if (ulFirstFour == 0x47494638)
	{
		enFileType = HEADERTYPE_GIF;
		// sub-types of GIF?
		/*
		if (::memcmp(pBuffer, "GIF87a", 6) == 0)
		{}
		else if (::memcmp(pBuffer, "GIF89a", 6) == 0)
		{}
		*/
		return enFileType;
	}
	// note of TIFF: may also be 49492A00 ?
	else if (ulFirstFour == 0x4D4D002A)
	{
		// big-endian TIFF
		return HEADERTYPE_TIFF;
	}
	else if (ulFirstFour == 0x49492A00)
	{
		// little-endian TIFF
		return HEADERTYPE_TIFF;
	}
	else if (ulFirstFour == 0x7F454C46)
	{
		// executable (multi-platform)
		return HEADERTYPE_UNIXELF;
	}
	else if (ulFirstFour == 0x000003F3)
	{
		return HEADERTYPE_AMIGAOS;
	}
	else if (ulFirstFour == 0x000003E7)
	{
		return HEADERTYPE_AMIGAOSLIB;
	}

	/*
	// TAR (POSIX)	.tar	75 73 74 61 72	ustar (offset by 257 bytes)
	*/


	// otherwise, if no match, look deeper (e.g. LHA starts after first two bytes)
	// also, lha has multiple variants (H0..H5, ZS, Z5, Z4)
	char *pData = (char*)(&pBuffer[2]);

	// alternate to reduce comparisons 
	// (when different packing options are used)
	// can be: -lh0- .. -lh7-
	// or -lz0- .. -lz7- or -lzs-
	// or others..
	if (pData[4] == '-')
	{
		if (::memcmp(pData, "-lh", 3) == 0
			|| ::memcmp(pData, "-lz", 3) == 0)
		{
			return HEADERTYPE_LHA;
		}
	}

	return enFileType;
}

// temp..
tHeaderCategory CFileType::FileCategoryByType(const tHeaderType enType) const
{
	switch (enType)
	{
	case HEADERTYPE_LHA:
	case HEADERTYPE_LZX:
	case HEADERTYPE_RAR:
	case HEADERTYPE_ZIP:
	//case HEADERTYPE_TAR:
	//case HEADERTYPE_ZOO:
	//case HEADERTYPE_ARC:
	//case HEADERTYPE_ARJ:
		return HEADERCAT_ARCHIVE;
		
	case HEADERTYPE_PP20:
	case HEADERTYPE_XPK_GENERIC:
	case HEADERTYPE_XPK_SQSH:
	case HEADERTYPE_XPK_NUKE:
	case HEADERTYPE_GZIP:
	case HEADERTYPE_BZIP2:
	case HEADERTYPE_Z:
	case HEADERTYPE_7Z:
	case HEADERTYPE_XZ:
		return HEADERCAT_PACKER;
		
	case HEADERTYPE_MOD:
	case HEADERTYPE_OCTAMED:
	case HEADERTYPE_DBM:
	case HEADERTYPE_DBPRO:
	case HEADERTYPE_SYMMOD:
	//case HEADERTYPE_XM:
	//case HEADERTYPE_S3M:
	//case HEADERTYPE_S3I:
		return HEADERCAT_SOUND_MODULE;
		
	case HEADERTYPE_8SVX:
	case HEADERTYPE_WAVE:
	//case HEADERTYPE_AIFF:
	//case HEADERTYPE_AU:
	//case HEADERTYPE_SND:
	//case HEADERTYPE_OGG:
	//case HEADERTYPE_MP2:
	//case HEADERTYPE_MP3:
		return HEADERCAT_SOUND_VOICE;
		
	case HEADERTYPE_ANIM:
	case HEADERTYPE_AVI:
	//case HEADERTYPE_MOV:
	//case HEADERTYPE_MPEG:
	//case HEADERTYPE_MATROSKA:
		return HEADERCAT_VIDEO_ANIM;
		
	case HEADERTYPE_LWOB:
	case HEADERTYPE_LWLO:
	case HEADERTYPE_LWSC:
	case HEADERTYPE_LWO2:
	case HEADERTYPE_TDDD:
	//case HEADERTYPE_DXF:
	//case HEADERTYPE_AUTOCAD:
	//case HEADERTYPE_CINEMA4D:
		return HEADERCAT_3DOBJECT;
		
	case HEADERTYPE_ILBM:
	case HEADERTYPE_JPEG:
	case HEADERTYPE_PNG:
	case HEADERTYPE_GIF:
	case HEADERTYPE_TIFF:
		return HEADERCAT_PICIMAGE;
		
	case HEADERTYPE_ADFDOS_DISK:
	case HEADERTYPE_DMS_DISK:
	//case HEADERTYPE_XDM_DISK:
		return HEADERCAT_FILEIMAGE;
		
	case HEADERTYPE_AMIGAOS:
	case HEADERTYPE_AMIGAOSLIB:
	case HEADERTYPE_MSDOSWINDOWS:
	case HEADERTYPE_UNIXELF:
		return HEADERCAT_PROGRAM;
		
	}
	return HEADERCAT_UNKNOWN;
}

void CFileType::DetermineFileType(const uint8_t *pBuffer, const uint32_t ulLength)
{
	// determine type from header (if possible)
	if (ulLength >= 12)
	{
		// when at least header should be availalbe,
		// see if file-type can be determined from it
		m_enFileType = FileTypeFromHeader(pBuffer, ulLength);
		m_enFileCategory = FileCategoryByType(m_enFileType);
	}
}

wstring CFileType::GetNameOfType()
{
	switch (m_enFileType)
	{
	case HEADERTYPE_LHA:
		return _T("LhA");
	case HEADERTYPE_LZX:
		return _T("LZX");
	case HEADERTYPE_PP20:
		return _T("PowerPacker");
	case HEADERTYPE_XPK_GENERIC:
		return _T("XPK");
	case HEADERTYPE_XPK_SQSH:
		return _T("XPK (SQSH)");
	case HEADERTYPE_XPK_NUKE:
		return _T("XPK (NUKE)");

	case HEADERTYPE_RAR:
		return _T("RAR");
	case HEADERTYPE_ZIP:
		return _T("PK-Zip");
	case HEADERTYPE_GZIP:
		return _T("GNU-Zip");
	case HEADERTYPE_BZIP2:
		return _T("BZip2");
	case HEADERTYPE_7Z:
		return _T("7-Zip");
	case HEADERTYPE_XZ:
		return _T("XZ");
	case HEADERTYPE_Z:
		return _T("Z-compress");
		
		/*
	case HEADERTYPE_OCTAMED:
		return _T("OctaMED");
	case HEADERTYPE_DBM:
		return _T("Digibooster");
	case HEADERTYPE_DBPRO:
		return _T("Digibooster PRO");
	case HEADERTYPE_SYMMOD:
		return _T("Symphonie");
		
	case HEADERTYPE_ADFDOS_DISK:
		return _T("AmigaDOS disk-file");
	case HEADERTYPE_DMS_DISK:
		return _T("DiskMasher disk-file");
	//case HEADERTYPE_XDM_DISK:
		//return _T("xDM disk-file");
		*/
	}
	
	return wstring();
}

wstring CFileType::GetNameOfCategory()
{
	switch (m_enFileCategory)
	{
	case HEADERCAT_ARCHIVE:
		return _T("Archive");
	case HEADERCAT_PACKER:
		return _T("Packer/Compressor");
		/*
	case HEADERCAT_SOUND_MODULE:
		return _T("Sound module");
	case HEADERCAT_SOUND_VOICE:
		return _T("Sound voice");
	case HEADERCAT_VIDEO_ANIM:
		return _T("Video/animation");
	case HEADERCAT_3DOBJECT:
		return _T("3D-Mesh object");
	case HEADERCAT_PICIMAGE:
		return _T("Picture-image");
	case HEADERCAT_FILEIMAGE:
		return _T("File-image");
	case HEADERCAT_PROGRAM:
		return _T("Program binary or library");
		*/
	}
	return wstring();
}

