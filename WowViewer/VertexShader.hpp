#pragma once
#include "framework.h"

#include "IWSStream.hpp"

class VertexShader
{
	VertexShader() = delete;
	VertexShader(const VertexShader&) = delete;
	VertexShader& operator=(const VertexShader&) = delete;

	CComPtr<ID3D11Device> m_pD3D11Device;

	CComPtr<ID3D11VertexShader> m_pShader;
	CComPtr<ID3D11InputLayout> m_pLayout;

public:
	VertexShader(ID3D11Device* pD3D11Device, IWSStream& file, ID3D11InputLayout* pLayout)
		: m_pD3D11Device(pD3D11Device),
		m_pLayout(pLayout)
	{
		uint64_t size = file.GetLength();

		std::vector<BYTE> pBuffer(size);

		DWORD BytesRead = 0;

		file.Seek(0, WSSeekOrigin::Begin);
		file.Read(&pBuffer[0], size);

		HRESULT hr = pD3D11Device->CreateVertexShader(&pBuffer[0], size, NULL, &m_pShader);
		if (FAILED(hr))
			throw std::exception("VertexShader CreateVertexShader failed");
	}
	VertexShader(ID3D11Device* pD3D11Device, IWSStream& file, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layoutDesc)
		: m_pD3D11Device(pD3D11Device)
	{
		uint64_t size = file.GetLength();

		std::vector<BYTE> pBuffer(size);

		DWORD BytesRead = 0;

		file.Seek(0, WSSeekOrigin::Begin);
		file.Read(&pBuffer[0], size);

		HRESULT hr = pD3D11Device->CreateVertexShader(&pBuffer[0], size, NULL, &m_pShader);
		if (FAILED(hr))
			throw std::exception("VertexShader CreateVertexShader failed");

		hr = pD3D11Device->CreateInputLayout(&layoutDesc[0], layoutDesc.size(), &pBuffer[0],
			size, &m_pLayout);
		if (FAILED(hr))
			throw std::exception("VertexShader CreateInputLayout failed");

	}
	~VertexShader()
	{

	}

	ID3D11VertexShader* GetShader()
	{
		return m_pShader;
	}
	ID3D11InputLayout* GetLayout()
	{
		return m_pLayout;
	}
};