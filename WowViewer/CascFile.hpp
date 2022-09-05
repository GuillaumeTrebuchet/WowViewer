#pragma once
#include "framework.h"

#include "IWSStream.hpp"

static inline std::string WStringToString(const std::wstring& wstr)
{
	BOOL usedDefaultChar = FALSE;
	int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), nullptr, 0, NULL, &usedDefaultChar);
	std::string s;
	s.resize(size);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), s.data(), s.size(), NULL, &usedDefaultChar);
	return s;
}

class CascFile
	: public IWSStream
{
	CascFile() = delete;
	CascFile(const CascFile&) = delete;
	CascFile& operator=(const CascFile&) = delete;

	HANDLE m_handle = nullptr;
public:
	CascFile(HANDLE hStorage, std::string_view filename)
	{
		if (!CascOpenFile(hStorage, filename.data(), NULL, CASC_OPEN_BY_NAME, &m_handle))
			throw std::exception();
	}
	CascFile(HANDLE hStorage, uint32_t fileID)
	{
		if (!CascOpenFile(hStorage, CASC_FILE_DATA_ID(fileID), NULL, CASC_OPEN_BY_FILEID, &m_handle))
			throw std::exception();
	}

	~CascFile()
	{
		if (m_handle)
		{
			CascCloseFile(m_handle);
			m_handle = nullptr;
		}
	}
	virtual uint64_t Read(VOID* buffer, DWORD count)
	{
		DWORD bytesRead = 0;
		if (!CascReadFile(m_handle, buffer, count, &bytesRead))
			throw std::exception();
		return bytesRead;
	}
	virtual uint64_t Tell()
	{
		ULONGLONG pos = 0;
		if (!CascSetFilePointer64(m_handle, 0, &pos, FILE_CURRENT))
			throw std::exception();
		return pos;
	}
	virtual uint64_t Seek(int64_t position, WSSeekOrigin origin)
	{
		ULONGLONG pos = 0;
		DWORD dir = 0;
		switch (origin)
		{
		case WSSeekOrigin::Begin:
			dir = FILE_BEGIN;
			break;
		case WSSeekOrigin::Current:
			dir = FILE_CURRENT;
			break;
		case WSSeekOrigin::End:
			dir = FILE_END;
			break;
		default:
			throw std::exception();
		}
		if (!CascSetFilePointer64(m_handle, position, &pos, dir))
			throw std::exception();
		return pos;
	}
	virtual uint64_t GetLength()
	{
		ULONGLONG size = 0;
		if (!CascGetFileSize64(m_handle, &size))
			throw std::exception();
		return size;
	}

};