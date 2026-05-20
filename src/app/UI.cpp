#include "minispire/UI.h"

#include <algorithm>
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
