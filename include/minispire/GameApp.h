#pragma once

#include "minispire/Core.h"

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>
#include <vector>

namespace minispire {

class Scene;

class ResourceManager {
public:
    bool load();
    const sf::Font& font() const;
    bool fontLoaded() const;
    const std::string& fontPath() const;

private:
    sf::Font font_;
    bool fontLoaded_ {false};
    std::string fontPath_;
};

class GameApp {
public:
    GameApp();
    ~GameApp();

    int run();
    void changeScene(std::unique_ptr<Scene> scene);
    void quit();

    sf::RenderWindow& window();
    ResourceManager& resources();
    RunController& runState();

private:
    void applyPendingScene();

    sf::RenderWindow window_;
    ResourceManager resources_;
    RunController runState_;
    std::unique_ptr<Scene> scene_;
    std::unique_ptr<Scene> pendingScene_;
    bool quitRequested_ {false};
};

std::unique_ptr<Scene> makeMainMenuScene(GameApp& app);
std::unique_ptr<Scene> makeMapScene(GameApp& app);
std::unique_ptr<Scene> makeCombatScene(GameApp& app);
std::unique_ptr<Scene> makeRewardScene(GameApp& app);
std::unique_ptr<Scene> makeShopScene(GameApp& app);
std::unique_ptr<Scene> makeRestScene(GameApp& app);
std::unique_ptr<Scene> makeEventScene(GameApp& app);
std::unique_ptr<Scene> makeVictoryScene(GameApp& app, bool won);

} // namespace minispire
