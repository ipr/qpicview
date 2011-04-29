//////////////////////////////////////////////////////////////////////////
//
// CMemoryMappedFile : simple wrapper-helper for 
// using memory-mapped files on Windows.
//
// (c) Ilkka Prusi, 2011
//

#include "MemoryMappedFile.h"


CMemoryMappedFile::CMemoryMappedFile(void)
	: m_hFile(INVALID_HANDLE_VALUE)
	, m_hMapping(NULL)
	, m_dwLastError(0)
	, m_i64FileSize(0)
	, m_pFileView(NULL)
{
}

CMemoryMappedFile::CMemoryMappedFile(LPCTSTR szPathName)
	: m_hFile(INVALID_HANDLE_VALUE)
	, m_hMapping(NULL)
	, m_dwLastError(0)
	, m_i64FileSize(0)
	, m_pFileView(NULL)
{
	Create(szPathName);
}

CMemoryMappedFile::~CMemoryMappedFile(void)
{
	Destroy();
}

// Open given file as memory-mapped
// for read-only access.
//
bool CMemoryMappedFile::Create(LPCTSTR szPathName)
{
	// previous mapping open?
	if (m_hFile != INVALID_HANDLE_VALUE
		|| m_hMapping != NULL)
	{
		// cleanup
		Destroy();
	}

    m_hFile = ::CreateFile(szPathName,
							GENERIC_READ,			// reading, no write/execute
							FILE_SHARE_READ,		// share for reading 
							NULL,					// security attributes (use default)
							OPEN_EXISTING,			// open existing file only 
							FILE_ATTRIBUTE_NORMAL,	// normal file attribs
							NULL);					// no attribs (not creating)
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		m_dwLastError = ::GetLastError();
		return false;
	}

	LARGE_INTEGER liFileSize;

	// get size (needs GENERIC_READ or GENERIC_WRITE)
	if (::GetFileSizeEx(m_hFile, &liFileSize) == FALSE)
	{
		m_dwLastError = ::GetLastError();
		return false;
	}

	// keep for user needs
	m_i64FileSize = liFileSize.QuadPart;

	// create new file mapping (not shared)
	//
	m_hMapping = ::CreateFileMapping(m_hFile, 
									NULL,			// security attributes (use default)
									PAGE_READONLY,	// protection: for reading only
									0,				// size of file (high-part), use maximum mapping
									0,				// size of file (low-part), use maximum mapping
									NULL);			// name of mapping object (use anonymous)
	if (m_hMapping == NULL)
	{
		m_dwLastError = ::GetLastError();
		return false;
	}

	// map file to process address space
	//
	m_pFileView = ::MapViewOfFile(m_hMapping, 
								FILE_MAP_READ,		// for reading only
								0,					// start at beginning (high-part of file-size)
								0,					// start at beginning (low-part of file-size)
								0);					// map entire file (not just part)
	if (m_pFileView == NULL)
	{
		m_dwLastError = ::GetLastError();
		return false;
	}
	// note: to get size of mapped view, use VirtualQuery()

	return true;
}

void CMemoryMappedFile::Destroy()
{
	// on destroying, check each in case of aborted loading
	// (avoid handle leakage).

	if (m_pFileView != NULL)
	{
		::UnmapViewOfFile(m_pFileView);
		m_pFileView = NULL;
	}

	if (m_hMapping != NULL)
	{
		::CloseHandle(m_hMapping);
		m_hMapping = NULL;
	}

	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	// also reset so we detect attempts to use after destroying..
	m_i64FileSize = 0;
}
