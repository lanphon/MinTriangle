#include "../01.CreateDevice/D3DManager.h"
#include <memory>


///
/// SingletonÇÃD3Dä«óùÉNÉâÉX
///
class D3DManagerWithD2D: public D3DManager
{
	Microsoft::WRL::ComPtr<struct ID2D1Device> m_pD2DDevice;
	Microsoft::WRL::ComPtr<struct ID2D1DeviceContext> m_pD2DDeviceContext;
    Microsoft::WRL::ComPtr<struct ID2D1Bitmap1> m_pD2DBitmap;

    std::shared_ptr<class D2DRenderUtil> m_d2dRenderer;

public:
    D3DManagerWithD2D();
    ~D3DManagerWithD2D();

	void Destroy()override;
    bool CreateDevice(UINT adapterIndex=0)override;
    void Resize(int w, int h)override;

protected:
    void RenderSelf()override;
};

