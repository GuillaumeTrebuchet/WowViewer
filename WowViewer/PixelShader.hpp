#pragma once
#include "framework.h"

#include "IWSStream.hpp"

class PixelShader
{
	PixelShader() = delete;
	PixelShader(const PixelShader&) = delete;
	PixelShader& operator=(const PixelShader&) = delete;

	CComPtr<ID3D11Device> m_pD3D11Device;

	CComPtr<ID3D11PixelShader> m_pShader;

public:
	PixelShader(ID3D11Device* pD3D11Device, IWSStream& file)
		: m_pD3D11Device(pD3D11Device)
	{
		uint64_t size = file.GetLength();

		std::vector<BYTE> pBuffer(size);

		DWORD BytesRead = 0;

		file.Seek(0, WSSeekOrigin::Begin);
		file.Read(&pBuffer[0], size);

		HRESULT hr = pD3D11Device->CreatePixelShader(&pBuffer[0], size, NULL, &m_pShader);
		if (FAILED(hr))
			throw std::exception("VertexShader CreatePixelShader failed");
	}
	~PixelShader()
	{

	}

	ID3D11PixelShader* GetShader()
	{
		return m_pShader;
	}
};