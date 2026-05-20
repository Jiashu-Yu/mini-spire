#pragma once

#include <SFML/Graphics.hpp>

namespace minispire {

class GameApp;

class Scene {
public:
    explicit Scene(GameApp& app);
    virtual ~Scene() = default;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

protected:
    GameApp& app_;
};

} // namespace minispire
