#pragma once
#include "framework.h"

#include "ImmutableVertexBuffer.hpp"
#include "ImmutableIndexBuffer.hpp"
#include "ConstantBuffer.hpp"

#include "IWSStream.hpp"

#include "M2Skin.hpp"
#include "M2Anim.hpp"

#include "VertexShader.hpp"
#include "PixelShader.hpp"

#include "IWSModel.hpp"

struct ChunckHeader
{
	char magic[4];
	uint32_t size;
};
struct M2_header
{
	/*0x000*/	UINT32	Magic;					//	 "MD20"
	/*0x004*/	UINT32	Version;				//	 For Cataclysm this is 263 < version < 272. Files get handled differently depending on this!
	/*0x008*/	UINT32	lName;					//	 Length of the model's name including the trailing \0
	/*0x00C*/	UINT32	ofsName;				//	Offset to the name, it seems like models can get reloaded by this name.should be unique, i guess.
	/*0x010*/	UINT32	GlobalModelFlags;		//	 1: tilt x, 2 : tilt y, 4 : , 8 : add another field in header, 16 : ; (no other flags as of 3.1.1); list at M2 / WotLK / flags; *(p + 0x11) & 1 is camera related.
	/*0x014*/	UINT32	nGlobalSequences;		//
	/*0x018*/	UINT32	ofsGlobalSequences;		//	A list of timestamps.
	/*0x01C*/	UINT32	nAnimations;			//
	/*0x020*/	UINT32	ofsAnimations;			//	Information about the animations in the model.
	/*0x024*/	UINT32	nAnimationLookup;		//
	/*0x028*/	UINT32	ofsAnimationLookup;		//	Mapping of global IDs to the entries in the Animation sequences block.
	/*0x02C*/	UINT32	nBones;					//
	/*0x030*/	UINT32	ofsBones;				//	Information about the bones in this model.
	/*0x034*/	UINT32	nKeyBoneLookup;			//
	/*0x038*/	UINT32	ofsKeyBoneLookup;		//	Lookup table for key skeletal bones.
	/*0x03C*/	UINT32	nVertices;				//
	/*0x040*/	UINT32	ofsVertices;			//	Vertices of the model.
	/*0x044*/	UINT32	nViews;					//	Views(LOD) are now in.skins.
	/*0x048*/	UINT32	nColors;				//
	/*0x04C*/	UINT32	ofsColors;				//	 Color definitions.
	/*0x050*/	UINT32	nTextures;				//
	/*0x054*/	UINT32	ofsTextures;			//	Textures of this model.
	/*0x058*/	UINT32	nTransparency;			//
	/*0x05C*/	UINT32	ofsTransparency;		//	Transparency of textures.
	/*0x060*/	UINT32	nUVAnimation;			//
	/*0x064*/	UINT32	ofsUVAnimation;			//
	/*0x068*/	UINT32	nTexReplace;			//
	/*0x06C*/	UINT32	ofsTexReplace;			//	Replaceable Textures.
	/*0x070*/	UINT32	nRenderFlags;			//
	/*0x074*/	UINT32	ofsRenderFlags;			//	Blending modes / render flags.
	/*0x078*/	UINT32	nBoneLookupTable;		//
	/*0x07C*/	UINT32	ofsBoneLookupTable;		//	A bone lookup table.
	/*0x080*/	UINT32	nTexLookup;				//
	/*0x084*/	UINT32	ofsTexLookup;			//	The same for textures.
	/*0x088*/	UINT32	nTexUnits;				//
	/*0x08C*/	UINT32	ofsTexUnits;			//	And texture units.Somewhere they have to be too.
	/*0x090*/	UINT32	nTransLookup;			//
	/*0x094*/	UINT32	ofsTransLookup;			//	Everything needs its lookup.Here are the transparencies.
	/*0x098*/	UINT32	nUVAnimLookup;			//
	/*0x09C*/	UINT32	ofsUVAnimLookup;		//
	/*0x0A0*/	float	VertexBox[3][2];		//	 min / max([1].z, 2.0277779f) - 0.16f seems to be the maximum camera height
	/*0x0B8*/	float	VertexRadius;			//
	/*0x0BC*/	float	BoundingBox[3][2];		//
	/*0x0D4*/	float	BoundingRadius;			//
	/*0x0D8*/	UINT32	nBoundingTriangles;		//
	/*0x0DC*/	UINT32	ofsBoundingTriangles;	//	Our bounding volumes.Similar structure like in the old ofsViews.
	/*0x0E0*/	UINT32	nBoundingVertices;		//
	/*0x0E4*/	UINT32	ofsBoundingVertices;	//
	/*0x0E8*/	UINT32	nBoundingNormals;		//
	/*0x0EC*/	UINT32	ofsBoundingNormals;		//
	/*0x0F0*/	UINT32	nAttachments;			//
	/*0x0F4*/	UINT32	ofsAttachments;			//	Attachments are for weapons etc.
	/*0x0F8*/	UINT32	nAttachLookup;			//
	/*0x0FC*/	UINT32	ofsAttachLookup;		//	Of course with a lookup.
	/*0x100*/	UINT32	nEvents;				//
	/*0x104*/	UINT32	ofsEvents;				//	Used for playing sounds when dying and a lot else.
	/*0x108*/	UINT32	nLights;				//
	/*0x10C*/	UINT32	ofsLights;				//	Lights are mainly used in loginscreens but in wands and some doodads too.
	/*0x110*/	UINT32	nCameras;				//	 Format of Cameras changed with version 271!
	/*0x114*/	UINT32	ofsCameras;				//	The cameras are present in most models for having a model in the Character - Tab.
	/*0x118*/	UINT32	nCameraLookup;			//
	/*0x11C*/	UINT32	ofsCameraLookup;		//	And lookup - time again.
	/*0x120*/	UINT32	nRibbonEmitters;		//
	/*0x124*/	UINT32	ofsRibbonEmitters;		//	Things swirling around.See the CoT - entrance for light - trails.
	/*0x128*/	UINT32	nParticleEmitters;		//
	/*0x12C*/	UINT32	ofsParticleEmitters;	//	Spells and weapons, doodads and loginscreens use them.Blood dripping of a blade ? Particles.
	/*0x130*/	UINT32	nUnknown;				//	 This field is getting added in models with the 8 - flag only.If that flag is not set, this field does not exist!
	/*0x134*/	UINT32	ofsUnknown;				//	 It has a array out of shorts.Its related to renderflags.
};
struct M2_FileVertex
{
	/*0x00*/	float	Position[3];		//A vector to the position of the vertex.
	/*0x0C*/	UINT8	BoneWeight[4];		//The vertex weight for 4 bones.
	/*0x10*/	UINT8	BoneIndices[4];		//Which are referenced here.
	/*0x14*/	float	Normal[3];			//A normal vector.
	/*0x20*/	float	TextureCoords[2];	//Coordinates for a texture.
	/*0x28*/	float	Unknown[2];			//Null? 
};

