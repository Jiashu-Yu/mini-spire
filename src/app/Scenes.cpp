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

void drawTopBar(sf::RenderWindow& window, GameApp& app)
{
    const Player& player = app.runState().player();
    ui::drawPanel(window, {0.0F, 0.0F, ui::WindowWidth, 58.0F}, sf::Color(26, 28, 38), sf::Color(56, 60, 75), 0.0F);
    ui::drawText(window, app.resources(), "Mini Spire", {24.0F, 16.0F}, 24, ui::accentColor());
    ui::drawText(window, app.resources(), "生命 " + std::to_string(player.hp()) + "/" + std::to_string(player.maxHp()),
                 {190.0F, 18.0F}, 18, sf::Color(244, 196, 201));
    ui::drawText(window, app.resources(), "金币 " + std::to_string(player.gold()), {345.0F, 18.0F}, 18, sf::Color(239, 202, 101));
    ui::drawText(window, app.resources(), "牌组 " + std::to_string(app.runState().deck().size()), {460.0F, 18.0F}, 18, sf::Color(196, 217, 255));
    ui::drawText(window, app.resources(), "层数 " + std::to_string(app.runState().floor()), {580.0F, 18.0F}, 18, sf::Color(210, 210, 220));

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
    ui::drawText(window, app.resources(), relicText, {710.0F, 18.0F}, 16, sf::Color(201, 190, 230));
}

class MainMenuScene final : public Scene {
public:
    explicit MainMenuScene(GameApp& app)
        : Scene(app)
        , startButton_({505.0F, 430.0F, 270.0F, 56.0F}, "开始爬塔")
        , quitButton_({505.0F, 500.0F, 270.0F, 48.0F}, "退出")
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
        sf::RectangleShape glow({ui::WindowWidth, ui::WindowHeight});
        glow.setFillColor(ui::backgroundColor());
        window.draw(glow);

        ui::drawText(window, app_.resources(), "Mini Spire", {412.0F, 118.0F}, 66, ui::accentColor());
        ui::drawText(window, app_.resources(), "原创卡牌构筑 / 回合制战斗 / OOP 练习项目", {380.0F, 205.0F}, 24, sf::Color(225, 219, 203));

        ui::drawPanel(window, {270.0F, 275.0F, 740.0F, 104.0F}, sf::Color(34, 39, 55), sf::Color(89, 98, 124), 1.0F);
        ui::drawText(window, app_.resources(), "核心设计：卡牌由 Effect 组合，战斗由 CombatState 统一解释，界面只负责表现与输入。", {306.0F, 306.0F}, 20);
        ui::drawText(window, app_.resources(), "这会让新增卡牌、敌人、场景时不必重写主循环。", {306.0F, 338.0F}, 18, sf::Color(206, 210, 226));

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
        ui::drawText(window, app_.resources(), "选择下一处节点", {536.0F, 82.0F}, 28, sf::Color::White);

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
                    app_.runState().setWon(true);
                    app_.changeScene(makeVictoryScene(app_, true));
                } else {
                    app_.changeScene(makeRewardScene(app_));
                }
            } else {
                app_.changeScene(makeVictoryScene(app_, false));
            }
            return;
        }

        if (endTurnButton_.clicked(event, app_.window())) {
            combat_.endPlayerTurn();
            return;
        }

        const sf::Vector2f point = mousePoint(event, app_.window());
        const std::size_t count = combat_.deck().hand().size();
        for (std::size_t reverse = 0; reverse < count; ++reverse) {
            const std::size_t index = count - reverse - 1;
            if (handCardRect(index, count).contains(point)) {
                lastMessage_ = combat_.playCard(index).message;
                return;
            }
        }
    }

    void update(float) override {}

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

        sf::CircleShape enemyBody(72.0F);
        enemyBody.setOrigin(72.0F, 72.0F);
        enemyBody.setPosition(1045.0F, 350.0F);
        enemyBody.setFillColor(sf::Color(96, 54, 63));
        enemyBody.setOutlineColor(sf::Color(218, 153, 94));
        enemyBody.setOutlineThickness(3.0F);
        window.draw(enemyBody);

        sf::CircleShape playerBody(62.0F);
        playerBody.setOrigin(62.0F, 62.0F);
        playerBody.setPosition(230.0F, 215.0F);
        playerBody.setFillColor(sf::Color(55, 87, 112));
        playerBody.setOutlineColor(sf::Color(141, 203, 238));
        playerBody.setOutlineThickness(3.0F);
        window.draw(playerBody);

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
