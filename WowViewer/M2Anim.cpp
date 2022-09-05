#include "framework.h"
#include "M2Anim.hpp"

void M2Anim::ReadBoneScale(IWSStream& file,
	int iBone,
	const M2_AnimKeyList& timestampList,
	const M2_AnimKeyList& keyList,
	uint32_t baseOfs)
{
	unsigned long BytesRead = 0;

	int nEntries = timestampList.nEntries;

	std::vector<UINT32> timestamps(nEntries);
	std::vector<XMFLOAT3> keys(nEntries);

	file.Seek(timestampList.ofsEntries + baseOfs, WSSeekOrigin::Begin);
	file.Read(&timestamps[0], sizeof(UINT32) * nEntries);

	file.Seek(keyList.ofsEntries + baseOfs, WSSeekOrigin::Begin);
	file.Read(&keys[0], sizeof(XMFLOAT3) * nEntries);

	m_boneKeys[iBone].scale.resize(nEntries);
	for (int iKey = 0; iKey < nEntries; ++iKey)
	{
		m_boneKeys[iBone].scale[iKey].timestamp = timestamps[iKey];
		XMFLOAT3 v = keys[iKey];
		m_boneKeys[iBone].scale[iKey].key = XMFLOAT3(v.y, v.z, -v.x);
	}
}

float M2Anim::QuaternionShortToFloat(int16_t i)
{
	return float(i < 0 ? i + 32768 : i - 32767) / 32767.0f;
}
void M2Anim::ReadBoneRotation(IWSStream& file,
	int iBone,
	const M2_AnimKeyList& timestampList,
	const M2_AnimKeyList& keyList,
	uint32_t baseOfs)
{
	unsigned long BytesRead = 0;

	int nEntries = timestampList.nEntries;

	std::vector<UINT32> timestamps(nEntries);
	std::vector<XMSHORT4> keys(nEntries);

	file.Seek(timestampList.ofsEntries + baseOfs, WSSeekOrigin::Begin);
	file.Read(&timestamps[0], sizeof(UINT32) * nEntries);

	file.Seek(keyList.ofsEntries + baseOfs, WSSeekOrigin::Begin);
	file.Read(&keys[0], sizeof(XMSHORT4) * nEntries);

	m_boneKeys[iBone].rotation.resize(nEntries);
	for (int iKey = 0; iKey < nEntries; ++iKey)
	{
		m_boneKeys[iBone].rotation[iKey].timestamp = timestamps[iKey];

		XMSHORT4 v = keys[iKey];
		XMVECTOR Q = XMVectorSet(
			-QuaternionShortToFloat(v.y),
			-QuaternionShortToFloat(v.z),
			QuaternionShortToFloat(v.x),
			QuaternionShortToFloat(v.w));

		//Q = XMQuaternionNormalize(Q);
		XMStoreFloat4(&m_boneKeys[iBone].rotation[iKey].key, Q);
		/*m_boneKeys[iBone].rotation[iKey].key = XMFLOAT4(
		QuaternionShortToFloat(v.y),
		QuaternionShortToFloat(v.z),
		-QuaternionShortToFloat(v.x),
		QuaternionShortToFloat(v.w));*/

		//float qdzd = QuaternionShortToFloat(v.x);
		//XMVECTOR vdzqd = XMQuaternionLength(XMLoadFloat4(&m_boneKeys[iBone].rotation[iKey].key));
		//XMVECTOR normalizedQ = XMQuaternionNormalize(XMLoadFloat4(&m_boneKeys[iBone].rotation[iKey].key));
		int qzdqzd = 0;
	}
}
void M2Anim::ReadBoneTranslation(IWSStream& file,
	int iBone,
	const M2_AnimKeyList& timestampList,
	const M2_AnimKeyList& keyList,
	uint32_t baseOfs)
{
	unsigned long BytesRead = 0;

	int nEntries = timestampList.nEntries;

	std::vector<UINT32> timestamps(nEntries);
	std::vector<XMFLOAT3> keys(nEntries);

	file.Seek(timestampList.ofsEntries + baseOfs, WSSeekOrigin::Begin);
	file.Read(&timestamps[0], sizeof(UINT32) * nEntries);

	file.Seek(keyList.ofsEntries + baseOfs, WSSeekOrigin::Begin);
	file.Read(&keys[0], sizeof(XMFLOAT3) * nEntries);

	m_boneKeys[iBone].translation.resize(nEntries);
	for (int iKey = 0; iKey < nEntries; ++iKey)
	{
		m_boneKeys[iBone].translation[iKey].timestamp = timestamps[iKey];
		XMFLOAT3 v = keys[iKey];
		m_boneKeys[iBone].translation[iKey].key = XMFLOAT3(v.y, v.z, -v.x);
	}
}

