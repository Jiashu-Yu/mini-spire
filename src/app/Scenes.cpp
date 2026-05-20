#include "minispire/GameApp.h"

#include "minispire/Scene.h"
#include "minispire/UI.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace minispire {
namespace {

sf::Vector2f mousePoint(const sf::Event& event, const sf::RenderWindow& window)
{
    return window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
}

sf::FloatRect handCardRect(std::size_t index, std::size_t count)
{
    constexpr float width = 132.0F;
    constexpr float height = 186.0F;
    const float step = count <= 1 ? 0.0F : std::min(146.0F, (1180.0F - width) / static_cast<float>(count - 1));
    const float total = count <= 1 ? width : width + step * static_cast<float>(count - 1);
    const float start = (ui::WindowWidth - total) / 2.0F;
    return {start + static_cast<float>(index) * step, 512.0F, width, height};
}

sf::FloatRect rewardCardRect(std::size_t index)
{
    return {310.0F + static_cast<float>(index) * 220.0F, 240.0F, 160.0F, 220.0F};
}

std::uint32_t randomSeed()
{
    return std::random_device{}();
}

enum class VisualEffectKind {
    Slash,
    Shield,
    Heal,
    Energy,
    Debuff,
    EnemyStrike,
    BossPulse
};

struct VisualEffect {
    VisualEffectKind kind {};
    sf::Vector2f start {};
    sf::Vector2f end {};
    float age {0.0F};
    float duration {0.55F};
    sf::Color color {sf::Color::White};
    std::string label;
};

sf::Color visualColorForEffect(EffectType type)
{
    switch (type) {
    case EffectType::Damage:
        return sf::Color(242, 88, 70);
    case EffectType::Block:
        return sf::Color(98, 181, 245);
    case EffectType::Draw:
        return sf::Color(164, 122, 235);
    case EffectType::Strength:
        return sf::Color(242, 174, 76);
    case EffectType::Vulnerable:
        return sf::Color(246, 126, 72);
    case EffectType::Weak:
        return sf::Color(118, 148, 198);
    case EffectType::Heal:
        return sf::Color(90, 210, 130);
    case EffectType::GainEnergy:
        return sf::Color(244, 218, 74);
    case EffectType::Ritual:
        return sf::Color(198, 98, 232);
    }
    return sf::Color::White;
}

VisualEffectKind visualKindForEffect(EffectType type)
{
    switch (type) {
    case EffectType::Damage:
        return VisualEffectKind::Slash;
    case EffectType::Block:
        return VisualEffectKind::Shield;
    case EffectType::Heal:
        return VisualEffectKind::Heal;
    case EffectType::GainEnergy:
    case EffectType::Draw:
    case EffectType::Strength:
    case EffectType::Ritual:
        return VisualEffectKind::Energy;
    case EffectType::Vulnerable:
    case EffectType::Weak:
        return VisualEffectKind::Debuff;
    }
    return VisualEffectKind::Energy;
}

float easeOut(float t)
{
    t = std::clamp(t, 0.0F, 1.0F);
    return 1.0F - (1.0F - t) * (1.0F - t);
}

int nodesInRow(const std::vector<MapNode>& nodes, int row)
{
    return static_cast<int>(std::count_if(nodes.begin(), nodes.end(), [row](const MapNode& node) {
        return node.row == row;
    }));
}

sf::Vector2f nodePosition(const std::vector<MapNode>& nodes, const MapNode& node)
{
    const int count = std::max(1, nodesInRow(nodes, node.row));
    const float spacing = 230.0F;
    const float x = ui::WindowWidth / 2.0F + (static_cast<float>(node.lane) - (static_cast<float>(count) - 1.0F) / 2.0F) * spacing;
    const float y = 142.0F + static_cast<float>(node.row) * 104.0F;
    return {x, y};
}

std::string nodeGlyph(NodeType type)
{
    switch (type) {
    case NodeType::Battle:
        return "剑";
    case NodeType::Elite:
        return "精";
    case NodeType::Boss:
        return "王";
    case NodeType::Shop:
        return "店";
    case NodeType::Rest:
        return "息";
    case NodeType::Event:
        return "?";
    }
    return "?";
}

void drawVisualEffect(sf::RenderWindow& window, const ResourceManager& resources, const VisualEffect& effect)
{
    const float t = std::clamp(effect.age / std::max(0.01F, effect.duration), 0.0F, 1.0F);
    const float k = easeOut(t);
    const sf::Vector2f pos {
        effect.start.x + (effect.end.x - effect.start.x) * k,
        effect.start.y + (effect.end.y - effect.start.y) * k,
    };
    sf::Color color = effect.color;
    color.a = static_cast<sf::Uint8>(std::max(0.0F, 220.0F * (1.0F - t)));

    switch (effect.kind) {
    case VisualEffectKind::Slash: {
        sf::RectangleShape slash({132.0F * (1.0F + t), 9.0F});
        slash.setOrigin(66.0F * (1.0F + t), 4.5F);
        slash.setPosition(pos);
        slash.setRotation(-24.0F);
        slash.setFillColor(color);
        window.draw(slash);
        break;
    }
    case VisualEffectKind::Shield: {
        sf::CircleShape ring(44.0F + 22.0F * t);
        ring.setOrigin(44.0F + 22.0F * t, 44.0F + 22.0F * t);
        ring.setPosition(pos);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(color);
        ring.setOutlineThickness(5.0F * (1.0F - t));
        window.draw(ring);
        break;
    }
    case VisualEffectKind::Heal:
    case VisualEffectKind::Energy:
    case VisualEffectKind::Debuff:
    case VisualEffectKind::BossPulse: {
        const int particles = effect.kind == VisualEffectKind::BossPulse ? 12 : 6;
        for (int i = 0; i < particles; ++i) {
            const float angle = (static_cast<float>(i) / static_cast<float>(particles)) * 6.28318F;
            const float radius = (28.0F + 56.0F * t);
            sf::CircleShape spark(effect.kind == VisualEffectKind::BossPulse ? 7.0F : 5.0F);
            spark.setOrigin(spark.getRadius(), spark.getRadius());
            spark.setPosition(pos.x + std::cos(angle) * radius, pos.y + std::sin(angle) * radius);
            spark.setFillColor(color);
            window.draw(spark);
        }
        break;
    }
    case VisualEffectKind::EnemyStrike: {
        sf::RectangleShape beam({150.0F, 6.0F});
        beam.setOrigin(75.0F, 3.0F);
        beam.setPosition(pos);
        beam.setRotation(18.0F);
        beam.setFillColor(color);
        window.draw(beam);
        break;
    }
    }

    if (!effect.label.empty()) {
        sf::Text label = ui::makeText(resources, effect.label, 20, color);
        const sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin(bounds.left + bounds.width / 2.0F, bounds.top + bounds.height / 2.0F);
        label.setPosition(pos.x, pos.y - 52.0F * t);
        window.draw(label);
    }
}

void drawMenuCard(sf::RenderWindow& window, sf::Vector2f center, sf::Vector2f size, float rotation, sf::Color fill, sf::Color outline)
{
    sf::RectangleShape card(size);
    card.setOrigin(size.x / 2.0F, size.y / 2.0F);
    card.setPosition(center);
    card.setRotation(rotation);
    card.setFillColor(fill);
    card.setOutlineColor(outline);
    card.setOutlineThickness(2.0F);
    window.draw(card);

    sf::RectangleShape inner({size.x - 14.0F, size.y - 18.0F});
    inner.setOrigin(inner.getSize().x / 2.0F, inner.getSize().y / 2.0F);
    inner.setPosition(center);
    inner.setRotation(rotation);
    inner.setFillColor(sf::Color(26, 42, 52, 120));
    inner.setOutlineColor(sf::Color(outline.r, outline.g, outline.b, 100));
    inner.setOutlineThickness(1.0F);
    window.draw(inner);
}

void drawMainMenuBackground(sf::RenderWindow& window)
{
    sf::RectangleShape base({ui::WindowWidth, ui::WindowHeight});
    base.setFillColor(sf::Color(17, 17, 24));
    window.draw(base);

    const sf::Vector2f glowCenter {695.0F, 192.0F};
    for (int i = 0; i < 18; ++i) {
        const float radius = 360.0F - static_cast<float>(i) * 16.0F;
        sf::CircleShape glow(radius);
        glow.setOrigin(radius, radius);
        glow.setPosition(glowCenter.x, glowCenter.y + static_cast<float>(i) * 4.0F);
        glow.setFillColor(sf::Color(255, static_cast<sf::Uint8>(120 + i * 3), 45, static_cast<sf::Uint8>(8 + i)));
        window.draw(glow);
    }

    const std::array<sf::Vector3f, 18> clouds {{
        {90.0F, 120.0F, 140.0F}, {220.0F, 92.0F, 190.0F}, {385.0F, 126.0F, 180.0F},
        {560.0F, 112.0F, 220.0F}, {760.0F, 98.0F, 230.0F}, {960.0F, 128.0F, 210.0F},
        {1135.0F, 115.0F, 170.0F}, {150.0F, 318.0F, 220.0F}, {362.0F, 302.0F, 250.0F},
        {620.0F, 286.0F, 280.0F}, {872.0F, 304.0F, 260.0F}, {1092.0F, 322.0F, 230.0F},
        {70.0F, 560.0F, 210.0F}, {270.0F, 586.0F, 240.0F}, {520.0F, 570.0F, 230.0F},
        {780.0F, 584.0F, 250.0F}, {1035.0F, 565.0F, 240.0F}, {1220.0F, 590.0F, 180.0F},
    }};
    for (std::size_t i = 0; i < clouds.size(); ++i) {
        const auto cloud = clouds.at(i);
        sf::CircleShape puff(cloud.z);
        puff.setOrigin(cloud.z, cloud.z);
        puff.setPosition(cloud.x, cloud.y);
        const sf::Uint8 alpha = static_cast<sf::Uint8>(34 + (i % 4) * 8);
        puff.setFillColor(i % 3 == 0 ? sf::Color(88, 42, 43, alpha) : sf::Color(34, 38, 58, alpha));
        puff.setScale(1.45F, 0.72F);
        window.draw(puff);
    }

    sf::ConvexShape tower;
    tower.setPointCount(4);
    tower.setPoint(0, {642.0F, 198.0F});
    tower.setPoint(1, {724.0F, 182.0F});
    tower.setPoint(2, {905.0F, ui::WindowHeight + 95.0F});
    tower.setPoint(3, {740.0F, ui::WindowHeight + 110.0F});
    tower.setFillColor(sf::Color(28, 25, 28, 235));
    tower.setOutlineColor(sf::Color(164, 87, 55, 115));
    tower.setOutlineThickness(2.0F);
    window.draw(tower);

    for (int i = 0; i < 18; ++i) {
        sf::RectangleShape windowLight({8.0F, 16.0F});
        windowLight.setOrigin(4.0F, 8.0F);
        windowLight.setPosition(705.0F + static_cast<float>(i % 3) * 23.0F + static_cast<float>(i) * 5.2F,
                                262.0F + static_cast<float>(i) * 28.0F);
        windowLight.setRotation(14.0F);
        windowLight.setFillColor(sf::Color(231, 128, 62, static_cast<sf::Uint8>(70 + (i % 4) * 28)));
        window.draw(windowLight);
    }

    drawMenuCard(window, {94.0F, 215.0F}, {104.0F, 146.0F}, -20.0F, sf::Color(30, 83, 91, 135), sf::Color(109, 183, 165, 115));
    drawMenuCard(window, {214.0F, 650.0F}, {86.0F, 122.0F}, 18.0F, sf::Color(76, 55, 84, 130), sf::Color(190, 119, 180, 105));
    drawMenuCard(window, {1072.0F, 130.0F}, {76.0F, 108.0F}, 26.0F, sf::Color(62, 84, 105, 120), sf::Color(149, 185, 213, 100));
    drawMenuCard(window, {1134.0F, 620.0F}, {92.0F, 130.0F}, -31.0F, sf::Color(74, 83, 55, 120), sf::Color(174, 187, 108, 100));
    drawMenuCard(window, {462.0F, 96.0F}, {58.0F, 84.0F}, 15.0F, sf::Color(54, 76, 105, 115), sf::Color(123, 164, 214, 90));
    drawMenuCard(window, {795.0F, 630.0F}, {60.0F, 88.0F}, -12.0F, sf::Color(88, 64, 48, 112), sf::Color(214, 154, 99, 88));

    sf::RectangleShape shade({ui::WindowWidth, ui::WindowHeight});
    shade.setFillColor(sf::Color(5, 6, 11, 108));
    window.draw(shade);
}

void drawTopBar(sf::RenderWindow& window, GameApp& app)
{
    const Player& player = app.runState().player();
    ui::drawPanel(window, {0.0F, 0.0F, ui::WindowWidth, 58.0F}, sf::Color(26, 28, 38), sf::Color(56, 60, 75), 0.0F);
    ui::drawText(window, app.resources(), "Mini Spire", {24.0F, 16.0F}, 24, ui::accentColor());
    ui::drawText(window, app.resources(), "生命 " + std::to_string(player.hp()) + "/" + std::to_string(player.maxHp()),
                 {190.0F, 18.0F}, 18, sf::Color(244, 196, 201));
    ui::drawText(window, app.resources(), "金币 " + std::to_string(player.gold()), {345.0F, 18.0F}, 18, sf::Color(239, 202, 101));
    ui::drawText(window, app.resources(), "牌组 " + std::to_string(app.runState().deck().size()), {460.0F, 18.0F}, 18, sf::Color(196, 217, 255));
    ui::drawText(window, app.resources(), "Act " + std::to_string(app.runState().act()) + "/" + std::to_string(app.runState().maxActs()),
                 {580.0F, 18.0F}, 18, sf::Color(210, 210, 220));
    ui::drawText(window, app.resources(), "楼层 " + std::to_string(app.runState().floor()), {682.0F, 18.0F}, 18, sf::Color(210, 210, 220));

    std::string relicText = "遗物 ";
    if (player.relics().empty()) {
        relicText += "无";
    } else {
        for (std::size_t i = 0; i < player.relics().size(); ++i) {
            if (i > 0) {
                relicText += " / ";
            }
            relicText += player.relics().at(i);
        }
    }
    ui::drawText(window, app.resources(), relicText, {795.0F, 18.0F}, 16, sf::Color(201, 190, 230));
}

class MainMenuScene final : public Scene {
public:
    explicit MainMenuScene(GameApp& app)
        : Scene(app)
        , startButton_({514.0F, 418.0F, 252.0F, 56.0F}, "开始")
        , quitButton_({514.0F, 490.0F, 252.0F, 50.0F}, "退出")
    {
    }