struct M2_Texture
{
	/*0x000*/	UINT32	Type;			//	 The type of the texture.See below.
	/*0x004*/	UINT32	Flags;			//	Textures have some flags.See below too.
	/*0x008*/	UINT32	lenFilename;	//	Here is the length of the filename, if the type is not "0 - hardcoded" then it's just 1 and points to a zero
	/*0x00C*/	UINT32	ofsFilename;	//	And the offset to the filename.
};


//	D3D11
struct M2Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoords;
	XMUBYTE4 BoneIndices;
	XMUBYTE4 BoneWeight;
};

struct M2Color
{
	M2FileTrack<XMFLOAT3> Color;
	M2FileTrack<int16_t> Alpha;
};
class WSResourceManager;

class M2Model
{
	M2Model() = delete;
	M2Model(const M2Model&) = delete;
	M2Model& operator=(const M2Model&) = delete;

	CComPtr<ID3D11Device> m_pD3D11Device;

	WSResourceManager* m_resourceManager = nullptr;

	std::vector<IWSTexture*> m_pTextures;
	std::vector<UINT16> m_textureLookup;

	std::unique_ptr<ImmutableVertexBuffer<M2Vertex>> m_pVB;

	VertexShader* m_vs = nullptr;
	PixelShader* m_ps = nullptr;

	std::vector<std::unique_ptr<M2Anim>> m_animations;

	std::vector<M2Track<XMFLOAT3>> m_colors;
	std::vector<M2Track<float>> m_alphas;

	std::vector<uint32_t> m_sfids;
	std::vector<uint32_t> m_txids;
	std::vector<std::unique_ptr<M2Skin>> m_skins;


	XMFLOAT3 GetBonePosition(const M2_Bone* b, const std::vector<M2_Bone>& bones);
	void BuildBonesDebug(const std::vector<M2_Bone>& bones);
	void LoadAnimations(const M2_header& hdr, IWSStream& file, uint32_t baseOfs);
	void LoadTextures(const M2_header& hdr, IWSStream& file, uint32_t baseOfs, std::span<uint32_t> txids);
	void LoadVertices(const M2_header& hdr, IWSStream& file, uint32_t baseOfs);
	void LoadColors(const M2_header& hdr, IWSStream& file, uint32_t baseOfs);
	void ReadMD20(IWSStream& file, std::span<uint32_t> sfids, std::span<uint32_t> txids);
	void LoadSkins();
public:
	M2Model(ID3D11Device* pD3D11Device,
		std::string_view filename,
		IWSStream& file,
		WSResourceManager* resourceManager,
		VertexShader* pVS,
		PixelShader* pPS,
		VertexShader* pDebugBonesVS,
		PixelShader* pDebugBonesPS
	);
	~M2Model();


	void Draw(ID3D11DeviceContext* pDC,
		ConstantBuffer<M2_ConstantBuffer>& cb,
		ID3D11SamplerState* pSampler,
		ConstantBuffer<M2_AnimConstantBuffer>& animCB,
		int32_t animID,
		uint64_t animTime,
		std::span<IWSTexture*> textures,
		uint32_t skinID);
};