XMVECTOR M2Anim::GetBoneRotation(int iBone, float time)
{
	int iKey = 0;
	for (; iKey < m_boneKeys[iBone].rotation.size(); ++iKey)
	{
		if (m_boneKeys[iBone].rotation[iKey].timestamp > time)
			break;
	}

	if (iKey == 0)
	{
		//	no keys
		if (m_boneKeys[iBone].rotation.size() == 0)
		{
			//	Return quaternion identity (no rotation)
			return XMVectorSet(0, 0, 0, 1);
		}
		//	keys but before 1st key
		else
		{
			//	Return 1st key
			return XMLoadFloat4(&m_boneKeys[iBone].rotation[0].key);
		}
	}
	//	keys but over last key
	else if (iKey >= m_boneKeys[iBone].rotation.size())
	{
		//	Return last key
		return XMLoadFloat4(&m_boneKeys[iBone].rotation[m_boneKeys[iBone].rotation.size() - 1].key);
	}
	else
	{
		//	Compute slerp of two keys
		float tlen = m_boneKeys[iBone].rotation[iKey].timestamp - m_boneKeys[iBone].rotation[iKey - 1].timestamp;
		float t = (time - m_boneKeys[iBone].rotation[iKey - 1].timestamp) / tlen;

		return XMQuaternionSlerp(XMLoadFloat4(&m_boneKeys[iBone].rotation[iKey - 1].key),
			XMLoadFloat4(&m_boneKeys[iBone].rotation[iKey].key),
			t);
	}
}
XMVECTOR M2Anim::GetBoneScale(int iBone, float time)
{
	int iKey = 0;
	for (; iKey < m_boneKeys[iBone].scale.size(); ++iKey)
	{
		if (m_boneKeys[iBone].scale[iKey].timestamp > time)
			break;
	}

	if (iKey == 0)
	{
		//	no keys
		if (m_boneKeys[iBone].scale.size() == 0)
		{
			//	Return normal scale
			return XMVectorSet(1, 1, 1, 0);
		}
		//	keys but before 1st key
		else
		{
			//	Return 1st key
			return XMLoadFloat3(&m_boneKeys[iBone].scale[0].key);
		}
	}
	//	keys but over last key
	else if (iKey >= m_boneKeys[iBone].scale.size())
	{
		//	Return last key
		return XMLoadFloat3(&m_boneKeys[iBone].scale[m_boneKeys[iBone].scale.size() - 1].key);
	}
	else
	{
		//	Linear interpolation
		float tlen = m_boneKeys[iBone].scale[iKey].timestamp - m_boneKeys[iBone].scale[iKey - 1].timestamp;
		float t = (time - m_boneKeys[iBone].scale[iKey - 1].timestamp) / tlen;

		return XMVectorLerp(XMLoadFloat3(&m_boneKeys[iBone].scale[iKey - 1].key),
			XMLoadFloat3(&m_boneKeys[iBone].scale[iKey].key),
			t);
	}
}
XMVECTOR M2Anim::GetBoneTranslation(int iBone, float time)
{
	int iKey = 0;
	for (; iKey < m_boneKeys[iBone].translation.size(); ++iKey)
	{
		if (m_boneKeys[iBone].translation[iKey].timestamp > time)
			break;
	}

	if (iKey == 0)
	{
		//	no keys
		if (m_boneKeys[iBone].translation.size() == 0)
		{
			//	Return no translation
			return XMVectorSet(0, 0, 0, 0);
		}
		//	keys but before 1st key
		else
		{
			//	Return 1st key
			return XMLoadFloat3(&m_boneKeys[iBone].translation[0].key);
		}
	}
	//	keys but over last key
	else if (iKey >= m_boneKeys[iBone].translation.size())
	{
		//	Return last key
		return XMLoadFloat3(&m_boneKeys[iBone].translation[m_boneKeys[iBone].translation.size() - 1].key);
	}
	else
	{
		//	Linear interpolation
		float tlen = m_boneKeys[iBone].translation[iKey].timestamp - m_boneKeys[iBone].translation[iKey - 1].timestamp;
		float t = (time - m_boneKeys[iBone].translation[iKey - 1].timestamp) / tlen;

		return XMVectorLerp(XMLoadFloat3(&m_boneKeys[iBone].translation[iKey - 1].key),
			XMLoadFloat3(&m_boneKeys[iBone].translation[iKey].key),
			t);
	}
}

