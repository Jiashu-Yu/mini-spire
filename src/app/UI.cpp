#include "minispire/UI.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace minispire::ui {
namespace {

sf::String utf8String(const std::string& text)
{
    return sf::String::fromUtf8(text.begin(), text.end());
}

std::string trimForCard(const std::string& text, std::size_t maxBytes)
{
    if (text.size() <= maxBytes) {
        return text;
    }
    return text.substr(0, maxBytes) + "...";
}

std::string effectGlyph(EffectType type)
{
    switch (type) {
    case EffectType::Damage:
        return "伤";
    case EffectType::Block:
        return "盾";
    case EffectType::Draw:
        return "抽";
    case EffectType::Strength:
        return "力";
    case EffectType::Vulnerable:
        return "破";
    case EffectType::Weak:
        return "弱";
    case EffectType::Heal:
        return "愈";
    case EffectType::GainEnergy:
        return "能";
    case EffectType::Ritual:
        return "仪";
    }
    return "?";
}

sf::Color effectColor(EffectType type)
{
    switch (type) {
    case EffectType::Damage:
        return sf::Color(213, 78, 70);
    case EffectType::Block:
        return sf::Color(88, 156, 218);
    case EffectType::Draw:
        return sf::Color(154, 118, 224);
    case EffectType::Strength:
        return sf::Color(232, 162, 68);
    case EffectType::Vulnerable:
        return sf::Color(231, 122, 74);
    case EffectType::Weak:
        return sf::Color(112, 142, 190);
    case EffectType::Heal:
        return sf::Color(88, 178, 112);
    case EffectType::GainEnergy:
        return sf::Color(235, 206, 78);
    case EffectType::Ritual:
        return sf::Color(190, 92, 216);
    }
    return sf::Color::White;
}

void drawEye(sf::RenderWindow& window, sf::Vector2f center, float radius, sf::Color color)
{
    sf::CircleShape eye(radius);
    eye.setOrigin(radius, radius);
    eye.setPosition(center);
    eye.setFillColor(color);
    window.draw(eye);
}

void drawTriangle(sf::RenderWindow& window, sf::Vector2f a, sf::Vector2f b, sf::Vector2f c, sf::Color color)
{
    sf::ConvexShape triangle;
    triangle.setPointCount(3);
    triangle.setPoint(0, a);
    triangle.setPoint(1, b);
    triangle.setPoint(2, c);
    triangle.setFillColor(color);
    window.draw(triangle);
}

} // namespace

sf::Color backgroundColor()
{
    return sf::Color(20, 22, 31);
}

sf::Color panelColor()
{
    return sf::Color(35, 39, 54);
}

sf::Color accentColor()
{
    return sf::Color(237, 180, 88);
}

sf::Color cardColor(CardType type)
{
    switch (type) {
    case CardType::Attack:
        return sf::Color(110, 48, 52);
    case CardType::Skill:
        return sf::Color(44, 79, 97);
    case CardType::Power:
        return sf::Color(82, 62, 112);
    }
    return sf::Color(72, 72, 80);
}

sf::Text makeText(const ResourceManager& resources, const std::string& utf8, unsigned int size, sf::Color color)
{
    sf::Text text;
    text.setFont(resources.font());
    text.setString(utf8String(utf8));
    text.setCharacterSize(size);
    text.setFillColor(color);
    return text;
}

void drawText(sf::RenderWindow& window,
              const ResourceManager& resources,
              const std::string& utf8,
              sf::Vector2f position,
              unsigned int size,
              sf::Color color)
{
    sf::Text text = makeText(resources, utf8, size, color);
    text.setPosition(position);
    window.draw(text);
}

void drawPanel(sf::RenderWindow& window, sf::FloatRect rect, sf::Color fill, sf::Color outline, float thickness)
{
    sf::RectangleShape panel({rect.width, rect.height});
    panel.setPosition({rect.left, rect.top});
    panel.setFillColor(fill);
    panel.setOutlineColor(outline);
    panel.setOutlineThickness(thickness);
    window.draw(panel);
}

