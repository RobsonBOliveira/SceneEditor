/**********************************************************************************
// SceneEditor (Arquivo de Cabeçalho)
//
// Criação:     27 Abr 2016
// Atualização: 22 Jun 2025
// Compilador:  Visual C++ 2022
//
// Descrição:   Cena 3D usando múltiplos buffers, um por objeto
//
**********************************************************************************/

#include "DXUT.h"

// ------------------------------------------------------------------------------

struct Constants
{
    XMFLOAT4X4 WorldViewProj =
    { 1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f };
};

struct Object
{
    XMFLOAT4X4 world = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f };

    Mesh* mesh = nullptr;
    VertexBuffer<Vertex>* vbuffer = nullptr;
    IndexBuffer<uint>* ibuffer = nullptr;
    ConstantBuffer<Constants>* cbuffer[4];
};

// ------------------------------------------------------------------------------

class SceneEditor : public App
{
private:
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    ID3D12PipelineState* linePipelineState = nullptr;
    VertexBuffer<Vertex>* lineVBuffer = nullptr;
    ConstantBuffer<Constants>* lineCBuffer = nullptr;
    
    OrbitCamera camera;
    XMFLOAT4X4 Proj;
    XMFLOAT4X4 ProjO;
    static Timer timer;
    bool spinning = true;
	
    bool multipleViews = false;
	XMMATRIX views[4]; // frontal, superior, lateral direita, perpectiva
	D3D12_VIEWPORT viewports[4]; // tl, bl, tr, br
    
    vector<Object> scene;

public:
    void Init();
    void Update();
    void Draw();
    void Finalize();
	void drawDivisorLines();

    static void Pause()  { timer.Stop();  }
    static void Resume() { timer.Start(); }

    void BuildRootSignature();
    void BuildPipelineState();
};

// ------------------------------------------------------------------------------