    void handleEvent(const sf::Event& event) override
    {
        if (startButton_.clicked(event, app_.window())) {
            app_.runState().startNewRun(randomSeed());
            app_.changeScene(makeMapScene(app_));
        } else if (quitButton_.clicked(event, app_.window())) {
            app_.quit();
        }
    }

    void update(float) override {}

    void render(sf::RenderWindow& window) override
    {
        drawMainMenuBackground(window);

        sf::Text shadow = ui::makeText(app_.resources(), "Mini Spire", 78, sf::Color(0, 0, 0, 190));
        shadow.setPosition(416.0F, 146.0F);
        window.draw(shadow);
        ui::drawText(window, app_.resources(), "Mini Spire", {410.0F, 138.0F}, 78, sf::Color(255, 193, 91));

        if (!app_.resources().fontLoaded()) {
            ui::drawText(window, app_.resources(), "警告：没有找到字体，文字可能不可见。请安装微软雅黑或把字体放入 assets/fonts。", {260.0F, 650.0F}, 16, sf::Color(255, 160, 140));
        }
        startButton_.draw(window, app_.resources());
        quitButton_.draw(window, app_.resources());
    }

private:
    ui::Button startButton_;
    ui::Button quitButton_;
};

class MapScene final : public Scene {
public:
    explicit MapScene(GameApp& app)
        : Scene(app)
    {
    }