void drawBar(sf::RenderWindow& window, sf::FloatRect rect, float fraction, sf::Color fill, sf::Color back)
{
    fraction = std::clamp(fraction, 0.0F, 1.0F);
    drawPanel(window, rect, back, sf::Color(78, 80, 96), 1.0F);
    sf::RectangleShape foreground({rect.width * fraction, rect.height});
    foreground.setPosition({rect.left, rect.top});
    foreground.setFillColor(fill);
    window.draw(foreground);
}

bool pointIn(sf::FloatRect rect, sf::Vector2f point)
{
    return rect.contains(point);
}

Button::Button(sf::FloatRect bounds, std::string label)
    : bounds_(bounds)
    , label_(std::move(label))
{
}

void Button::setEnabled(bool enabled)
{
    enabled_ = enabled;
}

bool Button::enabled() const
{
    return enabled_;
}

bool Button::contains(sf::Vector2f point) const
{
    return bounds_.contains(point);
}

bool Button::clicked(const sf::Event& event, const sf::RenderWindow& window) const
{
    if (!enabled_ || event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) {
        return false;
    }
    const sf::Vector2f point = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
    return contains(point);
}

void Button::draw(sf::RenderWindow& window, const ResourceManager& resources) const
{
    const sf::Vector2i pixel = sf::Mouse::getPosition(window);
    const sf::Vector2f mouse = window.mapPixelToCoords(pixel);
    const bool hovered = enabled_ && contains(mouse);
    const sf::Color fill = enabled_ ? (hovered ? sf::Color(93, 105, 132) : sf::Color(65, 74, 96)) : sf::Color(50, 53, 62);
    const sf::Color outline = hovered ? accentColor() : sf::Color(96, 105, 128);
    drawPanel(window, bounds_, fill, outline, hovered ? 2.0F : 1.0F);

    sf::Text text = makeText(resources, label_, 20, enabled_ ? sf::Color::White : sf::Color(150, 150, 150));
    const sf::FloatRect local = text.getLocalBounds();
    text.setOrigin(local.left + local.width / 2.0F, local.top + local.height / 2.0F);
    text.setPosition(bounds_.left + bounds_.width / 2.0F, bounds_.top + bounds_.height / 2.0F - 1.0F);
    window.draw(text);
}

