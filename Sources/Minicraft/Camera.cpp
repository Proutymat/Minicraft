#include "pch.h"

#include "Camera.h"
#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera(float fov, float aspectRatio) : fov(fov) {
    UpdateAspectRatio(aspectRatio);
    view = Matrix::CreateLookAt(position, position + Vector3::Forward, Vector3::Up);
}

Camera::~Camera() {
    if (cbCamera) delete cbCamera;
    cbCamera = nullptr;
}

void Camera::UpdateAspectRatio(float aspectRatio) {
    projection = Matrix::CreatePerspectiveFieldOfView(fov * XM_PI / 180.0f, aspectRatio, nearPlane, farPlane);
}   

void Camera::Update(float dt, Keyboard::State kb, Mouse* mouse) {
    float camSpeedRot = 0.25f;
    float camSpeedMouse = 10.0f;
    float camSpeed = 2.0f;
    if (kb.LeftShift) camSpeed *= 2.0f;

    Mouse::State mstate = mouse->GetState();
    const Matrix viewInverse = view.Invert();

    if (kb.LeftControl) camSpeed *= 2.0f;

    Vector3 move;
    
    if (kb.W) move += Vector3::Forward;
    if (kb.S) move += Vector3::Backward;
    if (kb.A) move += Vector3::Left;
    if (kb.D) move += Vector3::Right;
    if (kb.Space) move += Vector3::Up;
    if (kb.LeftShift) move += Vector3::Down;

    position += Vector3::TransformNormal(move, viewInverse) * camSpeed * dt;

    if (mstate.positionMode == Mouse::MODE_RELATIVE) {
        float dx = mstate.x;
        float dy = mstate.y;
        if (mstate.rightButton) { 
            Vector3 deltaMouse = { dx, -dy,0 };
            position += Vector3::TransformNormal(deltaMouse, viewInverse) * camSpeedMouse * dt;

        } else if (mstate.leftButton) {
            rotation *= Quaternion::CreateFromAxisAngle(Vector3::TransformNormal(Vector3::Right, viewInverse), -dy * dt);
            rotation *= Quaternion::CreateFromAxisAngle(Vector3::Up, -dx * dt);
        } else {
            mouse->SetMode(Mouse::MODE_ABSOLUTE);
        }
    } else if (mstate.rightButton || mstate.leftButton) {
        mouse->SetMode(Mouse::MODE_RELATIVE);
    }

    Vector3 newForward = Vector3::Transform(Vector3::Forward, rotation);
    Vector3 newUp = Vector3::Transform(Vector3::Up, rotation);
    
    view = Matrix::CreateLookAt(
        position,
        position + newForward,      // Point ciblé (origine)
        newUp         // Orientation "haut"
    );
}

void Camera::ApplyCamera(DeviceResources* deviceRes) {
    if (!cbCamera) {
        cbCamera = new ConstantBuffer<MatrixData>();
        cbCamera->Create(deviceRes);
    }

    cbCamera->ApplyToVS(deviceRes, 1);
    
    cbCamera->data.mView = view.Transpose();
    cbCamera->data.mProjection = projection.Transpose();
    cbCamera->UpdateBuffer(deviceRes);
}