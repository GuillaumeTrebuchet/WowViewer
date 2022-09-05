#pragma once
#include "framework.h"

#include "IWSStream.hpp"

#include "ImmutableIndexBuffer.hpp"

#include "IWSTexture.hpp"
#include "IWSModel.hpp"

struct M2_Skin_header
{
	/*0x00*/	UINT32	Magic;				//		SKIN
	/*0x04*/	UINT32	nIndices;			//
	/*0x08*/	UINT32	ofsIndices;			//	Indices used in this View.
	/*0x0C*/	UINT32	nTriangles;			//
	/*0x10*/	UINT32	ofsTriangles;		//	The triangles made with them.
	/*0x14*/	UINT32	nProperties;		//
	/*0x18*/	UINT32	ofsProperties;		//	Properties of the vertices.
	/*0x1C*/	UINT32	nSubmeshes;			//
	/*0x20*/	UINT32	ofsSubmeshes;		//	Submeshes(Geosets) of this View.
	/*0x24*/	UINT32	nTextureUnits;		//
	/*0x28*/	UINT32	ofsTextureUnits;	//	Texture Units.
	/*0x2C*/	UINT32	bones;				//		WoW takes this and divides it by the number of bones in each submesh, then stores the biggest one.
};

struct M2_Skin_submesh
{
	/*0x00*/ 	UINT32	ID;						//	Mesh part ID, see below.
	/*0x04*/ 	UINT16	StartVertex;			//	Starting vertex number.
	/*0x06*/ 	UINT16	nVertices;				//	Number of vertices.
	/*0x08*/ 	UINT16	StartTriangle;			//	Starting triangle index(that's 3* the number of triangles drawn so far).
	/*0x0A*/ 	UINT16	nTriangles;				//	Number of triangle indices.
	/*0x0C*/ 	UINT16	nBones;					//	Number of elements in the bone lookup table.
	/*0x0E*/ 	UINT16	StartBones;				//	Starting index in the bone lookup table.
	/*0x10*/ 	UINT16	Unknown;				//
	/*0x12*/ 	UINT16	RootBone;				//	Not sure.
	/*0x14*/ 	float	CenterMass[3];			//	Average position of all the vertices in the submesh.
	/*0x20*/ 	float	CenterBoundingBox[3];	//	The center of the box when an axis aligned box is built around the vertices in the submesh.
	/*0x2C*/ 	float	Radius;					//	Distance of the vertex farthest from CenterBoundingBox.
};

struct M2Batch
{
	uint8_t flags;                       // Usually 16 for static textures, and 0 for animated textures. &0x1: materials invert something; &0x2: transform &0x4: projected texture; &0x10: something batch compatible; &0x20: projected texture?; &0x40: possibly don't multiply transparency by texture weight transparency to get final transparency value(?)
	int8_t priorityPlane;
	uint16_t shader_id;                  // See below.
	uint16_t skinSectionIndex;           // A duplicate entry of a submesh from the list above.
	uint16_t geosetIndex;                // See below. New name: flags2. 0x2 - projected. 0x8 - EDGF chunk in m2 is mandatory and data from is applied to this mesh
	int16_t colorIndex;                 // A Color out of the Colors-Block or -1 if none.
	uint16_t materialIndex;              // The renderflags used on this texture-unit.
	uint16_t materialLayer;              // Capped at 7 (see CM2Scene::BeginDraw)
	uint16_t textureCount;               // 1 to 4. See below. Also seems to be the number of textures to load, starting at the texture lookup in the next field (0x10).
	uint16_t textureComboIndex;          // Index into Texture lookup table
	uint16_t textureCoordComboIndex;     // Index into the texture unit lookup table.
	uint16_t textureWeightComboIndex;    // Index into transparency lookup table.
	uint16_t textureTransformComboIndex; // Index into uvanimation lookup table. 
};
class M2Skin
{
	M2Skin() = delete;
	M2Skin(const M2Skin&) = delete;
	M2Skin& operator=(const M2Skin&) = delete;

	CComPtr<ID3D11Device> m_pD3D11Device;

	std::unique_ptr<ImmutableIndexBuffer> m_pIB;

	std::vector<M2_Skin_submesh> m_submeshes;
	std::vector<M2Batch> m_textures;

