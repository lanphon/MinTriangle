#pragma once
#include <Windows.h>
#include <functional>
#include <memory>
#include <string>
#include <list>
#include <boost/property_tree/ptree.hpp>


class D2DSolidColorBrush;
class D2DTextFormat;
class UIRect
{
    enum ORIENTATION_TYPE
    {
        ORIENTATION_HORIZONTAL,
        ORIENTATION_VERTICAL,
    };
    ORIENTATION_TYPE m_orientation;

    // 要求サイズ
    float m_left;
    float m_top;
	float m_width;
    float m_height;

    // 描画文字列
    typedef std::function<std::wstring()> TextFunc;
    TextFunc m_text;

    // Layout()で計算する値
public:
    struct Rect
    {
        float left;
        float top;
        float right;
        float bottom;

        Rect(float l, float t, float r, float b)
            : left(l), top(t), right(r), bottom(b)
        {}
    };
private:
    Rect m_rect;

    // 子ノード
    std::list<std::shared_ptr<UIRect>> m_children;

    // 描画ブラシ
    std::shared_ptr<D2DTextFormat> m_textformat;
	std::shared_ptr<D2DSolidColorBrush> m_bg;
	std::shared_ptr<D2DSolidColorBrush> m_fg;

public:
	UIRect();
    void SetOrientation(ORIENTATION_TYPE orientation){ m_orientation=orientation; }
    void SetLeft(float left){ m_left=left; }
    void SetTop(float top){ m_top=top; }
    void SetWidth(float width){ m_width=width; }
    float GetWidth()const{ return m_width; }
    void SetHeight(float height){ m_height=height; }
    float GetHeight()const{ return m_height; } 
    void SetText(const TextFunc &text){ m_text=text; }
    void SetFG(const std::shared_ptr<D2DSolidColorBrush> &fg){ m_fg=fg; }
    void SetTextFormat(const std::shared_ptr<D2DTextFormat> &textformat){ m_textformat=textformat; }
    void AddChild(const std::shared_ptr<UIRect> &child) { m_children.push_back(child); }
    void Traverse(const boost::property_tree::wptree &pt);
    void Layout(const Rect &rect);
    void Render(struct ID2D1DeviceContext *pRenderTarget
            , struct IDWriteTextFormat *pTextFormat=nullptr
            , struct ID2D1Brush *pBG=nullptr
            , struct ID2D1Brush *pFG=nullptr
            );

private:
    void RenderSelf(struct ID2D1DeviceContext *pRenderTarget
            , struct IDWriteTextFormat *pTextFormat
            , struct ID2D1Brush *pBG
            , struct ID2D1Brush *pFG
            );
};

