struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

struct Light
{
	float3 dir;
	float4 ambient;
	float4 diffuse;
};

cbuffer cbPerFrame
{
	Light light;
};

cbuffer cbPerObject
{
	float4x4 WVP;
	float4x4 World;
};
VS_OUTPUT main(float4 position : POSITION, float4 colour : COLOR)
{
	VS_OUTPUT output;

	output.position = mul(position, WVP);
	output.colour = colour;

	return output;
}