	void LoadTextures(const M2_Skin_header& hdr, IWSStream& file)
	{
		unsigned long BytesRead = 0;

		m_textures.resize(hdr.nTextureUnits);

		//	Read indices
		file.Seek(hdr.ofsTextureUnits, WSSeekOrigin::Begin);
		file.Read(&m_textures[0], sizeof(M2Batch) * hdr.nTextureUnits);
	}
	void LoadSubmeshes(const M2_Skin_header& hdr, IWSStream& file)
	{
		unsigned long BytesRead = 0;

		m_submeshes.resize(hdr.nSubmeshes);

		//	Read indices
		file.Seek(hdr.ofsSubmeshes, WSSeekOrigin::Begin);
		file.Read(&m_submeshes[0], sizeof(M2_Skin_submesh) * hdr.nSubmeshes);
	}
	void LoadIndices(const M2_Skin_header& hdr, IWSStream& file)
	{
		unsigned long BytesRead = 0;

		std::vector<UINT16> fileIndices(hdr.nIndices);
		std::vector<UINT16> fileTriangles(hdr.nTriangles);

		//	True indices that will be used to make index buffer
		std::vector<UINT16> indices(hdr.nTriangles);

		//	Read indices
		file.Seek(hdr.ofsIndices, WSSeekOrigin::Begin);
		file.Read(&fileIndices[0], sizeof(UINT16) * hdr.nIndices);

		//	Read triangles
		file.Seek(hdr.ofsTriangles, WSSeekOrigin::Begin);
		file.Read(&fileTriangles[0], sizeof(UINT16) * hdr.nTriangles);

		for (int i = 0; i < hdr.nTriangles / 3; ++i)
		{
			indices[i * 3] = fileIndices[fileTriangles[i * 3]];
			indices[i * 3 + 1] = fileIndices[fileTriangles[i * 3 + 1]];
			indices[i * 3 + 2] = fileIndices[fileTriangles[i * 3 + 2]];
		}

		m_pIB = std::make_unique<ImmutableIndexBuffer>(m_pD3D11Device, indices);
	}
public:
	M2Skin(ID3D11Device* pD3D11Device, IWSStream& file)
		: m_pD3D11Device(pD3D11Device),
		m_pIB(nullptr)
	{
		unsigned long BytesRead = 0;

		M2_Skin_header hdr;

		file.Seek(0, WSSeekOrigin::Begin);
		file.Read(&hdr, sizeof(M2_Skin_header));

		LoadIndices(hdr, file);
		LoadSubmeshes(hdr, file);
		LoadTextures(hdr, file);

	}
	~M2Skin()
	{
	}

	void Draw(ID3D11DeviceContext* pDC,
		ConstantBuffer<M2_ConstantBuffer>& cb,
		std::span<IWSTexture*> textures,
		std::span<uint16_t> textureLookup,
		std::span<XMFLOAT4> colors)
	{

		for (int i = 0; i < m_submeshes.size(); ++i)
		{
			std::array<ID3D11ShaderResourceView*, 30> pSRV;

			uint32_t textureCount = 0;
			for (int iTex = 0; iTex < m_textures.size(); ++iTex)
			{
				if (m_textures[iTex].skinSectionIndex == i)
				{
					if (textureLookup[m_textures[iTex].textureComboIndex] >= textures.size())
						continue;

					IWSTexture* pTex = textures[textureLookup[m_textures[iTex].textureComboIndex]];
					pSRV[i] = pTex->GetShaderResource();
					if (m_textures[iTex].colorIndex == -1)
						cb.cb.Color[textureCount] = XMFLOAT4(1, 1, 1, 1);
					else
						cb.cb.Color[textureCount] = colors[m_textures[iTex].colorIndex];
					++textureCount;
				}
			}

			cb.cb.textureCount = textureCount;
			cb.Update(pDC);

			ID3D11Buffer* pCB[] =
			{
				cb.GetBuffer(),
			};

			pDC->VSSetConstantBuffers(0, 1, pCB);
			pDC->PSSetConstantBuffers(0, 1, pCB);

			pDC->VSSetShaderResources(0, textureCount, &pSRV[0]);
			pDC->PSSetShaderResources(0, textureCount, &pSRV[0]);

			pDC->IASetIndexBuffer(m_pIB->GetBuffer(), DXGI_FORMAT_R16_UINT, 0);
			pDC->DrawIndexed(m_submeshes[i].nTriangles, m_submeshes[i].StartTriangle, 0);
		}
	}
};