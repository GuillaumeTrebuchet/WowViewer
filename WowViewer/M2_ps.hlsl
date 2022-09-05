Texture2D texture0[3] : register(t0);

SamplerState texSampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 CamPos;
	float4 TextureColor[3];
	uint TextureCount;
}

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float2 TexCoord : TEXCOORD0;
	float4 ViewVec : POSITION1;
	float Dist : NORMAL1;
};

float4 main(PS_INPUT input) : SV_Target
{
	float4 finalColor = 0;

	float3	Normal = normalize(input.Norm);
	float4	ViewDir = normalize(input.ViewVec);

	//do NdotL lighting for 2 lights
	//for (int i = 0; i<2; i++)
	//{
	float3	Vert2Light = normalize(float3(5, 5, -5));

	float4	Diff = saturate(dot(Normal, Vert2Light));
	float4	Reflect = normalize(2 * Diff * float4(Normal, 0) - float4(Vert2Light, 0));
	float4	Specular = pow(saturate(dot(Reflect, ViewDir)), 50);

	float4 FinalDiffuse = saturate(input.Dist * Diff * float4(1, 1, 1, 1) * float4(1, 1, 1, 1) * 0.8);
	float4 FinalSpecular = input.Dist * Specular * 0.6;

	finalColor += FinalDiffuse + FinalSpecular;
	//}

	//finalColor += amblight.Color * amblight.Intensity;
	finalColor += float4(0.5, 0.5, 0.5, 1);

	for (uint i = 0; i < TextureCount; ++i)
	{
		float4 s = texture0[i].Sample(texSampler, input.TexCoord);
		s.w = 1.0;
		finalColor *= s;
		finalColor *= TextureColor[i];
		//finalColor.a = 1;// saturate(finalColor.a);
		//finalColor.a = saturate(finalColor.a) * 0.5;
	}

	return finalColor;
}
