#pragma once
#include <string>
#include <memory>
#include <chrono>
#include <list>
#include <d2d1_1.h>
#include <dwrite.h>


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
	D2D1::TypeTraits<FLOAT>::Rect m_rect;

public:
    virtual void Layout(const D2D1::TypeTraits<FLOAT>::Rect &rect)=0;
    virtual void Render(ID2D1DeviceContext *pRenderTarget
            , IDWriteTextFormat *pTextFormat)=0;
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
    std::shared_ptr<UIGroupBase> m_root;
    std::list<std::shared_ptr<UIItemBase>> m_items;

public:
    HUD();
    bool Load(const std::string &path);
    void Update(const std::chrono::milliseconds &elapsed);
    void Render(ID2D1DeviceContext *pContext);
};

