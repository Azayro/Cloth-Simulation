
#include "CGModelInstance.h"
#include "CGBaseModel.h"
#include "buffers.h"

CGModelInstance::CGModelInstance() {

	model = nullptr;

	T = XMFLOAT3(0.0f, 0.0f, 0.0f);
	E = XMFLOAT3(0.0f, 0.0f, 0.0f);
}


CGModelInstance::CGModelInstance(CGBaseModel *_model, const XMFLOAT3& initT, const XMFLOAT3& initE) {

	model = _model;

	T = initT;
	E = initE;
}


void CGModelInstance::translate(const XMFLOAT3& dT) {

	T = XMFLOAT3(T.x + dT.x, T.y + dT.y, T.z + dT.z);
}


void CGModelInstance::rotate(const XMFLOAT3& dE) {

	E = XMFLOAT3(E.x + dE.x, E.y + dE.y, E.z + dE.z);
}


void CGModelInstance::setupCBuffer(ID3D11DeviceContext *context, ID3D11Buffer *cbuffer) {

	worldTransformStruct	W;
	XMVECTOR				det;

	W.worldMatrix = XMMatrixRotationRollPitchYaw(E.x, E.y, E.z) * XMMatrixTranslation(T.x, T.y, T.z);
	W.normalMatrix = XMMatrixTranspose(XMMatrixInverse(&det, W.worldMatrix));
	
	mapBuffer<worldTransformStruct>(context, &W, cbuffer);
}


void CGModelInstance::render(ID3D11DeviceContext *context) {

	if (model)
		model->render(context);
}
