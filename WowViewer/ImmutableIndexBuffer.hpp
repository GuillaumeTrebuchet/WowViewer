#pragma once
#include "framework.h"

class ImmutableIndexBuffer
{
	ImmutableIndexBuffer() = delete;
	ImmutableIndexBuffer(const ImmutableIndexBuffer&) = delete;
	ImmutableIndexBuffer& operator=(const ImmutableIndexBuffer&) = delete;

	CComPtr<ID3D11Device> m_pD3D11Device;
	CComPtr<ID3D11Buffer> m_pBuffer;
	UINT	m_count;
public:
	ImmutableIndexBuffer(ID3D11Device* pD3D11Device, std::span<uint16_t> data)
		: m_pD3D11Device(pD3D11Device),
		m_count(data.size())
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint16_t) * data.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = &data[0];

		pD3D11Device->CreateBuffer(&bd, &InitData, &m_pBuffer);
	}
	ImmutableIndexBuffer(ID3D11Device* pD3D11Device, const uint16_t* data, int size)
		: m_pD3D11Device(pD3D11Device),
		m_count(size)
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint16_t) * size;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = data;

		pD3D11Device->CreateBuffer(&bd, &InitData, &m_pBuffer);
	}
	~ImmutableIndexBuffer()
	{
	}


	ID3D11Buffer* GetBuffer()
	{
		return m_pBuffer;
	}

	UINT GetStride()
	{
		return sizeof(UINT16);
	}
	UINT GetCount()
	{
		return m_count;
	}
};