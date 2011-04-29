//////////////////////////////////
//
// CFileType : file type detection
//
// Ilkka Prusi, 2010
//

#pragma once

#ifndef FILETYPE_H
#define FILETYPE_H

#include <stdint.h>

#include <tchar.h>
#include <Windows.h>

#include <string>
//#include <wstring>
#include <map>
#include <vector>
using namespace std;


// type of file data
// according to header in file
//
typedef enum tHeaderType
{
	HEADERTYPE_UNKNOWN = 0, // indeterminate or unsupported
	
	// "container" types, indeterminate "payload"
	HEADERTYPE_IFF_GENERIC, // IFF-format 
	HEADERTYPE_RIFF_GENERIC, // RIFF-format (similar, MS-specific)

	// archive (file collection) types
	HEADERTYPE_LHA, // LHA-archive (multi-file)
	HEADERTYPE_LZX, // LZX-archive (multi-file)
	HEADERTYPE_RAR, // RAR-archive (multi-file)
	HEADERTYPE_ZIP, // PK-zip and variants, 0x04034B50
	//HEADERTYPE_TAR, // "tape-archive" (POSIX) (multi-file)
	//HEADERTYPE_ZOO, // 0xfdc4a7dc ?
	//HEADERTYPE_ARC, // 
	//HEADERTYPE_ARJ, // 
	
	// packer types
	HEADERTYPE_PP20, // Powerpacker
	HEADERTYPE_XPK_GENERIC, // XPK, multi-algorithm (SQSH etc.)
	HEADERTYPE_XPK_SQSH, // XPK
	HEADERTYPE_XPK_NUKE, // XPK
	HEADERTYPE_GZIP, // GNU-zip packer (gz)
	HEADERTYPE_BZIP2, // BZIP2 packer (bz2)
	HEADERTYPE_Z, // Un*X compress (Z)
	HEADERTYPE_7Z, // 7Z packer (7zip)
	HEADERTYPE_XZ, // XZ Utils packer (xz)

	// audio types, modules ("trackers")
	HEADERTYPE_MOD,
	HEADERTYPE_OCTAMED,	 // OctaMED (MMD0..3, MED2..4)
	HEADERTYPE_DBM,	 // Digibooster
	HEADERTYPE_DBPRO,	 // Digibooster PRO
	HEADERTYPE_SYMMOD,	 // Symphonie mod (SYMMOD)
	//HEADERTYPE_XM,
	//HEADERTYPE_S3M,
	//HEADERTYPE_S3I,
	
	// audio types, voice/sound
	HEADERTYPE_8SVX, // 8-bit sound "voice" (sample)(IFF)
	HEADERTYPE_WAVE, // wave-sound (RIFF)
	//HEADERTYPE_AIFF, // Apple AIFF audio (IFF-subtype)
	//HEADERTYPE_AU, // audacity audio-file
	//HEADERTYPE_SND,
	//HEADERTYPE_OGG,
	//HEADERTYPE_MP2,
	//HEADERTYPE_MP3,

	// video, animation
	HEADERTYPE_ANIM, // Amiga ANIM (IFF)
	HEADERTYPE_AVI, // MS-AVI video (RIFF)
	//HEADERTYPE_MOV, // Apple-Quicktime video
	//HEADERTYPE_MPEG, // 
	//HEADERTYPE_MATROSKA, // 
	
	// 3d-object types (3d-mesh)
	HEADERTYPE_LWOB, // older LWOB-format (IFF)
	HEADERTYPE_LWLO, // older LWOB with layers (IFF)
	HEADERTYPE_LWSC, // older LW scene (IFF)
	HEADERTYPE_LWO2, // newer LWO2-format (IFF)
	HEADERTYPE_TDDD, // Imagine TDDD-format (IFF)
	//HEADERTYPE_DXF,
	//HEADERTYPE_AUTOCAD,
	//HEADERTYPE_CINEMA4D,

	// picture types
	HEADERTYPE_ILBM, // ILBM-picture (IFF)
	//HEADERTYPE_JPEG_JFIF, // separate alternates?
	//HEADERTYPE_JPEG_EXIF, // separate alternates?
	//HEADERTYPE_JPEG_SPIFF, // separate alternates?
	HEADERTYPE_JPEG,
	HEADERTYPE_PNG,
	HEADERTYPE_GIF,
	HEADERTYPE_TIFF,
	//HEADERTYPE_TARGA,
	//HEADERTYPE_ADF_PIC, // somewhere there is this kind of "picture" file?
	
	// document types
	//HEADERTYPE_PDF,
	//HEADERTYPE_EPS, // encapsulated postscript

	// disk images
	HEADERTYPE_ADFDOS_DISK, // amiga disk file format, TODO: is there header signature or just booblock of disk?
	HEADERTYPE_DMS_DISK, // Diskmasher compressed disk file, 0x444D5321 == DMS! (big-endian)
	//HEADERTYPE_XDM_DISK, // xDM compressed disk file
	//HEADERTYPE_AMIGA_HARDFILE,
	//HEADERTYPE_ISOCD,
	//HEADERTYPE_MDF_CDIMAGE, //
	
	// executables
	HEADERTYPE_AMIGAOS, // AmigaOS loadseg()ble executable/binary
	HEADERTYPE_AMIGAOSLIB, // AmigaOS object/library data
	HEADERTYPE_MSDOSWINDOWS, // MS-DOS,OS/2,Windows
	HEADERTYPE_UNIXELF, // Un*x ELF

	// for future
	HEADERTYPE_RESERVED // (reserved for future use)
};

