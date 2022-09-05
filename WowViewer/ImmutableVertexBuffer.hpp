#pragma once
#include "framework.h"

template <class T>
class ImmutableVertexBuffer
{
	ImmutableVertexBuffer() = delete;
	ImmutableVertexBuffer(const ImmutableVertexBuffer&) = delete;
	ImmutableVertexBuffer& operator=(const ImmutableVertexBuffer&) = delete;

	CComPtr<ID3D11Device> m_pD3D11Device;
	CComPtr<ID3D11Buffer> m_pBuffer;
	UINT	m_count;
public:
	ImmutableVertexBuffer(ID3D11Device* pD3D11Device, std::span<T> data)
		: m_pD3D11Device(pD3D11Device),
		m_count(data.size())
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(T) * data.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = &data[0];

		pD3D11Device->CreateBuffer(&bd, &InitData, &m_pBuffer);
	}
	ImmutableVertexBuffer(ID3D11Device* pD3D11Device, T* data, int size)
		: m_pD3D11Device(pD3D11Device),
		m_count(size)
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(T) * size;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = data;

		pD3D11Device->CreateBuffer(&bd, &InitData, &m_pBuffer);
	}

	~ImmutableVertexBuffer()
	{
	}

	ID3D11Buffer* GetBuffer()
	{
		return m_pBuffer;
	}

	UINT GetStride()
	{
		return sizeof(T);
	}

	UINT GetCount()
	{
		return m_count;
	}
};