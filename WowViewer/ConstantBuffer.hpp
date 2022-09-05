#pragma once
#include "framework.h"


template <class T>
class ConstantBuffer
{
	ConstantBuffer() = delete;
	ConstantBuffer(const ConstantBuffer&) = delete;
	ConstantBuffer& operator=(const ConstantBuffer&) = delete;

	CComPtr<ID3D11Device> m_pD3D11Device;
	CComPtr<ID3D11Buffer> m_pBuffer;
	T* pCB;

public:
	T& cb;

	ConstantBuffer(ID3D11Device* pD3D11Device)
		: m_pD3D11Device(pD3D11Device),
		pCB((T*)_aligned_malloc(sizeof(T), 16)),
		cb(*pCB)
	{
		ZeroMemory(pCB, sizeof(T));

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;

		UINT rest = sizeof(T) % 16;
		if (rest)
			bd.ByteWidth = sizeof(T) + (16 - rest);
		else
			bd.ByteWidth = sizeof(T);

		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;

		pD3D11Device->CreateBuffer(&bd, NULL, &m_pBuffer);
	}
	ConstantBuffer(ID3D11Device* pD3D11Device, T& cb)
		: pCB(_aligned_malloc(sizeof(T), 16)),
		cb(*pCB)
	{
		memcpy(pCB, &cb, sizeof(T));

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;

		UINT rest = sizeof(T) % 16;
		if (rest)
			bd.ByteWidth = sizeof(T) + (16 - rest);
		else
			bd.ByteWidth = sizeof(T);

		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = pCB;

		pD3D11Device->CreateBuffer(&bd, &InitData, &m_pBuffer);
	}
	~ConstantBuffer()
	{
		_aligned_free(pCB);
	}

	void Update(ID3D11DeviceContext* pDC, int start, int end)
	{
		D3D11_BOX box = {};
		box.left = start;
		box.right = end;

		pDC->UpdateSubresource(m_pBuffer, 0, &box, &cb, 0, 0);
	}
	void Update(ID3D11DeviceContext* pDC)
	{
		pDC->UpdateSubresource(m_pBuffer, 0, NULL, &cb, 0, 0);
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
		return 1;
	}
};