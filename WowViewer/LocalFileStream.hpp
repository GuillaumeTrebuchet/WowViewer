#pragma once
#include "framework.h"

#include "IWSStream.hpp"

class LocalFileStream
	: public IWSStream
{
	LocalFileStream() = delete;
	LocalFileStream(const LocalFileStream&) = delete;
	LocalFileStream& operator=(const LocalFileStream&) = delete;

	HANDLE m_hFile;
	std::wstring m_filename;

	void Close()
	{
		if (m_hFile)
		{
			CloseHandle(m_hFile);
			m_hFile = NULL;
		}
	}
public:
	LocalFileStream(std::string_view filename)
		: m_hFile(NULL)
	{
		m_hFile = CreateFileA(filename.data(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			throw std::exception();
	}
	LocalFileStream(std::wstring_view filename)
		: m_hFile(NULL),
		m_filename(filename)
	{
		m_hFile = CreateFileW(filename.data(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			throw std::exception();
	}
	~LocalFileStream()
	{
		Close();
	}

	uint64_t Seek(int64_t position, WSSeekOrigin origin)
	{
		switch (origin)
		{
		case WSSeekOrigin::Begin:
			return SetFilePointer(m_hFile, position, 0, FILE_BEGIN);
		case WSSeekOrigin::Current:
			return SetFilePointer(m_hFile, position, 0, FILE_CURRENT);
		case WSSeekOrigin::End:
			return SetFilePointer(m_hFile, position, 0, FILE_END);
		default:
			throw std::exception();
		}

	}
	uint64_t Tell()
	{
		return SetFilePointer(m_hFile, 0, nullptr, FILE_BEGIN);
	}

	uint64_t Read(VOID* buffer, DWORD count)
	{
		DWORD BytesRead = 0;
		bool b = ReadFile(m_hFile, buffer, count, &BytesRead, NULL);
		return BytesRead;
	}

	uint64_t GetLength()
	{
		return GetFileSize(m_hFile, NULL);
	}

	HRESULT GetFileName(wchar_t* pFileName, int length)
	{
		wcscpy_s(pFileName, length, &m_filename[0]);
		return S_OK;
	}
};