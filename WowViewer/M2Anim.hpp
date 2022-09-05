#pragma once
#include "framework.h"

#include "IWSStream.hpp"
#include "ConstantBuffer.hpp"

#include "IWSModel.hpp"


struct M2_AnimSequence
{
	/*0x00*/	UINT16	AnimationID;		//	Animation id in AnimationData.dbc
	/*0x02*/	UINT16	SubAnimationID;		//	Sub - animation id : Which number in a row of animations this one is.
	/*0x04*/	UINT32	Length;				//	The length(timestamps) of the animation.I believe this actually the length of the animation in milliseconds.
	/*0x08*/	float	MovingSpeed;		//	This is the speed the character moves with in this animation.
	/*0x0C*/	UINT32	Flags;				//	 See below.
	/*0x10*/	INT16	Probability;		//	This is used to determine how often the animation is played.For all animations of the same type, this adds up to 0x7FFF (32767).
	/*0x12*/	UINT16	Unused;				//
	/*0x14*/	UINT32	Unknown1;			//	These two are connected.Most of the time, they are 0.
	/*0x18*/	UINT32	Unknown2;			//	But if there is data in one, there is data in both of them.
	/*0x1C*/	UINT32	PlaybackSpeed;		//	Values : 0, 50, 100, 150, 200, 250, 300, 350, 500.
	/*0x20*/	float	MinimumExtent[3];	//	Minimum Extent
	/*0x2C*/	float	MaximumExtent[3];	//	MaximumExtent
	/*0x38*/	float	BoundsRadius;		//	The radius.
	/*0x3C*/	INT16	NextAnimation;		//	Id of the following animation of this AnimationID, points to an Index or is - 1 if none.
	/*0x3E*/	UINT16	Index;				//	Id in the list of animations.
};

struct M2_AnimKeyList
{
	UINT32 nEntries;
	UINT32 ofsEntries;
};
struct M2_AnimBlock
{
	UINT16 interpolation_type;
	UINT16 global_sequence;
	UINT32 nTimestamps;
	UINT32 ofsTimestamps;
	UINT32 nKeys;
	UINT32 ofsKeys;
};
struct M2_Bone
{
	/*0x00*/	INT32			KeyBoneID;		//	Back - reference to the key bone lookup table. - 1 if this is no key bone.
	/*0x04*/	UINT32			Flags;			//	Only known flags : 8 - billborded and 512 - transformed
	/*0x08*/	INT16			ParentBone;		//	Parent bone ID or - 1 if there is none.
	/*0x0A*/	UINT16			Unknown[3];		//	The first one might be related to the parts of the bodies.
	/*0x10*/	M2_AnimBlock	Translation;	//	An animationblock for translation.Should be 3 * floats.
	/*0x24*/	M2_AnimBlock	Rotation;		//	An animationblock for rotation.Should be 4 * shorts, see Quaternion values and 2.x.
	/*0x38*/	M2_AnimBlock	Scaling;		//	An animationblock for scaling.Should be 3 * floats.
	/*0x4C*/	float			PivotPoint[3];	//	The pivot point of that bone.Its a vector.
};


template <typename T>
struct M2KeyFrame
{
	UINT32 timestamp;
	T key;
};
struct M2BoneKeyFrame
{
	M2_Bone bone;
	std::vector<M2KeyFrame<XMFLOAT3>> translation;
	std::vector<M2KeyFrame<XMFLOAT4>> rotation;
	std::vector<M2KeyFrame<XMFLOAT3>> scale;
};
struct M2Animation
{
	M2_AnimSequence sequence;
	std::vector<M2BoneKeyFrame> bonesKeyFrames;
};

/*
struct M2_D3D11_AnimKey
{
XMFLOAT4 time;
XMFLOAT4 lastPos;
XMFLOAT4 nextPos;
};
struct M2_D3D11_Bone
{
XMUINT4 Parent;
XMFLOAT4 PivotPoint;

M2_D3D11_AnimKey translation;
M2_D3D11_AnimKey rotation;
M2_D3D11_AnimKey scale;
};*/

template<typename T>
struct M2Array
{
	/*0x00*/  uint32_t number;
	/*0x04*/  uint32_t offset_elements;
	/*0x08*/
};

template<typename T>
struct M2FileTrack
{
	/*0x00*/  uint16_t interpolation_type;
	/*0x02*/  uint16_t global_sequence;
	/*0x04*/  M2Array<M2Array<uint32_t>> timestamps;
	/*0x0C*/  M2Array<M2Array<T>> values;
	/*0x14*/
};

enum class M2Interpolation
	: uint16_t
{
	None = 0,
	Linear = 1,
	Bezier = 2,
	Hermite = 3,
};

template<typename T>
struct M2Track
{
	M2Interpolation interpolation;
	std::vector<std::vector<M2KeyFrame<T>>> keyframes;
};

class M2_header;
class M2Anim
{
	M2Anim() = delete;
	M2Anim(const M2Anim&) = delete;
	M2Anim& operator=(const M2Anim&) = delete;

	M2_AnimSequence m_sequence;

	std::vector<M2BoneKeyFrame> m_boneKeys;
	std::span<M2Track<XMFLOAT3>> m_colors;
	std::span<M2Track<float>> m_alphas;
	int32_t m_animID = -1;

	void ReadBoneScale(IWSStream& file,
		int iBone,
		const M2_AnimKeyList& timestampList,
		const M2_AnimKeyList& keyList,
		uint32_t baseOfs);

	float QuaternionShortToFloat(int16_t i);
	void ReadBoneRotation(IWSStream& file,
		int iBone,
		const M2_AnimKeyList& timestampList,
		const M2_AnimKeyList& keyList,
		uint32_t baseOfs);
	void ReadBoneTranslation(IWSStream& file,
		int iBone,
		const M2_AnimKeyList& timestampList,
		const M2_AnimKeyList& keyList,
		uint32_t baseOfs);

	XMVECTOR GetBoneRotation(int iBone, float time);
	XMVECTOR GetBoneScale(int iBone, float time);
	XMVECTOR GetBoneTranslation(int iBone, float time);

	void GetBoneMatrix(int iBone, float time, ConstantBuffer<M2_AnimConstantBuffer>& cb, std::vector<bool>& loadedBones);

public:
	M2Anim(IWSStream& file, int indexOfAnim, const M2_AnimSequence& anim, const std::vector<M2_Bone>& bones, std::span<M2Track<XMFLOAT3>> colors, std::span<M2Track<float>> alphas, uint32_t baseOfs);
	~M2Anim();
	void UpdateAnimConstantBuffer(uint64_t timestamp, ConstantBuffer<M2_AnimConstantBuffer>& cb);

	XMFLOAT4 GetColor(uint64_t timestamp, int i);
};