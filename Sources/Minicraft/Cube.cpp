#include "pch.h"
#include "Cube.h"
#include <iostream>

Cube::Cube() {
    model = Matrix::Identity;
}

void Cube::PushFace(Vector3 position, Vector3 up, Vector3 right, const int textureIndex) {

    float offset = 1. / 16.;
    float x = (textureIndex % 16) / 16.f;
    float y = floor(textureIndex / 16.) / 16.;
    
    auto a = vertexBuffer.PushVertex({ToVec4(position), {x + offset, y + offset}});
    auto b = vertexBuffer.PushVertex({ToVec4(position + up), {x + offset, y}});
    auto c = vertexBuffer.PushVertex({ToVec4(position + right), {x, y + offset}});
    auto d = vertexBuffer.PushVertex({ToVec4(position + up + right), {x, y}});

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



