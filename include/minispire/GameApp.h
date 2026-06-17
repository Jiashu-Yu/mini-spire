#pragma once

#include "minispire/Core.h"

#include <SFML/Graphics.hpp>

#include <map>
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
    const sf::Texture* texture(const std::string& key) const;

private:
    bool loadTexture(const std::string& key, const std::vector<std::string>& candidates);

    sf::Font font_;
    bool fontLoaded_ {false};
    std::string fontPath_;
    std::map<std::string, sf::Texture> textures_;
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
    bool helpOpen() const;
    void toggleHelp();
    void closeHelp();

private:
    void applyPendingScene();
    bool handleGlobalEvent(const sf::Event& event);
    void drawGlobalOverlay();
    void applyLetterboxView(unsigned int width, unsigned int height);
    void toggleFullscreen();
    void recreateWindow();

    sf::VideoMode windowedMode_ {1280, 720};
    sf::View logicalView_;
    sf::RenderWindow window_;
    ResourceManager resources_;
    RunController runState_;
    std::unique_ptr<Scene> scene_;
    std::unique_ptr<Scene> pendingScene_;
    bool quitRequested_ {false};
    bool helpOpen_ {false};
    bool fullscreen_ {false};
};

std::unique_ptr<Scene> makeMainMenuScene(GameApp& app);
std::unique_ptr<Scene> makeHistoryScene(GameApp& app);
std::unique_ptr<Scene> makeCharacterSelectScene(GameApp& app);
std::unique_ptr<Scene> makeMapScene(GameApp& app);
std::unique_ptr<Scene> makeCombatScene(GameApp& app);
std::unique_ptr<Scene> makeRewardScene(GameApp& app);
std::unique_ptr<Scene> makeLevelRewardScene(GameApp& app);
std::unique_ptr<Scene> makeShopScene(GameApp& app);
std::unique_ptr<Scene> makeRestScene(GameApp& app);
std::unique_ptr<Scene> makeEventScene(GameApp& app);
std::unique_ptr<Scene> makeVictoryScene(GameApp& app, bool won);

} // namespace minispire
