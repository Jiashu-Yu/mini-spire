#include "minispire/Core.h"

#include <cassert>
#include <iostream>
#include <utility>

using namespace minispire;

namespace {

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

    assert(combat.isPlayerTurn());
    assert(combat.turn() == 1);
    assert(combat.player().energy() == 3);
    assert(combat.deck().hand().size() == 5);
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

    assert(result.accepted);
    assert(combat.player().energy() == 2);
    assert(combat.enemy().hp() < beforeHp);
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

    assert(combat.player().hp() == hpBefore);
}

void testRunMapUnlocksNextNodes()
{
    RunController run;
    run.startNewRun(42);
    assert(run.availableNodeIds().size() == 2);
    const int first = run.availableNodeIds().front();
    assert(run.selectNode(first));
    run.completeActiveNode();
    assert(!run.availableNodeIds().empty());
    assert(run.floor() == 1);
}

void testRewardsComeFromPool()
{
    RunController run;
    run.startNewRun(99);
    const std::vector<Card> rewards = run.makeRewards(3);
    assert(rewards.size() == 3);
    for (const Card& card : rewards) {
        assert(!card.id.empty());
        assert(!card.effects.empty());
    }
}

} // namespace

int main()
{
    testCombatStartsWithHandAndEnergy();
    testCardSpendsEnergyAndDealsDamage();
    testBlockPreventsEnemyDamage();
    testRunMapUnlocksNextNodes();
    testRewardsComeFromPool();

    std::cout << "All Mini Spire core tests passed.\n";
    return 0;
}
