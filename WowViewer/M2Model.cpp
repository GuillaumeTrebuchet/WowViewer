#include "framework.h"
#include "M2Model.hpp"
#include "WSResourceManager.hpp"


void M2Model::LoadAnimations(const M2_header& hdr, IWSStream& file, uint32_t baseOfs)
{
	unsigned long BytesRead = 0;

	//	Read animation sequences
	std::vector<M2_AnimSequence> animSequences(hdr.nAnimations);

	file.Seek(hdr.ofsAnimations + baseOfs, WSSeekOrigin::Begin);
	file.Read(&animSequences[0], sizeof(M2_AnimSequence) * hdr.nAnimations);

	//	anim sequences lookup
	std::vector<INT16> animSequencesLookup(hdr.nAnimationLookup);

	if (hdr.nAnimationLookup > 0)
	{
		file.Seek(hdr.ofsAnimationLookup + baseOfs, WSSeekOrigin::Begin);
		file.Read(&animSequencesLookup[0], sizeof(UINT16) * hdr.nAnimationLookup);
	}

	//	Read bones
	std::vector<M2_Bone> bones(hdr.nBones);

	if (hdr.nBones > 0)
	{
		file.Seek(hdr.ofsBones + baseOfs, WSSeekOrigin::Begin);
		file.Read(&bones[0], sizeof(M2_Bone) * hdr.nBones);
	}

	//	bones lookup
	std::vector<UINT16> bonesLookup(hdr.nBoneLookupTable);

	if (hdr.nBoneLookupTable > 0)
	{
		file.Seek(hdr.ofsBoneLookupTable + baseOfs, WSSeekOrigin::Begin);
		file.Read(&bonesLookup[0], sizeof(UINT16) * hdr.nBoneLookupTable);
	}

	//	key bones lookup
	std::vector<UINT16> keyBonesLookup(hdr.nKeyBoneLookup);

	if (hdr.nKeyBoneLookup > 0)
	{
		file.Seek(hdr.ofsKeyBoneLookup + baseOfs, WSSeekOrigin::Begin);
		file.Read(&keyBonesLookup[0], sizeof(UINT16) * hdr.nKeyBoneLookup);
	}

	for (int i = 0; i < hdr.nAnimations; ++i)
	{
		m_animations.push_back(std::make_unique<M2Anim>(file, i, animSequences[i], bones, m_colors, m_alphas, baseOfs));
	}

}
void M2Model::LoadTextures(const M2_header& hdr, IWSStream& file, uint32_t baseOfs, std::span<uint32_t> txids)
{
	unsigned long BytesRead = 0;

	//	Read texture lookup
	m_textureLookup.resize(hdr.nTexLookup);
	file.Seek(hdr.ofsTexLookup + baseOfs, WSSeekOrigin::Begin);
	file.Read(&m_textureLookup[0], sizeof(UINT16) * hdr.nTexLookup);

	//	Read textures
	std::vector<M2_Texture> textures(hdr.nTextures);

	unsigned int textureCount = hdr.nTextures;

	file.Seek(hdr.ofsTextures + baseOfs, WSSeekOrigin::Begin);
	file.Read(&textures[0], sizeof(M2_Texture) * textureCount);

	m_pTextures.resize(textureCount);

	for (int i = 0; i < textureCount; ++i)
	{
		switch (textures[i].Type)
		{
		case 0:		//	Filename given
		{
			std::string filename;
			filename.resize(textures[i].lenFilename);

			/*file.Seek(textures[i].ofsFilename, WSSeekOrigin::Begin);
			file.Read(&filename[0], textures[i].lenFilename, &BytesRead);*/

			/*std::wstring filenameW = mystd::StringToWString(filename);

			IWSTexture* pTexture = m_resourceManager->LoadTexture(filenameW.c_str());
			m_pTextures[i] = pTexture;*/
			break;
		}
		case 11:
		case 12:
		case 13:
			//	In CreatureDisplayInfo.dbc

			//std::string modelPath = mystd::WStringToString(m_filename.substr(0, m_filename.find_last_of(L'\\') + 1));

			//DBC* pCreatureModelData = m_resourceManager->LoadDBC(L"DBFilesClient\\CreatureModelData.dbc");
			//DBC* pCreatureDisplayInfo = m_resourceManager->LoadDBC(L"DBFilesClient\\CreatureDisplayInfo.dbc");

			//std::string upperFileName = mystd::WStringToString(m_filename.substr(0, m_filename.find_last_of(L'.') + 1)) + "mdx";
			//mystd::ToUpper(upperFileName);

			////	Search for model ID
			//int modelID = -1;

			//for (int i = 0; i < pCreatureModelData->GetRecordCount(); ++i)
			//{
			//	std::string dbc_string = pCreatureModelData->GetRecord(i)[2].AsString();
			//	mystd::ToUpper(dbc_string);

			//	//	Check if model are same
			//	if (upperFileName == dbc_string)
			//	{
			//		modelID = pCreatureModelData->GetRecord(i)[0].AsInt();
			//		break;
			//	}
			//}

			//if (modelID == -1)
			//	throw std::exception();

			//std::string textureFileName;
			//int textureNbr = textures[i].Type - 10;

			//for (int i = 0; i < pCreatureDisplayInfo->GetRecordCount(); ++i)
			//{
			//	if (pCreatureDisplayInfo->GetRecord(i)[1].AsInt() == modelID)
			//	{
			//		textureFileName = modelPath + pCreatureDisplayInfo->GetRecord(i)[6 + textureNbr - 1].AsString() + ".blp";
			//		break;
			//	}
			//}

			//IWSTexture* pTexture = m_resourceManager->LoadTexture(mystd::StringToWString(textureFileName).c_str());
			//m_pTextures[i] = pTexture;

			//m_resourceManager->FreeDBC(pCreatureModelData);
			//m_resourceManager->FreeDBC(pCreatureDisplayInfo);
			break;
		}
	}
}
void M2Model::LoadVertices(const M2_header& hdr, IWSStream& file, uint32_t baseOfs)
{
	unsigned long BytesRead = 0;

	unsigned int vertexCount = hdr.nVertices;

	std::vector<M2_FileVertex> fileVertices(vertexCount);
	std::vector<M2Vertex> vertices(vertexCount);

	file.Seek(hdr.ofsVertices + baseOfs, WSSeekOrigin::Begin);
	file.Read(&fileVertices[0], sizeof(M2_FileVertex) * vertexCount);

	for (int i = 0; i < vertexCount; ++i)
	{
		vertices[i] =
		{
			XMFLOAT3(
			fileVertices[i].Position[1],
			fileVertices[i].Position[2],
			-fileVertices[i].Position[0]
			),

			XMFLOAT3(
			fileVertices[i].Normal[1],
			fileVertices[i].Normal[2],
			-fileVertices[i].Normal[0]),

			XMFLOAT2(fileVertices[i].TextureCoords),
			XMUBYTE4(fileVertices[i].BoneIndices),
			XMUBYTE4(fileVertices[i].BoneWeight),
		};
	}

	m_pVB = std::make_unique<ImmutableVertexBuffer<M2Vertex>>(m_pD3D11Device, vertices);
}
template<typename U, class Converter, typename T = decltype(std::declval<Converter>()(std::declval<U>()))>
M2Track<T> ReadKeyFrames(const M2FileTrack<U>& track, IWSStream& file, uint32_t baseOfs, Converter conv)
{
	M2Track<T> result;
	result.interpolation = (M2Interpolation)track.interpolation_type;

	std::vector<M2Array<uint32_t>> timestampsArray(track.timestamps.number);
	std::vector<M2Array<U>> valuesArray(track.timestamps.number);
	file.Seek(track.timestamps.offset_elements + baseOfs, WSSeekOrigin::Begin);
	file.Read(&timestampsArray[0], timestampsArray.size() * sizeof(M2Array<uint32_t>));
	file.Seek(track.values.offset_elements + baseOfs, WSSeekOrigin::Begin);
	file.Read(&valuesArray[0], valuesArray.size() * sizeof(M2Array<U>));

	for (int i = 0; i < track.timestamps.number; ++i)
	{
		std::vector<uint32_t> timestamps(timestampsArray[i].number);
		std::vector<U> values(valuesArray[i].number);

		if (timestamps.size() != values.size())
			throw std::exception();

		if (timestamps.size() > 0)
		{
			file.Seek(timestampsArray[i].offset_elements + baseOfs, WSSeekOrigin::Begin);
			file.Read(&timestamps[0], timestamps.size() * sizeof(uint32_t));
			file.Seek(valuesArray[i].offset_elements + baseOfs, WSSeekOrigin::Begin);
			file.Read(&values[0], values.size() * sizeof(U));
		}

		std::vector<M2KeyFrame<T>> keyframes(timestamps.size());
		for (int j = 0; j < timestamps.size(); ++j)
			keyframes[j] = { timestamps[j], conv(values[j]) };
		result.keyframes.push_back(std::move(keyframes));
	}

	return std::move(result);
}
template<typename U, typename T = U>
M2Track<T> ReadKeyFrames(const M2FileTrack<U>& track, IWSStream& file, uint32_t baseOfs)
{
	return ReadKeyFrames(track, file, baseOfs, [](U u) { return u; });
}
void M2Model::LoadColors(const M2_header& hdr, IWSStream& file, uint32_t baseOfs)
{
	std::vector<M2Color> colorTracks(hdr.nColors);
	if (colorTracks.size() > 0)
	{
		file.Seek(hdr.ofsColors + baseOfs, WSSeekOrigin::Begin);
		file.Read(&colorTracks[0], sizeof(M2Color) * colorTracks.size());

		for (auto itColor : colorTracks)
		{
			m_colors.push_back(ReadKeyFrames(itColor.Color, file, baseOfs));
			m_alphas.push_back(ReadKeyFrames(itColor.Alpha, file, baseOfs, [](int16_t i) { return i / (float)0x7FFF; }));
		}
	}

	std::vector<M2FileTrack<int16_t>> transparencyTracks(hdr.nTransparency);
	//file.Seek(hdr.ofsTransparency + baseOfs, WSSeekOrigin::Begin);
	//file.Read(&transparencyTracks[0], sizeof(M2Track<int16_t>) * transparencyTracks.size());
	
	for (auto itTrack : transparencyTracks)
	{
		//auto qzdqzd = ReadKeyFrames(itTrack, file, baseOfs, [](int16_t i) { return i / (float)0x7FFF; });
		//int zqdq = 0;
	}
}

