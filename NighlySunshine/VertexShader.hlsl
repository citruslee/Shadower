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
};

VS_OUTPUT main(float4 position : POSITION, float4 colour : COLOR)
{
	VS_OUTPUT output;

	float4x4 modelMatrix = _Object2World;

	float4x4 viewMatrix = ViewMatrix;// mul(modelMatrixInverse, ModelViewMatrix);

	float4 lightDirection;
	
	lightDirection = -normalize(_WorldSpaceLightPos0);

	float4 vertexInWorldSpace = mul(position, modelMatrix);
	float4 world2ReceiverRow1 = float4(_World2Receiver[1][0], _World2Receiver[1][1], _World2Receiver[1][2], _World2Receiver[1][3]);
	float distanceOfVertex = dot(world2ReceiverRow1, vertexInWorldSpace);

	float lengthOfLightDirectionInY = dot(world2ReceiverRow1, lightDirection);

	if (distanceOfVertex > 0.0 && lengthOfLightDirectionInY < 0.0)
	{
		lightDirection = mul((distanceOfVertex / (lengthOfLightDirectionInY)), lightDirection);
	}
	else
	{
		lightDirection = float4(0.0, 0.0, 0.0, 0.0);
	}

	output.position = mul((vertexInWorldSpace + lightDirection), mul(viewMatrix, ProjectionMatrix));

	/*float3 sun = float3(-0.5f, -0.1f, -0.5f);
	
	float4x4 temp = WVP;
	temp[1][0] = (-sun.x / sun.y);
	temp[1][2] = (-sun.z / sun.y);

	output.position = mul(position, temp);*/
	output.colour = colour;

	return output;
}