struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

cbuffer cbPerObject
{
	float4x4 ModelViewMatrix;
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
	float4x4 _World2Receiver;
	float4x4 _Object2World;
	float4x4 _World2Object;
	float4	_Scale;
	float4	_WorldSpaceLightPos0;
	float4x4 WVP;
	float4x4 World;
	float4 sunParam;
};

VS_OUTPUT main(float4 position : POSITION, float4 colour : COLOR)
{
	VS_OUTPUT output;

	output.position = mul(position, WVP);
	output.colour = colour;

	return output;
}

void Rotate(inout float2 v, float rad)
{
	float2 temp = v;
	v.x = cos(rad) * temp.x + sin(rad) * temp.y;
	v.y = -sin(rad) * temp.x + cos(rad) * temp.y;
}

float deg2rad(float deg)
{
	float rad = 0;
	rad = (deg * 3.14159f / 180.0f);
	return rad;
}

VS_OUTPUT shadowmain(float4 position : POSITION, float4 colour : COLOR)
{
	VS_OUTPUT output;

	float4 planePoint = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 planeNormal = float4(0.0f, 1.0f, 0.0f, 1.0f);

	float3 sun = normalize(float3(0.0f, 1.0f, 0.0f));

	Rotate(sun.xy, deg2rad(sunParam.x));
	Rotate(sun.xz, deg2rad(sunParam.y));

	float sunDir = dot(sun, planeNormal);
	float3 offset = sun - (planeNormal * sunDir);

	float4 vertexWorldPos = mul(position, _Object2World);
	float3 difference = planePoint.xyz - vertexWorldPos.xyz;
	float distanceToPlane = dot(difference, planeNormal);
	//float3 projectedVertex = vertexWorldPos.xyz + (distanceToPlane * (planeNormal.xyz + (offset / sunDir)));
	float3 projectedVertex = vertexWorldPos.xyz + (distanceToPlane * (planeNormal.xyz + offset));
	output.position = mul(float4(projectedVertex, 1.0), mul(ViewMatrix, ProjectionMatrix));
	output.colour = colour;

	return output;
}