#include <Windows.h>
#include <wrl/client.h>
#include <boost/optional.hpp>

///
/// SingletonÇÃD3Dä«óùÉNÉâÉX
///
class D3DManager
{
	Microsoft::WRL::ComPtr<class ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<class IDXGISwapChain1> m_pSwapChain;
	Microsoft::WRL::ComPtr<class ID3D11RenderTargetView> m_pRTV;

	BOOL m_isFullscreen;

	boost::optional<BOOL> m_fullscreenRequest;
	boost::optional<std::pair<int, int>> m_resizeRequest;

    /// @name singleton
    /// {
    D3DManager();
    ~D3DManager();
    D3DManager(const D3DManager &);
    D3DManager& operator=(const D3DManager &);
public:
    static D3DManager& GetInstance();
    /// @}

	void Destroy();

	Microsoft::WRL::ComPtr<class IDXGIFactory2> GetFactory()const;
    Microsoft::WRL::ComPtr<class IDXGIAdapter> GetAdapter(UINT adapterIndex)const;

    bool CreateDevice(UINT adapterIndex=0);
    void Resize(int w, int h);
    void Render(HWND hWnd);

private:
	bool CreateSwapChainForWindow(HWND hWnd);
};

