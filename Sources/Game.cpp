//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "PerlinNoise.hpp"
#include "Engine/Shader.h"
#include "Engine/Buffers.h"
#include "Engine/VertexLayout.h"
#include "Engine/Texture.h"
#include "Minicraft/Cube.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

// Global stuff
Shader* basicShader;


struct ModelData {
	Matrix model;
};
struct CameraData {
	Matrix view;
	Matrix projection;
};

Matrix view;
Matrix projection;

Cube cube;
Texture texture(L"terrain");

VertexBuffer<VertexLayout_PositionUV> vertexBuffer;
IndexBuffer indexBuffer;
ConstantBuffer<ModelData> constantBufferModel;
ConstantBuffer<CameraData> constantBufferCamera;

// Game
Game::Game() noexcept(false) {
	m_deviceResources = std::make_unique<DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT, 2);
	m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game() {
	delete basicShader;
	g_inputLayouts.clear();
}

void Game::Initialize(HWND window, int width, int height) {
	// Create input devices
	m_gamePad = std::make_unique<GamePad>();
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);

	// Initialize the Direct3D resources
	m_deviceResources->SetWindow(window, width, height);
	m_deviceResources->CreateDeviceResources();
	m_deviceResources->CreateWindowSizeDependentResources();

	basicShader = new Shader(L"Basic");
	basicShader->Create(m_deviceResources.get());
	GenerateInputLayout<VertexLayout_PositionUV>(m_deviceResources.get(), basicShader);

	texture.Create(m_deviceResources.get());
	
	projection = Matrix::CreatePerspectiveFieldOfView(75.0f * XM_PI / 180.0f, (float)width / (float)height, 0.01f, 100.0f);

	cube.Generate(m_deviceResources.get());
	
	// Vertex buffer
	vertexBuffer.PushVertex({{-0.5f,  0.5f,  0.0f, 1.0f}, {0.0f, 1.0f}});
	vertexBuffer.PushVertex({{ 0.5f, -0.5f,  0.0f, 1.0f}, {1.0f, 0.0f}}); // v1
	vertexBuffer.PushVertex({{-0.5f, -0.5f,  0.0f, 1.0f}, {0.0f, 0.0f}}); // v2
	vertexBuffer.PushVertex({{ 0.5f,  0.5f,  0.0f, 1.0f}, {1.0f, 1.0f}}); // v3
	vertexBuffer.Create(m_deviceResources.get());

	// Index buffer
	indexBuffer.PushTriangle(0, 1, 2);
	indexBuffer.PushTriangle(0, 3, 1);
	indexBuffer.Create(m_deviceResources.get());

	// Model and Camera buffer
	constantBufferModel.Create(m_deviceResources.get());
	constantBufferCamera.Create(m_deviceResources.get());
}

void Game::Tick() {
	// DX::StepTimer will compute the elapsed time and call Update() for us
	// We pass Update as a callback to Tick() because StepTimer can be set to a "fixed time" step mode, allowing us to call Update multiple time in a row if the framerate is too low (useful for physics stuffs)
	m_timer.Tick([&]() { Update(m_timer); });

	Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer) {
	auto const kb = m_keyboard->GetState();
	auto const ms = m_mouse->GetState();
	
	// add kb/mouse interact here
	view = Matrix::CreateLookAt(
		Vector3(2 * sin(timer.GetTotalSeconds()), 2 * sin(timer.GetTotalSeconds()), 2 * cos(timer.GetTotalSeconds())),
		Vector3::Zero,      // Point ciblé (origine)
		Vector3::Up         // Orientation "haut"
	);
	
	if (kb.Escape)
		ExitGame();

	auto const pad = m_gamePad->GetState(0);
}

// Draws the scene.
void Game::Render() {
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
		return;

	auto context = m_deviceResources->GetD3DDeviceContext(); //API pour envoyer des commandes afin de manipuler le pipeline
	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();
	auto const viewport = m_deviceResources->GetScreenViewport();

	context->ClearRenderTargetView(renderTarget, Colors::WhiteSmoke);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);
	
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ApplyInputLayout<VertexLayout_PositionUV>(m_deviceResources.get());

	basicShader->Apply(m_deviceResources.get());
	
	constantBufferModel.ApplyToVS(m_deviceResources.get(), 0);
	constantBufferCamera.ApplyToVS(m_deviceResources.get(), 1);

	texture.Apply(m_deviceResources.get());
	
	for(float x = -10; x < 10; x += 0.2) {
		constantBufferModel.data.model = Matrix::CreateTranslation(Vector3(x, x, x)).Transpose();
		constantBufferModel.UpdateBuffer(m_deviceResources.get());
		constantBufferCamera.data.view = view.Transpose();
		constantBufferCamera.data.projection = projection.Transpose();
		constantBufferCamera.UpdateBuffer(m_deviceResources.get());

		cube.Draw(m_deviceResources.get());
	}
	
	m_deviceResources->Present(); // Envoie nos commandes au GPU pour être affiché à l'écran
}


#pragma region Message Handlers
void Game::OnActivated() {}

void Game::OnDeactivated() {}

void Game::OnSuspending() {}

void Game::OnResuming() {
	m_timer.ResetElapsedTime();
}

void Game::OnWindowMoved() {
	auto const r = m_deviceResources->GetOutputSize();
	m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange() {
	m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height) {
	if (!m_deviceResources->WindowSizeChanged(width, height))
		return;

	projection = Matrix::CreatePerspectiveFieldOfView(
		75.0f * XM_PI / 180.0f, (float)width / (float)height, 0.01f, 100.0f
	);
	
	// The windows size has changed:
	// We can realloc here any resources that depends on the target resolution (post processing etc)
}

void Game::OnDeviceLost() {
	// We have lost the graphics card, we should reset resources [TODO]
}

void Game::OnDeviceRestored() {
	// We have a new graphics card context, we should realloc resources [TODO]
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept {
	width = 800;
	height = 600;
}

#pragma endregion
