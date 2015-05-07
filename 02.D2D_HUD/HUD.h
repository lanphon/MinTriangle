#pragma once
#include <string>
#include <memory>
#include <chrono>
#include <list>
#include <d2d1_1.h>
#include <dwrite.h>
#include <boost/optional.hpp>

class ID2DResource
{
public:
    virtual ~ID2DResource(){}
    virtual void Release()=0;
    virtual bool Create(ID2D1DeviceContext *pRenderTarget)=0;
};


class UIItemBase
{
protected:
    // ロード時に確定する
    float m_left;
    float m_top;
	boost::optional<float> m_requiredWidth;
    boost::optional<float> m_requiredHeight;
    std::wstring m_text;

    // Layout()で計算する値
    D2D1::TypeTraits<float>::Rect m_rect;

public:
    UIItemBase()
        : m_left(0), m_top(0)
    {}
    void SetLeft(float left){ m_left=left; }
    void SetTop(float top){ m_top=top; }
    void SetWidth(float width){ m_requiredWidth=width; }
    void SetHeight(float height){ m_requiredHeight=height; }
    void SetText(const std::wstring &text){ m_text=text; }

    virtual void Layout(const D2D1::TypeTraits<FLOAT>::Rect &rect)=0;
    virtual void Render(ID2D1DeviceContext *pRenderTarget
            , IDWriteTextFormat *pTextFormat
            , ID2D1Brush *pBrush)=0;
};


class UIGroupBase: public UIItemBase
{
protected:
    std::list<std::shared_ptr<UIItemBase>> m_children;

public:
    void AddChild(const std::shared_ptr<UIItemBase> &child)
    {
        m_children.push_back(child);
    }
};


///
/// HUDレイアウト
///
class HUD
{
    std::list<std::shared_ptr<ID2DResource>> m_resources;
    std::shared_ptr<class D2DTextFormat> m_textformat;
    std::shared_ptr<class D2DSolidColorBrush> m_fg;
    std::shared_ptr<UIGroupBase> m_root;
    std::list<std::shared_ptr<UIItemBase>> m_items;

public:
    HUD();
    bool Load(const std::string &path);
    void Update(const std::chrono::milliseconds &elapsed);
    void Render(ID2D1DeviceContext *pContext);
};

