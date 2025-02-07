#include "pch.h"
#include "Chunk.h"

Chunk::Chunk() {
    model = Matrix::Identity;
}

Chunk::Chunk(Vector3 pos) {
    model = Matrix::CreateTranslation(pos);
}

void Chunk::PushFace(Vector3 position, Vector3 up, Vector3 right, const int textureIndex) {

    float x = (textureIndex % 16) / 16.f;
    float y = floor(textureIndex / 16.) / 16.;
    
    auto a = vertexBuffer.PushVertex({ToVec4(position), {x + BLOCK_TEXSIZE, y + BLOCK_TEXSIZE}});
    auto b = vertexBuffer.PushVertex({ToVec4(position + up), {x + BLOCK_TEXSIZE, y}});
    auto c = vertexBuffer.PushVertex({ToVec4(position + right), {x, y + BLOCK_TEXSIZE}});
    auto d = vertexBuffer.PushVertex({ToVec4(position + up + right), {x, y}});

    indexBuffer.PushTriangle(a, b, c);
    indexBuffer.PushTriangle(d, c, b);
}

void Chunk::PushCube(float x, float y, float z, BlockData block) {
    
    PushFace({-0.5f + x, -0.5f + y, 0.5f + z}, Vector3::Up, Vector3::Right, block.texIdSide);
    PushFace({0.5f + x, -0.5f + y, 0.5f + z}, Vector3::Up, Vector3::Forward, block.texIdSide);
    PushFace({0.5f + x, -0.5f + y, -0.5f + z}, Vector3::Up, Vector3::Left, block.texIdSide);
    PushFace({-0.5f + x, -0.5f + y, -0.5f + z}, Vector3::Up, Vector3::Backward, block.texIdSide);
    PushFace({-0.5f + x, 0.5f + y, 0.5f + z}, Vector3::Forward, Vector3::Right, block.texIdTop);
    PushFace({-0.5f + x, -0.5f + y, -0.5f + z}, Vector3::Backward, Vector3::Right, block.texIdBottom);
}

void Chunk::Generate(DeviceResources* deviceResources) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                PushCube(x, y, z, BlockData::Get(GRASS));
            }
        }
    }
    
    if (vertexBuffer.Size() != 0)
        vertexBuffer.Create(deviceResources);
    if (indexBuffer.Size() != 0)
        indexBuffer.Create(deviceResources);
}


void Chunk::Draw(DeviceResources* deviceRes) {
    vertexBuffer.Apply(deviceRes, 0);
    indexBuffer.Apply(deviceRes);
    deviceRes->GetD3DDeviceContext()->DrawIndexed(indexBuffer.Size(), 0, 0);
}



