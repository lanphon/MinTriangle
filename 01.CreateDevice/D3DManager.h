#include <Windows.h>
#include <wrl/client.h>
#include <boost/optional.hpp>

///
/// SingletonÇÃD3Dä«óùÉNÉâÉX
///
class D3DManager
{
	Microsoft::WRL::ComPtr<struct ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<struct IDXGISwapChain1> m_pSwapChain;
	Microsoft::WRL::ComPtr<struct ID3D11RenderTargetView> m_pRTV;

	BOOL m_isFullscreen;

	boost::optional<BOOL> m_fullscreenRequest;
	boost::optional<std::pair<int, int>> m_resizeRequest;

    /// @name singleton
    /// {
    D3DManager(const D3DManager &);
    D3DManager& operator=(const D3DManager &);
protected:
    D3DManager();
    virtual ~D3DManager();
public:
    static D3DManager& GetInstance();
    /// @}

	virtual void Destroy();

	Microsoft::WRL::ComPtr<struct IDXGIDevice1> GetDXGIDevice()const;
	Microsoft::WRL::ComPtr<struct IDXGIFactory2> GetFactory()const;
    Microsoft::WRL::ComPtr<struct IDXGIAdapter> GetAdapter(UINT adapterIndex)const;
    Microsoft::WRL::ComPtr<struct IDXGISurface> GetSurface()const;

    virtual bool CreateDevice(UINT adapterIndex=0);
    virtual void Resize(int w, int h);
    void Render(HWND hWnd);

protected:
    bool BeginRender(HWND hWnd);
    void EndRender();
    virtual void RenderSelf();

private:
	bool CreateSwapChainForWindow(HWND hWnd);
};

