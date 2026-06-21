#pragma once

#include <algorithm>
#include <cstddef>

namespace minispire::layout {

inline constexpr float CanvasWidth = 1280.0F;
inline constexpr float CanvasHeight = 720.0F;

struct Vec2 {
    float x {0.0F};
    float y {0.0F};
};

struct Rect {
    float left {0.0F};
    float top {0.0F};
    float width {0.0F};
    float height {0.0F};
};

struct CombatLayout {
    Rect playerPanel {46.0F, 326.0F, 350.0F, 164.0F};
    Rect playerCreaturePanel {62.0F, 338.0F, 318.0F, 122.0F};
    Vec2 playerSpriteCenter {230.0F, 212.0F};
    float playerSpriteScale {0.74F};
    Rect deckInfoPanel {70.0F, 536.0F, 178.0F, 82.0F};
    Vec2 deckTextPosition {86.0F, 548.0F};
    Vec2 potionLabelPosition {70.0F, 468.0F};

    Rect enemyPanel {878.0F, 92.0F, 340.0F, 142.0F};
    Rect enemyCreaturePanel {892.0F, 104.0F, 312.0F, 120.0F};
    Vec2 enemySpriteCenter {1038.0F, 350.0F};
    float enemySpriteScale {0.78F};
    float bossSpriteScale {0.92F};

    Rect logPanel {448.0F, 92.0F, 372.0F, 274.0F};
    Vec2 logTitlePosition {470.0F, 112.0F};
    Vec2 logFirstLinePosition {470.0F, 150.0F};
    Vec2 lastMessagePosition {470.0F, 340.0F};

    Rect endTurnButton {1098.0F, 424.0F, 142.0F, 52.0F};
};

struct CreaturePanelRows {
    float titleTop {0.0F};
    float subtitleTop {0.0F};
    float statusTop {0.0F};
    float blockTop {0.0F};
    float hpBarTop {0.0F};
    float hpTextTop {0.0F};
};

inline CombatLayout combatLayout()
{
    return {};
}

inline CreaturePanelRows creaturePanelRows(Rect rect)
{
    return {rect.top + 12.0F,
            rect.top + 42.0F,
            rect.top + 64.0F,
            rect.top + 64.0F,
            rect.top + rect.height - 30.0F,
            rect.top + rect.height - 32.0F};
}

inline Rect combatHandCardRect(std::size_t index, std::size_t count)
{
    constexpr float width = 132.0F;
    constexpr float height = 178.0F;
    constexpr float leftHudWidth = 300.0F;
    constexpr float handAreaWidth = 880.0F;
    const float step = count <= 1 ? 0.0F : std::min(146.0F, (handAreaWidth - width) / static_cast<float>(count - 1));
    const float total = count <= 1 ? width : width + step * static_cast<float>(count - 1);
    const float start = leftHudWidth + (handAreaWidth - total) / 2.0F;
    return {start + static_cast<float>(index) * step, 528.0F, width, height};
}

inline Rect combatPotionSlotRect(std::size_t index)
{
    return {70.0F + static_cast<float>(index) * 152.0F, 488.0F, 138.0F, 32.0F};
}

inline Rect topBarRelicRect()
{
    return {1018.0F, 10.0F, 184.0F, 38.0F};
}

inline Rect topBarRelicDetailsRect(std::size_t relicCount)
{
    const float height = 58.0F + static_cast<float>(std::min<std::size_t>(relicCount, 7)) * 45.0F;
    return {832.0F, 64.0F, 370.0F, height};
}

inline bool contains(Rect rect, Vec2 point)
{
    return point.x >= rect.left && point.x < rect.left + rect.width && point.y >= rect.top && point.y < rect.top + rect.height;
}

inline bool topBarRelicTriggerContains(Vec2 point)
{
    return contains(topBarRelicRect(), point);
}

inline Rect spriteBounds(Vec2 center, Vec2 maxSize, float verticalOffset)
{
    return {center.x - maxSize.x / 2.0F, center.y + verticalOffset - maxSize.y / 2.0F, maxSize.x, maxSize.y};
}

inline float renderScaleForWindow(unsigned int width, unsigned int height)
{
    if (width == 0 || height == 0) {
        return 1.0F;
    }
    return std::max(0.5F, std::min(static_cast<float>(width) / CanvasWidth, static_cast<float>(height) / CanvasHeight));
}

} // namespace minispire::layout
