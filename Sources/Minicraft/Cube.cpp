#include "pch.h"
#include "Cube.h"

Cube::Cube() {
    model = Matrix::Identity;
}

Cube::Cube(Vector3 pos) {
    model = Matrix::CreateTranslation(pos);
}

void Cube::PushFace(Vector3 position, Vector3 up, Vector3 right, const int textureIndex) {

    float x = (textureIndex % 16) / 16.f;
    float y = floor(textureIndex / 16.) / 16.;
    
    auto a = vertexBuffer.PushVertex({ToVec4(position), {x + BLOCK_TEXSIZE, y + BLOCK_TEXSIZE}});
    auto b = vertexBuffer.PushVertex({ToVec4(position + up), {x + BLOCK_TEXSIZE, y}});
    auto c = vertexBuffer.PushVertex({ToVec4(position + right), {x, y + BLOCK_TEXSIZE}});
    auto d = vertexBuffer.PushVertex({ToVec4(position + up + right), {x, y}});

    indexBuffer.PushTriangle(a, b, c);
    indexBuffer.PushTriangle(d, c, b);
}

void Cube::Generate(DeviceResources* deviceRes) {
    BlockData block = BlockData::Get(id);;
    
    PushFace({-0.5f, -0.5f, 0.5f}, Vector3::Up, Vector3::Right, block.texIdSide);
    PushFace({0.5f, -0.5f, 0.5f}, Vector3::Up, Vector3::Forward, block.texIdSide);
    PushFace({0.5f, -0.5f, -0.5f}, Vector3::Up, Vector3::Left, block.texIdSide);
    PushFace({-0.5f, -0.5f, -0.5f}, Vector3::Up, Vector3::Backward, block.texIdSide);
    PushFace({-0.5f, 0.5f, 0.5f}, Vector3::Forward, Vector3::Right, block.texIdTop);
    PushFace({-0.5f, -0.5f, -0.5f}, Vector3::Backward, Vector3::Right, block.texIdBottom);
    
    vertexBuffer.Create(deviceRes);
    indexBuffer.Create(deviceRes);
}

void Cube::Draw(DeviceResources* deviceRes) {
    vertexBuffer.Apply(deviceRes, 0);
    indexBuffer.Apply(deviceRes);
    deviceRes->GetD3DDeviceContext()->DrawIndexed(indexBuffer.Size(), 0, 0);
}



