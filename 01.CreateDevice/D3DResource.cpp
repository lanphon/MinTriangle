#include "D3DResource.h"
#include "CompileShaderFromFile.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// D3DSwapChainRTVResource
//////////////////////////////////////////////////////////////////////////////
bool D3DSwapChainRTVResource::Create(DXGIManager *pDxgi)
{
    if(m_pRTV)return true;

	auto pDevice = pDxgi->GetD3DDevice();
	auto pSwapChain = pDxgi->GetSwapChain();

	// SwapChainからバックバッファを取得
    Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
    auto hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
    if (FAILED(hr))return false;
    pBackBuffer->GetDesc(&m_backbufferDesc);

    // RTVを作る
    hr = pDevice->CreateRenderTargetView(pBackBuffer.Get()
            , NULL, m_pRTV.ReleaseAndGetAddressOf());
    if (FAILED(hr))return false;

    return true;
}

void D3DSwapChainRTVResource::Release()
{
    m_pRTV.Reset();
}

void D3DSwapChainRTVResource::SetupPipeline(ID3D11DeviceContext *pDeviceContext)
{
    // Output-Merger stage
	ID3D11RenderTargetView* rtvList[] = { m_pRTV.Get() };
	pDeviceContext->OMSetRenderTargets(1, rtvList, nullptr);

    if(m_pRTV){
        // Rasterizer stage
        D3D11_VIEWPORT vp;
        vp.Width = static_cast<float>(m_backbufferDesc.Width);
        vp.Height = static_cast<float>(m_backbufferDesc.Height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        pDeviceContext->RSSetViewports(1, &vp);
    }
}

void D3DSwapChainRTVResource::Clear(ID3D11DeviceContext *pDeviceContext, const float clearColor[4])
{
    pDeviceContext->ClearRenderTargetView(m_pRTV.Get(), clearColor);
}


//////////////////////////////////////////////////////////////////////////////
// Shader
//////////////////////////////////////////////////////////////////////////////
static DXGI_FORMAT GetDxgiFormat(D3D10_REGISTER_COMPONENT_TYPE type, BYTE mask)
{
    if ((mask & 0x0F)==0x0F)
    {
        // xyzw
        switch (type)
        {
        case D3D10_REGISTER_COMPONENT_FLOAT32:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
    }

    if ((mask & 0x07)==0x07)
    {
        // xyz
        switch (type)
        {
        case D3D10_REGISTER_COMPONENT_FLOAT32:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        }
    }

    if ((mask & 0x03)==0x03)
    {
        // xy
        switch (type)
        {
        case D3D10_REGISTER_COMPONENT_FLOAT32:
            return DXGI_FORMAT_R32G32_FLOAT;
        }
    }

    if ((mask & 0x1)==0x1)
    {
        // x
        switch (type)
        {
        case D3D10_REGISTER_COMPONENT_FLOAT32:
            return DXGI_FORMAT_R32_FLOAT;
        }
    }

    return DXGI_FORMAT_UNKNOWN;
}

/// 生成する
bool D3DShader::Create(DXGIManager *pDxgi)
{
    if(m_pVsh)return true;

    auto pDevice=pDxgi->GetD3DDevice();

    // vertex shader
    {
        Microsoft::WRL::ComPtr<ID3DBlob> vblob;
        HRESULT hr = CompileShaderFromFile(m_info.path, m_info.vs
                , vblob.GetAddressOf());
        if (FAILED(hr)) return false;

        hr = pDevice->CreateVertexShader(
                vblob->GetBufferPointer(), vblob->GetBufferSize()
                , NULL, m_pVsh.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;

        // vertex shader reflection
        Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;
        hr = D3DReflect(vblob->GetBufferPointer(), vblob->GetBufferSize()
                , IID_ID3D11ShaderReflection, &pReflector);
        if (FAILED(hr)) return false;

        D3D11_SHADER_DESC shaderdesc;
        pReflector->GetDesc(&shaderdesc);

        // Create InputLayout
        std::vector<D3D11_INPUT_ELEMENT_DESC> vbElement;
        for (size_t i = 0; i < shaderdesc.InputParameters; ++i){
            D3D11_SIGNATURE_PARAMETER_DESC sigdesc;
            pReflector->GetInputParameterDesc(i, &sigdesc);

            auto format = GetDxgiFormat(sigdesc.ComponentType, sigdesc.Mask);

            D3D11_INPUT_ELEMENT_DESC eledesc = {
                sigdesc.SemanticName
                , sigdesc.SemanticIndex
                , format
                , 0 // 決め打ち
                , D3D11_APPEND_ALIGNED_ELEMENT // 決め打ち
                , D3D11_INPUT_PER_VERTEX_DATA // 決め打ち
                , 0 // 決め打ち
            };
            vbElement.push_back(eledesc);
        }

        if (!vbElement.empty()){
            hr = pDevice->CreateInputLayout(&vbElement[0], vbElement.size(),
                vblob->GetBufferPointer(), vblob->GetBufferSize(), &m_pInputLayout);
            if (FAILED(hr)) return false;
        }
    }

    // pixel shader
    {
        Microsoft::WRL::ComPtr<ID3DBlob> pblob;
        auto hr = CompileShaderFromFile(m_info.path, m_info.ps
                , pblob.GetAddressOf());
        if (FAILED(hr)) return false;

        hr = pDevice->CreatePixelShader(
                pblob->GetBufferPointer(), pblob->GetBufferSize()
                , NULL, m_pPsh.ReleaseAndGetAddressOf());
        if (FAILED(hr)) return false;
    }

    return true;
}

/// 解放する
void D3DShader::Release()
{
    m_pVsh.Reset();
    m_pPsh.Reset();
    m_pInputLayout.Reset();
}

/// パイプラインにセットする
void D3DShader::SetupPipeline(ID3D11DeviceContext *pDeviceContext)
{
    pDeviceContext->VSSetShader(m_pVsh.Get(), NULL, 0);
    pDeviceContext->PSSetShader(m_pPsh.Get(), NULL, 0);

    pDeviceContext->IASetInputLayout(m_pInputLayout.Get());
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


//////////////////////////////////////////////////////////////////////////////
// InputAssembler
//////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    DirectX::XMFLOAT4 pos;
    DirectX::XMFLOAT4 color;
};

/// 生成する
bool D3DInputAssembler::Create(DXGIManager *pDxgi)
{
    if(m_pVertexBuf)return true;

    auto pDevice=pDxgi->GetD3DDevice();

    {
        // Create VB
        auto size = 0.5f;
        Vertex pVertices[] =
        {
            // x
			{ { 1, -1, 0, 1}, { 0, 1, 0, 1 } },
			{ { -1, -1, 0, 1 }, { 1, 0, 0, 1 } },
			{ { 0, 1, 0, 1 }, { 0, 0, 1, 1 } },
		};
        unsigned int vsize = sizeof(pVertices);

        D3D11_BUFFER_DESC vdesc;
        ZeroMemory(&vdesc, sizeof(vdesc));
        vdesc.ByteWidth = vsize;
        vdesc.Usage = D3D11_USAGE_DEFAULT;
        vdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vdesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA vertexData;
        ZeroMemory(&vertexData, sizeof(vertexData));
        vertexData.pSysMem = pVertices;

        HRESULT hr = pDevice->CreateBuffer(&vdesc, &vertexData, m_pVertexBuf.GetAddressOf());
        if (FAILED(hr)){
            return false;
        }
    }

    {
        unsigned int pIndices[] =
        {
			0, 1, 2,
        };
        unsigned int isize = sizeof(pIndices);
        m_indices = isize / sizeof(pIndices[0]);

        D3D11_BUFFER_DESC idesc;
        ZeroMemory(&idesc, sizeof(idesc));
        idesc.ByteWidth = isize;
        idesc.Usage = D3D11_USAGE_DEFAULT;
        idesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        idesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA indexData;
        ZeroMemory(&indexData, sizeof(indexData));
        indexData.pSysMem = pIndices;

        HRESULT hr = pDevice->CreateBuffer(&idesc, &indexData, m_pIndexBuf.GetAddressOf());
        if (FAILED(hr)){
            return false;
        }
    }

    return true;
}

/// 解放する
void D3DInputAssembler::Release()
{
	m_pVertexBuf.Reset();
	m_pIndexBuf.Reset();
}

/// パイプラインにセットする
void D3DInputAssembler::SetupPipeline(ID3D11DeviceContext *pDeviceContext)
{
    // VBのセット
    ID3D11Buffer* pBufferTbl[] = { m_pVertexBuf.Get() };
    UINT SizeTbl[] = { sizeof(Vertex) };
    UINT OffsetTbl[] = { 0 };
    pDeviceContext->IASetVertexBuffers(0, 1, pBufferTbl, SizeTbl, OffsetTbl);

    // IBのセット
    pDeviceContext->IASetIndexBuffer(m_pIndexBuf.Get(), DXGI_FORMAT_R32_UINT, 0);

}

/// パイプラインを描画する
void D3DInputAssembler::Draw(ID3D11DeviceContext *pDeviceContext)
{
    pDeviceContext->DrawIndexed(m_indices // index count
            , 0, 0);
}