void M2Anim::GetBoneMatrix(int iBone, float time, ConstantBuffer<M2_AnimConstantBuffer>& cb, std::vector<bool>& loadedBones)
{
	//	If iBone is loaded, do nothing
	if (loadedBones[iBone])
		return;

	XMFLOAT3 pivot = XMFLOAT3(m_boneKeys[iBone].bone.PivotPoint[1],
		m_boneKeys[iBone].bone.PivotPoint[2],
		-m_boneKeys[iBone].bone.PivotPoint[0]);

	XMVECTOR Vpivot = XMLoadFloat3(&pivot);

	//	Calculate parent bones
	if (m_boneKeys[iBone].bone.ParentBone != -1)
	{
		GetBoneMatrix(m_boneKeys[iBone].bone.ParentBone, time, cb, loadedBones);

		XMMATRIX parentMatrix = cb.cb.bones[m_boneKeys[iBone].bone.ParentBone];

		XMVECTOR rotation = GetBoneRotation(iBone, time);
		XMVECTOR translation = GetBoneTranslation(iBone, time);
		XMVECTOR scale = GetBoneScale(iBone, time);

		XMMATRIX transform = XMMatrixAffineTransformation(scale, Vpivot, rotation, translation);//XMVectorSet(0, 0, 0, 0));

		cb.cb.bones[iBone] = XMMatrixMultiply(transform, parentMatrix);
	}
	else
	{
		XMVECTOR rotation = GetBoneRotation(iBone, time);
		XMVECTOR translation = GetBoneTranslation(iBone, time);
		XMVECTOR scale = GetBoneScale(iBone, time);

		XMMATRIX M = XMMatrixAffineTransformation(scale, Vpivot, rotation, translation);// XMVectorSet(0, 0, 0, 0));

		cb.cb.bones[iBone] = M;
	}

	loadedBones[iBone] = true;
}

M2Anim::M2Anim(IWSStream& file, int indexOfAnim, const M2_AnimSequence& anim, const std::vector<M2_Bone>& bones, std::span<M2Track<XMFLOAT3>> colors, std::span<M2Track<float>> alphas, uint32_t baseOfs)
	: m_sequence(anim),
	m_animID(indexOfAnim),
	m_colors(colors),
	m_alphas(alphas)
{
	unsigned long BytesRead = 0;

	m_boneKeys.resize(bones.size());

	for (int iBone = 0; iBone < bones.size(); ++iBone)
	{
		//	Keep bone structure
		m_boneKeys[iBone].bone = bones[iBone];

		//	Check for errors
		if (bones[iBone].Translation.nTimestamps != bones[iBone].Translation.nKeys)
			throw std::exception();

		if (bones[iBone].Translation.nTimestamps > indexOfAnim)
		{
			//	Read translation keys -- One list for each animation
			M2_AnimKeyList translationTimestampList;// (bones[iBone].Translation.nTimestamps);
			M2_AnimKeyList translationKeyList;// (bones[iBone].Translation.nKeys);

			file.Seek(bones[iBone].Translation.ofsTimestamps + sizeof(M2_AnimKeyList) * indexOfAnim + baseOfs, WSSeekOrigin::Begin);
			file.Read(&translationTimestampList, sizeof(M2_AnimKeyList));

			file.Seek(bones[iBone].Translation.ofsKeys + sizeof(M2_AnimKeyList) * indexOfAnim + baseOfs, WSSeekOrigin::Begin);
			file.Read(&translationKeyList, sizeof(M2_AnimKeyList));

			if (translationTimestampList.nEntries != translationKeyList.nEntries)
				throw std::exception();

			if (translationTimestampList.nEntries != 0)
				ReadBoneTranslation(file, iBone, translationTimestampList, translationKeyList, baseOfs);
		}

		//	Check for errors
		if (bones[iBone].Rotation.nTimestamps != bones[iBone].Rotation.nKeys)
			throw std::exception();

		if (bones[iBone].Rotation.nTimestamps > indexOfAnim)
		{
			//	Read translation keys -- One list for each animation
			M2_AnimKeyList rotationTimestampList;// (bones[iBone].Translation.nTimestamps);
			M2_AnimKeyList rotationKeyList;// (bones[iBone].Translation.nKeys);

			file.Seek(bones[iBone].Rotation.ofsTimestamps + sizeof(M2_AnimKeyList) * indexOfAnim + baseOfs, WSSeekOrigin::Begin);
			file.Read(&rotationTimestampList, sizeof(M2_AnimKeyList));

			file.Seek(bones[iBone].Rotation.ofsKeys + sizeof(M2_AnimKeyList) * indexOfAnim + baseOfs, WSSeekOrigin::Begin);
			file.Read(&rotationKeyList, sizeof(M2_AnimKeyList));

			if (rotationTimestampList.nEntries != rotationKeyList.nEntries)
				throw std::exception();

			if (rotationTimestampList.nEntries != 0)
				ReadBoneRotation(file, iBone, rotationTimestampList, rotationKeyList, baseOfs);
		}

		//	Check for errors
		if (bones[iBone].Scaling.nTimestamps != bones[iBone].Scaling.nKeys)
			throw std::exception();

		if (bones[iBone].Scaling.nTimestamps > indexOfAnim)
		{
			//	Read translation keys -- One list for each animation
			M2_AnimKeyList scalingTimestampList;// (bones[iBone].Translation.nTimestamps);
			M2_AnimKeyList scalingKeyList;// (bones[iBone].Translation.nKeys);

			file.Seek(bones[iBone].Scaling.ofsTimestamps + sizeof(M2_AnimKeyList) * indexOfAnim + baseOfs, WSSeekOrigin::Begin);
			file.Read(&scalingTimestampList, sizeof(M2_AnimKeyList));

			file.Seek(bones[iBone].Scaling.ofsKeys + sizeof(M2_AnimKeyList) * indexOfAnim + baseOfs, WSSeekOrigin::Begin);
			file.Read(&scalingKeyList, sizeof(M2_AnimKeyList));

			if (scalingTimestampList.nEntries != scalingKeyList.nEntries)
				throw std::exception();

			if (scalingTimestampList.nEntries != 0)
				ReadBoneScale(file, iBone, scalingTimestampList, scalingKeyList, baseOfs);
		}
	}
}
M2Anim::~M2Anim()
{

}
void M2Anim::UpdateAnimConstantBuffer(uint64_t timestamp, ConstantBuffer<M2_AnimConstantBuffer>& cb)
{
	timestamp = timestamp % m_sequence.Length;

	std::vector<bool> loadedBones(m_boneKeys.size());

	for (int iBone = 0; iBone < m_boneKeys.size(); ++iBone)
	{
		GetBoneMatrix(iBone, timestamp, cb, loadedBones);
	}
	for (int iBone = 0; iBone < m_boneKeys.size(); ++iBone)
	{
		cb.cb.bones[iBone] = XMMatrixTranspose(cb.cb.bones[iBone]);
	}
	int qdzqd = 0;
}