void drawCard(sf::RenderWindow& window,
              const ResourceManager& resources,
              const Card& card,
              sf::FloatRect rect,
              bool playable,
              bool hovered)
{
    const sf::Color base = cardColor(card.type);
    sf::Color fill = base;
    if (!playable) {
        fill = sf::Color(base.r / 2, base.g / 2, base.b / 2, 230);
    } else if (hovered) {
        fill = sf::Color(static_cast<sf::Uint8>(std::min(255, base.r + 28)),
                         static_cast<sf::Uint8>(std::min(255, base.g + 28)),
                         static_cast<sf::Uint8>(std::min(255, base.b + 28)));
    }
    drawPanel(window, rect, fill, hovered ? accentColor() : sf::Color(164, 154, 132), hovered ? 3.0F : 1.5F);

    sf::CircleShape cost(16.0F);
    cost.setPosition(rect.left + 8.0F, rect.top + 8.0F);
    cost.setFillColor(sf::Color(37, 48, 79));
    cost.setOutlineColor(sf::Color(195, 219, 255));
    cost.setOutlineThickness(1.0F);
    window.draw(cost);
    sf::Text costText = makeText(resources, std::to_string(card.cost), 18, sf::Color::White);
    sf::FloatRect costBounds = costText.getLocalBounds();
    costText.setOrigin(costBounds.left + costBounds.width / 2.0F, costBounds.top + costBounds.height / 2.0F);
    costText.setPosition(rect.left + 24.0F, rect.top + 24.0F);
    window.draw(costText);

    drawText(window, resources, trimForCard(card.name, 18), {rect.left + 45.0F, rect.top + 12.0F}, 17, sf::Color::White);
    drawText(window, resources, toString(card.type) + " / " + card.rarity, {rect.left + 12.0F, rect.top + 44.0F}, 12, sf::Color(217, 213, 198));
    drawPanel(window, {rect.left + 10.0F, rect.top + 68.0F, rect.width - 20.0F, rect.height - 82.0F}, sf::Color(244, 235, 211), sf::Color(68, 54, 42), 1.0F);
    drawText(window, resources, card.description, {rect.left + 18.0F, rect.top + 78.0F}, 14, sf::Color(34, 29, 28));

    float badgeX = rect.left + 14.0F;
    const float badgeY = rect.top + rect.height - 32.0F;
    for (const Effect& effect : card.effects) {
        sf::CircleShape badge(12.0F);
        badge.setPosition(badgeX, badgeY);
        badge.setFillColor(effectColor(effect.type));
        badge.setOutlineColor(sf::Color(40, 35, 35));
        badge.setOutlineThickness(1.0F);
        window.draw(badge);

        sf::Text glyph = makeText(resources, effectGlyph(effect.type), 12, sf::Color::White);
        const sf::FloatRect glyphBounds = glyph.getLocalBounds();
        glyph.setOrigin(glyphBounds.left + glyphBounds.width / 2.0F, glyphBounds.top + glyphBounds.height / 2.0F);
        glyph.setPosition(badgeX + 12.0F, badgeY + 11.0F);
        window.draw(glyph);
        badgeX += 28.0F;
        if (badgeX > rect.left + rect.width - 28.0F) {
            break;
        }
    }
}

void drawCreaturePanel(sf::RenderWindow& window,
                       const ResourceManager& resources,
                       const Creature& creature,
                       sf::FloatRect rect,
                       const std::string& subtitle)
{
    drawPanel(window, rect, sf::Color(41, 43, 58), sf::Color(103, 111, 136), 1.0F);
    drawText(window, resources, creature.name(), {rect.left + 16.0F, rect.top + 12.0F}, 24, sf::Color::White);
    drawText(window, resources, subtitle, {rect.left + 16.0F, rect.top + 42.0F}, 15, sf::Color(214, 209, 190));
    const float hpFraction = static_cast<float>(creature.hp()) / static_cast<float>(std::max(1, creature.maxHp()));
    drawBar(window, {rect.left + 16.0F, rect.top + rect.height - 42.0F, rect.width - 32.0F, 18.0F}, hpFraction, sf::Color(185, 58, 67));
    drawText(window, resources, std::to_string(creature.hp()) + "/" + std::to_string(creature.maxHp()), {rect.left + 24.0F, rect.top + rect.height - 44.0F}, 14);
    if (creature.block() > 0) {
        drawText(window, resources, "格挡 " + std::to_string(creature.block()), {rect.left + rect.width - 86.0F, rect.top + rect.height - 68.0F}, 15, sf::Color(159, 210, 255));
    }
    drawStatusLine(window, resources, creature, {rect.left + 16.0F, rect.top + rect.height - 68.0F});
}