void M2Model::ReadMD20(IWSStream& file, std::span<uint32_t> sfids, std::span<uint32_t> txids)
{
	M2_header hdr;
	uint32_t baseOfs = file.Tell();
	file.Seek(baseOfs, WSSeekOrigin::Begin);
	file.Read(&hdr, sizeof(M2_header));


	LoadVertices(hdr, file, baseOfs);
	LoadTextures(hdr, file, baseOfs, txids);
	//LoadShaders(hdr, file);
	LoadColors(hdr, file, baseOfs);
	LoadAnimations(hdr, file, baseOfs);
	LoadSkins();
}

M2Model::M2Model(ID3D11Device* pD3D11Device,
	std::string_view filename,
	IWSStream& file,
	WSResourceManager* resourceManager,
	VertexShader* vs,
	PixelShader* ps,
	VertexShader* pDebugBonesVS,
	PixelShader* pDebugBonesPS
)
	: m_pD3D11Device(pD3D11Device),
	m_vs(vs),
	m_ps(ps),
	m_resourceManager(resourceManager)
{
	uint64_t fileSize = file.GetLength();

	int64_t md21Ofs = -1;

	uint32_t ofs = 0;
	while (file.Tell() < fileSize)
	{
		ChunckHeader hdr;
		file.Seek(ofs, WSSeekOrigin::Begin);
		file.Read(&hdr, sizeof(hdr));
		if (memcmp(hdr.magic, "MD21", 4) == 0)
		{
			md21Ofs = file.Tell();
		}
		else if (memcmp(hdr.magic, "SFID", 4) == 0)
		{
			m_sfids.resize(hdr.size / sizeof(uint32_t));
			file.Read(&m_sfids[0], m_sfids.size() * sizeof(uint32_t));
		}
		else if (memcmp(hdr.magic, "TXID", 4) == 0)
		{
			m_txids.resize(hdr.size / sizeof(uint32_t));
			file.Read(&m_txids[0], m_txids.size() * sizeof(uint32_t));
		}
		ofs += hdr.size + sizeof(hdr);
	}

	if (md21Ofs != -1)
	{
		file.Seek(md21Ofs, WSSeekOrigin::Begin);
		ReadMD20(file, m_sfids, m_txids);
	}
	else
	{
		throw std::exception("non MD21 format not supported");
	}

}
M2Model::~M2Model()
{
}

