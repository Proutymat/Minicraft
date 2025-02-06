#pragma once

#include "Minicraft/Block.h"
#include "Minicraft/Cube.h"

#define WORLD_SIZE 16

class World {
    BlockId data[WORLD_SIZE * WORLD_SIZE * WORLD_SIZE];
    std::vector<Cube> cubes;

    struct ModelData {
        Matrix model;
    };
    ConstantBuffer<ModelData> constantBufferModel;
    
public:
    void Generate(DeviceResources* deviceRes);
    void Draw(DeviceResources* deviceRes);

    BlockId* GetCube(int gx, int gy, int gz);
};