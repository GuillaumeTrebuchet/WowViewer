//#include "Quaternions.hlsli"

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 CamPos;
	float4 Color[3];
	uint textureCount;
}

//struct Key
//{
//float4 time;
//float4 lastPos;
//float4 nextPos;
//};
//struct Bone
//{
//uint4 Parent;
//float4 PivotPoint;
//
//Key translation;
//Key rotation;
//Key scale;
//
//};

cbuffer ConstantBuffer2 : register(b1)
{
	//float4 currentTime;
	//Bone bones[150];
	matrix bones[255];
}

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 TexCoord : TEXCOORD0;
	uint4 BoneIndices : BLENDINDICES0;
	uint4 BoneWeights : BLENDINDICES1;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float2 TexCoord : TEXCOORD0;
	float4 ViewVec : POSITION1;
	float Dist : NORMAL1;
};

float3 RotateVertexByBoneNoParent(float3 pos, uint iBone)
{
	float3 Result = pos;


	//	If bone has parent, rotate vertex by parent bone
	/*if (bones[iBone].Parent.x != -1)
	Result = RotateVertexByBone(pos, bones[iBone].Parent.x);
	else
	Result = pos;

	//	Slerp bone rotation*/
	/*float Tlen = bones[iBone].rotation.time.y - bones[iBone].rotation.time.x;
	float T = (currentTime - bones[iBone].rotation.time.x) / Tlen;
	float4 Q = QuaternionSlerp(bones[iBone].rotation.lastPos, bones[iBone].rotation.nextPos, T);

	//	Rotate vertex by interpolated bone
	float3 pivotPoint = bones[iBone].PivotPoint.xyz;
	Result = QuaternionRotate(Result - pivotPoint, Q) + pivotPoint;*/
	return Result;
}
/*float3 RotateVertexByBoneWithParent(float3 pos, uint iBone)
{
float3 Result = pos;

uint iParentBone = bones[iBone].Parent.x;

while (iParentBone != -1)
{
Result = RotateVertexByBoneNoParent(Result, iParentBone);
iParentBone = bones[iParentBone].Parent.x
}
//	If bone has parent, rotate vertex by parent bone
if (bones[iBone].Parent.x != -1)
Result = RotateVertexByBone(pos, bones[iBone].Parent.x);
else
Result = pos;
}*/
float4 GetVertexPos(float4 pos, uint4 boneIndices, uint4 boneWeights)
{
	float4 pos0 = mul(pos, bones[boneIndices[0]]) * (float)boneWeights[0] / 255.0f;
	float4 pos1 = mul(pos, bones[boneIndices[1]]) * (float)boneWeights[1] / 255.0f;
	float4 pos2 = mul(pos, bones[boneIndices[2]]) * (float)boneWeights[2] / 255.0f;
	float4 pos3 = mul(pos, bones[boneIndices[3]]) * (float)boneWeights[3] / 255.0f;

	return pos0 + pos1 + pos2 + pos3;


	//return float4(RotateVertexByBoneNoParent(pos.xyz, boneIndices[0]), 1);

	//float4 Q = QuaternionSlerp(float4(1, 0, 0, 0), float4(0, 0, 0, -1), 0.5);
	//return float4(QuaternionRotate(pos.xyz, Q), 1);

	//return float4(QuaternionRotate(pos.xyz, bones[0].rotation.lastPos), 1);
	//return float4(QuaternionRotate(pos.xyz, float4(0.707106769, 0, 0, 0.707106709)).xyz, 1);
}
PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	output.Pos = input.Pos;

	output.Pos = GetVertexPos(output.Pos, input.BoneIndices, input.BoneWeights);

	output.Pos = mul(output.Pos, World);


	output.Dist = 1;// / length(CamPos - output.Pos);
	output.ViewVec = normalize(CamPos - output.Pos);

	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Norm = mul(input.Norm, World);
	//output.LightnessNorm = mul( input.Norm, World );
	//output.LightnessNorm = mul( output.LightnessNorm, View );
	//output.LightnessNorm = mul( output.LightnessNorm, Projection );

	output.TexCoord = input.TexCoord;


	return output;
}