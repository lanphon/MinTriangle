#include <Windows.h>
#include <wrl/client.h>

///
/// SingletonÇÃD3Dä«óùÉNÉâÉX
///
class D3DManager
{
	Microsoft::WRL::ComPtr<class ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<class IDXGISwapChain1> m_pSwapChain;
	Microsoft::WRL::ComPtr<class ID3D11RenderTargetView> m_pRTV;

    /// @name singleton
    /// {
    D3DManager();
    D3DManager(const D3DManager &);
    D3DManager& operator=(const D3DManager &);
public:
    static D3DManager& GetInstance();
    /// @}

	void Destroy();
    bool CreateDeviceFromDefaultAdapter();
    void Resize(int w, int h);
    void Render(HWND hWnd);

private:
	bool CreateSwapChainForWindow(HWND hWnd);
};
