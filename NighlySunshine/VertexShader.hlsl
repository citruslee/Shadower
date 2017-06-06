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
	float3 sun = float3(0.5f, 0.1f, 0.5f);
	VS_OUTPUT output;
	float4x4 temp = WVP;
	temp[1][0] = (-sun.x / sun.y);
	temp[1][2] = (-sun.z / sun.y);

	output.position = mul(position, temp);
	output.colour = colour;

	return output;
}