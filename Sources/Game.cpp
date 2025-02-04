//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "PerlinNoise.hpp"
#include "Engine/Shader.h"
#include "Engine/VertexLayout.h"

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

ComPtr<ID3D11Buffer> vertexBuffer;
ComPtr<ID3D11Buffer> indexBuffer;
ComPtr<ID3D11Buffer> constantBufferModel;
ComPtr<ID3D11Buffer> constantBufferCamera;

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
	GenerateInputLayout<VertexLayout_Position>(m_deviceResources.get(), basicShader);

	projection = Matrix::CreatePerspectiveFieldOfView(75.0f * XM_PI / 180.0f, (float)width / (float)height, 0.01f, 100.0f);
	
	auto device = m_deviceResources->GetD3DDevice();

	// TP: allouer vertexBuffer ici
	std::vector<VertexLayout_Position> data = {
		{{-0.7f,0.5f, 0.0f, 1.0f}}, //v0
		{{0.5f, 0.5f, 0.0f, 1.0f}}, //v1
		{{0.5f, -0.5f, 0.0f, 1.0f}}, //v2
		{{-0.5f, -0.5f, 0.0f, 1.0f}}, //v3
	};

	std::vector<uint32_t> indexData =
	{
		0,1,2,
		2,3,0
	};


	CD3D11_BUFFER_DESC desc(
		sizeof(VertexLayout_Position) * data.size(), //la description a besoin de la taille en bit de notre tableau
		D3D11_BIND_VERTEX_BUFFER); //on peut mettre plusieurs flag en gros la on utilise vertex psk on met que les positions
	
	CD3D11_BUFFER_DESC indexDesc(
		sizeof(uint32_t) * indexData.size(),
		D3D11_BIND_INDEX_BUFFER);

	CD3D11_BUFFER_DESC descModel(
		sizeof(ModelData),
		D3D11_BIND_CONSTANT_BUFFER);
	
	CD3D11_BUFFER_DESC descCamera(
		sizeof(CameraData),
		D3D11_BIND_CONSTANT_BUFFER);
	
	D3D11_SUBRESOURCE_DATA initData = {}; //obliger de l'initialisé psk sinon ca ne marche qu'en debug et pas en release
	D3D11_SUBRESOURCE_DATA indexInitData = {};

	indexInitData.pSysMem = indexData.data();
	initData.pSysMem = data.data(); //data.data() ca donne un ptr vers le premier float

	device->CreateBuffer(&desc, &initData, vertexBuffer.ReleaseAndGetAddressOf());
	device->CreateBuffer(&indexDesc, &indexInitData, indexBuffer.ReleaseAndGetAddressOf());
	device->CreateBuffer(&descModel, nullptr, constantBufferModel.ReleaseAndGetAddressOf());
	device->CreateBuffer(&descCamera, nullptr, constantBufferCamera.ReleaseAndGetAddressOf());
	
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
		Vector3(2 * sin(timer.GetTotalSeconds()), 0, 2 * cos(timer.GetTotalSeconds())),
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
	ApplyInputLayout<VertexLayout_Position>(m_deviceResources.get());

	basicShader->Apply(m_deviceResources.get());

	// TP: Tracer votre vertex buffer ici
	ID3D11Buffer* vbs[] = {vertexBuffer.Get()};
	const UINT strides[] = {sizeof(VertexLayout_Position)};
	const UINT offsets[] = {0};

	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	ModelData dataModel = {};
	dataModel.model = Matrix::CreateTranslation(Vector3(0.5f, 0, 0)).Transpose();
	CameraData dataCamera = {};
	dataCamera.view = view.Transpose();
	dataCamera.projection = projection.Transpose();
	context->UpdateSubresource(constantBufferModel.Get(), 0, nullptr, &dataModel, 0, 0);
	context->UpdateSubresource(constantBufferCamera.Get(), 0, nullptr, &dataCamera, 0, 0);

	ID3D11Buffer* cbs[] = { constantBufferModel.Get(), constantBufferCamera.Get() };
	context->VSSetConstantBuffers(0, 2, cbs);

	context->DrawIndexed(6, 0, 0);
	
	// envoie nos commandes au GPU pour etre afficher � l'�cran
	m_deviceResources->Present();
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
