#include "D3DManager.h"
#include <d3d11.h>
#include <dxgi1_2.h>


D3DManager::D3DManager()
{
}

D3DManager& D3DManager::GetInstance()
{
    static D3DManager s_instance;
    return s_instance;
}

void D3DManager::Destroy()
{
	m_pRTV.Reset();
	m_pSwapChain.Reset();
	m_pDevice.Reset();
}

bool D3DManager::CreateDeviceFromDefaultAdapter()
{
	Destroy();

	// CreateDevice
	D3D_DRIVER_TYPE dtype = D3D_DRIVER_TYPE_HARDWARE;
	UINT flags = 0;
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	UINT numFeatureLevels = sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL);
	UINT sdkVersion = D3D11_SDK_VERSION;
	D3D_FEATURE_LEVEL validFeatureLevel;
	HRESULT hr = D3D11CreateDevice(nullptr, dtype, nullptr, flags, featureLevels, numFeatureLevels, sdkVersion
		, m_pDevice.ReleaseAndGetAddressOf(), &validFeatureLevel, nullptr);
	if (FAILED(hr))return false;

	return true;
}

/// D3D11デバイスからDXGIFactoryを得て、SwapChainを生成する。
bool D3DManager::CreateSwapChainForWindow(HWND hWnd)
{
	if (!m_pDevice)return false;

	// ID3D11DeviceからIDXGIFactory2をゲットする
	Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice;
	HRESULT hr = m_pDevice.As(&pDXGIDevice);
	if (FAILED(hr))return false;

	Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
	hr = pDXGIDevice->GetAdapter(pDXGIAdapter.GetAddressOf());
	if (FAILED(hr))return false;

	Microsoft::WRL::ComPtr<IDXGIFactory2> pDXGIFactory;
	hr = pDXGIAdapter->GetParent(IID_PPV_ARGS(pDXGIFactory.GetAddressOf()));
	if (FAILED(hr))return false;

	// hWndに対するSwapChainを作る
	DXGI_SWAP_CHAIN_DESC1 sd = { 0 };
	sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2;
	sd.Scaling = DXGI_SCALING_NONE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	hr = pDXGIFactory->CreateSwapChainForHwnd(m_pDevice.Get(),
		hWnd,
		&sd,
		nullptr,
		nullptr,
		m_pSwapChain.ReleaseAndGetAddressOf());
	if (FAILED(hr))return false;

	// alt + F5のフルスクリーンを止める
	pDXGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_WINDOW_CHANGES
		| DXGI_MWA_NO_ALT_ENTER);
}

void D3DManager::Resize(int w, int h)
{
	if (!m_pSwapChain)return;

	// レンダーターゲット破棄
	m_pRTV.Reset();

	// resize swapchain
	DXGI_SWAP_CHAIN_DESC desc;
	m_pSwapChain->GetDesc(&desc);
	m_pSwapChain->ResizeBuffers(desc.BufferCount,
		0, 0,	// ClientRect を参照する
		desc.BufferDesc.Format,
		0 // flags
		);
}

void D3DManager::Render(HWND hWnd)
{
	if (!m_pSwapChain){
		if (!CreateSwapChainForWindow(hWnd))return;
	}

	if (!m_pRTV){
		// SwapChainからバックバッファを取得
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
		auto hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
		if (FAILED(hr))return;

		// RTVを作る
		hr = m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, m_pRTV.ReleaseAndGetAddressOf());
		if (FAILED(hr))return;
	}

	{
		// レンダリング

		// DeviceContext
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext;
		m_pDevice->GetImmediateContext(pDeviceContext.GetAddressOf());

		// clear rendertarget
		float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
		pDeviceContext->ClearRenderTargetView(m_pRTV.Get(), clearColor);
	}

	// update swapchain
	m_pSwapChain->Present(NULL, NULL);
}
