#pragma once
#include "framework.h"

enum class WSSeekOrigin
	: DWORD
{
	Begin,
	Current,
	End,
};

class IWSStream
{
public:
	virtual uint64_t Read(VOID* buffer, DWORD count) = 0;
	virtual uint64_t Tell() = 0;
	virtual uint64_t Seek(int64_t position, WSSeekOrigin origin) = 0;
	virtual uint64_t GetLength() = 0;

	virtual ~IWSStream() = 0;
};

inline IWSStream::~IWSStream() {}