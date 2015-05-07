#include "HUD.h"
#include <wrl/client.h>
#include <dwrite.h>
#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>



inline std::wstring to_WideChar(UINT uCodePage, const std::string &text)
{
	int size = MultiByteToWideChar(uCodePage, 0, text.c_str(), -1, NULL, 0);
	std::vector<wchar_t> buf(size);
	size = MultiByteToWideChar(uCodePage, 0, text.c_str(), -1, &buf[0], buf.size());
	return std::wstring(buf.begin(), buf.begin() + size);
}


class D2DSolidColorBrush: public ID2DResource
{
    D2D1::ColorF m_color;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush; 

public:
    D2DSolidColorBrush(const D2D1::ColorF &color)
        : m_color(color)
    {}
	ID2D1SolidColorBrush* Get()const{ return m_brush.Get(); }
    void Release()override { m_brush.Reset(); }
    bool Create(ID2D1DeviceContext *pRenderTarget)override
    {
        if(m_brush)return true;

        // Create a gray brush.
        auto hr = pRenderTarget->CreateSolidColorBrush(
                m_color
                , m_brush.ReleaseAndGetAddressOf()
                );
        if(FAILED(hr))return false;
        return true;
    }
};


//static const WCHAR msc_fontName[] = L"Verdana";
//static const FLOAT msc_fontSize = 50;
//D2D1::ColorF(D2D1::ColorF::Black),
class D2DTextFormat: public ID2DResource
{
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_pTextFormat;
    std::wstring m_font;
    float m_fontsize;

public:
    D2DTextFormat(const std::wstring &font, float fontsize)
        : m_font(font), m_fontsize(fontsize)
    {}
	IDWriteTextFormat* Get()const{ return m_pTextFormat.Get(); }

    void Release()override{ m_pTextFormat.Reset(); }
    bool Create(ID2D1DeviceContext *pRenderTarget)override
    {
        if(m_pTextFormat)return true;

        // Create a DirectWrite factory.
        Microsoft::WRL::ComPtr<IDWriteFactory> pDWriteFactory;
        auto hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(pDWriteFactory),
                reinterpret_cast<IUnknown **>(pDWriteFactory.GetAddressOf())
                );
        if(FAILED(hr))return false;

        // Create a DirectWrite text format object.
        hr = pDWriteFactory->CreateTextFormat(
                m_font.c_str(),
                NULL,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                m_fontsize,
                L"", //locale
                m_pTextFormat.GetAddressOf()
                );
        if (FAILED(hr))return false;

        m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

        return true;
    }
};


class UIRect: public UIItemBase
{
	std::shared_ptr<D2DSolidColorBrush> m_bg;
	std::shared_ptr<D2DSolidColorBrush> m_fg;

public:

    void Layout(const D2D1::TypeTraits<FLOAT>::Rect &rect)override
    {
		m_rect = rect;
    }

    void Render(ID2D1DeviceContext *pRenderTarget
            , IDWriteTextFormat *pTextFormat
			, ID2D1Brush *pBrush)override
    {
		if (m_bg){
			m_bg->Create(pRenderTarget);
			pRenderTarget->FillRectangle(m_rect, m_bg->Get());
		}

		if (m_fg){
			m_fg->Create(pRenderTarget);
		}

		if (!m_text.empty()){
			pRenderTarget->DrawText(
				m_text.c_str(),
				m_text.size(),
				pTextFormat,
				m_rect,
				m_fg ? m_fg->Get() : pBrush
				);
		}
    }
};


///
/// ï™äÑ
///
class UIStack: public UIGroupBase
{
    void Layout(const D2D1::TypeTraits<FLOAT>::Rect &rect)override
    {
		for (auto &child : m_children)
		{
			child->Layout(rect);
		}
    }

	void Render(ID2D1DeviceContext *pRenderTarget
		, IDWriteTextFormat *pTextFormat
		, ID2D1Brush *pBrush)override
    {
		for (auto &child : m_children)
		{
			child->Render(pRenderTarget, pTextFormat, pBrush);
		}
    }
};


static void Traverse(const boost::property_tree::ptree &pt
	, const std::shared_ptr<UIGroupBase> &item)
{
    std::shared_ptr<UIItemBase> base;
	if (auto layout = pt.get_child_optional("Layout")){
		// node
		auto node = std::make_shared<UIStack>();
        base=node;
		item->AddChild(node);

		for (auto &child : *layout)
		{
			Traverse(child.second, node);
		}
	}
	else{
        // leaf
        base=std::make_shared<UIRect>();
        item->AddChild(base);
    }

    // ëÆê´
    if(auto rect = pt.get_child_optional("Rect")){
        std::vector<float> values;
        for(auto &value: *rect)
        {
			values.push_back(boost::lexical_cast<float>(value.second.data()));
        }
		base->SetLeft(values[0]);
		base->SetTop(values[1]);
		if (values[2] != 0)base->SetWidth(values[2]);
		if (values[3] != 0)base->SetHeight(values[3]);
    }

	if (auto value = pt.get_optional<float>("Width")){
		base->SetWidth(*value);
	}

	if (auto value = pt.get_optional<float>("Height")){
		base->SetHeight(*value);
	}

	if (auto value = pt.get_optional<std::string>("Text")){
		base->SetText(to_WideChar(CP_UTF8, *value));
	}
}


HUD::HUD()
{}

bool HUD::Load(const std::string &path)
{
    boost::property_tree::ptree pt;
	boost::property_tree::read_json(path.c_str(), pt);
	if (pt.empty()){
        return false;
    }

    auto &textformat=pt.get_child("TextFormat");
	m_textformat = std::make_shared<D2DTextFormat>(L"Verdana", 50);

    m_fg=std::make_shared<D2DSolidColorBrush>(D2D1::ColorF::Black);

	// build tree
	if (auto layout = pt.get_child_optional("Layout")){
		m_root = std::make_shared<UIStack>();
		for (auto &child : *layout){
			Traverse(child.second, m_root);
		}
	}

    return true;
}

void HUD::Update(const std::chrono::milliseconds &elapsed)
{
	if (!m_root)return;
}

void HUD::Render(ID2D1DeviceContext *pRenderTarget)
{
	if (!m_root)return;

    // create
	m_textformat->Create(pRenderTarget);
    m_fg->Create(pRenderTarget);

    pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
	m_root->Layout(D2D1::RectF(0, 0, rtSize.width, rtSize.height));
    m_root->Render(pRenderTarget, m_textformat->Get(), m_fg->Get());
}

