//////////////////////////////////////////////////////////////////////////
//
// CMemoryMappedFile : simple wrapper-helper for 
// using memory-mapped files on Windows.
//
// (c) Ilkka Prusi, 2011
//
// Example usage:
//
// 1) opening view of file
//
// CMemoryMappedFile DataFile("C:\\mydata.bin");
// if (DataFile.IsCreated() == false) 
//		throw std::exception();
// 
// 2) handling data
// 
// int64_t nOffset = 0;
// while (nOffset < DataFile.GetSize())
// {
//		BYTE *pData = (BYTE*)DataFile.GetView();
//		value = pData[nOffset];
// }
//
// 3) releasing resources is automatic on destroying object
//

#ifndef _MEMORYMAPPEDFILE_H_
#define _MEMORYMAPPEDFILE_H_

// note: if not stdint.h header is not in SDK, use:
//
//typedef __int64 int64_t;
//
#include <stdint.h>

// note: needs both _UNICODE and UNICODE definitions
// for proper wide-char handling
//
#include <tchar.h>
#include <Windows.h>

class CMemoryMappedFile
{
private:

	HANDLE m_hFile;
	HANDLE m_hMapping;
	
	DWORD m_dwLastError;

	// keep size of file
	//
	int64_t m_i64FileSize;

	// view of file
	//
	LPVOID m_pFileView;

public:
	CMemoryMappedFile(void);
	CMemoryMappedFile(LPCTSTR szPathName);
	virtual ~CMemoryMappedFile(void);

	bool Create(LPCTSTR szPathName);
	void Destroy();

	bool IsCreated()
	{
		if (m_i64FileSize > 0
			&& m_pFileView != NULL)
		{
			return true;
		}
		return false;
	}

	int64_t GetSize()
	{
		return m_i64FileSize;
	}

	void *GetView()
	{
		return m_pFileView;
	}

};

#endif // _MEMORYMAPPEDFILE_H_


