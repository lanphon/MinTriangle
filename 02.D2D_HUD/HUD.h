#pragma once
#include <string>
#include <memory>
#include <chrono>


///
/// HUDレイアウト
///
class HUD
{
    std::shared_ptr<class UIRect> m_root;

public:
    HUD();
    bool Load(const std::string &path);
    void Update(const std::chrono::milliseconds &elapsed);
    void Render(struct ID2D1DeviceContext *pContext);
};

