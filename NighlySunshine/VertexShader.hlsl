//output vertex structure
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

cbuffer cbPerObject
{
	float4x4 ProjectionMatrix;	//camera projection matrix
	float4x4 ViewMatrix;		//camera view matrix
	float4x4 _Object2World;		//object world matrix (transform, rotation, scale)
	float4x4 WVP;				//premultiplied world view projection matrix, used only for debug in main(...)
	float4 sunParam;			//sun parameters. x is for azimuth, y for zenith angle, z and w component unused, only there for constant buffer alignment
};

//only for debug, nothing interesting here
VS_OUTPUT main(float4 position : POSITION, float4 colour : COLOR)
{
	VS_OUTPUT output;

	output.position = mul(position, WVP);
	output.colour = colour;

	return output;
}

//rotate for a plane by angle in radians
void Rotate(inout float2 v, float rad)
{
	float2 temp = v;
	v.x = cos(rad) * temp.x + sin(rad) * temp.y;
	v.y = -sin(rad) * temp.x + cos(rad) * temp.y;
}

//function for converting degrees to radians
float deg2rad(float deg)
{
	float rad = 0;
	rad = (deg * 3.14159f / 180.0f);
	return rad;
}

//this is where shadowing occurs
VS_OUTPUT shadowmain(float4 position : POSITION, float4 colour : COLOR)
{
	VS_OUTPUT output;

	//this is the plane definition. Currently is the XZ plane
	float4 planePoint = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 planeNormal = float4(0.0f, 1.0f, 0.0f, 1.0f);

	//the initial sun direction, not for tampering
	float3 sun = normalize(float3(0.0f, 1.0f, 0.0f));

	//transform the sun by azimuth and zenith angle data
	Rotate(sun.xy, deg2rad(sunParam.x));	//azimuth
	Rotate(sun.xz, deg2rad(sunParam.y));	//zenith

	//calculate where is the sun directing at
	float sunDir = dot(sun, planeNormal);
	float3 offset = sun - (planeNormal * sunDir);

	//transform the vertices to world coordinates
	float4 vertexWorldPos = mul(position, _Object2World);

	float3 difference = planePoint.xyz - vertexWorldPos.xyz;
	float distanceToPlane = dot(difference, planeNormal);
	//uncomment one line, comment the other, these two lines are about how the shadows are cast
	//float3 projectedVertex = vertexWorldPos.xyz + (distanceToPlane * (planeNormal.xyz + (offset / sunDir))); // this is more realistic
	float3 projectedVertex = vertexWorldPos.xyz + (distanceToPlane * (planeNormal.xyz + offset));	//this is softer
	//apply view and projection matrices to the final position of the projected vertex
	output.position = mul(float4(projectedVertex, 1.0), mul(ViewMatrix, ProjectionMatrix));
	output.colour = colour;

	return output;
}