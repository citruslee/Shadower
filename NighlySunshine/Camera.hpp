#pragma once

#include <DirectXMath.h>
#include <dinput.h>

using namespace DirectX;

class Camera
{
public:

	void UpdateCamera()
	{
		camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
		camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
		camTarget = XMVector3Normalize(camTarget);

		// Free-Look Camera
		camRight = XMVector3TransformCoord(DefaultRight, camRotationMatrix);
		camForward = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
		camUp = XMVector3Cross(camForward, camRight);
		///////////////**************new**************////////////////////

		camPosition += moveLeftRight*camRight;
		camPosition += moveBackForward*camForward;

		moveLeftRight = 0.0f;
		moveBackForward = 0.0f;

		camTarget = camPosition + camTarget;

		camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
	}

	void InitCamera(bool perspective, int width, int height)
	{
		//Camera information
		camPosition = XMVectorSet(0.0f, 5.0f, -8.0f, 0.0f);
		camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		//Set the View matrix
		camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

		//Set the Projection matrix
		if (perspective)
		{
			camProjection = XMMatrixPerspectiveFovLH(0.50f*3.14f, (float)width / height, 1.0f, 100000.0f);
		}
		else
		{
			camProjection = XMMatrixOrthographicLH(width, height, 1.0f, 100000.0f);
		}
	}

	void DetectInput(BYTE keyboardState[256], DIMOUSESTATE mouseCurrState, DIMOUSESTATE mouseLastState, double time)
	{
		float speed = 1500.0f * time;

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
			moveBackForward += speed;
		}
		if (keyboardState[DIK_S] & 0x80)
		{
			moveBackForward -= speed;
		}
		if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
		{
			camYaw += mouseLastState.lX * 0.001f;

			camPitch += mouseCurrState.lY * 0.001f;

			mouseLastState = mouseCurrState;
		}
	}

	float rotx = 0;
	float rotz = 0;
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;

	float camYaw = 0.0f;
	float camPitch = 0.0f;

	XMMATRIX Rotationx;
	XMMATRIX Rotationz;

	XMMATRIX cube1World;
	XMMATRIX cube2World;
	XMMATRIX camView;
	XMMATRIX camProjection;

	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;
	XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMMATRIX camRotationMatrix;
};
