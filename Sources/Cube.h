#pragma once

#include "Engine/VertexLayout.h"

struct Orientation
{
    int position;
    int color;
};

class Cube
{
public:

    Cube();
    
    void PushFace(std::vector<float> vertices, std::vector<uint32_t> indices,
        VertexLayout_Position position, VertexLayout_PositionColor up, VertexLayout_PositionColor right);


private:
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    
};
