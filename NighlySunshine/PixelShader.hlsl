float4 main(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

float4 shadowmain(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}