    void handleEvent(const sf::Event& event) override
    {
        if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) {
            return;
        }
        const sf::Vector2f point = mousePoint(event, app_.window());
        for (const MapNode& node : app_.runState().map()) {
            const sf::Vector2f center = nodePosition(app_.runState().map(), node);
            const sf::FloatRect bounds(center.x - 34.0F, center.y - 34.0F, 68.0F, 68.0F);
            if (!node.available || node.completed || !bounds.contains(point)) {
                continue;
            }
            if (!app_.runState().selectNode(node.id)) {
                return;
            }
            switch (node.type) {
            case NodeType::Battle:
            case NodeType::Elite:
            case NodeType::Boss:
                app_.changeScene(makeCombatScene(app_));
                break;
            case NodeType::Shop:
                app_.changeScene(makeShopScene(app_));
                break;
            case NodeType::Rest:
                app_.changeScene(makeRestScene(app_));
                break;
            case NodeType::Event:
                app_.changeScene(makeEventScene(app_));
                break;
            }
            return;
        }
    }

    void update(float) override {}

    void render(sf::RenderWindow& window) override
    {
        drawTopBar(window, app_);
        ui::drawText(window, app_.resources(), app_.runState().actName(), {500.0F, 76.0F}, 26, ui::accentColor());
        ui::drawText(window, app_.resources(), "选择下一处节点", {536.0F, 108.0F}, 22, sf::Color::White);

        const std::vector<MapNode>& nodes = app_.runState().map();
        for (const MapNode& node : nodes) {
            const sf::Vector2f from = nodePosition(nodes, node);
            for (int nextId : node.next) {
                const MapNode* next = app_.runState().findNode(nextId);
                if (next == nullptr) {
                    continue;
                }
                const sf::Vector2f to = nodePosition(nodes, *next);
                const sf::Color lineColor = node.completed ? sf::Color(155, 136, 86) : sf::Color(65, 69, 88);
                sf::Vertex line[] = {sf::Vertex(from, lineColor), sf::Vertex(to, lineColor)};
                window.draw(line, 2, sf::Lines);
            }
        }

        for (const MapNode& node : nodes) {
            const sf::Vector2f center = nodePosition(nodes, node);
            sf::CircleShape circle(34.0F);
            circle.setOrigin(34.0F, 34.0F);
            circle.setPosition(center);
            if (node.completed) {
                circle.setFillColor(sf::Color(69, 73, 82));
                circle.setOutlineColor(sf::Color(132, 125, 98));
            } else if (node.available) {
                circle.setFillColor(sf::Color(68, 84, 111));
                circle.setOutlineColor(ui::accentColor());
            } else {
                circle.setFillColor(sf::Color(35, 38, 50));
                circle.setOutlineColor(sf::Color(75, 79, 95));
            }
            circle.setOutlineThickness(node.available && !node.completed ? 3.0F : 1.0F);
            window.draw(circle);

            sf::Text glyph = ui::makeText(app_.resources(), nodeGlyph(node.type), 24, node.available || node.completed ? sf::Color::White : sf::Color(120, 124, 140));
            const sf::FloatRect bounds = glyph.getLocalBounds();
            glyph.setOrigin(bounds.left + bounds.width / 2.0F, bounds.top + bounds.height / 2.0F);
            glyph.setPosition(center.x, center.y - 4.0F);
            window.draw(glyph);
            ui::drawText(window, app_.resources(), toString(node.type), {center.x - 28.0F, center.y + 42.0F}, 14, sf::Color(218, 216, 205));
        }
    }
};

