#pragma once
#include "framework.h"

#include "CascFile.hpp"

class CascStorageEnumerator
{
	CascStorageEnumerator() = delete;
	CascStorageEnumerator(const CascStorageEnumerator&) = delete;
	CascStorageEnumerator& operator=(const CascStorageEnumerator&) = delete;

public:
	/*CascStorageEnumerator()
	{

	}*/


};
class CascStorage
{
	CascStorage() = delete;
	CascStorage(const CascStorage&) = delete;
	CascStorage& operator=(const CascStorage&) = delete;

	HANDLE m_handle = nullptr;
public:
	CascStorage(std::wstring_view path)
	{
		if (!CascOpenStorageEx(path.data(), nullptr, false, &m_handle))
			throw std::exception();
	}

	~CascStorage()
	{
		if (m_handle)
		{
			CascCloseStorage(m_handle);
			m_handle = nullptr;
		}
	}

	std::unique_ptr<IWSStream> OpenFile(std::string_view filename)
	{
		return std::make_unique<CascFile>(m_handle, filename);
	}

	std::unique_ptr<IWSStream> OpenFile(uint32_t fileID)
	{
		return std::make_unique<CascFile>(m_handle, fileID);
	}

};