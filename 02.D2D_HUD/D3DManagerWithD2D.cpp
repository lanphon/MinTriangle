#include "D3DManagerWithD2D.h"
#include <d2d1_1.h>
#include <dwrite.h>
#include <sstream>


class D2DRenderUtil
{
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_pLightSlateGrayBrush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_pCornflowerBlueBrush;

public:
    void DrawGrid(const Microsoft::WRL::ComPtr<ID2D1RenderTarget> &pRenderTarget)
    {
        if(!m_pLightSlateGrayBrush)
        {
            // Create a gray brush.
            auto hr = pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::LightSlateGray),
                    m_pLightSlateGrayBrush.ReleaseAndGetAddressOf()
					);
        }

        if(!m_pCornflowerBlueBrush)
        {
            // Create a blue brush.
            auto hr = pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
					m_pCornflowerBlueBrush.ReleaseAndGetAddressOf()
                    );
        }

		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        D2D1_SIZE_F rtSize = pRenderTarget->GetSize();

        // Draw a grid background.
        int width = static_cast<int>(rtSize.width);
        int height = static_cast<int>(rtSize.height);

        for (int x = 0; x < width; x += 10)
        {
            pRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                    D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
                    m_pLightSlateGrayBrush.Get(),
                    0.5f
                    );
        }

        for (int y = 0; y < height; y += 10)
        {
            pRenderTarget->DrawLine(
                    D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
                    D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
                    m_pLightSlateGrayBrush.Get(),
                    0.5f
                    );
        }

        /*
		{
			// Draw two rectangles.
			D2D1_RECT_F rectangle1 = D2D1::RectF(
				rtSize.width / 2 - 50.0f,
				rtSize.height / 2 - 50.0f,
				rtSize.width / 2 + 50.0f,
				rtSize.height / 2 + 50.0f
				);
			// Draw a filled rectangle.
			pRenderTarget->FillRectangle(&rectangle1, m_pLightSlateGrayBrush.Get());
		}

		{
			// Draw the outline of a rectangle.
			D2D1_RECT_F rectangle2 = D2D1::RectF(
				rtSize.width / 2 - 100.0f,
				rtSize.height / 2 - 100.0f,
				rtSize.width / 2 + 100.0f,
				rtSize.height / 2 + 100.0f
				);
			pRenderTarget->DrawRectangle(&rectangle2, m_pCornflowerBlueBrush.Get());
		}
        */
    }

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_pBlackBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_pTextFormat;

    void DrawText(const Microsoft::WRL::ComPtr<ID2D1RenderTarget> &pRenderTarget
            , const std::wstring &text)
    {
        if(!m_pTextFormat){
            // Create a DirectWrite factory.
			Microsoft::WRL::ComPtr<IDWriteFactory> pDWriteFactory;
            auto hr = DWriteCreateFactory(
                    DWRITE_FACTORY_TYPE_SHARED,
					__uuidof(pDWriteFactory),
					reinterpret_cast<IUnknown **>(pDWriteFactory.GetAddressOf())
					);
            if(FAILED(hr))return;

            // Create a DirectWrite text format object.
            static const WCHAR msc_fontName[] = L"Verdana";
            static const FLOAT msc_fontSize = 50;
            hr = pDWriteFactory->CreateTextFormat(
                    msc_fontName,
                    NULL,
                    DWRITE_FONT_WEIGHT_NORMAL,
                    DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL,
                    msc_fontSize,
                    L"", //locale
                    m_pTextFormat.GetAddressOf()
                    );
            if (FAILED(hr))return;

            // Center the text horizontally and vertically.
            m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }

        if(!m_pBlackBrush){
            // Create a blue brush.
            auto hr = pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::Black),
                    m_pBlackBrush.ReleaseAndGetAddressOf()
                    );
        }

        D2D1_SIZE_F renderTargetSize = pRenderTarget->GetSize();

        pRenderTarget->DrawText(
                text.c_str(),
                text.size(),
                m_pTextFormat.Get(),
                D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height),
				m_pBlackBrush.Get()
                );
    }
};


D3DManagerWithD2D::D3DManagerWithD2D()
{
}

D3DManagerWithD2D& D3DManagerWithD2D::GetInstance()
{
    static D3DManagerWithD2D s_instance;
    return s_instance;
}

void D3DManagerWithD2D::Destroy()
{
    m_d2dRenderer.reset();
    m_pD2DBitmap.Reset();
    m_pD2DDeviceContext.Reset();
    m_pD2DDevice.Reset();

    D3DManager::Destroy();
}

bool D3DManagerWithD2D::CreateDevice(UINT adapterIndex)
{
    if(!D3DManager::CreateDevice(adapterIndex))return false;

    // ID2D1Factory1
    Microsoft::WRL::ComPtr<ID2D1Factory1> pD2D1Factory;
    auto hr=D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED
            , IID_PPV_ARGS(pD2D1Factory.GetAddressOf()));
    if(FAILED(hr))return false;

    // IDXGIDevice
    auto pDXGIDevice=GetDXGIDevice();
    if(!pDXGIDevice)return false;

    // ID2D1Device
    hr=pD2D1Factory->CreateDevice(pDXGIDevice.Get()
                    , m_pD2DDevice.ReleaseAndGetAddressOf());
    if(FAILED(hr))return false;

    // ID2D1DeviceContext
    hr=m_pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE
            , m_pD2DDeviceContext.GetAddressOf());
    if(FAILED(hr))return false;

    m_d2dRenderer=std::make_shared<D2DRenderUtil>();

    return true;
}

void D3DManagerWithD2D::Resize(int w, int h)
{
    if(m_pD2DDeviceContext){
        // clear rendertarget
        m_pD2DDeviceContext->SetTarget(nullptr);
        m_pD2DBitmap.Reset();
    }

    D3DManager::Resize(w, h);
}

void D3DManagerWithD2D::RenderSelf()
{
    if(!m_pD2DBitmap){
        // IDXGISurface
        auto pDXGISurface=GetSurface();
        if(!pDXGISurface)return;

        // ID2D1Bitmap1
        const auto bp = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, 
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
        auto hr=m_pD2DDeviceContext->CreateBitmapFromDxgiSurface(pDXGISurface.Get()
                , &bp, &m_pD2DBitmap);
        if(FAILED(hr))return;
    }

    m_pD2DDeviceContext->SetTarget(m_pD2DBitmap.Get());

    m_pD2DDeviceContext->BeginDraw();

    m_d2dRenderer->DrawGrid(m_pD2DDeviceContext);

	static int i = 0;
	++i;
	std::wstringstream ss;
	ss << i << L"Frame";

	m_d2dRenderer->DrawText(m_pD2DDeviceContext, ss.str());

    m_pD2DDeviceContext->EndDraw();
}

