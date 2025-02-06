#pragma once

#include "Engine/VertexLayout.h"
#include "Engine/Buffers.h"
#include "Minicraft/Block.h"


struct Orientation
{
    int position;
    int color;
};

class Cube {
    VertexBuffer<VertexLayout_PositionUV> vertexBuffer;
    IndexBuffer indexBuffer;

public:
    Matrix model;
    BlockId id = BlockId::GRASS;
        
    Cube();
    Cube(Vector3 position);

    void Draw(DeviceResources* deviceResources);    
    void Generate(DeviceResources* deviceResources);

private:
    Vector4 ToVec4(const Vector3& v) {
        return Vector4(v.x, v.y, v.z, 1.0f);
    }
    void PushFace(Vector3 position, Vector3 up, Vector3 right, const int textureIndex);
};
