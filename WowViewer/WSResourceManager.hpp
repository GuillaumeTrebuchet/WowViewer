#pragma once
#include "framework.h"

#include "IWSStream.hpp"

#include "BlpTexture.hpp"

#include "M2Model.hpp"

#include "LocalFileStream.hpp"
#include "CascStorage.hpp"

#include "db2.hpp"

class WSResourceManager
{
	WSResourceManager() = delete;
	WSResourceManager(const WSResourceManager&) = delete;
	WSResourceManager& operator=(const WSResourceManager&) = delete;

	CComPtr<ID3D11Device> m_pD3D11Device;

	std::unordered_map<uint32_t, std::weak_ptr<IWSTexture>> m_textureList;
	std::unordered_map<uint32_t, std::weak_ptr<M2Model>> m_modelList;
	std::unordered_map<uint32_t, std::weak_ptr<db2>> m_db2List;

	std::unique_ptr<VertexShader>	m_pM2_VS;
	std::unique_ptr<PixelShader>	m_pM2_PS;
	std::unique_ptr<VertexShader>	m_pM2_DebugBonesVS;
	std::unique_ptr<PixelShader>	m_pM2_DebugBonesPS;

	void LoadShaders()
	{
		//	M2 shaders
		/*std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		m_pM2_VS = std::make_unique<VertexShader>(m_pD3D11Device, (IWSStream&)LocalFileStream(L"vs_debug.cso"), layout);
		m_pM2_PS = std::make_unique<PixelShader>(m_pD3D11Device, (IWSStream&)LocalFileStream(L"ps_debug.cso"));*/
		std::vector<D3D11_INPUT_ELEMENT_DESC> M2_layoutDesc =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		m_pM2_VS = std::make_unique<VertexShader>(m_pD3D11Device, (IWSStream&)LocalFileStream(L"M2_vs.cso"), M2_layoutDesc);
		m_pM2_PS = std::make_unique<PixelShader>(m_pD3D11Device, (IWSStream&)LocalFileStream(L"M2_ps.cso"));
	}
	std::unique_ptr<CascStorage> m_storage;
public:
	WSResourceManager(ID3D11Device* pD3D11Device)
		: m_pD3D11Device(pD3D11Device)
	{
	}

	void Initialize(std::wstring_view storagePath)
	{
		LoadShaders();
		m_storage = std::make_unique<CascStorage>(storagePath);
	}

	std::unique_ptr<IWSStream> OpenFile(std::string_view filename)
	{
		try
		{
			return m_storage->OpenFile(filename);
		}
		catch (std::exception)
		{
			return std::make_unique<LocalFileStream>(filename);
		}
	}

	std::unique_ptr<IWSStream> OpenFile(uint32_t fileID)
	{
		return m_storage->OpenFile(fileID);
	}

	std::shared_ptr<IWSTexture> LoadTexture(uint32_t fileID)
	{
		auto found = m_textureList.find(fileID);
		if (found != m_textureList.end())
			return std::shared_ptr(found->second);

		auto file = OpenFile(fileID);
		std::shared_ptr<BlpTexture> texture(new BlpTexture(m_pD3D11Device, *file),
			[this, fileID](BlpTexture* t)
			{
				m_textureList.erase(fileID);
				delete t;
			});
		m_textureList.insert(std::pair(fileID, texture));
		return texture;
	}

	std::shared_ptr<db2> LoadDB2(uint32_t fileID)
	{
		auto found = m_db2List.find(fileID);
		if (found != m_db2List.end())
			return std::shared_ptr(found->second);

		auto file = OpenFile(fileID);
		std::shared_ptr<db2> db(new db2(*file),
			[this, fileID](db2* p)
		{
			m_db2List.erase(fileID);
			delete p;
		});
		m_db2List.insert(std::pair(fileID, db));
		return db;
	}

	std::shared_ptr<M2Model> LoadModel(uint32_t fileID)
	{
		auto found = m_modelList.find(fileID);
		if (found != m_modelList.end())
			return std::shared_ptr(found->second);

		auto file = OpenFile(fileID);
		std::shared_ptr<M2Model> model(new M2Model(m_pD3D11Device, "", *file, this, m_pM2_VS.get(), m_pM2_PS.get(), nullptr, nullptr),
			[this, fileID](M2Model* p)
		{
			m_modelList.erase(fileID);
			delete p;
		});
		m_modelList.insert(std::pair(fileID, model));
		return model;
	}
};