class CombatScene final : public Scene {
public:
    explicit CombatScene(GameApp& app)
        : Scene(app)
        , combat_(app.runState().player(), app.runState().makeEnemyForActiveNode(), app.runState().deck(), randomSeed())
        , endTurnButton_({1098.0F, 424.0F, 142.0F, 52.0F}, "结束回合")
        , continueButton_({536.0F, 345.0F, 208.0F, 56.0F}, "继续")
    {
        combat_.start();
    }

    void handleEvent(const sf::Event& event) override
    {
        if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) {
            return;
        }

        if (combat_.finished()) {
            if (!continueButton_.clicked(event, app_.window())) {
                return;
            }
            if (combat_.victory()) {
                const bool boss = isActiveBossNode();
                app_.runState().syncPlayerAfterCombat(combat_.player());
                app_.runState().completeActiveNode();
                if (boss) {
                    if (app_.runState().finalAct()) {
                        app_.runState().setWon(true);
                        app_.changeScene(makeVictoryScene(app_, true));
                    } else {
                        app_.changeScene(makeActRewardScene(app_));
                    }
                } else {
                    app_.changeScene(makeRewardScene(app_));
                }
            } else {
                app_.changeScene(makeVictoryScene(app_, false));
            }
            return;
        }

        if (endTurnButton_.clicked(event, app_.window())) {
            spawnEnemyEffect(combat_.enemy().previewMove());
            combat_.endPlayerTurn();
            return;
        }

