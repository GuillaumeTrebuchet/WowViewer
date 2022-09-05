#pragma once
#include "framework.h"

#include "IWSTexture.hpp"
#include "IWSStream.hpp"

struct BLP_header
{
	/*0x00*/ UINT32	Magic;			//	always 'BLP2'
	/*0x04*/ UINT32	Version;		//	always 1
	/*0x08*/ UINT8	Compression;	// : 1 for uncompressed, 2 for DXTC, 3 (cataclysm) for plain A8R8G8B8 textures(see remarks)
	/*0x09*/ UINT8	AlphaDepth;		//	channel bit depth : 0, 1 or 8
	/*0x0A*/ UINT8	CompressionMode;//	0 : DXT1, 1 : DXT3, 7 : DXT5
	/*0x0B*/ UINT8	Unknown2;		//	when 0 there is only 1 mipmaplevel.compressed : 0, 1 or 2, uncompressed : 1 or 2 (mostly 1)
	/*0x0C*/ UINT32	Width;			//	resolution(power of 2)
	/*0x10*/ UINT32	Height;			//	Y resolution(power of 2)
	/*0x14*/ UINT32	MipmapOfs[16];	//	offsets for every mipmap level(or 0 when there is no more mipmap level)
	/*0x54*/ UINT32	MipmapSize[16];	//	sizes for every mipmap level(or 0 when there is no more mipmap level)
};

#define BLP_COMPRESSION_MODE_DXT1	0
#define BLP_COMPRESSION_MODE_DXT3	1
//#define BLP_COMPRESSION_MODE_DXT3	3
#define BLP_COMPRESSION_MODE_DXT5	7

