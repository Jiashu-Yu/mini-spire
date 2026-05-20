#include "minispire/GameApp.h"

#include "minispire/Scene.h"
#include "minispire/UI.h"

#include <array>
#include <filesystem>
#include <utility>

namespace minispire {

bool ResourceManager::load()
{
    const std::array<std::string, 7> candidates {
        "assets/fonts/NotoSansCJKsc-Regular.otf",
        "assets/fonts/SourceHanSansSC-Regular.otf",
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/simsun.ttc",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
    };

    for (const std::string& path : candidates) {
        if (std::filesystem::exists(path) && font_.loadFromFile(path)) {
            fontPath_ = path;
            fontLoaded_ = true;
            return true;
        }
    }
    fontLoaded_ = false;
    return false;
}

const sf::Font& ResourceManager::font() const
{
    return font_;
}

bool ResourceManager::fontLoaded() const
{
    return fontLoaded_;
}

const std::string& ResourceManager::fontPath() const
{
    return fontPath_;
}

GameApp::GameApp()
    : window_(sf::VideoMode(static_cast<unsigned int>(ui::WindowWidth), static_cast<unsigned int>(ui::WindowHeight)),
              "Mini Spire - OOP Deckbuilder",
              sf::Style::Titlebar | sf::Style::Close)
{
    window_.setFramerateLimit(60);
}

int GameApp::run()
{
    resources_.load();
    scene_ = makeMainMenuScene(*this);

    sf::Clock clock;
    while (window_.isOpen() && !quitRequested_) {
        const float dt = clock.restart().asSeconds();
        sf::Event event {};
        while (window_.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                quit();
                continue;
            }
            if (scene_) {
                scene_->handleEvent(event);
            }
            applyPendingScene();
        }

        if (scene_) {
            scene_->update(dt);
        }
        applyPendingScene();

        window_.clear(ui::backgroundColor());
        if (scene_) {
            scene_->render(window_);
        }
        window_.display();
    }
    window_.close();
    return 0;
}

void GameApp::changeScene(std::unique_ptr<Scene> scene)
{
    pendingScene_ = std::move(scene);
}

void GameApp::quit()
{
    quitRequested_ = true;
}

sf::RenderWindow& GameApp::window()
{
    return window_;
}

ResourceManager& GameApp::resources()
{
    return resources_;
}

RunController& GameApp::runState()
{
    return runState_;
}

void GameApp::applyPendingScene()
{
    if (pendingScene_) {
        scene_ = std::move(pendingScene_);
    }
}

Scene::Scene(GameApp& app)
    : app_(app)
{
}

} // namespace minispire