        const sf::Vector2f point = mousePoint(event, app_.window());
        const std::size_t count = combat_.deck().hand().size();
        for (std::size_t reverse = 0; reverse < count; ++reverse) {
            const std::size_t index = count - reverse - 1;
            if (handCardRect(index, count).contains(point)) {
                const Card card = combat_.deck().hand().at(index);
                const PlayResult result = combat_.playCard(index);
                lastMessage_ = result.message;
                if (result.accepted) {
                    spawnCardEffects(card);
                }
                return;
            }
        }
    }

    void update(float dt) override
    {
        for (VisualEffect& effect : effects_) {
            effect.age += dt;
        }
        effects_.erase(std::remove_if(effects_.begin(), effects_.end(), [](const VisualEffect& effect) {
                           return effect.age >= effect.duration;
                       }),
                       effects_.end());
    }

    void render(sf::RenderWindow& window) override
    {
        drawTopBar(window, app_);

        ui::drawPanel(window, {26.0F, 76.0F, 410.0F, 400.0F}, sf::Color(29, 32, 44), sf::Color(76, 83, 105), 1.0F);
        ui::drawCreaturePanel(window, app_.resources(), combat_.player(), {56.0F, 290.0F, 330.0F, 128.0F},
                              "能量 " + std::to_string(combat_.player().energy()) + "/" + std::to_string(combat_.player().maxEnergy()));
        ui::drawText(window, app_.resources(), "抽牌 " + std::to_string(combat_.deck().drawCount()) +
                                           "  弃牌 " + std::to_string(combat_.deck().discardCount()) +
                                           "  消耗 " + std::to_string(combat_.deck().exhaustCount()),
                     {68.0F, 438.0F}, 16, sf::Color(205, 210, 225));

        ui::drawPanel(window, {843.0F, 76.0F, 410.0F, 400.0F}, sf::Color(29, 32, 44), sf::Color(76, 83, 105), 1.0F);
        const EnemyMove intent = combat_.enemy().previewMove();
        ui::drawCreaturePanel(window, app_.resources(), combat_.enemy(), {890.0F, 160.0F, 310.0F, 134.0F}, "意图：" + intent.intentText);

        ui::drawEnemySprite(window, app_.resources(), combat_.enemy().kind(), enemyCenter(), isActiveBossNode() ? 1.0F : 0.85F);
        ui::drawPlayerSprite(window, app_.resources(), playerCenter(), 0.92F);

        for (const VisualEffect& effect : effects_) {
            drawVisualEffect(window, app_.resources(), effect);
        }

        ui::drawPanel(window, {465.0F, 88.0F, 350.0F, 388.0F}, sf::Color(32, 35, 49), sf::Color(77, 84, 107), 1.0F);
        ui::drawText(window, app_.resources(), "战斗记录", {486.0F, 108.0F}, 22, ui::accentColor());
        float y = 146.0F;
        for (const CombatEvent& event : combat_.events()) {
            ui::drawText(window, app_.resources(), event.text, {486.0F, y}, 15, sf::Color(219, 219, 225));
            y += 32.0F;
        }
        if (!lastMessage_.empty() && lastMessage_ != "ok") {
            ui::drawText(window, app_.resources(), lastMessage_, {486.0F, 420.0F}, 16, sf::Color(255, 174, 137));
        }

        endTurnButton_.setEnabled(combat_.isPlayerTurn());
        endTurnButton_.draw(window, app_.resources());

        const std::vector<Card>& hand = combat_.deck().hand();
        const sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (std::size_t i = 0; i < hand.size(); ++i) {
            sf::FloatRect rect = handCardRect(i, hand.size());
            const bool hovered = rect.contains(mouse);
            if (hovered) {
                rect.top -= 18.0F;
            }
            ui::drawCard(window, app_.resources(), hand.at(i), rect, combat_.isPlayerTurn() && hand.at(i).cost <= combat_.player().energy(), hovered);
        }

        if (combat_.finished()) {
            ui::drawPanel(window, {430.0F, 250.0F, 420.0F, 210.0F}, sf::Color(24, 26, 35, 240), ui::accentColor(), 2.0F);
            ui::drawText(window, app_.resources(), combat_.victory() ? "战斗胜利" : "本次爬塔失败", {548.0F, 288.0F}, 32, combat_.victory() ? ui::accentColor() : sf::Color(255, 140, 135));
            continueButton_.draw(window, app_.resources());
        }
    }