template<typename T>
std::pair<M2KeyFrame<T>, M2KeyFrame<T>> GetBoundFrames(std::span<M2KeyFrame<T>> keyframes, uint64_t timestamp)
{
	auto it = std::upper_bound(keyframes.begin(), keyframes.end(), timestamp,
		[](uint64_t t, M2KeyFrame<T>& kf) { return t < kf.timestamp; });
	auto start = it - 1;
	auto end = it == keyframes.end() ? start : it;
	return std::pair(*start, *end);
}
XMFLOAT4 M2Anim::GetColor(uint64_t timestamp, int i)
{
	uint64_t t = timestamp % m_sequence.Length;

	M2KeyFrame<XMFLOAT3> c1 = { 0, XMFLOAT3(1, 1, 1) };
	M2KeyFrame<XMFLOAT3> c2 = { 0, XMFLOAT3(1, 1, 1) };
	M2KeyFrame<float> a1 = {0, 1};
	M2KeyFrame<float> a2 = {0, 1};
	if (m_animID < m_colors[i].keyframes.size() && m_colors[i].keyframes[m_animID].size() > 0)
		std::tie(c1, c2) = GetBoundFrames(std::span(m_colors[i].keyframes[m_animID]), t);
	if (m_animID < m_alphas[i].keyframes.size() && m_alphas[i].keyframes[m_animID].size() > 0)
		std::tie(a1, a2) = GetBoundFrames(std::span(m_alphas[i].keyframes[m_animID]), t);
	
	XMFLOAT4 c;
	if (t >= c2.timestamp)
		c = XMFLOAT4(c2.key.x, c2.key.y, c2.key.z, 0);
	else
	{
		switch (m_colors[i].interpolation)
		{
		case M2Interpolation::None:
			c = XMFLOAT4(c1.key.x, c1.key.y, c1.key.z, 0);
			break;
		case M2Interpolation::Linear:
			XMStoreFloat4(&c, XMVectorLerp(XMLoadFloat3(&c1.key), XMLoadFloat3(&c2.key), (t - c1.timestamp) / (float)(c2.timestamp - c1.timestamp)));
			break;
		case M2Interpolation::Bezier: // bezier/hermite only used with M2SplineKey apparently
			throw std::exception();
		case M2Interpolation::Hermite:
			//XMStoreFloat4(&c, XMVectorHermite(XMLoadFloat3(&c1.key), XMLoadFloat3(&c2.key), (t - c1.timestamp) / (float)(c2.timestamp - c1.timestamp)));
			throw std::exception();
		}
	}

	if (t >= a2.timestamp)
		c.w = a2.key;
	else
	{
		switch (m_alphas[i].interpolation)
		{
		case M2Interpolation::None:
			c.w = a1.key;
			break;
		case M2Interpolation::Linear:
		{
			float f = (t - a1.timestamp) / (float)(a2.timestamp - a1.timestamp);
			c.w = a2.key * f + a1.key * (1 - f);
			break;
		}
		case M2Interpolation::Bezier:
		case M2Interpolation::Hermite:
			throw std::exception();
		}
	}

	return c;
}