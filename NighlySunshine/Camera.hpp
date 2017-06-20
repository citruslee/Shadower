#pragma once

#include <DirectXMath.h>
#include <dinput.h>

using namespace DirectX;

class Camera
{
public:
#define DEFAULTHEIGHT 100.0f
	double deg2rad(double degrees) {
		return degrees * 4.0 * atan(1.0) / 180.0;
	}

	//Custom LookAt method which is not inverted like in DirectXMath
	//works the very same though
	XMMATRIX LookAt(XMVECTOR pos, XMVECTOR target, XMVECTOR up)
	{
		//float3 z = normalize(target - pos);
		//float3 y = normalize(upVector);
		//float3 x = normalize(cross(y, z));
		//y = cross(z, x);
		auto z = XMVector3Normalize(target - pos);
		auto y = XMVector3Normalize(up);
		auto x = XMVector3Normalize(XMVector3Cross(y, z));
		y = XMVector3Cross(z, x);

		auto lookatmat = XMMatrixSet(x.m128_f32[0], x.m128_f32[1], x.m128_f32[2], 0, y.m128_f32[0], y.m128_f32[1], y.m128_f32[2], 0, z.m128_f32[0], z.m128_f32[1], z.m128_f32[2], 0, pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2], 1);
		return lookatmat;
	}

	void UpdateCamera()
	{
		XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR DefaultUp = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMVECTOR camTopDown = XMVectorSet(0.0f, DEFAULTHEIGHT, 1.0f, 0.0f);
		XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		
		XMMATRIX camRotationMatrix;

		camRotationMatrix = XMMatrixRotationRollPitchYaw(deg2rad(90), 0, 0);
		camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
		camTarget = XMVector3Normalize(camTarget);

		// Free-Look Camera
		camForward = XMVector3TransformCoord(DefaultUp, camRotationMatrix);
		camTopDown = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
		camRight = XMVector3TransformCoord(DefaultRight, camRotationMatrix);
		camUp = XMVector3Cross(camForward, camRight);
		///////////////**************new**************////////////////////

		camPosition += moveLeftRight*camRight;
		camPosition += moveBackForward*camForward;
		camPosition += moveTopDown*camTopDown;
		moveLeftRight = 0.0f;
		moveBackForward = 0.0f;
		moveTopDown = 0.0f;

		camTarget = camPosition + camTarget;

		camView = LookAt(camPosition, camTarget, camUp);

		camView = XMMatrixInverse(nullptr, camView);
	}

	void InitCamera(bool perspective, int width, int height)
	{
		//Camera information
		camPosition = XMVectorSet(1.0f, DEFAULTHEIGHT, 0.0f, 1.0f);
		camTarget = XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f);
		camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		//Set the View matrix
		camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

		//Set the Projection matrix
		if (perspective)
		{
			camProjection = XMMatrixPerspectiveFovLH(0.50f*3.14f, (float)width / height, 0.01f, 100000.0f);
		}
		else
		{
			camProjection = XMMatrixOrthographicLH(width, height, 1.0f, 100000.0f);
		}
	}

	void DetectInput(BYTE keyboardState[256], DIMOUSESTATE &mouseCurrState, DIMOUSESTATE &mouseLastState, double time)
	{
		float speed = 150.0f * time;

		if (keyboardState[DIK_A] & 0x80)
		{
			moveLeftRight -= speed;
		}
		if (keyboardState[DIK_D] & 0x80)
		{
			moveLeftRight += speed;
		}
		if (keyboardState[DIK_W] & 0x80)
		{
			moveBackForward -= speed;
		}
		if (keyboardState[DIK_S] & 0x80)
		{
			moveBackForward += speed;
		}
		if (keyboardState[DIK_Q] & 0x80)
		{
			moveTopDown -= speed;
		}
		if (keyboardState[DIK_E] & 0x80)
		{
			moveTopDown += speed;
		}
		
	}

	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	float moveTopDown = 0.0f;

	XMMATRIX camView;
	XMMATRIX camProjection;
	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;

#undef DEFAULTHEIGHT	
};