private:
    sf::Vector2f playerCenter() const
    {
        return {230.0F, 220.0F};
    }

    sf::Vector2f enemyCenter() const
    {
        return {1045.0F, 348.0F};
    }

    void spawnCardEffects(const Card& card)
    {
        for (const Effect& effect : card.effects) {
            VisualEffect visual;
            visual.kind = visualKindForEffect(effect.type);
            visual.color = visualColorForEffect(effect.type);
            visual.label = effect.amount > 0 ? std::to_string(effect.amount) : "";
            visual.duration = effect.type == EffectType::Damage ? 0.45F : 0.62F;
            if (effect.type == EffectType::Damage || effect.type == EffectType::Vulnerable || effect.type == EffectType::Weak) {
                visual.start = playerCenter();
                visual.end = enemyCenter();
            } else {
                visual.start = playerCenter();
                visual.end = playerCenter();
            }
            effects_.push_back(visual);
        }
    }

    void spawnEnemyEffect(const EnemyMove& move)
    {
        if (move.damage > 0) {
            effects_.push_back(VisualEffect {VisualEffectKind::EnemyStrike, enemyCenter(), playerCenter(), 0.0F, 0.5F, sf::Color(255, 116, 92), std::to_string(move.damage)});
        }
        if (move.block > 0 || move.strength > 0) {
            effects_.push_back(VisualEffect {VisualEffectKind::BossPulse, enemyCenter(), enemyCenter(), 0.0F, 0.7F, sf::Color(236, 190, 88), ""});
        }
        if (move.vulnerable > 0 || move.weak > 0) {
            effects_.push_back(VisualEffect {VisualEffectKind::Debuff, playerCenter(), playerCenter(), 0.0F, 0.65F, sf::Color(172, 126, 232), ""});
        }
    }

    bool isActiveBossNode() const
    {
        const std::optional<int>& id = app_.runState().activeNodeId();
        const MapNode* node = id ? app_.runState().findNode(*id) : nullptr;
        return node != nullptr && node->type == NodeType::Boss;
    }

    CombatState combat_;
    ui::Button endTurnButton_;
    ui::Button continueButton_;
    std::string lastMessage_;
    std::vector<VisualEffect> effects_;
};

class RewardScene final : public Scene {
public:
    explicit RewardScene(GameApp& app)
        : Scene(app)
        , rewards_(app.runState().makeRewards(3))
        , skipButton_({544.0F, 535.0F, 192.0F, 48.0F}, "跳过拿金币")
    {
        gold_ = 26 + app_.runState().floor() * 4;
        relicReward_ = app_.runState().floor() % 2 == 0;
    }

    void handleEvent(const sf::Event& event) override
    {
        if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) {
            return;
        }
        const sf::Vector2f point = mousePoint(event, app_.window());
        for (std::size_t i = 0; i < rewards_.size(); ++i) {
            if (rewardCardRect(i).contains(point)) {
                finishWithCard(i);
                return;
            }
        }
        if (skipButton_.clicked(event, app_.window())) {
            grantBaseReward();
            app_.changeScene(makeMapScene(app_));
        }
    }

    void update(float) override {}

    void render(sf::RenderWindow& window) override
    {
        drawTopBar(window, app_);
        ui::drawText(window, app_.resources(), "选择一张牌加入牌组", {478.0F, 102.0F}, 30, sf::Color::White);
        ui::drawText(window, app_.resources(), "奖励金币：" + std::to_string(gold_) + (relicReward_ ? "  /  遗物：铜质罗盘" : ""),
                     {458.0F, 150.0F}, 18, sf::Color(224, 213, 178));

        const sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (std::size_t i = 0; i < rewards_.size(); ++i) {
            const sf::FloatRect rect = rewardCardRect(i);
            ui::drawCard(window, app_.resources(), rewards_.at(i), rect, true, rect.contains(mouse));
        }
        skipButton_.draw(window, app_.resources());
    }

private:
    void grantBaseReward()
    {
        if (rewardGranted_) {
            return;
        }
        app_.runState().player().gainGold(gold_);
        if (relicReward_) {
            app_.runState().player().addRelic("铜质罗盘");
        }
        rewardGranted_ = true;
    }

    void finishWithCard(std::size_t index)
    {
        grantBaseReward();
        app_.runState().addCardToDeck(rewards_.at(index));
        app_.changeScene(makeMapScene(app_));
    }

    std::vector<Card> rewards_;
    ui::Button skipButton_;
    int gold_ {0};
    bool relicReward_ {false};
    bool rewardGranted_ {false};
};

class ActRewardScene final : public Scene {
public:
    explicit ActRewardScene(GameApp& app)
        : Scene(app)
        , rewardCards_(app.runState().makeRewards(3))
        , vitalityButton_({418.0F, 484.0F, 220.0F, 52.0F}, "生命核心")
        , treasureButton_({530.0F, 548.0F, 220.0F, 52.0F}, "战利品箱")
        , relicButton_({642.0F, 484.0F, 220.0F, 52.0F}, "Boss 遗物")
        , completedAct_(app.runState().act())
    {
    }

    void handleEvent(const sf::Event& event) override
    {
        if (vitalityButton_.clicked(event, app_.window())) {
            Player& player = app_.runState().player();
            player.setMaxHp(player.maxHp() + 8);
            player.heal(28);
            app_.runState().startNextAct();
            app_.changeScene(makeMapScene(app_));
        } else if (treasureButton_.clicked(event, app_.window())) {
            app_.runState().player().gainGold(120);
            app_.runState().addCardToDeck(rewardCards_.at(0));
            app_.runState().startNextAct();
            app_.changeScene(makeMapScene(app_));
        } else if (relicButton_.clicked(event, app_.window())) {
            app_.runState().player().addRelic("第 " + std::to_string(completedAct_) + " 层 Boss 印记");
            app_.runState().addCardToDeck(rewardCards_.at(1));
            app_.runState().startNextAct();
            app_.changeScene(makeMapScene(app_));
        }
    }

    void update(float) override {}

