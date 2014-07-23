#pragma once

#include <D3DX11.h>
#include <xnamath.h>

class CGPivotCamera {

private:
	float							cameraXZ, cameraY, cameraR;

public:

	CGPivotCamera()
	{
		cameraXZ = 0.0f;
		cameraY = 0.0f;
		cameraR = 10.0f;
	}

	CGPivotCamera(float init_cameraXZ, float init_cameraY, float init_cameraR)
	{
		cameraXZ = init_cameraXZ;
		cameraY = init_cameraY;
		cameraR = init_cameraR;
	}

	void rotateElevation(float t)
	{
		if ( fabsf(cameraXZ + t) <= 180.0f )
			cameraXZ += t;
	}

	void rotateOnYAxis(float t)
	{
		cameraY += t;
	}

	void zoomCamera(float zoomFactor)
	{
		if (zoomFactor > 0.0f)
			cameraR *= zoomFactor;
	}
	
	XMMATRIX dxViewTransform()
	{
		XMMATRIX rXZ = XMMatrixRotationX(cameraXZ);
		XMMATRIX rY = XMMatrixRotationY(cameraY);
		XMMATRIX T = XMMatrixTranslation(0.0, 0.0, cameraR);

		//XMMATRIX R = T * rXZ * rY;
		XMMATRIX R = rY * rXZ * T; 

		return R;
	}

	XMVECTOR getCameraPos()
	{
		XMMATRIX R = XMMatrixRotationY(cameraY) * XMMatrixRotationX(cameraXZ);

		XMVECTOR v = XMVectorSet(R._31*cameraR, R._32*cameraR, -R._33*cameraR, 1.0f);

		return v;
		// Yaw around the Y axis, a pitch around the X axis, and a roll around the Z axis.
		// XMMatrixRotationYawPitchRoll( &R, cameraY, cameraXZ, 0.0);
		// *pos = D3DXVECTOR3(R._31*cameraR, R._32*cameraR, -R._33*cameraR);
	}

};

