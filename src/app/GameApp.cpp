#include "minispire/GameApp.h"

#include "minispire/Layout.h"
#include "minispire/Scene.h"
#include "minispire/UI.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <utility>

namespace minispire {
namespace {

sf::FloatRect helpButtonRect()
{
    return {1220.0F, 12.0F, 34.0F, 34.0F};
}

sf::FloatRect helpCloseRect()
{
    return {1018.0F, 112.0F, 82.0F, 36.0F};
}

} // namespace

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
            break;
        }
    }

    loadTexture("bg_title",
                {"assets/images/backgrounds/title_spire.png",
                 "../assets/images/backgrounds/title_spire.png",
                 "../../assets/images/backgrounds/title_spire.png"});
    loadTexture("bg_map_act1",
                {"assets/images/backgrounds/map_act1_ashen_prison.png",
                 "../assets/images/backgrounds/map_act1_ashen_prison.png",
                 "../../assets/images/backgrounds/map_act1_ashen_prison.png"});
    loadTexture("bg_map_act2",
                {"assets/images/backgrounds/map_act2_crystal_greenhouse.png",
                 "../assets/images/backgrounds/map_act2_crystal_greenhouse.png",
                 "../../assets/images/backgrounds/map_act2_crystal_greenhouse.png"});
    loadTexture("bg_map_act3",
                {"assets/images/backgrounds/map_act3_star_clock_spire.png",
                 "../assets/images/backgrounds/map_act3_star_clock_spire.png",
                 "../../assets/images/backgrounds/map_act3_star_clock_spire.png"});
    loadTexture("bg_combat_act1",
                {"assets/images/backgrounds/combat_act1_ashen_arena.png",
                 "../assets/images/backgrounds/combat_act1_ashen_arena.png",
                 "../../assets/images/backgrounds/combat_act1_ashen_arena.png"});
    loadTexture("bg_combat_act2",
                {"assets/images/backgrounds/combat_act2_crystal_arena.png",
                 "../assets/images/backgrounds/combat_act2_crystal_arena.png",
                 "../../assets/images/backgrounds/combat_act2_crystal_arena.png"});
    loadTexture("bg_combat_act3",
                {"assets/images/backgrounds/combat_act3_star_clock_arena.png",
                 "../assets/images/backgrounds/combat_act3_star_clock_arena.png",
                 "../../assets/images/backgrounds/combat_act3_star_clock_arena.png"});

    loadTexture("node_event",
                {"assets/images/ui/node_event.png",
                 "../assets/images/ui/node_event.png",
                 "../../assets/images/ui/node_event.png"});
    loadTexture("node_shop",
                {"assets/images/ui/node_shop.png",
                 "../assets/images/ui/node_shop.png",
                 "../../assets/images/ui/node_shop.png"});
    loadTexture("node_rest",
                {"assets/images/ui/node_rest.png",
                 "../assets/images/ui/node_rest.png",
                 "../../assets/images/ui/node_rest.png"});

    loadTexture("sprite_player",
                {"assets/images/sprites/player_rift_traveler.png",
                 "../assets/images/sprites/player_rift_traveler.png",
                 "../../assets/images/sprites/player_rift_traveler.png"});
    loadTexture("sprite_player_rift",
                {"assets/images/sprites/player_rift_traveler.png",
                 "../assets/images/sprites/player_rift_traveler.png",
                 "../../assets/images/sprites/player_rift_traveler.png"});
    loadTexture("sprite_player_ember",
                {"assets/images/sprites/player_ember_adept.png",
                 "../assets/images/sprites/player_ember_adept.png",
                 "../../assets/images/sprites/player_ember_adept.png"});
    loadTexture("sprite_player_crystal",
                {"assets/images/sprites/player_crystal_warden.png",
                 "../assets/images/sprites/player_crystal_warden.png",
                 "../../assets/images/sprites/player_crystal_warden.png"});
    loadTexture("enemy_ash_cultist",
                {"assets/images/sprites/enemy_ash_cultist.png",
                 "../assets/images/sprites/enemy_ash_cultist.png",
                 "../../assets/images/sprites/enemy_ash_cultist.png"});
    loadTexture("enemy_acid_slime",
                {"assets/images/sprites/enemy_acid_slime.png",
                 "../assets/images/sprites/enemy_acid_slime.png",
                 "../../assets/images/sprites/enemy_acid_slime.png"});
    loadTexture("enemy_bell_guard",
                {"assets/images/sprites/enemy_bell_guard.png",
                 "../assets/images/sprites/enemy_bell_guard.png",
                 "../../assets/images/sprites/enemy_bell_guard.png"});
    loadTexture("enemy_thorn_lurker",
                {"assets/images/sprites/enemy_thorn_lurker.png",
                 "../assets/images/sprites/enemy_thorn_lurker.png",
                 "../../assets/images/sprites/enemy_thorn_lurker.png"});
    loadTexture("enemy_crystal_wisp",
                {"assets/images/sprites/enemy_crystal_wisp.png",
                 "../assets/images/sprites/enemy_crystal_wisp.png",
                 "../../assets/images/sprites/enemy_crystal_wisp.png"});
    loadTexture("enemy_ember_duelist",
                {"assets/images/sprites/enemy_ember_duelist.png",
                 "../assets/images/sprites/enemy_ember_duelist.png",
                 "../../assets/images/sprites/enemy_ember_duelist.png"});
    loadTexture("enemy_null_priest",
                {"assets/images/sprites/enemy_null_priest.png",
                 "../assets/images/sprites/enemy_null_priest.png",
                 "../../assets/images/sprites/enemy_null_priest.png"});
    loadTexture("enemy_chrono_knight",
                {"assets/images/sprites/enemy_chrono_knight.png",
                 "../assets/images/sprites/enemy_chrono_knight.png",
                 "../../assets/images/sprites/enemy_chrono_knight.png"});
    loadTexture("enemy_iron_sentinel",
                {"assets/images/sprites/enemy_iron_sentinel.png",
                 "../assets/images/sprites/enemy_iron_sentinel.png",
                 "../../assets/images/sprites/enemy_iron_sentinel.png"});
    loadTexture("boss_root_matriarch",
                {"assets/images/sprites/boss_root_matriarch.png",
                 "../assets/images/sprites/boss_root_matriarch.png",
                 "../../assets/images/sprites/boss_root_matriarch.png"});
    loadTexture("boss_clockwork_dragon",
                {"assets/images/sprites/boss_clockwork_dragon.png",
                 "../assets/images/sprites/boss_clockwork_dragon.png",
                 "../../assets/images/sprites/boss_clockwork_dragon.png"});
    loadTexture("boss_spire_architect",
                {"assets/images/sprites/boss_spire_architect.png",
                 "../assets/images/sprites/boss_spire_architect.png",
                 "../../assets/images/sprites/boss_spire_architect.png"});

    return fontLoaded_;
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