    void render(sf::RenderWindow& window) override
    {
        drawTopBar(window, app_);
        ui::drawText(window, app_.resources(), "第 " + std::to_string(completedAct_) + " 层 Boss 已击败", {440.0F, 96.0F}, 32, ui::accentColor());
        ui::drawText(window, app_.resources(), "选择一份 Boss 奖励，然后进入下一层。", {454.0F, 146.0F}, 20, sf::Color(224, 219, 203));

        const sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (std::size_t i = 0; i < rewardCards_.size(); ++i) {
            const sf::FloatRect rect = rewardCardRect(i);
            ui::drawCard(window, app_.resources(), rewardCards_.at(i), rect, true, rect.contains(mouse));
        }

        ui::drawPanel(window, {390.0F, 452.0F, 500.0F, 170.0F}, sf::Color(31, 35, 48), sf::Color(90, 101, 128), 1.0F);
        ui::drawText(window, app_.resources(), "生命核心：最大生命 +8，回复 28", {430.0F, 462.0F}, 16, sf::Color(224, 220, 208));
        ui::drawText(window, app_.resources(), "战利品箱：金币 +120，获得左侧卡牌", {430.0F, 526.0F}, 16, sf::Color(224, 220, 208));
        ui::drawText(window, app_.resources(), "Boss 遗物：获得印记，获得中间卡牌", {430.0F, 590.0F}, 16, sf::Color(224, 220, 208));
        vitalityButton_.draw(window, app_.resources());
        treasureButton_.draw(window, app_.resources());
        relicButton_.draw(window, app_.resources());
    }

private:
    std::vector<Card> rewardCards_;
    ui::Button vitalityButton_;
    ui::Button treasureButton_;
    ui::Button relicButton_;
    int completedAct_ {1};
};

class ShopScene final : public Scene {
public:
    explicit ShopScene(GameApp& app)
        : Scene(app)
        , cards_(app.runState().makeShopCards(4))
        , sold_(cards_.size(), false)
        , leaveButton_({552.0F, 610.0F, 176.0F, 48.0F}, "离开商店")
    {
    }

    void handleEvent(const sf::Event& event) override
    {
        if (leaveButton_.clicked(event, app_.window())) {
            app_.runState().completeActiveNode();
            app_.changeScene(makeMapScene(app_));
            return;
        }
        if (event.type != sf::Event::MouseButtonPressed || event.mouseButton.button != sf::Mouse::Left) {
            return;
        }
        const sf::Vector2f point = mousePoint(event, app_.window());
        for (std::size_t i = 0; i < cards_.size(); ++i) {
            if (sold_.at(i) || !shopCardRect(i).contains(point)) {
                continue;
            }
            const int price = priceFor(i);
            if (app_.runState().player().spendGold(price)) {
                app_.runState().addCardToDeck(cards_.at(i));
                sold_.at(i) = true;
            }
        }
    }

    void update(float) override {}

    void render(sf::RenderWindow& window) override
    {
        drawTopBar(window, app_);
        ui::drawText(window, app_.resources(), "商店", {604.0F, 92.0F}, 34, ui::accentColor());
        ui::drawText(window, app_.resources(), "点击卡牌购买，金币不足时不会扣费。", {470.0F, 138.0F}, 18, sf::Color(214, 210, 196));
        const sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (std::size_t i = 0; i < cards_.size(); ++i) {
            const sf::FloatRect rect = shopCardRect(i);
            ui::drawCard(window, app_.resources(), cards_.at(i), rect, !sold_.at(i) && app_.runState().player().gold() >= priceFor(i), rect.contains(mouse));
            ui::drawText(window, app_.resources(), sold_.at(i) ? "已售出" : std::to_string(priceFor(i)) + " 金币",
                         {rect.left + 34.0F, rect.top + rect.height + 14.0F}, 18, sold_.at(i) ? sf::Color(150, 150, 150) : sf::Color(239, 202, 101));
        }
        leaveButton_.draw(window, app_.resources());
    }

private:
    sf::FloatRect shopCardRect(std::size_t index) const
    {
        return {195.0F + static_cast<float>(index) * 225.0F, 220.0F, 160.0F, 220.0F};
    }

    int priceFor(std::size_t index) const
    {
        static const std::array<int, 4> prices {45, 55, 68, 82};
        return prices.at(index % prices.size());
    }

    std::vector<Card> cards_;
    std::vector<bool> sold_;
    ui::Button leaveButton_;
};

class RestScene final : public Scene {
public:
    explicit RestScene(GameApp& app)
        : Scene(app)
        , restButton_({486.0F, 320.0F, 308.0F, 58.0F}, "休息：回复 22 生命")
        , leaveButton_({536.0F, 400.0F, 208.0F, 48.0F}, "返回地图")
    {
    }

    void handleEvent(const sf::Event& event) override
    {
        if (restButton_.clicked(event, app_.window())) {
            app_.runState().rest();
            rested_ = true;
            restButton_.setEnabled(false);
        } else if (leaveButton_.clicked(event, app_.window())) {
            if (!rested_) {
                app_.runState().rest();
            }
            app_.runState().completeActiveNode();
            app_.changeScene(makeMapScene(app_));
        }
    }

    void update(float) override {}