class BlpTexture
	: public IWSTexture
{
	BlpTexture() = delete;
	BlpTexture(const BlpTexture&) = delete;
	BlpTexture& operator=(const BlpTexture&) = delete;

	CComPtr<ID3D11Device> m_pD3D11Device;

	CComPtr<ID3D11Texture2D> m_pTexture;
	CComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;

	void LoadDXTC(const BLP_header& hdr, IWSStream& file)
	{
		unsigned long BytesRead = 0;

		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		UINT memPitch = 0;

		std::vector<BYTE> data(hdr.MipmapSize[0]);

		switch (hdr.AlphaDepth)
		{
		case 0:
			format = DXGI_FORMAT_BC1_UNORM;
			memPitch = hdr.Width * 2;
			file.Seek(hdr.MipmapOfs[0], WSSeekOrigin::Begin);
			file.Read(&data[0], hdr.MipmapSize[0]);
			break;
		case 1:
			format = DXGI_FORMAT_BC1_UNORM;
			memPitch = hdr.Width * 2;
			file.Seek(hdr.MipmapOfs[0], WSSeekOrigin::Begin);
			file.Read(&data[0], hdr.MipmapSize[0]);
			break;
			/*case 4:
				format = DXGI_FORMAT_BC2_UNORM;
				memPitch = hdr.Width * 4;
				file.Seek(hdr.MipmapOfs[0], WSSeekOrigin::Begin);
				file.Read(&data[0], hdr.MipmapSize[0], &BytesRead);
				break;*/
		case 72:
		case 8:
			switch (hdr.CompressionMode)
			{
			case BLP_COMPRESSION_MODE_DXT3:
				format = DXGI_FORMAT_BC2_UNORM;
				memPitch = hdr.Width * 4;
				file.Seek(hdr.MipmapOfs[0], WSSeekOrigin::Begin);
				file.Read(&data[0], hdr.MipmapSize[0]);
				break;
			case BLP_COMPRESSION_MODE_DXT5:
				format = DXGI_FORMAT_BC3_UNORM;
				memPitch = hdr.Width * 4;
				file.Seek(hdr.MipmapOfs[0], WSSeekOrigin::Begin);
				file.Read(&data[0], hdr.MipmapSize[0]);
				break;
			default:
				throw std::exception("BLP CompressionMode invalid");
			}
			break;
		default:
			throw std::exception();
		}

		//	Create texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = hdr.Width;
		desc.Height = hdr.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subres_data = {};
		subres_data.pSysMem = &data[0];
		subres_data.SysMemPitch = memPitch;

		HRESULT hr = m_pD3D11Device->CreateTexture2D(&desc, &subres_data, &m_pTexture);

		// Create the shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		m_pD3D11Device->CreateShaderResourceView(m_pTexture, &shaderResourceViewDesc, &m_pShaderResourceView);
	}
	void LoadUncompressed(const BLP_header& hdr, IWSStream& file)
	{
		unsigned long BytesRead = 0;

		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		UINT memPitch = 0;

		std::vector<BYTE> compressedData(hdr.MipmapSize[0]);
		std::vector<BYTE> uncompressedData(hdr.Width * hdr.Height * 4);

		//	Read palette
		BYTE palette[256 * 4];
		file.Seek(sizeof(BLP_header), WSSeekOrigin::Begin);
		file.Read(palette, 256 * 4);

		//	Read data
		file.Seek(hdr.MipmapOfs[0], WSSeekOrigin::Begin);
		file.Read(&compressedData[0], hdr.MipmapSize[0]);

		switch (hdr.AlphaDepth)
		{
		case 0:
			format = DXGI_FORMAT_R8G8B8A8_UINT;
			memPitch = hdr.Width * 4;

			for (int i = 0; i < hdr.Width * hdr.Height; ++i)
			{
				uncompressedData[i * 4] = palette[compressedData[i] * 4 + 2];
				uncompressedData[i * 4 + 1] = palette[compressedData[i] * 4 + 1];
				uncompressedData[i * 4 + 2] = palette[compressedData[i] * 4];
				uncompressedData[i * 4 + 3] = 255;
			}
			break;
		case 1:
		{
			format = DXGI_FORMAT_R8G8B8A8_UINT;
			memPitch = hdr.Width * 4;

			int alphaOfs = hdr.Width * hdr.Height;
			for (int i = 0; i < hdr.Width * hdr.Height; ++i)
			{
				uncompressedData[i * 4] = palette[compressedData[i] * 4 + 2];
				uncompressedData[i * 4 + 1] = palette[compressedData[i] * 4 + 1];
				uncompressedData[i * 4 + 2] = palette[compressedData[i] * 4];

				int iAlpha = i / 8;
				int bitAlpha = i % 8;

				if ((compressedData[alphaOfs + iAlpha] >> bitAlpha) & 0x1)
					uncompressedData[i * 4 + 3] = 255;
				else
					uncompressedData[i * 4 + 3] = 0;
			}
			break;
		}
		case 4:
			format = DXGI_FORMAT_R8G8B8A8_UINT;
			memPitch = hdr.Width * 4;

			for (int i = 0; i < hdr.Width * hdr.Height; ++i)
			{
				uncompressedData[i * 4] = palette[compressedData[i] * 4 + 2];
				uncompressedData[i * 4 + 1] = palette[compressedData[i] * 4 + 1];
				uncompressedData[i * 4 + 2] = palette[compressedData[i] * 4];
				uncompressedData[i * 4 + 3] = 255;	//	broken
			}
			break;
		case 72:
		case 8:
		{
			format = DXGI_FORMAT_R8G8B8A8_UINT;
			memPitch = hdr.Width * 4;

			int alphaOfs = hdr.Width * hdr.Height;
			for (int i = 0; i < hdr.Width * hdr.Height; ++i)
			{
				uncompressedData[i * 4] = palette[compressedData[i] * 4 + 2];
				uncompressedData[i * 4 + 1] = palette[compressedData[i] * 4 + 1];
				uncompressedData[i * 4 + 2] = palette[compressedData[i] * 4];
				uncompressedData[i * 4 + 3] = compressedData[alphaOfs + i];	//	broken
			}
			break;
		}
		default:
			throw std::exception();
		}

		//	Create texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = hdr.Width;
		desc.Height = hdr.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subres_data = {};
		subres_data.pSysMem = &uncompressedData[0];
		subres_data.SysMemPitch = memPitch;

		HRESULT hr = m_pD3D11Device->CreateTexture2D(&desc, &subres_data, &m_pTexture);

		// Create the shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		m_pD3D11Device->CreateShaderResourceView(m_pTexture, &shaderResourceViewDesc, &m_pShaderResourceView);
	}
	void LoadA8R8G8B8(const BLP_header& hdr, IWSStream& file)
	{
		unsigned long BytesRead = 0;

		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UINT;
		UINT memPitch = hdr.Width * 4;

		std::vector<BYTE> compressedData(hdr.MipmapSize[0]);
		std::vector<BYTE> uncompressedData(hdr.Width * hdr.Height * 4);

		//	Read data
		file.Seek(hdr.MipmapOfs[0], WSSeekOrigin::Begin);
		file.Read(&compressedData[0], hdr.MipmapSize[0]);

		for (int i = 0; i < hdr.Width * hdr.Height; ++i)
		{
			uncompressedData[i * 4] = compressedData[i * 4 + 1];
			uncompressedData[i * 4 + 1] = compressedData[i * 4 + 2];
			uncompressedData[i * 4 + 2] = compressedData[i * 4 + 3];
			uncompressedData[i * 4 + 3] = compressedData[i * 4 + 0];
		}

		//	Create texture
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = hdr.Width;
		desc.Height = hdr.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subres_data = {};
		subres_data.pSysMem = &uncompressedData[0];
		subres_data.SysMemPitch = memPitch;

		HRESULT hr = m_pD3D11Device->CreateTexture2D(&desc, &subres_data, &m_pTexture);

		// Create the shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = desc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		m_pD3D11Device->CreateShaderResourceView(m_pTexture, &shaderResourceViewDesc, &m_pShaderResourceView);
	}
public:
	BlpTexture(ID3D11Device* pD3D11Device, IWSStream& file)
		: m_pD3D11Device(pD3D11Device)
	{
		unsigned long BytesRead = 0;
		BLP_header hdr;

		file.Seek(0, WSSeekOrigin::Begin);
		file.Read(&hdr, sizeof(BLP_header));

		if (hdr.Magic != '2PLB')
			throw std::exception("Wrong file format");

		switch (hdr.Compression)
		{
		case 1:		//	uncompressed
			LoadUncompressed(hdr, file);
			break;
		case 2:		//	DXTC
			LoadDXTC(hdr, file);
			break;
		case 3:		//	plain A8R8G8B8
			LoadA8R8G8B8(hdr, file);
			break;
		default:
			throw std::exception("BlpTexture hdr.Compression unknown");
		}
	}
	~BlpTexture()
	{

	}

	ID3D11ShaderResourceView* GetShaderResource()
	{
		return m_pShaderResourceView;
	}
};