const sf::Texture* ResourceManager::texture(const std::string& key) const
{
    const auto found = textures_.find(key);
    return found == textures_.end() ? nullptr : &found->second;
}

bool ResourceManager::loadTexture(const std::string& key, const std::vector<std::string>& candidates)
{
    for (const std::string& path : candidates) {
        if (!std::filesystem::exists(path)) {
            continue;
        }
        sf::Texture texture;
        if (texture.loadFromFile(path)) {
            texture.setSmooth(true);
            textures_[key] = std::move(texture);
            return true;
        }
    }
    return false;
}

GameApp::GameApp()
    : logicalView_(sf::FloatRect(0.0F, 0.0F, ui::WindowWidth, ui::WindowHeight))
    , window_(windowedMode_,
              "Mini Spire - OOP Deckbuilder",
              sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close)
{
    window_.setFramerateLimit(60);
    applyLetterboxView(windowedMode_.width, windowedMode_.height);
}

GameApp::~GameApp() = default;

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
                const bool consumed = handleGlobalEvent(event);
                if (!consumed && !helpOpen_) {
                    scene_->handleEvent(event);
                }
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
        drawGlobalOverlay();
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

bool GameApp::helpOpen() const
{
    return helpOpen_;
}

void GameApp::toggleHelp()
{
    helpOpen_ = !helpOpen_;
}

void GameApp::closeHelp()
{
    helpOpen_ = false;
}

void GameApp::applyPendingScene()
{
    if (pendingScene_) {
        scene_ = std::move(pendingScene_);
    }
}

bool GameApp::handleGlobalEvent(const sf::Event& event)
{
    if (event.type == sf::Event::Resized) {
        applyLetterboxView(event.size.width, event.size.height);
        return false;
    }

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F11 || (event.key.alt && event.key.code == sf::Keyboard::Enter)) {
            toggleFullscreen();
            return true;
        }
        if (event.key.code == sf::Keyboard::F1) {
            toggleHelp();
            return true;
        }
        if (event.key.code == sf::Keyboard::Escape && helpOpen_) {
            closeHelp();
            return true;
        }
    }

    if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) {
        return false;
    }

    const sf::Vector2f point = window_.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
    if (helpButtonRect().contains(point)) {
        toggleHelp();
        return true;
    }
    if (helpOpen_ && helpCloseRect().contains(point)) {
        closeHelp();
        return true;
    }
    return helpOpen_;
}

