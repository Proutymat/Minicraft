#include "pch.h"
#include "Cube.h"

Cube::Cube() {
    model = Matrix::Identity;
}

void Cube::PushFace(Vector3 position, Vector3 up, Vector3 right) {
    auto a = vertexBuffer.PushVertex({ToVec4(position), {0.0f, 0.0f}});
    auto b = vertexBuffer.PushVertex({ToVec4(position + up), {0.0f, 1.0f}});
    auto c = vertexBuffer.PushVertex({ToVec4(position + right), {1.0f, 0.0f}});
    auto d = vertexBuffer.PushVertex({ToVec4(position + up + right), {1.0f, 1.0f}});

    indexBuffer.PushTriangle(a, b, c);
    indexBuffer.PushTriangle(d, c, b);
}

void Cube::Generate(DeviceResources* deviceRes) {
    PushFace({-0.5f, -0.5f, 0.5f}, Vector3::Up, Vector3::Right);
    PushFace({0.5f, -0.5f, 0.5f}, Vector3::Up, Vector3::Forward);
    PushFace({0.5f, -0.5f, -0.5f}, Vector3::Up, Vector3::Left);
    PushFace({-0.5f, -0.5f, -0.5f}, Vector3::Up, Vector3::Backward);
    PushFace({-0.5f, 0.5f, 0.5f}, Vector3::Forward, Vector3::Right);
    PushFace({-0.5f, -0.5f, -0.5f}, Vector3::Backward, Vector3::Right);
    
    vertexBuffer.Create(deviceRes);
    indexBuffer.Create(deviceRes);
}

void Cube::Draw(DeviceResources* deviceRes) {
    vertexBuffer.Apply(deviceRes, 0);
    indexBuffer.Apply(deviceRes);
    deviceRes->GetD3DDeviceContext()->DrawIndexed(indexBuffer.Size(), 0, 0);
}



