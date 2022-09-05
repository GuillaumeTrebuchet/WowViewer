#pragma once
#include "framework.h"

#include "WSResourceManager.hpp"

class CreatureInstance
{
	XMVECTOR m_position;
	std::shared_ptr<M2Model> m_model;
	std::vector<std::shared_ptr<IWSTexture>> m_textures;
	uint32_t m_geosetID = 0;

	int32_t m_animID = 1;
	uint64_t m_animStart = 0;


	uint32_t GetCreatureIDFromModelID(WSResourceManager& resourceManager, uint32_t modelID, uint32_t index = 0)
	{
		auto creatureDisplayInfoDb2 = resourceManager.LoadDB2(1108759);
		for (uint32_t i = 0; i < creatureDisplayInfoDb2->GetCount(); ++i)
		{
			auto r = creatureDisplayInfoDb2->GetRecord(i);
			if (r.GetField(1).AsUInt32() == modelID)
			{
				if (index == 0)
					return r.GetField(0).AsUInt32();
				--index;
			}
		}
		throw std::exception();
	}
public:

	CreatureInstance()
	{

	}

	void LoadFromDisplayInfo(WSResourceManager& resourceManager, uint32_t creatureID)
	{
		auto creatureDisplayInfoDb2 = resourceManager.LoadDB2(1108759);
		auto creatureModelDataDb2 = resourceManager.LoadDB2(1365368);
		auto r = creatureDisplayInfoDb2->GetRecordById(creatureID);
		uint32_t modelId = r.GetField(1).AsUInt32();
		auto textureIds = r.GetField(24).AsUInt32Array<3>();
		for (auto it : textureIds)
		{
			if (it != 0)
				m_textures.push_back(resourceManager.LoadTexture(it));
		}

		auto r2 = creatureModelDataDb2->GetRecordById(modelId);
		auto modelFileId = r2.GetField(3).AsUInt32();
		m_model = resourceManager.LoadModel(modelFileId);
		m_geosetID = 0;// r2.GetField(17).AsUInt32();
	}

	void LoadFromM2FileID(WSResourceManager& resourceManager, uint32_t fileID, uint32_t index = 0)
	{
		auto creatureModelDataDb2 = resourceManager.LoadDB2(1365368);
		for (uint32_t i = 0; i < creatureModelDataDb2->GetCount(); ++i)
		{
			auto r = creatureModelDataDb2->GetRecord(i);
			if (r.GetField(3).AsUInt32() == fileID)
			{
				if (index == 0)
				{
					LoadFromDisplayInfo(resourceManager, GetCreatureIDFromModelID(resourceManager, r.GetField(0).AsUInt32()));
					return;
				}
				--index;
			}
		}
		throw std::exception();
	}

	void Draw(ID3D11DeviceContext* pDC, ConstantBuffer<M2_ConstantBuffer>& cb, ID3D11SamplerState* pSampler, ConstantBuffer<M2_AnimConstantBuffer>& animCB, uint64_t time)
	{
		auto textures = std::ranges::views::transform(m_textures, [](auto p) { return p.get(); }) | to_vector();
		if (m_model)
			m_model->Draw(pDC, cb, pSampler, animCB, m_animID, time - m_animStart, textures, m_geosetID);
	}

	int GetAnimID()
	{
		return m_animID;
	}
	void SetAnimID(int id)
	{
		m_animID = id;
	}
};