    void render(sf::RenderWindow& window) override
    {
        drawTopBar(window, app_);
        ui::drawText(window, app_.resources(), "休息点", {588.0F, 130.0F}, 34, ui::accentColor());
        ui::drawPanel(window, {420.0F, 210.0F, 440.0F, 300.0F}, sf::Color(35, 42, 52), sf::Color(91, 105, 124), 1.0F);
        ui::drawText(window, app_.resources(), "火光很小，但足够让你重新整理牌组与呼吸。", {462.0F, 250.0F}, 18, sf::Color(225, 220, 202));
        restButton_.draw(window, app_.resources());
        leaveButton_.draw(window, app_.resources());
    }

private:
    ui::Button restButton_;
    ui::Button leaveButton_;
    bool rested_ {false};
};

class EventScene final : public Scene {
public:
    explicit EventScene(GameApp& app)
        : Scene(app)
        , goldButton_({450.0F, 292.0F, 380.0F, 48.0F}, "搜寻废墟：获得 45 金币")
        , healButton_({450.0F, 356.0F, 380.0F, 48.0F}, "短暂冥想：回复 14 生命")
        , cardButton_({450.0F, 420.0F, 380.0F, 48.0F}, "拾取卷轴：获得随机卡牌")
    {
    }

    void handleEvent(const sf::Event& event) override
    {
        if (goldButton_.clicked(event, app_.window())) {
            app_.runState().eventGainGold();
            finish();
        } else if (healButton_.clicked(event, app_.window())) {
            app_.runState().eventHeal();
            finish();
        } else if (cardButton_.clicked(event, app_.window())) {
            app_.runState().addCardToDeck(app_.runState().makeRandomReward());
            finish();
        }
    }

    void update(float) override {}

    void render(sf::RenderWindow& window) override
    {
        drawTopBar(window, app_);
        ui::drawText(window, app_.resources(), "事件：回声大厅", {510.0F, 132.0F}, 32, ui::accentColor());
        ui::drawPanel(window, {368.0F, 205.0F, 544.0F, 320.0F}, sf::Color(33, 37, 51), sf::Color(88, 96, 119), 1.0F);
        ui::drawText(window, app_.resources(), "石壁里传来旧时代的低语，你可以选择一种收益。", {420.0F, 244.0F}, 18, sf::Color(224, 221, 207));
        goldButton_.draw(window, app_.resources());
        healButton_.draw(window, app_.resources());
        cardButton_.draw(window, app_.resources());
    }

private:
    void finish()
    {
        app_.runState().completeActiveNode();
        app_.changeScene(makeMapScene(app_));
    }

    ui::Button goldButton_;
    ui::Button healButton_;
    ui::Button cardButton_;
};

class VictoryScene final : public Scene {
public:
    VictoryScene(GameApp& app, bool won)
        : Scene(app)
        , won_(won)
        , newRunButton_({510.0F, 420.0F, 260.0F, 54.0F}, "再来一局")
        , quitButton_({510.0F, 492.0F, 260.0F, 48.0F}, "退出")
    {
    }

    void handleEvent(const sf::Event& event) override
    {
        if (newRunButton_.clicked(event, app_.window())) {
            app_.runState().startNewRun(randomSeed());
            app_.changeScene(makeMapScene(app_));
        } else if (quitButton_.clicked(event, app_.window())) {
            app_.quit();
        }
    }

    void update(float) override {}

    void render(sf::RenderWindow& window) override
    {
        ui::drawText(window, app_.resources(), won_ ? "尖塔核心已被改写" : "旅途在此中断", {390.0F, 170.0F}, 42, won_ ? ui::accentColor() : sf::Color(255, 144, 132));
        ui::drawText(window, app_.resources(), won_ ? "你完成了一次完整爬塔。现在可以继续扩充卡牌、敌人和遗物。" : "失败也是一次数据回收：调整牌组，再试一次。",
                     {360.0F, 245.0F}, 20, sf::Color(225, 220, 205));
        newRunButton_.draw(window, app_.resources());
        quitButton_.draw(window, app_.resources());
    }

private:
    bool won_ {false};
    ui::Button newRunButton_;
    ui::Button quitButton_;
};

} // namespace

std::unique_ptr<Scene> makeMainMenuScene(GameApp& app)
{
    return std::make_unique<MainMenuScene>(app);
}

std::unique_ptr<Scene> makeMapScene(GameApp& app)
{
    return std::make_unique<MapScene>(app);
}

std::unique_ptr<Scene> makeCombatScene(GameApp& app)
{
    return std::make_unique<CombatScene>(app);
}

std::unique_ptr<Scene> makeRewardScene(GameApp& app)
{
    return std::make_unique<RewardScene>(app);
}

std::unique_ptr<Scene> makeActRewardScene(GameApp& app)
{
    return std::make_unique<ActRewardScene>(app);
}

std::unique_ptr<Scene> makeShopScene(GameApp& app)
{
    return std::make_unique<ShopScene>(app);
}

std::unique_ptr<Scene> makeRestScene(GameApp& app)
{
    return std::make_unique<RestScene>(app);
}

std::unique_ptr<Scene> makeEventScene(GameApp& app)
{
    return std::make_unique<EventScene>(app);
}

std::unique_ptr<Scene> makeVictoryScene(GameApp& app, bool won)
{
    return std::make_unique<VictoryScene>(app, won);
}

} // namespace minispire
