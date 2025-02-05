#include "pch.h"
#include "Cube.h"
#include <iostream>

Cube::Cube() {
    model = Matrix::Identity;
}

void Cube::PushFace(Vector3 position, Vector3 up, Vector3 right, int textureIndex) {

    float x = textureIndex % 16;
    float y = textureIndex / 16;

    auto a = vertexBuffer.PushVertex({ToVec4(position), {1 + x, 1 + y}});
    auto b = vertexBuffer.PushVertex({ToVec4(position + up), {1 + x, 0 + y}});
    auto c = vertexBuffer.PushVertex({ToVec4(position + right), {0 + x, 1 + y}});
    auto d = vertexBuffer.PushVertex({ToVec4(position + up + right), {0 + x, 0 + y}});

    indexBuffer.PushTriangle(a, b, c);
    indexBuffer.PushTriangle(d, c, b);
}

void Cube::Generate(DeviceResources* deviceRes, BlockData blockData) {
    PushFace({-0.5f, -0.5f, 0.5f}, Vector3::Up, Vector3::Right, blockData.texIdSide);
    PushFace({0.5f, -0.5f, 0.5f}, Vector3::Up, Vector3::Forward, blockData.texIdSide);
    PushFace({0.5f, -0.5f, -0.5f}, Vector3::Up, Vector3::Left, blockData.texIdSide);
    PushFace({-0.5f, -0.5f, -0.5f}, Vector3::Up, Vector3::Backward, blockData.texIdSide);
    PushFace({-0.5f, 0.5f, 0.5f}, Vector3::Forward, Vector3::Right, blockData.texIdTop);
    PushFace({-0.5f, -0.5f, -0.5f}, Vector3::Backward, Vector3::Right, blockData.texIdBottom);
    
    vertexBuffer.Create(deviceRes);
    indexBuffer.Create(deviceRes);
}

void Cube::Draw(DeviceResources* deviceRes) {
    vertexBuffer.Apply(deviceRes, 0);
    indexBuffer.Apply(deviceRes);
    deviceRes->GetD3DDeviceContext()->DrawIndexed(indexBuffer.Size(), 0, 0);
}



