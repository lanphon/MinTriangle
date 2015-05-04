#include "D3DManager.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <assert.h>


D3DManager::D3DManager()
	: m_isFullscreen(FALSE)
{
}

D3DManager::~D3DManager()
{
    Destroy();
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

// ID3D11DeviceからIDXGIFactory2をゲットする
Microsoft::WRL::ComPtr<class IDXGIFactory2> D3DManager::GetFactory()const
{
    Microsoft::WRL::ComPtr<IDXGIFactory2> pDXGIFactory;
    if (!m_pDevice)return pDXGIFactory;

    Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice;
    HRESULT hr = m_pDevice.As(&pDXGIDevice);
    if (FAILED(hr))return pDXGIFactory;

    Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
    hr = pDXGIDevice->GetAdapter(pDXGIAdapter.GetAddressOf());
    if (FAILED(hr))return pDXGIFactory;

    hr = pDXGIAdapter->GetParent(IID_PPV_ARGS(pDXGIFactory.GetAddressOf()));
    if (FAILED(hr))return pDXGIFactory;

    return pDXGIFactory;
}

Microsoft::WRL::ComPtr<IDXGIAdapter> D3DManager::GetAdapter(
        UINT adapterIndex)const
{
    Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;

    auto pDXGIFactory=GetFactory();
    if(!pDXGIFactory)return pDXGIAdapter;

    auto hr=pDXGIFactory->EnumAdapters(adapterIndex, &pDXGIAdapter);
    if(FAILED(hr))return pDXGIAdapter;

    return pDXGIAdapter;
}

bool D3DManager::CreateDevice(UINT adapterIndex)
{
	Destroy();

	// CreateDevice
	D3D_DRIVER_TYPE dtype = D3D_DRIVER_TYPE_HARDWARE;
	UINT flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
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
	HRESULT hr = D3D11CreateDevice(GetAdapter(adapterIndex).Get()
            , dtype, nullptr, flags
            , featureLevels, numFeatureLevels, sdkVersion
            , m_pDevice.ReleaseAndGetAddressOf(), &validFeatureLevel, nullptr);
	if (FAILED(hr))return false;

	return true;
}

/// hWndに対するSwapChainを作る
bool D3DManager::CreateSwapChainForWindow(HWND hWnd)
{
    auto pDXGIFactory=GetFactory();
    if(!pDXGIFactory)return false;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC sfd = { 0 };
	sfd.Windowed = TRUE;

	DXGI_SWAP_CHAIN_DESC1 sd = { 0 };
	sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2;
	//sd.Scaling = DXGI_SCALING_NONE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	//sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	auto hr = pDXGIFactory->CreateSwapChainForHwnd(m_pDevice.Get(),
		hWnd,
		&sd,
		&sfd,
		nullptr,
		m_pSwapChain.ReleaseAndGetAddressOf());
	if (FAILED(hr))return false;

	// alt + F5のフルスクリーンを止める
	pDXGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_WINDOW_CHANGES
		| DXGI_MWA_NO_ALT_ENTER);

    return true;
}

void D3DManager::Resize(int w, int h)
{
	if (!m_pSwapChain)return;

	// レンダーターゲット破棄
	auto rc=m_pRTV.Reset();
	assert(rc == 0);

	m_resizeRequest = std::make_pair(w, h);

	Microsoft::WRL::ComPtr<IDXGIOutput> pOutput;
	BOOL isFullscreen;
	auto hr = m_pSwapChain->GetFullscreenState(&isFullscreen, pOutput.GetAddressOf());
	m_fullscreenRequest = isFullscreen;
}

void D3DManager::Render(HWND hWnd)
{
	if (!m_pSwapChain){
		if (!CreateSwapChainForWindow(hWnd))return;
	}

	if (!m_pRTV){
		DXGI_SWAP_CHAIN_DESC sd;
		m_pSwapChain->GetDesc(&sd);

		if (m_resizeRequest){
			auto hr=m_pSwapChain->ResizeBuffers(sd.BufferCount,
				m_resizeRequest->first, m_resizeRequest->second,
				sd.BufferDesc.Format,
				sd.Flags // flags
				);
			m_resizeRequest.reset();
			if (FAILED(hr)){
				return;
			}
		}

		if (m_fullscreenRequest){
			m_isFullscreen = *m_fullscreenRequest;
			m_fullscreenRequest.reset();
		}

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

