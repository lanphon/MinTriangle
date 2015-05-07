#include "UIRect.h"
#include "D2DResource.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>


UIRect::UIRect()
    : m_left(0), m_top(0), m_width(0), m_height(0)
    , m_orientation(ORIENTATION_HORIZONTAL)
    , m_rect(0, 0, 0, 0)
{
}

void UIRect::Traverse(const boost::property_tree::wptree &pt)
{
    for (auto &child : pt)
    {
        if (child.first == L"Rect")
        {
			auto node = std::make_shared<UIRect>();
			AddChild(node);
			node->Traverse(child.second);
        }
		else if (child.first == L"<xmlattr>")
		{
            boost::optional<std::wstring> font;
            float fontSize=32;

			// 属性
			for (auto &attr : child.second)
			{
				if (attr.first == L"Orientation")
				{
				    auto text=attr.second.data();
                    if(text==L"Vertical"){
                        SetOrientation(ORIENTATION_VERTICAL);
                    }
                    else{
                        SetOrientation(ORIENTATION_HORIZONTAL);
                    }
				}
				/*
				if (attr.first == L"Left"){
				    auto value=boost::lexical_cast<float>(attr.second.data());	
                    SetLeft(value);
				}
				else if (attr.first == L"Top"){
				    auto value=boost::lexical_cast<float>(attr.second.data());	
                    SetTop(value);
				}
				*/
				else if (attr.first == L"Width"){
				    auto value=boost::lexical_cast<float>(attr.second.data());	
                    SetWidth(value);
				}
				else if (attr.first == L"Height"){
				    auto value=boost::lexical_cast<float>(attr.second.data());	
                    SetHeight(value);
				}
				else if (attr.first == L"Text"){
				    auto text=attr.second.data();
					SetText([text](){ return text; });
				}
				else if (attr.first == L"Font")
				{
                    font=attr.second.data();
				}
				else if (attr.first == L"FontSize")
				{
				    fontSize=boost::lexical_cast<float>(attr.second.data());	
				}
				else if (attr.first == L"Background")
				{

				}
				else{
					int a = 0;
				}
			}

            if(font){
                // font
                auto textformat = std::make_shared<D2DTextFormat>(*font, fontSize);
                SetTextFormat(textformat);
            }
		}
		else{
			assert(false);
        }
    }
}

void UIRect::Layout(const Rect &rect)
{
    m_rect=rect;

    // 子ノードによる領域分割
    if(m_orientation==ORIENTATION_HORIZONTAL)
    {
        // 幅を測る
        float left=m_rect.left;
        int noWidth=0;
        float width=0; // 幅指定の無い場合の幅
        for (auto &child : m_children)
        {
            auto w=child->GetWidth();
            if(w==0){
                ++noWidth;
            }
            else{
                auto right=left+w;
                if(right>m_rect.right){
                    right=m_rect.right;
                }
                left=right;
            }
        }
        if(noWidth){
            width=(m_rect.right - left)/noWidth;
        }

        // 分割する
        left=m_rect.left;
        for (auto &child : m_children)
        {
            auto w=child->GetWidth();
            if(w==0){
                w=width;
            }

            auto childRect=m_rect;
            childRect.left=left;
            childRect.right=left+w;
            if(childRect.right>m_rect.right){
                childRect.right=m_rect.right;
            }
            child->Layout(childRect);

            left=childRect.right;
        }
    }
    else{
        // 高さを測る
        float top=m_rect.top;
        int noHeight=0;
        float height=0; // 幅指定の無い場合の幅
        for (auto &child : m_children)
        {
            auto h=child->GetHeight();
            if(h==0){
                ++noHeight;
            }
            else{
                auto bottom=top+h;
                if(bottom>m_rect.bottom){
                    bottom=m_rect.bottom;
                }
                top=bottom;
            }
        }
        if(noHeight){
            height=(m_rect.bottom - top)/noHeight;
        }

        // 分割する
        top=m_rect.top;;
        for (auto &child : m_children)
        {
            auto h=child->GetHeight();
            if(h==0){
                h=height;
            }

            auto childRect=m_rect;
            childRect.top=top;
            childRect.bottom=top+h;
            if(childRect.bottom>m_rect.bottom){
                childRect.bottom=m_rect.bottom;
            }
            child->Layout(childRect);

            top=childRect.bottom;
        }
    }

}

void UIRect::Render(struct ID2D1DeviceContext *pRenderTarget
        , IDWriteTextFormat *pTextFormat
        , ID2D1Brush *pBG
        , ID2D1Brush *pFG
        )
{
    if (m_bg){
        m_bg->Create(pRenderTarget);
        pBG=m_bg->Get();
    }

    if (m_fg){
        m_fg->Create(pRenderTarget);
        pFG=m_fg->Get();
    }
    if(m_textformat){
        m_textformat->Create(pRenderTarget);
        pTextFormat=m_textformat->Get();
    }

    // 自ノード
    RenderSelf(pRenderTarget
            , pTextFormat
			, pBG
			, pFG
            );

    // 子ノード
    for (auto &child : m_children)
    {
        child->Render(pRenderTarget, pTextFormat, pBG, pFG);
    }
}

void UIRect::RenderSelf(ID2D1DeviceContext *pRenderTarget
        , IDWriteTextFormat *pTextFormat
        , ID2D1Brush *pBG
        , ID2D1Brush *pFG
        )
{
    if (pBG){
        pRenderTarget->FillRectangle((const D2D1_RECT_F*)&m_rect, pBG);
    }

    if (m_text && pFG){
		auto text = m_text();
        pRenderTarget->DrawText(
            text.c_str(),
            text.size(),
            pTextFormat,
            (const D2D1_RECT_F*)&m_rect,
            pFG
            );
    }
}