typedef enum tHeaderCategory
{
	HEADERCAT_UNKNOWN = 0, // indeterminate or unsupported

	HEADERCAT_PROGRAM, // ELF..
	
	// categories for type of data
	HEADERCAT_PICIMAGE, // ILBM/JPEG/TIFF..
	HEADERCAT_SOUND_MODULE, // 
	HEADERCAT_SOUND_VOICE, // 
	HEADERCAT_VIDEO_ANIM, // 
	HEADERCAT_ARCHIVE, // LHA/LZX/RAR/ZIP..
	HEADERCAT_PACKER, // PP/XPK/GZIP..
	HEADERCAT_FILEIMAGE, // ADF/DMS..
	HEADERCAT_3DOBJECT, // LWO?/TDDD/DXF..

	// "super-type" of format
	// on which actual format is based on,
	// more generic container type
	HEADERCAT_IFF,
	HEADERCAT_RIFF,
	HEADERCAT_AIFF
};


class CFileType
{
public:
	tHeaderType m_enFileType;
	tHeaderCategory m_enFileCategory;
	
	uint32_t Swap4(const uint32_t val) const
	{
		// 4-byte int byteswap
		return (
				((val & 0x000000FF) << 24) + ((val & 0x0000FF00) <<8) |
				((val & 0x00FF0000) >> 8) + ((val & 0xFF000000) >>24)
				);
	};
	
	//tHeaderType SubTypeFromHeader(const tHeaderType enType, const uint8_t *pBuffer, const uint32_t ulLength);
	
public:
    CFileType()
		: m_enFileType(HEADERTYPE_UNKNOWN)
		, m_enFileCategory(HEADERCAT_UNKNOWN)
	{}
			
    ~CFileType()
	{}

	tHeaderType FileTypeFromHeader(const uint8_t *pBuffer, const uint32_t ulLength) const;
	tHeaderCategory FileCategoryByType(const tHeaderType enType) const;

	void DetermineFileType(const uint8_t *pBuffer, const uint32_t ulLength);
	
	wstring GetNameOfType();
	wstring GetNameOfCategory();

	/*
	bool IsArchive() const
	{
		if (m_enFileCategory == HEADERCAT_ARCHIVE
			|| m_enFileCategory == HEADERCAT_PACKER)
		{
			return true;
		}
		return false;
	}
	*/

};

#endif // FILETYPE_H
