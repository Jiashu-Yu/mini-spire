#pragma once

#include "minispire/Core.h"
#include "minispire/GameApp.h"

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

namespace minispire::ui {

inline constexpr float WindowWidth = 1280.0F;
inline constexpr float WindowHeight = 720.0F;

sf::Color backgroundColor();
sf::Color panelColor();
sf::Color accentColor();
sf::Color cardColor(CardType type);

void setTextRenderScale(float scale);
float textRenderScale();

sf::Text makeText(const ResourceManager& resources,
                  const std::string& utf8,
                  unsigned int size,
                  sf::Color color = sf::Color::White);

void drawText(sf::RenderWindow& window,
              const ResourceManager& resources,
              const std::string& utf8,
              sf::Vector2f position,
              unsigned int size,
              sf::Color color = sf::Color::White);

void drawPanel(sf::RenderWindow& window,
               sf::FloatRect rect,
               sf::Color fill = panelColor(),
               sf::Color outline = sf::Color(86, 96, 120),
               float thickness = 1.0F);

void drawBar(sf::RenderWindow& window,
             sf::FloatRect rect,
             float fraction,
             sf::Color fill,
             sf::Color back = sf::Color(52, 54, 68));

bool pointIn(sf::FloatRect rect, sf::Vector2f point);

class Button {
public:
    Button() = default;
    Button(sf::FloatRect bounds, std::string label);

    void setEnabled(bool enabled);
    bool enabled() const;
    bool contains(sf::Vector2f point) const;
    bool clicked(const sf::Event& event, const sf::RenderWindow& window) const;
    void draw(sf::RenderWindow& window, const ResourceManager& resources) const;

private:
    sf::FloatRect bounds_ {};
    std::string label_;
    bool enabled_ {true};
};

void drawCard(sf::RenderWindow& window,
              const ResourceManager& resources,
              const Card& card,
              sf::FloatRect rect,
              bool playable,
              bool hovered);

void drawCreaturePanel(sf::RenderWindow& window,
                       const ResourceManager& resources,
                       const Creature& creature,
                       sf::FloatRect rect,
                       const std::string& subtitle);

void drawPlayerSprite(sf::RenderWindow& window,
                      const ResourceManager& resources,
                      CharacterId character,
                      sf::Vector2f center,
                      float scale);

void drawEnemySprite(sf::RenderWindow& window,
                    const ResourceManager& resources,
                    EnemyKind kind,
                    sf::Vector2f center,
                    float scale);

void drawStatusLine(sf::RenderWindow& window,
                    const ResourceManager& resources,
                    const Creature& creature,
                    sf::Vector2f position);

std::string statusSummary(const Creature& creature);

} // namespace minispire::ui
