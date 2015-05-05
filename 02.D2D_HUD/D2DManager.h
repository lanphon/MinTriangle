#include "../01.CreateDevice/DXGIManager.h"
#include <memory>
#include <d2d1_1.h>
#include <dwrite.h>


///
/// D2DÇÃÉäÉ\Å[ÉXä«óù
///
class D2DManager: public IRenderResourceManager
{
	Microsoft::WRL::ComPtr<ID2D1Device> m_pD2DDevice;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_pD2DDeviceContext;

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_pBitmap;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_pLightSlateGrayBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_pCornflowerBlueBrush;

    std::wstring m_text;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_pBlackBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_pTextFormat;

public:
    D2DManager();
    ~D2DManager();
    void OnWindowResize(int w, int h)override;
    void OnDestroyDevice()override;
    void Render(DXGIManager *pDxgi)override;

    void SetText(const std::wstring &text){ m_text=text; }
};

