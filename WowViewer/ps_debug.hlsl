cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 CamPos;
}

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float4 ViewVec : POSITION1;
	float Dist : NORMAL1;
};

float4 main(PS_INPUT input) : SV_Target
{
	float4 finalColor = 0;

	float3	Normal = normalize(input.Norm);
	float4	ViewDir = normalize(input.ViewVec);

	float3	Vert2Light = normalize(float3(5, 5, -5));

	float4	Diff = saturate(dot(Normal, Vert2Light));
	float4	Reflect = normalize(2 * Diff * float4(Normal, 0) - float4(Vert2Light, 0));
	float4	Specular = pow(saturate(dot(Reflect, ViewDir)), 5);

	float4 MatColor = float4(1, 0.8, 0.7, 1);
	float4 FinalDiffuse = saturate(input.Dist * Diff * MatColor * float4(1, 1, 1, 1) * 0.8);
	float4 FinalSpecular = Specular * 0.3;

	finalColor += FinalDiffuse + FinalSpecular;
	finalColor += float4(0.5, 0.5, 0.5, 1) * MatColor;

	return finalColor;
}
