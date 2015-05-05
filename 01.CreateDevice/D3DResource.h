#pragma once
#include "DXGIManager.h"


///
/// SwapChain由来のRTV
///
class D3DSwapChainRTVResource: public IRenderResource
{
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRTV;

public:
    /// 生成する
    void Create(DXGIManager *pDxgi)override;

    /// 解放する
    void Release()override;

    /// レンダーターゲットをクリアする
    void Clear(ID3D11DeviceContext *pDeviceContext, const float clearColor[4]);
};

