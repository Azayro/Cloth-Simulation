
#include "CGBaseModel.h"


CGBaseModel::CGBaseModel() {

	vertexBuffer = nullptr;
	indexBuffer = nullptr;
	inputLayout = nullptr;
}


CGBaseModel::~CGBaseModel() {

	if (vertexBuffer)
		vertexBuffer->Release();

	if (indexBuffer)
		indexBuffer->Release();

	if (inputLayout)
		inputLayout->Release();
}

