#include "minispire/Core.h"
#include "minispire/Layout.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <stdexcept>
#include <utility>

using namespace minispire;

namespace {

void require(bool condition)
{
    if (!condition) {
        throw std::runtime_error("Mini Spire core test assertion failed");
    }
}

Card makeTestCard(std::string id, int cost, std::vector<Effect> effects)
{
    return Card {std::move(id), "测试牌", cost, CardType::Attack, "测试", "测试描述", std::move(effects), false};
}

void testCombatStartsWithHandAndEnergy()
{
    Player player;
    Enemy enemy = makeEnemy(EnemyKind::AshCultist, 0);
    CombatState combat(player, enemy, starterDeck(), 1234);
    combat.start();

    require(combat.isPlayerTurn());
    require(combat.turn() == 1);
    require(combat.player().energy() == 3);
    require(combat.deck().hand().size() == 5);
}

void testCardSpendsEnergyAndDealsDamage()
{
    Player player;
    Enemy enemy = makeEnemy(EnemyKind::AshCultist, 0);
    std::vector<Card> deck {
        makeTestCard("hit", 1, {{EffectType::Damage, 10}}),
        makeTestCard("hit2", 1, {{EffectType::Damage, 10}}),
        makeTestCard("hit3", 1, {{EffectType::Damage, 10}}),
        makeTestCard("hit4", 1, {{EffectType::Damage, 10}}),
        makeTestCard("hit5", 1, {{EffectType::Damage, 10}}),
    };

    CombatState combat(player, enemy, deck, 7);
    combat.start();
    const int beforeHp = combat.enemy().hp();
    const PlayResult result = combat.playCard(0);

    require(result.accepted);
    require(combat.player().energy() == 2);
    require(combat.enemy().hp() < beforeHp);
}

void testBlockPreventsEnemyDamage()
{
    Player player;
    Enemy enemy(EnemyKind::AshCultist, "训练假人", 30, {{"轻击", "攻击 6", 6, 1, 0, 0, 0, 0}});
    std::vector<Card> deck {
        makeTestCard("block", 1, {{EffectType::Block, 99}}),
        makeTestCard("block1", 1, {{EffectType::Block, 99}}),
        makeTestCard("block2", 1, {{EffectType::Block, 99}}),
        makeTestCard("block3", 1, {{EffectType::Block, 99}}),
        makeTestCard("block4", 1, {{EffectType::Block, 99}}),
    };

    CombatState combat(player, enemy, deck, 1);
    combat.start();
    combat.playCard(0);
    const int hpBefore = combat.player().hp();
    combat.endPlayerTurn();

    require(combat.player().hp() == hpBefore);
}

void testRunMapUnlocksNextNodes()
{
    RunController run;
    run.startNewRun(42);
    require(run.level() == 1);
    require(run.availableNodeIds().size() == 2);
    const int first = run.availableNodeIds().front();
    require(run.selectNode(first));
    run.completeActiveNode();
    require(!run.availableNodeIds().empty());
    require(run.steps() == 1);
}

void testRunAdvancesAcrossThreeLevels()
{
    RunController run;
    run.startNewRun(42);
    require(run.levelName() == "第一层：灰烬地牢");

    run.startNextLevel();
    require(run.level() == 2);
    require(!run.finalLevel());
    require(run.availableNodeIds().size() == 3);

    run.startNextLevel();
    require(run.level() == 3);
    require(run.finalLevel());
    require(run.availableNodeIds().size() == 3);
}

void testPotionSlotsUseAndDiscard()
{
    Player player;
    require(player.addPotion(Potion {"fire", "火焰药水", "造成伤害", {{EffectType::Damage, 20}}}));
    require(player.addPotion(Potion {"guard", "钢肤药水", "获得格挡", {{EffectType::Block, 14}}}));
    require(!player.addPotion(Potion {"extra", "额外药水", "槽位已满", {{EffectType::Heal, 1}}}));

    Enemy enemy = makeEnemy(EnemyKind::AshCultist, 0);
    CombatState combat(player, enemy, starterDeck(), 11);
    combat.start();
    const int beforeHp = combat.enemy().hp();
    require(combat.usePotion(0).accepted);
    require(combat.enemy().hp() < beforeHp);
    require(!combat.player().potions().at(0).has_value());

    require(combat.discardPotion(1).accepted);
    require(!combat.player().potions().at(1).has_value());
}

void testRelicAppliesAtCombatStart()
{
    Player player;
    player.addRelic("晨星羽饰");
    Enemy enemy = makeEnemy(EnemyKind::AshCultist, 0);
    CombatState combat(player, enemy, starterDeck(), 17);
    combat.start();

    require(combat.player().status(StatusType::Strength) >= 1);
}

void testRelicDescriptionsCoverKnownAndBossRelics()
{
    require(relicDescription("晨星羽饰") == "战斗开局力量 +1。");
    require(relicDescription("裂纹罗盘") == "战斗开局额外抽 1 张牌。");
    require(relicDescription("第 2 层 Boss 印记") == "战斗开局力量 +1。");
    require(relicDescription("不存在的圣遗物") == "神秘的塔内造物，效果仍在记录中。");
}

void testRewardsComeFromPool()
{
    RunController run;
    run.startNewRun(99);
    const std::vector<Card> rewards = run.makeRewards(3);
    require(rewards.size() == 3);
    for (const Card& card : rewards) {
        require(!card.id.empty());
        require(!card.effects.empty());
    }
}

void testHandLimitDiscardsOverflow()
{
    std::mt19937 rng(123);
    std::vector<Card> cards;
    for (int i = 0; i < 14; ++i) {
        cards.push_back(makeTestCard("overflow_" + std::to_string(i), 0, {{EffectType::Draw, 1}}));
    }

    Deck deck;
    deck.reset(cards, rng);
    deck.drawCards(12, rng);

    require(deck.hand().size() == Deck::MaxHandSize);
    require(deck.discardCount() == 2);
    require(deck.totalCount() == 14);
}

void testLevelTransitionClearsStatuses()
{
    RunController run;
    run.startNewRun(42);
    run.player().addStatus(StatusType::Strength, 3);
    run.player().addStatus(StatusType::Weak, 2);

    run.startNextLevel();

    require(run.player().status(StatusType::Strength) == 0);
    require(run.player().status(StatusType::Weak) == 0);
}

void testBossRecoveryFillsHealthAndClearsBlock()
{
    RunController run;
    run.startNewRun(42);
    run.player().receiveDamage(40);
    run.player().gainBlock(12);

    run.recoverAfterBoss();

    require(run.player().hp() == run.player().maxHp());
    require(run.player().block() == 0);
}

void testEventHealOnlyAvailableWhenDamaged()
{
    RunController run;
    run.startNewRun(42);
    require(!run.eventHealAvailable());

    run.player().receiveDamage(10);
    require(run.eventHealAvailable());

    run.recoverAfterBoss();
    run.startNextLevel();
    require(run.level() == 2);
    require(!run.eventHealAvailable());
}

void testCombatLayoutKeepsPanelsSeparated()
{
    const layout::CombatLayout combatLayout = layout::combatLayout();
    const layout::CreaturePanelRows rows = layout::creaturePanelRows(combatLayout.playerCreaturePanel);
    const layout::CreaturePanelRows enemyRows = layout::creaturePanelRows(combatLayout.enemyCreaturePanel);
    const layout::Rect playerSprite = layout::spriteBounds(combatLayout.playerSpriteCenter,
                                                           {196.0F * combatLayout.playerSpriteScale,
                                                            238.0F * combatLayout.playerSpriteScale},
                                                           18.0F * combatLayout.playerSpriteScale);
    const layout::Rect deckInfo = combatLayout.deckInfoPanel;
    const layout::Rect secondPotionSlot = layout::combatPotionSlotRect(1);
    const layout::Rect firstCard = layout::combatHandCardRect(0, 10);

    require(playerSprite.top + playerSprite.height + 6.0F <= combatLayout.playerPanel.top);
    require(rows.statusTop >= rows.subtitleTop + 22.0F);
    require(rows.hpBarTop >= rows.statusTop + 26.0F);
    require(deckInfo.top >= secondPotionSlot.top + secondPotionSlot.height + 8.0F);
    require(deckInfo.left + deckInfo.width + 20.0F <= firstCard.left);
    require(combatLayout.deckTextPosition.x >= deckInfo.left + 12.0F);
    require(combatLayout.deckTextPosition.y >= deckInfo.top + 10.0F);
    require(enemyRows.statusTop >= enemyRows.subtitleTop + 22.0F);
    require(enemyRows.hpBarTop >= enemyRows.statusTop + 26.0F);
    require(secondPotionSlot.top >= combatLayout.potionLabelPosition.y + 20.0F);
    require(secondPotionSlot.top + secondPotionSlot.height + 8.0F <= firstCard.top);
    require(combatLayout.enemyPanel.top + combatLayout.enemyPanel.height + 16.0F <= combatLayout.enemySpriteCenter.y);
    require(combatLayout.endTurnButton.top > combatLayout.enemyPanel.top + combatLayout.enemyPanel.height);
}

void testRelicDetailsOnlyTriggeredByTopBar()
{
    const layout::Rect trigger = layout::topBarRelicRect();
    require(layout::topBarRelicTriggerContains({trigger.left + 12.0F, trigger.top + 12.0F}));

    const layout::Rect details = layout::topBarRelicDetailsRect(3);
    const layout::Vec2 detailsOnlyPoint {details.left + details.width - 24.0F, details.top + 96.0F};
    require(detailsOnlyPoint.y > trigger.top + trigger.height);
    require(!layout::topBarRelicTriggerContains(detailsOnlyPoint));
}

void testWindowRenderScaleMatchesLetterboxedContent()
{
    require(layout::renderScaleForWindow(1280U, 720U) == 1.0F);
    require(layout::renderScaleForWindow(1920U, 1080U) == 1.5F);
    const float fourByThreeScale = layout::renderScaleForWindow(2048U, 1536U);
    require(fourByThreeScale > 1.59F && fourByThreeScale < 1.61F);
}

void testCharacterChoicesUseDifferentCardPools()
{
    const std::vector<CharacterDefinition> definitions = characterDefinitions();
    require(definitions.size() == 3);
    std::set<std::string> spriteKeys;
    for (const CharacterDefinition& definition : definitions) {
        require(!definition.spriteKey.empty());
        spriteKeys.insert(definition.spriteKey);
    }
    require(spriteKeys.size() == definitions.size());

    RunController rift;
    rift.startNewRun(CharacterId::RiftTraveler, 11);
    RunController ember;
    ember.startNewRun(CharacterId::EmberAdept, 11);
    RunController crystal;
    crystal.startNewRun(CharacterId::CrystalWarden, 11);

    require(rift.characterId() == CharacterId::RiftTraveler);
    require(ember.characterId() == CharacterId::EmberAdept);
    require(crystal.characterId() == CharacterId::CrystalWarden);
    require(rift.player().name() != ember.player().name());
    require(ember.player().maxHp() < crystal.player().maxHp());
    require(!ember.player().relics().empty());
    require(!crystal.player().relics().empty());
    require(ember.deck().front().id != crystal.deck().front().id);

    const Card emberReward = ember.makeRandomReward();
    const Card crystalReward = crystal.makeRandomReward();
    require(isCardInCharacterPool(CharacterId::EmberAdept, emberReward.id));
    require(isCardInCharacterPool(CharacterId::CrystalWarden, crystalReward.id));
}

void testShopRemovalRemovesCardAndChargesGold()
{
    RunController run;
    run.startNewRun(CharacterId::RiftTraveler, 42);
    const std::size_t beforeDeck = run.deck().size();
    const int beforeGold = run.player().gold();

    const PlayResult removed = run.removeCardFromDeck(0, 75);
    require(removed.accepted);
    require(run.deck().size() == beforeDeck - 1);
    require(run.player().gold() == beforeGold - 75);

    const std::size_t afterDeck = run.deck().size();
    const int afterGold = run.player().gold();
    const PlayResult invalid = run.removeCardFromDeck(999, 75);
    require(!invalid.accepted);
    require(run.deck().size() == afterDeck);
    require(run.player().gold() == afterGold);
}

void testRemovedCardPersistsAcrossSaveLoad()
{
    const std::string savePath = "mini_spire_removed_card_save.txt";

    RunController cleanup;
    cleanup.deleteSaveFile(savePath);

    RunController run;
    run.startNewRun(CharacterId::RiftTraveler, 12);
    const Card uniqueCard = cardPool(CharacterId::RiftTraveler).front();
    run.addCardToDeck(uniqueCard);
    const PlayResult removed = run.removeCardFromDeck(run.deck().size() - 1, 75);
    require(removed.accepted);
    require(std::none_of(run.deck().begin(), run.deck().end(), [&uniqueCard](const Card& card) {
        return card.id == uniqueCard.id;
    }));
    const std::size_t deckSizeAfterRemoval = run.deck().size();
    require(run.saveToFile(savePath));

    RunController loaded;
    require(loaded.loadFromFile(savePath));
    require(loaded.deck().size() == deckSizeAfterRemoval);
    require(std::none_of(loaded.deck().begin(), loaded.deck().end(), [&uniqueCard](const Card& card) {
        return card.id == uniqueCard.id;
    }));

    cleanup.deleteSaveFile(savePath);
}

void testSaveLoadAndHistory()
{
    const std::string savePath = "mini_spire_test_save.txt";
    const std::string historyPath = "mini_spire_test_history.txt";

    RunController cleanup;
    cleanup.deleteSaveFile(savePath);
    cleanup.deleteSaveFile(historyPath);

    RunController run;
    run.startNewRun(CharacterId::CrystalWarden, 77);
    run.player().setGold(321);
    run.player().addPotion(Potion {"fire_potion", "火焰药水", "造成伤害", {{EffectType::Damage, 20}}});
    run.addCardToDeck(cardPool().front());
    require(run.saveToFile(savePath));
    require(RunController::hasSaveFile(savePath));

    RunController loaded;
    require(loaded.loadFromFile(savePath));
    require(loaded.characterId() == CharacterId::CrystalWarden);
    require(loaded.player().gold() == 321);
    require(loaded.deck().size() == run.deck().size());
    require(loaded.player().potions().at(0).has_value());

    loaded.recordRunResult(true, historyPath);
    const RunHistorySummary history = RunController::loadHistory(historyPath);
    require(history.totalRuns == 1);
    require(history.wins == 1);
    require(history.recent.size() == 1);
    require(history.recent.front().won);

    cleanup.deleteSaveFile(savePath);
    cleanup.deleteSaveFile(historyPath);
}

} // namespace

int main()
{
    testCombatStartsWithHandAndEnergy();
    testCardSpendsEnergyAndDealsDamage();
    testBlockPreventsEnemyDamage();
    testRunMapUnlocksNextNodes();
    testRunAdvancesAcrossThreeLevels();
    testPotionSlotsUseAndDiscard();
    testRelicAppliesAtCombatStart();
    testRelicDescriptionsCoverKnownAndBossRelics();
    testRewardsComeFromPool();
    testHandLimitDiscardsOverflow();
    testLevelTransitionClearsStatuses();
    testBossRecoveryFillsHealthAndClearsBlock();
    testEventHealOnlyAvailableWhenDamaged();
    testCombatLayoutKeepsPanelsSeparated();
    testRelicDetailsOnlyTriggeredByTopBar();
    testWindowRenderScaleMatchesLetterboxedContent();
    testCharacterChoicesUseDifferentCardPools();
    testShopRemovalRemovesCardAndChargesGold();
    testRemovedCardPersistsAcrossSaveLoad();
    testSaveLoadAndHistory();

    std::cout << "All Mini Spire core tests passed.\n";
    return 0;
}