void M2Model::LoadSkins()
{
	for (auto fileID : m_sfids)
	{
		m_skins.push_back(std::make_unique<M2Skin>(m_pD3D11Device, *m_resourceManager->OpenFile(fileID)));
	}
}

void M2Model::Draw(ID3D11DeviceContext* pDC,
	ConstantBuffer<M2_ConstantBuffer>& cb,
	ID3D11SamplerState* pSampler,
	ConstantBuffer<M2_AnimConstantBuffer>& animCB,
	int32_t animID,
	uint64_t animTime,
	std::span<IWSTexture*> textures,
	uint32_t skinID)
{
	UINT stride = sizeof(M2Vertex);
	UINT offset = 0;

	pDC->IASetInputLayout(m_vs->GetLayout());


	std::vector<XMFLOAT4> colors(m_colors.size());
	if (animID != -1 && animID < m_animations.size())
	{
		m_animations[animID]->UpdateAnimConstantBuffer(animTime, animCB);
		animCB.Update(pDC);
		for (auto i = 0; i < colors.size(); ++i)
			colors[i] = m_animations[animID]->GetColor(animTime, i);
	}

	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer*
		pVB = m_pVB->GetBuffer();

	pDC->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);

	ID3D11Buffer* pCB[] =
	{
		animCB.GetBuffer(),
	};

	pDC->VSSetConstantBuffers(1, 1, pCB);
	pDC->PSSetConstantBuffers(1, 1, pCB);

	pDC->VSSetShader(m_vs->GetShader(), NULL, 0);
	pDC->PSSetShader(m_ps->GetShader(), NULL, 0);

	pDC->PSSetSamplers(0, 1, &pSampler);

	m_skins[skinID]->Draw(pDC, cb, textures, m_textureLookup, colors);

}