void GameApp::drawGlobalOverlay()
{
    if (!helpOpen_) {
        const sf::FloatRect rect = helpButtonRect();
        ui::drawPanel(window_, rect, sf::Color(49, 56, 76, 220), sf::Color(112, 122, 150), 1.0F);
        ui::drawText(window_, resources_, "?", {rect.left + 11.0F, rect.top + 5.0F}, 22, sf::Color(232, 236, 248));
        return;
    }

    sf::RectangleShape dim({ui::WindowWidth, ui::WindowHeight});
    dim.setFillColor(sf::Color(0, 0, 0, 170));
    window_.draw(dim);

    ui::drawPanel(window_, {168.0F, 86.0F, 944.0F, 548.0F}, sf::Color(25, 28, 40, 246), ui::accentColor(), 2.0F);
    ui::drawText(window_, resources_, "帮助文档", {202.0F, 118.0F}, 32, ui::accentColor());
    ui::drawText(window_, resources_, "F1 打开/关闭  Esc 关闭  F11 或 Alt+Enter 全屏", {202.0F, 158.0F}, 15, sf::Color(194, 202, 220));
    ui::drawPanel(window_, helpCloseRect(), sf::Color(64, 72, 94), sf::Color(126, 138, 166), 1.0F);
    ui::drawText(window_, resources_, "关闭", {1040.0F, 119.0F}, 16, sf::Color::White);

    const std::array<std::string, 12> leftLines {
        "目标：沿地图前进，击败三层 Boss。",
        "地图：高亮节点可点击，路径只能向前。",
        "节点：战斗、精英、商店、休息、事件、Boss。",
        "回合：你的回合抽 5 张牌并恢复能量。",
        "能量：费用高于当前能量的牌不能打出。",
        "手牌：最多 10 张，超过的抽牌进弃牌堆。",
        "牌堆：抽牌堆为空时洗回弃牌堆。",
        "消耗：消耗牌本场战斗不会再回来。",
        "攻击：先被格挡抵消，再扣除生命。",
        "格挡：通常在你的回合开始时清空。",
        "药水：战斗中左键使用，右键丢弃。",
        "药水槽：最多 2 个，会在顶部状态栏显示。"
    };
    const std::array<std::string, 11> rightLines {
        "敌人意图：敌方面板显示下一次行动。",
        "力量：提高你造成的攻击伤害。",
        "虚弱：降低攻击伤害。",
        "易伤：让目标受到更多伤害。",
        "圣遗物：提供整局有效的被动效果。",
        "Boss 后：触发剧情并回满生命。",
        "奖励：选择卡牌、金币、生命或 Boss 圣遗物。",
        "存档：地图界面可存档并返回主菜单。",
        "继续：主菜单会在有存档时显示继续游戏。",
        "历史：记录总局数、成功次数和最近 5 局。",
        "失败：返回主菜单，不会直接关闭程序。"
    };

    float y = 206.0F;
    for (const std::string& line : leftLines) {
        ui::drawText(window_, resources_, line, {210.0F, y}, 15, sf::Color(224, 226, 234));
        y += 31.0F;
    }

    y = 206.0F;
    for (const std::string& line : rightLines) {
        ui::drawText(window_, resources_, line, {654.0F, y}, 15, sf::Color(224, 226, 234));
        y += 31.0F;
    }
}

void GameApp::applyLetterboxView(unsigned int width, unsigned int height)
{
    if (width == 0 || height == 0) {
        return;
    }

    logicalView_.setCenter(ui::WindowWidth / 2.0F, ui::WindowHeight / 2.0F);
    logicalView_.setSize(ui::WindowWidth, ui::WindowHeight);

    const float windowRatio = static_cast<float>(width) / static_cast<float>(height);
    const float logicalRatio = ui::WindowWidth / ui::WindowHeight;
    sf::FloatRect viewport(0.0F, 0.0F, 1.0F, 1.0F);
    if (windowRatio > logicalRatio) {
        viewport.width = logicalRatio / windowRatio;
        viewport.left = (1.0F - viewport.width) / 2.0F;
    } else if (windowRatio < logicalRatio) {
        viewport.height = windowRatio / logicalRatio;
        viewport.top = (1.0F - viewport.height) / 2.0F;
    }
    logicalView_.setViewport(viewport);
    window_.setView(logicalView_);
    ui::setTextRenderScale(layout::renderScaleForWindow(width, height));
}

void GameApp::toggleFullscreen()
{
    if (!fullscreen_) {
        const sf::Vector2u size = window_.getSize();
        windowedMode_ = sf::VideoMode(std::max(960U, size.x), std::max(540U, size.y));
    }
    fullscreen_ = !fullscreen_;
    recreateWindow();
}

void GameApp::recreateWindow()
{
    const sf::VideoMode mode = fullscreen_ ? sf::VideoMode::getDesktopMode() : windowedMode_;
    const sf::Uint32 style = fullscreen_ ? sf::Style::Fullscreen : (sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
    window_.create(mode, "Mini Spire - OOP Deckbuilder", style);
    window_.setFramerateLimit(60);
    applyLetterboxView(mode.width, mode.height);
}

Scene::Scene(GameApp& app)
    : app_(app)
{
}

} // namespace minispire
