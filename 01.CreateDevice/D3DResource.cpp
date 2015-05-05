#include "D3DResource.h"


void D3DSwapChainRTVResource::Create(DXGIManager *pDxgi)
{
    if(m_pRTV)return;

	auto pDevice = pDxgi->GetD3DDevice();
	auto pSwapChain = pDxgi->GetSwapChain();

	// SwapChainからバックバッファを取得
    Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
    auto hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
    if (FAILED(hr))return;

    // RTVを作る
    hr = pDevice->CreateRenderTargetView(pBackBuffer.Get()
            , NULL, m_pRTV.ReleaseAndGetAddressOf());
    if (FAILED(hr))return;
}

void D3DSwapChainRTVResource::Release()
{
    m_pRTV.Reset();
}

void D3DSwapChainRTVResource::Clear(ID3D11DeviceContext *pDeviceContext, const float clearColor[4])
{
    pDeviceContext->ClearRenderTargetView(m_pRTV.Get(), clearColor);
}

