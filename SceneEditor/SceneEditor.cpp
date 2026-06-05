/**********************************************************************************
// SceneEditor (Código Fonte)
//
// Criação:     27 Abr 2016
// Atualização: 22 Jun 2025
// Compilador:  Visual C++ 2022
//
// Descrição:   Cena 3D usando múltiplos buffers, um por objeto
//
**********************************************************************************/

#include "SceneEditor.h"

// ------------------------------------------------------------------------------

Timer SceneEditor::timer;

// ------------------------------------------------------------------------------

void SceneEditor::Init()
{
    // -------------------------------------------
    // Divisórias da tela
    // -------------------------------------------

    Vertex lineVerts[4] = {
        { XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(0.0f,  1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }
    };

    lineVBuffer = new VertexBuffer<Vertex>(lineVerts, 4);

    lineCBuffer = new ConstantBuffer<Constants>();
    Constants identityMatrix;
    XMStoreFloat4x4(&identityMatrix.WorldViewProj, XMMatrixIdentity());
    lineCBuffer->Copy(&identityMatrix);

    // --------------------------------------
    // Transformação, Visualização e Projeção
    // --------------------------------------

    // posição inicial da câmera
    camera = { XM_PIDIV2, XM_PIDIV4, 5.0f };

	// inicializa a matriz de projeção perspectiva
    XMStoreFloat4x4(&Proj, XMMatrixPerspectiveFovLH(
        XMConvertToRadians(45.0f), 
        window->AspectRatio(), 
        1.0f, 100.0f));

    // inicializa a matriz de projeção ortográfica
    float aspectRatio = float(window->Width()) / float(window->Height());
	XMStoreFloat4x4(&ProjO, XMMatrixOrthographicLH(4 * aspectRatio, 4, 1.0f, 100.0f));

    // matriz de visualização frontal
    XMVECTOR posF = XMVectorSet(0, 0, 5, 1);
    XMVECTOR targetF = XMVectorZero();
    XMVECTOR upF = XMVectorSet(0, 1, 0, 0);
    views[0] = XMMatrixLookAtLH(posF, targetF, upF);

    // matriz de visualização superior
    XMVECTOR posT = XMVectorSet(0, -10, 0, 1);
    XMVECTOR targetT = XMVectorZero();
    XMVECTOR upT = XMVectorSet(0, 0, 1, 0);
    views[1] = XMMatrixLookAtLH(posT, targetT, upT);

    // matriz de visualização lateral direita
    XMVECTOR posR = XMVectorSet(10, 0, 0, 1);
    XMVECTOR targetR = XMVectorZero();
    XMVECTOR upR = XMVectorSet(0, 1, 0, 0);
    views[2] = XMMatrixLookAtLH(posR, targetR, upR);

    // ------------------
    // Viewports
    // ------------------

    // lado esquerdo superior
    viewports[0].TopLeftX = 0.0f;
    viewports[0].TopLeftY = 0.0f;
    viewports[0].Width = float(window->Width() / 2);
    viewports[0].Height = float(window->Height() / 2);
    viewports[0].MinDepth = 0.0f;
    viewports[0].MaxDepth = 1.0f;

    // lado esquerdo inferior
    viewports[1].TopLeftX = 0.0f;
    viewports[1].TopLeftY = float(window->Height() / 2);
    viewports[1].Width = float(window->Width() / 2);
    viewports[1].Height = float(window->Height() / 2);
    viewports[1].MinDepth = 0.0f;
    viewports[1].MaxDepth = 1.0f;

    // lado direito superior
    viewports[2].TopLeftX = float(window->Width() / 2);
    viewports[2].TopLeftY = 0.0f;
    viewports[2].Width = float(window->Width() / 2);
    viewports[2].Height = float(window->Height() / 2);
    viewports[2].MinDepth = 0.0f;
    viewports[2].MaxDepth = 1.0f;

    // lado direito inferior
    viewports[3].TopLeftX = float(window->Width() / 2);
    viewports[3].TopLeftY = float(window->Height() / 2);
    viewports[3].Width = float(window->Width() / 2);
    viewports[3].Height = float(window->Height() / 2);
    viewports[3].MinDepth = 0.0f;
    viewports[3].MaxDepth = 1.0f;

    // -------------------------------------------
    // Criação das Geometrias: Vértices e Índices
    // -------------------------------------------

    Box box(2.0f, 2.0f, 2.0f, Gray);
    Cylinder cylinder(1.0f, 0.5f, 3.0f, 20, 20, Gray);
    Sphere sphere(1.0f, 20, 20, Gray);
    Grid grid(3.0f, 3.0f, 20, 20, Gray);

    // -------------------------
    // Definição dos Objetos 3D
    // -------------------------

    // box
    Object boxObj;
    XMStoreFloat4x4(&boxObj.world,
        XMMatrixScaling(0.4f, 0.4f, 0.4f) *
        XMMatrixTranslation(-1.0f, 0.41f, 1.0f));
    boxObj.mesh = new Mesh(box);
    boxObj.vbuffer = new VertexBuffer<Vertex>(box);
    boxObj.ibuffer = new IndexBuffer<uint>(box);
    for (int i = 0; i < 4; i++) {
        boxObj.cbuffer[i] = new ConstantBuffer<Constants>();
    }
    scene.push_back(boxObj);

    // cylinder
    Object cylinderObj;
    XMStoreFloat4x4(&cylinderObj.world,
        XMMatrixScaling(0.5f, 0.5f, 0.5f) *
        XMMatrixTranslation(1.0f, 0.75f, -1.0f));
    cylinderObj.mesh = new Mesh(cylinder);
    cylinderObj.vbuffer = new VertexBuffer<Vertex>(cylinder);
    cylinderObj.ibuffer = new IndexBuffer<uint>(cylinder);
    for (int i = 0; i < 4; i++) {
        cylinderObj.cbuffer[i] = new ConstantBuffer<Constants>();
    }
    scene.push_back(cylinderObj);

    // sphere
    Object sphereObj;
    XMStoreFloat4x4(&sphereObj.world,
        XMMatrixScaling(0.5f, 0.5f, 0.5f) *
        XMMatrixTranslation(0.0f, 0.5f, 0.0f));
    sphereObj.mesh = new Mesh(sphere);
    sphereObj.vbuffer = new VertexBuffer<Vertex>(sphere);
    sphereObj.ibuffer = new IndexBuffer<uint>(sphere);
    for (int i = 0; i < 4; i++) {
        sphereObj.cbuffer[i] = new ConstantBuffer<Constants>();
    }
    scene.push_back(sphereObj);

    // grid
    Object gridObj;
    gridObj.mesh = new Mesh(grid);
    XMStoreFloat4x4(&gridObj.world,
        XMMatrixIdentity());
    gridObj.vbuffer = new VertexBuffer<Vertex>(grid);
    gridObj.ibuffer = new IndexBuffer<uint>(grid);
    for (int i = 0; i < 4; i++) {
        gridObj.cbuffer[i] = new ConstantBuffer<Constants>();
    }
    scene.push_back(gridObj);
 
    // ---------------------

    BuildRootSignature();
    BuildPipelineState();    

    // ----------------------

    timer.Start();
}

// ------------------------------------------------------------------------------

void SceneEditor::Update()
{
    // sai com o pressionamento da tecla ESC
    if (input->KeyPress(VK_ESCAPE))
        window->Close();

    // lógica de seleção
    if (input->KeyPress(VK_TAB))
    {
        if (!scene.empty())
        {
            if(scene.size() == 1 && selectedObjectIndex == 0)
				selectedObjectIndex = -1;
            else
            {
                selectedObjectIndex++;

                if (selectedObjectIndex >= scene.size())
                {
                    selectedObjectIndex = 0;
                }
            }
        }
    }

    if (input->KeyPress(VK_DELETE))
    {
        if (selectedObjectIndex >= 0 && selectedObjectIndex < scene.size())
        {
            graphics->WaitForGpu();

            delete scene[selectedObjectIndex].mesh;
            delete scene[selectedObjectIndex].vbuffer;
            delete scene[selectedObjectIndex].ibuffer;
            for (int i = 0; i < 4; i++) {
                delete scene[selectedObjectIndex].cbuffer[i];
            }

			// remove o objeto da cena
            scene.erase(scene.begin() + selectedObjectIndex);
            selectedObjectIndex = -1;
        }
    }

    if (input->KeyPress('B'))
    {
        Box box(2.0f, 2.0f, 2.0f, Gray);

        Object boxObj;
        XMStoreFloat4x4(&boxObj.world,
            XMMatrixScaling(0.4f, 0.4f, 0.4f));
        boxObj.mesh = new Mesh(box);
        boxObj.vbuffer = new VertexBuffer<Vertex>(box);
        boxObj.ibuffer = new IndexBuffer<uint>(box);
        for (int i = 0; i < 4; i++) {
            boxObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(boxObj);
    }

    if (input->KeyPress('C'))
    {
        Cylinder cylinder(1.0f, 0.5f, 3.0f, 20, 20, Gray);
        Object cylinderObj;
        XMStoreFloat4x4(&cylinderObj.world,
            XMMatrixScaling(0.5f, 0.5f, 0.5f));
        cylinderObj.mesh = new Mesh(cylinder);
        cylinderObj.vbuffer = new VertexBuffer<Vertex>(cylinder);
        cylinderObj.ibuffer = new IndexBuffer<uint>(cylinder);
        for (int i = 0; i < 4; i++) {
            cylinderObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(cylinderObj);
    }

    if (input->KeyPress('S'))
    {

        Sphere sphere(1.0f, 20, 20, Gray);
        Object sphereObj;
        XMStoreFloat4x4(&sphereObj.world,
            XMMatrixScaling(0.5f, 0.5f, 0.5f));
        sphereObj.mesh = new Mesh(sphere);
        sphereObj.vbuffer = new VertexBuffer<Vertex>(sphere);
        sphereObj.ibuffer = new IndexBuffer<uint>(sphere);
        for (int i = 0; i < 4; i++) {
            sphereObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(sphereObj);
    }

    if (input->KeyPress('G'))
    {
		GeoSphere sphere(1.0f, 3, Gray);
        Object sphereObj;
        XMStoreFloat4x4(&sphereObj.world,
            XMMatrixScaling(0.5f, 0.5f, 0.5f));
        sphereObj.mesh = new Mesh(sphere);
        sphereObj.vbuffer = new VertexBuffer<Vertex>(sphere);
        sphereObj.ibuffer = new IndexBuffer<uint>(sphere);
        for (int i = 0; i < 4; i++) {
            sphereObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(sphereObj);
    }


    // adicionar plano no 'X', tecla 'P' pausa a engine
    if (input->KeyPress('X'))
    {
        Grid grid(3.0f, 3.0f, 20, 20, Gray);
        Object gridObj;
        gridObj.mesh = new Mesh(grid);
        XMStoreFloat4x4(&gridObj.world,
            XMMatrixIdentity());
        gridObj.vbuffer = new VertexBuffer<Vertex>(grid);
        gridObj.ibuffer = new IndexBuffer<uint>(grid);
        for (int i = 0; i < 4; i++) {
            gridObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(gridObj);
    }

    if (input->KeyPress('Q'))
    {
		Quad quad(2.0f, 2.0f, Gray);
		Object quadObj;
		quadObj.mesh = new Mesh(quad);
		XMStoreFloat4x4(&quadObj.world,
			XMMatrixIdentity());
		quadObj.vbuffer = new VertexBuffer<Vertex>(quad);
		quadObj.ibuffer = new IndexBuffer<uint>(quad);
		for (int i = 0; i < 4; i++) {
			quadObj.cbuffer[i] = new ConstantBuffer<Constants>();
		}
		scene.push_back(quadObj);
    }

    if (input->KeyPress('1'))
    {
		GenericGeometry geo("Resources/ball.obj", Gray);

        Object geoObj;
        geoObj.mesh = new Mesh(geo);
        XMStoreFloat4x4(&geoObj.world,
            XMMatrixIdentity());
        geoObj.vbuffer = new VertexBuffer<Vertex>(geo);
        geoObj.ibuffer = new IndexBuffer<uint>(geo);
        for (int i = 0; i < 4; i++) {
            geoObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(geoObj);
    }

    if (input->KeyPress('2'))
    {
        GenericGeometry geo("Resources/thorus.obj", Gray);

        Object geoObj;
        geoObj.mesh = new Mesh(geo);
        XMStoreFloat4x4(&geoObj.world,
            XMMatrixIdentity());
        geoObj.vbuffer = new VertexBuffer<Vertex>(geo);
        geoObj.ibuffer = new IndexBuffer<uint>(geo);
        for (int i = 0; i < 4; i++) {
            geoObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(geoObj);
    }

    if (input->KeyPress('3'))
    {
        GenericGeometry geo("Resources/monkey.obj", Gray);

        Object geoObj;
        geoObj.mesh = new Mesh(geo);
        XMStoreFloat4x4(&geoObj.world,
            XMMatrixIdentity());
        geoObj.vbuffer = new VertexBuffer<Vertex>(geo);
        geoObj.ibuffer = new IndexBuffer<uint>(geo);
        for (int i = 0; i < 4; i++) {
            geoObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(geoObj);
    }

    if (input->KeyPress('4'))
    {
        GenericGeometry geo("Resources/capsule.obj", Gray);

        Object geoObj;
        geoObj.mesh = new Mesh(geo);
        XMStoreFloat4x4(&geoObj.world,
            XMMatrixIdentity());
        geoObj.vbuffer = new VertexBuffer<Vertex>(geo);
        geoObj.ibuffer = new IndexBuffer<uint>(geo);
        for (int i = 0; i < 4; i++) {
            geoObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(geoObj);
    }

    if (input->KeyPress('5'))
    {
        GenericGeometry geo("Resources/house.obj", Gray);

        Object geoObj;
        geoObj.mesh = new Mesh(geo);
        XMStoreFloat4x4(&geoObj.world,
            XMMatrixIdentity());
        geoObj.vbuffer = new VertexBuffer<Vertex>(geo);
        geoObj.ibuffer = new IndexBuffer<uint>(geo);
        for (int i = 0; i < 4; i++) {
            geoObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(geoObj);
    }

    if (input->KeyPress('6'))
    {
        GenericGeometry geo("Resources/cow.obj", Gray);

        Object geoObj;
        geoObj.mesh = new Mesh(geo);
        XMStoreFloat4x4(&geoObj.world,
            XMMatrixScaling(0.3f, 0.3f, 0.3f));
        geoObj.vbuffer = new VertexBuffer<Vertex>(geo);
        geoObj.ibuffer = new IndexBuffer<uint>(geo);
        for (int i = 0; i < 4; i++) {
            geoObj.cbuffer[i] = new ConstantBuffer<Constants>();
        }
        scene.push_back(geoObj);
    }

	// ativa ou desativa múltiplas visualizações
    if (input->KeyPress('V'))
    {
		multipleViews = !multipleViews;
    }

    if (selectedObjectIndex >= 0 && selectedObjectIndex < scene.size())
    {
        Object& obj = scene[selectedObjectIndex];
        XMMATRIX world = XMLoadFloat4x4(&obj.world);

        // translação
        // Setas (X, Z) e PageUp/PageDown (Y)
        float moveSpeed = 0.001f;
        float tx = 0.0f, ty = 0.0f, tz = 0.0f;

        if (input->KeyDown(VK_RIGHT)) tx -= moveSpeed;
        if (input->KeyDown(VK_LEFT))  tx += moveSpeed;
        if (input->KeyDown(VK_UP))    tz -= moveSpeed;
        if (input->KeyDown(VK_DOWN))  tz += moveSpeed;
        if (input->KeyDown(VK_PRIOR)) ty += moveSpeed;
        if (input->KeyDown(VK_NEXT))  ty -= moveSpeed;

        if (tx != 0.0f || ty != 0.0f || tz != 0.0f)
        {
            world = world * XMMatrixTranslation(tx, ty, tz);
        }

        // rotação
        // Teclas I, K, J, L, U, O
        float rotSpeed = 0.001f;
        float rx = 0.0f, ry = 0.0f, rz = 0.0f;

        if (input->KeyDown('I')) rx = rotSpeed; // inclinar pra frente (Eixo X)
        if (input->KeyDown('K')) rx = -rotSpeed; // inclinar pra trás (Eixo X)
        if (input->KeyDown('J')) ry = -rotSpeed; // virar esquerda (Eixo Y)
        if (input->KeyDown('L')) ry = rotSpeed; // virar direita (Eixo Y)
        if (input->KeyDown('U')) rz = rotSpeed; // rolar esquerda (Eixo Z)
        if (input->KeyDown('O')) rz = -rotSpeed; // rolar direita (Eixo Z)

        if (rx != 0.0f || ry != 0.0f || rz != 0.0f)
        {
            world = XMMatrixRotationX(rx) * XMMatrixRotationY(ry) * XMMatrixRotationZ(rz) * world;
        }

        // escala
        // - e = (os que não são do numpad)
        float scaleMultiplier = 1.0f;

        if (input->KeyDown(VK_OEM_PLUS))      scaleMultiplier = 1.001f;
        if (input->KeyDown(VK_OEM_MINUS)) scaleMultiplier = 0.999f;

        if (scaleMultiplier != 1.0f)
        {
            world = XMMatrixScaling(scaleMultiplier, scaleMultiplier, scaleMultiplier) * world;
        }

        XMStoreFloat4x4(&obj.world, world);
    }

	// atualiza posição da câmera
    camera.Update();

    // constrói a matriz de visualização
    XMVECTOR pos = XMVectorSet(camera.x, camera.y, camera.z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    views[3] = XMMatrixLookAtLH(pos, target, up);

}

// ------------------------------------------------------------------------------

void SceneEditor::Draw()
{
    // limpa o backbuffer
    graphics->Clear();

    // comandos de configuração comuns a todos os objetos
    graphics->CommandList()->SetPipelineState(pipelineState);
    graphics->CommandList()->SetGraphicsRootSignature(rootSignature);
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // desenha objetos da cena
	int objectIndex = 0;
    if (multipleViews)
    {
        drawDivisorLines();

        // carrega matriz de projeção
        XMMATRIX proj = XMLoadFloat4x4(&ProjO);

        for(int i = 0; i < 4; i++)
        {
			objectIndex = 0;
            // comandos de configuração específicos a cada objeto
            graphics->CommandList()->RSSetViewports(1, &viewports[i]);
            
            if(i == 3)
				proj = XMLoadFloat4x4(&Proj);
            else
				proj = XMLoadFloat4x4(&ProjO);

            for (auto& obj : scene)
            {
                // carrega matriz de mundo
                XMMATRIX world = XMLoadFloat4x4(&obj.world);

                // constrói matriz combinada
                XMMATRIX WorldViewProj = world * views[i] * proj;

                // atualiza o buffer constante com a matriz combinada
                Constants constants;
                XMStoreFloat4x4(&constants.WorldViewProj, XMMatrixTranspose(WorldViewProj));

				// lógica de destaque do objeto selecionado
                if (objectIndex == selectedObjectIndex)
                    constants.HighlightColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
                else
                    constants.HighlightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

                obj.cbuffer[i]->Copy(&constants);
            
                // comandos de configuração específicos a cada objeto
                graphics->CommandList()->SetGraphicsRootConstantBufferView(0, obj.cbuffer[i]->View());
                graphics->CommandList()->IASetVertexBuffers(0, 1, obj.vbuffer->View());
                graphics->CommandList()->IASetIndexBuffer(obj.ibuffer->View());

                // desenha objeto
                graphics->CommandList()->DrawIndexedInstanced(
                    obj.mesh->indexCount, 1,
                    obj.mesh->startIndex,
                    obj.mesh->baseVertex,
                    0);

				objectIndex++;
            }
        }
    }
    else 
    {
        // carrega matriz de projeção
        XMMATRIX proj = XMLoadFloat4x4(&Proj);
        for (auto& obj : scene)
        {
            // carrega matriz de mundo
            XMMATRIX world = XMLoadFloat4x4(&obj.world);

            // constrói matriz combinada
            XMMATRIX WorldViewProj = world * views[3] * proj;

            // atualiza o buffer constante com a matriz combinada
            Constants constants;
            XMStoreFloat4x4(&constants.WorldViewProj, XMMatrixTranspose(WorldViewProj));

            // lógica de destaque do objeto selecionado
            if (objectIndex == selectedObjectIndex)
                constants.HighlightColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            else
                constants.HighlightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

            obj.cbuffer[3]->Copy(&constants);

            // comandos de configuração específicos a cada objeto
            graphics->CommandList()->SetGraphicsRootConstantBufferView(0, obj.cbuffer[3]->View());
            graphics->CommandList()->IASetVertexBuffers(0, 1, obj.vbuffer->View());
            graphics->CommandList()->IASetIndexBuffer(obj.ibuffer->View());

            // desenha objeto
            graphics->CommandList()->DrawIndexedInstanced(
                obj.mesh->indexCount, 1,
                obj.mesh->startIndex,
                obj.mesh->baseVertex,
                0);

			objectIndex++;
        }
    }
    
    // apresenta o backbuffer na tela
    graphics->Present();    
}

// ------------------------------------------------------------------------------

void SceneEditor::Finalize()
{
    // espera GPU finalizar comandos pendentes
    graphics->WaitForGpu();

    // libera memória alocada
    rootSignature->Release();
    pipelineState->Release();

    linePipelineState->Release();
    delete lineVBuffer;
    delete lineCBuffer;

    for (auto& obj : scene)
    {
        delete obj.mesh;
        delete obj.vbuffer;
        delete obj.ibuffer;
        for (int i = 0; i < 4; i++) {
            delete obj.cbuffer[i];
        }
    }
}

// ------------------------------------------------------------------------------

void SceneEditor::drawDivisorLines()
{
    graphics->CommandList()->SetPipelineState(linePipelineState);
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    graphics->CommandList()->SetGraphicsRootConstantBufferView(0, lineCBuffer->View());
    graphics->CommandList()->IASetVertexBuffers(0, 1, lineVBuffer->View());
    graphics->CommandList()->DrawInstanced(4, 1, 0, 0);

    graphics->CommandList()->SetPipelineState(pipelineState);
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// ------------------------------------------------------------------------------
//                                  WinMain                                      
// ------------------------------------------------------------------------------

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    try
    {
        // cria motor e configura a janela
        Engine* engine = new Engine();
        engine->window->Mode(ASPECTRATIO);
        engine->window->Size(1024, 720);
        engine->window->Color(25, 25, 25);
        engine->window->Title("SceneEditor");
        engine->window->Icon("Icon");
        engine->window->LostFocus(SceneEditor::Pause);
        engine->window->InFocus(SceneEditor::Resume);

        // cria e executa a aplicação
        engine->Start(new SceneEditor());

        // finaliza execução
        delete engine;
    }
    catch (Error & e)
    {
        // exibe mensagem em caso de erro
        MessageBox(nullptr, e.ToString().data(), "SceneEditor", MB_OK);
    }

    return 0;
}

// ----------------------------------------------------------------------------
