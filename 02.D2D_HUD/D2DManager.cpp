#include "D2DManager.h"
#include <sstream>


D2DManager::D2DManager()
{
}

D2DManager::~D2DManager()
{
}

void D2DManager::OnDestroyDevice()
{
	m_pBitmap.Reset();

	m_pD2DDeviceContext.Reset();
    m_pD2DDevice.Reset();
}

void D2DManager::OnWindowResize(int w, int h)
{
    if(m_pD2DDeviceContext){
        // clear rendertarget
        m_pD2DDeviceContext->SetTarget(nullptr);
    }

	m_pBitmap.Reset();
}

void D2DManager::Render(DXGIManager *pDxgi)
{
    if(!m_pD2DDeviceContext){
        // ID2D1Factory1
        Microsoft::WRL::ComPtr<ID2D1Factory1> pD2D1Factory;
        auto hr=D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED
                , IID_PPV_ARGS(pD2D1Factory.GetAddressOf()));
        if(FAILED(hr))return;

        // IDXGIDevice
        auto pDXGIDevice=pDxgi->GetDXGIDevice();
        if(!pDXGIDevice)return;

        // ID2D1Device
        hr=pD2D1Factory->CreateDevice(pDXGIDevice.Get()
                        , m_pD2DDevice.ReleaseAndGetAddressOf());
        if(FAILED(hr))return;

        // ID2D1DeviceContext
        hr=m_pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE
                , m_pD2DDeviceContext.GetAddressOf());
        if(FAILED(hr))return;
    }

    if(!m_pBitmap)
    {
        // IDXGISurface
        auto pDXGISurface=pDxgi->GetSurface();
        if(!pDXGISurface)return;

        // ID2D1Bitmap1
        const auto bp = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, 
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
		auto hr = m_pD2DDeviceContext->CreateBitmapFromDxgiSurface(pDXGISurface.Get()
                , &bp, m_pBitmap.ReleaseAndGetAddressOf());
        if(FAILED(hr))return;
    }

    // resources
    if(!m_pLightSlateGrayBrush)
    {
        // Create a gray brush.
		auto hr = m_pD2DDeviceContext->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::LightSlateGray),
                m_pLightSlateGrayBrush.ReleaseAndGetAddressOf()
                );
    }

    /*
    if(!m_pCornflowerBlueBrush)
    {
        // Create a blue brush.
        auto hr = m_pD2DDeviceContext->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
                m_pCornflowerBlueBrush.ReleaseAndGetAddressOf()
                );
    }
    */

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
		auto hr = m_pD2DDeviceContext->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                m_pBlackBrush.ReleaseAndGetAddressOf()
                );
    }

    // render d2d
    m_pD2DDeviceContext->SetTarget(m_pBitmap.Get());
    m_pD2DDeviceContext->BeginDraw();
    {
        auto pRenderTarget=m_pD2DDeviceContext.Get();

		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		//pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        D2D1_SIZE_F rtSize = pRenderTarget->GetSize();

        // Draw a grid background.
        int width = static_cast<int>(rtSize.width);
        for (int x = 0; x < width; x += 10)
        {
            pRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                    D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
                    m_pLightSlateGrayBrush.Get(),
                    0.5f
                    );
        }

        int height = static_cast<int>(rtSize.height);
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

        auto text=m_text;
        pRenderTarget->DrawText(
                text.c_str(),
                text.size(),
                m_pTextFormat.Get(),
				D2D1::RectF(0, 0, rtSize.width, rtSize.height),
				m_pBlackBrush.Get()
                );
    }
    m_pD2DDeviceContext->EndDraw();
}

