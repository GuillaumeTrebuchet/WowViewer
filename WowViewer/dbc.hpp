#pragma once
#include "framework.h"

#include "IWSStream.hpp"

struct DBC_header
{
	UINT32 Magic;				// always 'WDBC'
	UINT32 RecordCount;			// records per file
	UINT32 FieldCount;			// fields per record
	UINT32 RecordSize;			// sum (sizeof (field_type_i)) | 0 <= i < field_count. field_type_i is NOT defined in the files.
	UINT32 StringTableSize;
};


class DBC_Field
{
	void* m_pBuffer;
	const char* m_pStringTable;
public:
	DBC_Field(void* pBuffer, const char* pStringTable)
		: m_pBuffer(pBuffer),
		m_pStringTable(pStringTable)
	{

	}

	int AsInt()
	{
		return *(int*)m_pBuffer;
	}
	unsigned int AsUInt()
	{
		return *(unsigned int*)m_pBuffer;
	}
	std::string AsString()
	{
		int ofs = AsInt();
		return &m_pStringTable[ofs];
	}
	float AsFloat()
	{
		return *(float*)m_pBuffer;
	}
};

class DBC_Record
{
	void* m_pBuffer;
	const char* m_pStringTable;
public:
	DBC_Record(void* pBuffer, const char* pStringTable)
		: m_pBuffer(pBuffer),
		m_pStringTable(pStringTable)
	{

	}

	inline DBC_Field GetField(int i)
	{
		return DBC_Field((void*)((unsigned long)m_pBuffer + i * 4), m_pStringTable);
	}
	inline DBC_Field operator[](int i)
	{
		return GetField(i);
	}
};

class DBC
{
	DBC() = delete;
	DBC(const DBC&) = delete;
	DBC& operator=(const DBC&) = delete;

	std::vector<BYTE> m_pBuffer;

	DBC_header* m_pHdr;
	const char* m_pStringTable;
public:
	DBC(IWSStream& file)
		: m_pBuffer(0),
		m_pHdr(nullptr),
		m_pStringTable(nullptr)
	{
		DWORD fileSize = file.GetLength();

		m_pBuffer.resize(fileSize);

		file.Seek(0, WSSeekOrigin::Begin);
		file.Read(&m_pBuffer[0], fileSize);

		m_pHdr = reinterpret_cast<DBC_header*>(&m_pBuffer[0]);

		unsigned long stringTableOfs = m_pHdr->RecordSize * m_pHdr->RecordCount + sizeof(DBC_header);
		m_pStringTable = reinterpret_cast<const char*>(&m_pBuffer[stringTableOfs]);
	}

	inline DBC_Record GetRecord(int i)
	{
		return DBC_Record((void*)(((unsigned long)&m_pBuffer[0]) + sizeof(DBC_header) + i * m_pHdr->RecordSize), m_pStringTable);
	}
	inline DBC_Record operator[](int i)
	{
		return GetRecord(i);
	}

	unsigned int GetRecordCount()
	{
		return m_pHdr->RecordCount;
	}
};