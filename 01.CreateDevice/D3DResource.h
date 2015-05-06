#pragma once
#include "DXGIManager.h"
#include "CompileShaderFromFile.h"


///
/// SwapChain由来のRTV
///
class D3DSwapChainRTVResource: public IRenderResource
{
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRTV;
    D3D11_TEXTURE2D_DESC m_backbufferDesc;

public:
    /// 生成する
    bool Create(DXGIManager *pDxgi)override;

    /// 解放する
    void Release()override;

    /// パイプラインにセットする
    void SetupPipeline(ID3D11DeviceContext *pDeviceContext)override;

    /// レンダーターゲットをクリアする
    void Clear(ID3D11DeviceContext *pDeviceContext, const float clearColor[4]);
};


///
/// Shader
///
class D3DShader: public IRenderResource
{
    ShaderInfo m_info;

    Microsoft::WRL::ComPtr<struct ID3D11VertexShader> m_pVsh;
    Microsoft::WRL::ComPtr<struct ID3D11PixelShader> m_pPsh;
    Microsoft::WRL::ComPtr<struct ID3D11InputLayout> m_pInputLayout;

public:
    void SetInfo(const ShaderInfo &info){ m_info=info; }
    ShaderInfo GetInfo()const{ return m_info; }

    /// 生成する
    bool Create(DXGIManager *pDxgi)override;

    /// 解放する
    void Release()override;

    /// パイプラインにセットする
    void SetupPipeline(ID3D11DeviceContext *pDeviceContext)override;
};


///
/// InputAssembler(頂点バッファ)
///
class D3DInputAssembler: public IRenderResource
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuf;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuf;
    int m_indices;

public:
    /// 生成する
    bool Create(DXGIManager *pDxgi)override;

    /// 解放する
    void Release()override;

    /// パイプラインにセットする
    void SetupPipeline(ID3D11DeviceContext *pDeviceContext)override;

    /// パイプラインを描画する
    void Draw(ID3D11DeviceContext *pDeviceContext);
};