void drawPlayerSprite(sf::RenderWindow& window, const ResourceManager& resources, sf::Vector2f center, float scale)
{
    sf::CircleShape aura(78.0F * scale);
    aura.setOrigin(78.0F * scale, 78.0F * scale);
    aura.setPosition(center.x, center.y + 8.0F * scale);
    aura.setFillColor(sf::Color(55, 89, 122, 72));
    window.draw(aura);

    sf::ConvexShape cloak;
    cloak.setPointCount(5);
    cloak.setPoint(0, {center.x - 44.0F * scale, center.y + 68.0F * scale});
    cloak.setPoint(1, {center.x - 26.0F * scale, center.y - 12.0F * scale});
    cloak.setPoint(2, {center.x, center.y - 52.0F * scale});
    cloak.setPoint(3, {center.x + 28.0F * scale, center.y - 12.0F * scale});
    cloak.setPoint(4, {center.x + 46.0F * scale, center.y + 68.0F * scale});
    cloak.setFillColor(sf::Color(43, 87, 117));
    cloak.setOutlineColor(sf::Color(146, 210, 238));
    cloak.setOutlineThickness(2.0F * scale);
    window.draw(cloak);

    sf::CircleShape head(21.0F * scale);
    head.setOrigin(21.0F * scale, 21.0F * scale);
    head.setPosition(center.x, center.y - 36.0F * scale);
    head.setFillColor(sf::Color(226, 203, 176));
    window.draw(head);

    sf::RectangleShape blade({72.0F * scale, 7.0F * scale});
    blade.setOrigin(10.0F * scale, 3.5F * scale);
    blade.setPosition(center.x + 8.0F * scale, center.y - 2.0F * scale);
    blade.setRotation(-24.0F);
    blade.setFillColor(sf::Color(219, 230, 236));
    blade.setOutlineColor(sf::Color(116, 156, 184));
    blade.setOutlineThickness(1.0F * scale);
    window.draw(blade);

    sf::Text sigil = makeText(resources, "旅", static_cast<unsigned int>(22.0F * scale), sf::Color(245, 231, 178));
    const sf::FloatRect bounds = sigil.getLocalBounds();
    sigil.setOrigin(bounds.left + bounds.width / 2.0F, bounds.top + bounds.height / 2.0F);
    sigil.setPosition(center.x, center.y + 12.0F * scale);
    window.draw(sigil);
}

void drawEnemySprite(sf::RenderWindow& window, const ResourceManager& resources, EnemyKind kind, sf::Vector2f center, float scale)
{
    sf::Color body = sf::Color(102, 58, 70);
    sf::Color outline = sf::Color(218, 153, 94);
    std::string glyph = "敌";
    float radius = 66.0F;

    switch (kind) {
    case EnemyKind::AshCultist:
        body = sf::Color(103, 55, 61);
        outline = sf::Color(234, 141, 75);
        glyph = "烬";
        break;
    case EnemyKind::AcidSlime:
        body = sf::Color(72, 134, 91);
        outline = sf::Color(160, 224, 112);
        glyph = "酸";
        radius = 58.0F;
        break;
    case EnemyKind::BellGuard:
        body = sf::Color(97, 91, 112);
        outline = sf::Color(222, 190, 92);
        glyph = "钟";
        break;
    case EnemyKind::ThornLurker:
        body = sf::Color(54, 107, 76);
        outline = sf::Color(163, 210, 112);
        glyph = "棘";
        break;
    case EnemyKind::CrystalWisp:
        body = sf::Color(83, 88, 151);
        outline = sf::Color(141, 225, 238);
        glyph = "晶";
        radius = 54.0F;
        break;
    case EnemyKind::EmberDuelist:
        body = sf::Color(135, 65, 48);
        outline = sf::Color(248, 164, 83);
        glyph = "决";
        radius = 70.0F;
        break;
    case EnemyKind::NullPriest:
        body = sf::Color(70, 57, 103);
        outline = sf::Color(199, 132, 228);
        glyph = "虚";
        break;
    case EnemyKind::ChronoKnight:
        body = sf::Color(92, 93, 109);
        outline = sf::Color(225, 198, 110);
        glyph = "刻";
        radius = 72.0F;
        break;
    case EnemyKind::IronSentinel:
        body = sf::Color(91, 85, 86);
        outline = sf::Color(196, 182, 156);
        glyph = "铁";
        radius = 72.0F;
        break;
    case EnemyKind::RootMatriarch:
        body = sf::Color(83, 105, 55);
        outline = sf::Color(211, 176, 86);
        glyph = "母";
        radius = 88.0F;
        break;
    case EnemyKind::ClockworkDragon:
        body = sf::Color(111, 83, 58);
        outline = sf::Color(237, 183, 82);
        glyph = "龙";
        radius = 92.0F;
        break;
    case EnemyKind::SpireArchitect:
        body = sf::Color(71, 83, 123);
        outline = sf::Color(216, 214, 168);
        glyph = "塔";
        radius = 94.0F;
        break;
    }

    sf::CircleShape shadow(radius * scale);
    shadow.setOrigin(radius * scale, radius * scale);
    shadow.setPosition(center.x, center.y + 20.0F * scale);
    shadow.setFillColor(sf::Color(0, 0, 0, 70));
    shadow.setScale(1.2F, 0.42F);
    window.draw(shadow);

    sf::CircleShape core(radius * scale);
    core.setOrigin(radius * scale, radius * scale);
    core.setPosition(center);
    core.setFillColor(body);
    core.setOutlineColor(outline);
    core.setOutlineThickness(3.0F * scale);
    window.draw(core);

    if (kind == EnemyKind::ClockworkDragon || kind == EnemyKind::SpireArchitect) {
        drawTriangle(window,
                     {center.x - 64.0F * scale, center.y - 22.0F * scale},
                     {center.x - 136.0F * scale, center.y + 16.0F * scale},
                     {center.x - 62.0F * scale, center.y + 36.0F * scale},
                     sf::Color(body.r / 2, body.g / 2, body.b / 2));
        drawTriangle(window,
                     {center.x + 64.0F * scale, center.y - 22.0F * scale},
                     {center.x + 136.0F * scale, center.y + 16.0F * scale},
                     {center.x + 62.0F * scale, center.y + 36.0F * scale},
                     sf::Color(body.r / 2, body.g / 2, body.b / 2));
    } else if (kind == EnemyKind::ThornLurker || kind == EnemyKind::RootMatriarch) {
        for (int i = -2; i <= 2; ++i) {
            const float x = center.x + static_cast<float>(i) * 28.0F * scale;
            drawTriangle(window,
                         {x, center.y - radius * scale - 18.0F * scale},
                         {x - 9.0F * scale, center.y - radius * scale + 12.0F * scale},
                         {x + 9.0F * scale, center.y - radius * scale + 12.0F * scale},
                         outline);
        }
    }

    drawEye(window, {center.x - 24.0F * scale, center.y - 10.0F * scale}, 7.0F * scale, sf::Color(249, 231, 164));
    drawEye(window, {center.x + 24.0F * scale, center.y - 10.0F * scale}, 7.0F * scale, sf::Color(249, 231, 164));

    sf::Text mark = makeText(resources, glyph, static_cast<unsigned int>(28.0F * scale), sf::Color(245, 238, 208));
    const sf::FloatRect markBounds = mark.getLocalBounds();
    mark.setOrigin(markBounds.left + markBounds.width / 2.0F, markBounds.top + markBounds.height / 2.0F);
    mark.setPosition(center.x, center.y + 26.0F * scale);
    window.draw(mark);
}

void drawStatusLine(sf::RenderWindow& window, const ResourceManager& resources, const Creature& creature, sf::Vector2f position)
{
    const std::string summary = statusSummary(creature);
    if (!summary.empty()) {
        drawText(window, resources, summary, position, 14, sf::Color(236, 207, 126));
    }
}

std::string statusSummary(const Creature& creature)
{
    std::ostringstream stream;
    bool first = true;
    for (const auto& [status, amount] : creature.statuses()) {
        if (amount <= 0) {
            continue;
        }
        if (!first) {
            stream << "  ";
        }
        first = false;
        stream << toString(status) << " " << amount;
    }
    return stream.str();
}

} // namespace minispire::ui
