#include "minispire/Core.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace minispire {
namespace {

Card makeCard(std::string id,
              std::string name,
              int cost,
              CardType type,
              std::string rarity,
              std::string description,
              std::vector<Effect> effects,
              bool exhaust = false)
{
    return Card {std::move(id), std::move(name), cost, type, std::move(rarity), std::move(description), std::move(effects), exhaust};
}

int clampNonNegative(int value)
{
    return std::max(0, value);
}

template <typename T>
const T& randomElement(const std::vector<T>& values, std::mt19937& rng)
{
    std::uniform_int_distribution<std::size_t> distribution(0, values.size() - 1);
    return values.at(distribution(rng));
}

std::optional<Card> findKnownCard(const std::string& id)
{
    std::vector<Card> cards;
    for (const CharacterDefinition& character : characterDefinitions()) {
        cards.insert(cards.end(), character.startingDeck.begin(), character.startingDeck.end());
        cards.insert(cards.end(), character.cardPool.begin(), character.cardPool.end());
    }
    const auto found = std::find_if(cards.begin(), cards.end(), [&id](const Card& card) {
        return card.id == id;
    });
    if (found == cards.end()) {
        return std::nullopt;
    }
    return *found;
}

CharacterId characterIdFromInt(int value)
{
    switch (value) {
    case 1:
        return CharacterId::EmberAdept;
    case 2:
        return CharacterId::CrystalWarden;
    default:
        return CharacterId::RiftTraveler;
    }
}

int characterIdToInt(CharacterId id)
{
    switch (id) {
    case CharacterId::RiftTraveler:
        return 0;
    case CharacterId::EmberAdept:
        return 1;
    case CharacterId::CrystalWarden:
        return 2;
    }
    return 0;
}

std::optional<Potion> findKnownPotion(const std::string& id)
{
    const std::vector<Potion> potions = potionPool();
    const auto found = std::find_if(potions.begin(), potions.end(), [&id](const Potion& potion) {
        return potion.id == id;
    });
    if (found == potions.end()) {
        return std::nullopt;
    }
    return *found;
}

std::string cardTypePrefix(CardType type)
{
    switch (type) {
    case CardType::Attack:
        return "攻击";
    case CardType::Skill:
        return "技能";
    case CardType::Power:
        return "能力";
    }
    return "未知";
}

} // namespace

Creature::Creature(std::string name, int maxHp)
    : name_(std::move(name))
    , maxHp_(std::max(1, maxHp))
    , hp_(std::max(1, maxHp))
{
}

const std::string& Creature::name() const
{
    return name_;
}

int Creature::hp() const
{
    return hp_;
}

int Creature::maxHp() const
{
    return maxHp_;
}

int Creature::block() const
{
    return block_;
}

bool Creature::isAlive() const
{
    return hp_ > 0;
}

bool Creature::isDead() const
{
    return !isAlive();
}

int Creature::status(StatusType type) const
{
    const auto found = statuses_.find(type);
    return found == statuses_.end() ? 0 : found->second;
}

void Creature::addStatus(StatusType type, int amount)
{
    if (amount == 0) {
        return;
    }
    statuses_[type] = std::max(0, status(type) + amount);
    if (statuses_[type] == 0) {
        statuses_.erase(type);
    }
}

void Creature::setStatus(StatusType type, int amount)
{
    if (amount <= 0) {
        statuses_.erase(type);
        return;
    }
    statuses_[type] = amount;
}

void Creature::reduceStatus(StatusType type, int amount)
{
    addStatus(type, -amount);
}

const std::map<StatusType, int>& Creature::statuses() const
{
    return statuses_;
}

void Creature::gainBlock(int amount)
{
    block_ += clampNonNegative(amount);
}

int Creature::receiveDamage(int amount)
{
    const int incoming = clampNonNegative(amount);
    const int absorbed = std::min(block_, incoming);
    block_ -= absorbed;
    const int healthLoss = incoming - absorbed;
    hp_ = std::max(0, hp_ - healthLoss);
    return healthLoss;
}

void Creature::heal(int amount)
{
    hp_ = std::min(maxHp_, hp_ + clampNonNegative(amount));
}

void Creature::clearBlock()
{
    block_ = 0;
}

void Creature::clearStatuses()
{
    statuses_.clear();
}

void Creature::setHp(int hp)
{
    hp_ = std::clamp(hp, 0, maxHp_);
}

void Creature::setMaxHp(int maxHp)
{
    maxHp_ = std::max(1, maxHp);
    hp_ = std::min(hp_, maxHp_);
}

Player::Player()
    : Creature("裂隙旅人", 72)
{
}

Player::Player(std::string name, int maxHp, int maxEnergy, int gold)
    : Creature(std::move(name), maxHp)
    , maxEnergy_(std::max(1, maxEnergy))
    , energy_(std::max(1, maxEnergy))
    , gold_(clampNonNegative(gold))
{
}

int Player::energy() const
{
    return energy_;
}

int Player::maxEnergy() const
{
    return maxEnergy_;
}

int Player::gold() const
{
    return gold_;
}

const std::vector<std::string>& Player::relics() const
{
    return relics_;
}

const std::array<std::optional<Potion>, 2>& Player::potions() const
{
    return potions_;
}

void Player::beginTurn()
{
    clearBlock();
    energy_ = maxEnergy_;
    if (status(StatusType::Ritual) > 0) {
        addStatus(StatusType::Strength, status(StatusType::Ritual));
    }
    reduceStatus(StatusType::Vulnerable, 1);
    reduceStatus(StatusType::Weak, 1);
}

bool Player::spendEnergy(int amount)
{
    if (amount < 0 || energy_ < amount) {
        return false;
    }
    energy_ -= amount;
    return true;
}

void Player::gainEnergy(int amount)
{
    energy_ += clampNonNegative(amount);
}

void Player::gainGold(int amount)
{
    gold_ += clampNonNegative(amount);
}

bool Player::spendGold(int amount)
{
    if (amount < 0 || gold_ < amount) {
        return false;
    }
    gold_ -= amount;
    return true;
}

void Player::addRelic(std::string relic)
{
    relics_.push_back(std::move(relic));
}

bool Player::addPotion(Potion potion)
{
    for (auto& slot : potions_) {
        if (!slot.has_value()) {
            slot = std::move(potion);
            return true;
        }
    }
    return false;
}

std::optional<Potion> Player::takePotion(std::size_t slot)
{
    if (slot >= potions_.size() || !potions_.at(slot).has_value()) {
        return std::nullopt;
    }
    std::optional<Potion> potion = potions_.at(slot);
    potions_.at(slot).reset();
    return potion;
}

bool Player::discardPotion(std::size_t slot)
{
    if (slot >= potions_.size() || !potions_.at(slot).has_value()) {
        return false;
    }
    potions_.at(slot).reset();
    return true;
}

void Player::setPotion(std::size_t slot, std::optional<Potion> potion)
{
    if (slot >= potions_.size()) {
        return;
    }
    potions_.at(slot) = std::move(potion);
}

void Player::setGold(int gold)
{
    gold_ = clampNonNegative(gold);
}

void Player::setMaxEnergy(int maxEnergy)
{
    maxEnergy_ = std::max(1, maxEnergy);
    energy_ = std::min(energy_, maxEnergy_);
}

Enemy::Enemy(EnemyKind kind, std::string name, int maxHp, std::vector<EnemyMove> moves)
    : Creature(std::move(name), maxHp)
    , kind_(kind)
    , moves_(std::move(moves))
{
}

EnemyKind Enemy::kind() const
{
    return kind_;
}

EnemyMove Enemy::previewMove() const
{
    if (moves_.empty()) {
        return EnemyMove {"等待", "无行动"};
    }
    return moves_.at(moveIndex_ % moves_.size());
}

EnemyMove Enemy::takeMove()
{
    EnemyMove move = previewMove();
    if (!moves_.empty()) {
        moveIndex_ = (moveIndex_ + 1) % moves_.size();
    }
    return move;
}

void Enemy::onHalfHealth()
{
    if (halfHealthTriggered_ || hp() > maxHp() / 2) {
        return;
    }

    switch (kind_) {
    case EnemyKind::RootMatriarch:
        halfHealthTriggered_ = true;
        addStatus(StatusType::Ritual, 1);
        gainBlock(18);
        break;
    case EnemyKind::ClockworkDragon:
        halfHealthTriggered_ = true;
        addStatus(StatusType::Strength, 4);
        gainBlock(20);
        break;
    case EnemyKind::SpireArchitect:
        halfHealthTriggered_ = true;
        addStatus(StatusType::Strength, 5);
        gainBlock(24);
        break;
    default:
        break;
    }
}

bool Enemy::halfHealthTriggered() const
{
    return halfHealthTriggered_;
}

void Deck::reset(std::vector<Card> cards, std::mt19937& rng)
{
    drawPile_ = std::move(cards);
    discardPile_.clear();
    hand_.clear();
    exhaustPile_.clear();
    std::shuffle(drawPile_.begin(), drawPile_.end(), rng);
}

void Deck::drawCards(int count, std::mt19937& rng)
{
    for (int i = 0; i < count; ++i) {
        if (drawPile_.empty()) {
            reshuffleDiscardIntoDraw(rng);
        }
        if (drawPile_.empty()) {
            return;
        }
        if (hand_.size() < MaxHandSize) {
            hand_.push_back(drawPile_.back());
        } else {
            discardPile_.push_back(drawPile_.back());
        }
        drawPile_.pop_back();
    }
}

bool Deck::hasPlayableCard(int energy) const
{
    return std::any_of(hand_.begin(), hand_.end(), [energy](const Card& card) {
        return card.cost <= energy;
    });
}

const std::vector<Card>& Deck::hand() const
{
    return hand_;
}

std::vector<Card>& Deck::hand()
{
    return hand_;
}

std::size_t Deck::drawCount() const
{
    return drawPile_.size();
}

std::size_t Deck::discardCount() const
{
    return discardPile_.size();
}

std::size_t Deck::exhaustCount() const
{
    return exhaustPile_.size();
}

std::size_t Deck::totalCount() const
{
    return drawPile_.size() + discardPile_.size() + hand_.size() + exhaustPile_.size();
}

Card Deck::removeFromHand(std::size_t index)
{
    if (index >= hand_.size()) {
        throw std::out_of_range("hand index");
    }
    Card card = hand_.at(index);
    hand_.erase(hand_.begin() + static_cast<std::ptrdiff_t>(index));
    return card;
}

void Deck::discard(Card card)
{
    discardPile_.push_back(std::move(card));
}

void Deck::exhaust(Card card)
{
    exhaustPile_.push_back(std::move(card));
}

void Deck::discardHand()
{
    while (!hand_.empty()) {
        discardPile_.push_back(hand_.back());
        hand_.pop_back();
    }
}

void Deck::reshuffleDiscardIntoDraw(std::mt19937& rng)
{
    drawPile_ = std::move(discardPile_);
    discardPile_.clear();
    std::shuffle(drawPile_.begin(), drawPile_.end(), rng);
}

CombatState::CombatState(Player player, Enemy enemy, std::vector<Card> deckCards, std::uint32_t seed)
    : player_(std::move(player))
    , enemy_(std::move(enemy))
    , rng_(seed)
{
    deck_.reset(std::move(deckCards), rng_);
}

void CombatState::start()
{
    if (started_) {
        return;
    }
    started_ = true;
    log("战斗开始：面对 " + enemy_.name());
    beginPlayerTurn();
    applyRelicsAtCombatStart();
}

PlayResult CombatState::playCard(std::size_t handIndex)
{
    if (!playerTurn_ || finished()) {
        return {false, "现在不能出牌"};
    }
    if (handIndex >= deck_.hand().size()) {
        return {false, "没有这张手牌"};
    }

    const Card preview = deck_.hand().at(handIndex);
    if (!player_.spendEnergy(preview.cost)) {
        return {false, "能量不足"};
    }

    Card card = deck_.removeFromHand(handIndex);
    log("使用：" + card.name);
    for (const Effect& effect : card.effects) {
        applyCardEffect(card, effect);
        checkEndConditions();
        if (finished()) {
            break;
        }
    }

    if (card.exhaust || card.type == CardType::Power) {
        deck_.exhaust(std::move(card));
    } else {
        deck_.discard(std::move(card));
    }

    const bool wasHalfHealthTriggered = enemy_.halfHealthTriggered();
    enemy_.onHalfHealth();
    if (!wasHalfHealthTriggered && enemy_.halfHealthTriggered()) {
        log(enemy_.name() + " 半血启动核心，力量上升并获得格挡。");
    }
    checkEndConditions();
    return {true, "ok"};
}

PlayResult CombatState::usePotion(std::size_t slot)
{
    if (finished()) {
        return {false, "战斗已经结束"};
    }

    std::optional<Potion> potion = player_.takePotion(slot);
    if (!potion) {
        return {false, "这个药水槽是空的"};
    }

    log("使用药水：" + potion->name);
    for (const Effect& effect : potion->effects) {
        applyPotionEffect(*potion, effect);
        checkEndConditions();
        if (finished()) {
            break;
        }
    }
    return {true, "ok"};
}

PlayResult CombatState::discardPotion(std::size_t slot)
{
    if (!player_.discardPotion(slot)) {
        return {false, "这个药水槽是空的"};
    }
    log("丢弃了一瓶药水。");
    return {true, "ok"};
}

void CombatState::endPlayerTurn()
{
    if (!playerTurn_ || finished()) {
        return;
    }
    playerTurn_ = false;
    deck_.discardHand();
    runEnemyTurn();
    if (!finished()) {
        beginPlayerTurn();
    }
}

const Player& CombatState::player() const
{
    return player_;
}

Player& CombatState::player()
{
    return player_;
}

const Enemy& CombatState::enemy() const
{
    return enemy_;
}

Enemy& CombatState::enemy()
{
    return enemy_;
}

const Deck& CombatState::deck() const
{
    return deck_;
}

Deck& CombatState::deck()
{
    return deck_;
}

const std::vector<CombatEvent>& CombatState::events() const
{
    return events_;
}

bool CombatState::isPlayerTurn() const
{
    return playerTurn_;
}

bool CombatState::victory() const
{
    return victory_;
}

bool CombatState::defeat() const
{
    return defeat_;
}

bool CombatState::finished() const
{
    return victory_ || defeat_;
}

int CombatState::turn() const
{
    return turn_;
}

void CombatState::beginPlayerTurn()
{
    ++turn_;
    playerTurn_ = true;
    player_.beginTurn();
    enemy_.reduceStatus(StatusType::Vulnerable, 1);
    enemy_.reduceStatus(StatusType::Weak, 1);
    deck_.drawCards(5, rng_);
    log("第 " + std::to_string(turn_) + " 回合，你抽 5 张牌。");
}

void CombatState::applyRelicsAtCombatStart()
{
    for (const std::string& relic : player_.relics()) {
        if (relic == "晨星羽饰") {
            player_.addStatus(StatusType::Strength, 1);
            log("晨星羽饰：开局力量 +1。");
        } else if (relic == "余烬血脉") {
            player_.addStatus(StatusType::Strength, 2);
            log("余烬血脉：开局力量 +2。");
        } else if (relic == "晶盾核心") {
            player_.gainBlock(8);
            log("晶盾核心：开局获得 8 格挡。");
        } else if (relic == "裂纹罗盘" || relic == "铜质罗盘") {
            deck_.drawCards(1, rng_);
            log(relic + "：开局额外抽 1 张牌。");
        } else if (relic == "余烬护符") {
            player_.gainBlock(6);
            log("余烬护符：开局获得 6 格挡。");
        } else if (relic == "晶化沙漏") {
            player_.gainEnergy(1);
            log("晶化沙漏：开局获得 1 能量。");
        } else if (relic == "旧塔徽章") {
            player_.heal(2);
            log("旧塔徽章：开局回复 2 生命。");
        } else if (relic == "静默钟摆") {
            enemy_.addStatus(StatusType::Weak, 1);
            log("静默钟摆：敌人开局获得 1 虚弱。");
        } else if (relic.find("Boss 印记") != std::string::npos) {
            player_.addStatus(StatusType::Strength, 1);
            log(relic + "：开局力量 +1。");
        }
    }
}

void CombatState::runEnemyTurn()
{
    enemy_.clearBlock();
    enemy_.onHalfHealth();
    if (enemy_.status(StatusType::Ritual) > 0) {
        enemy_.addStatus(StatusType::Strength, enemy_.status(StatusType::Ritual));
    }
    const EnemyMove move = enemy_.takeMove();
    log(enemy_.name() + " 行动：" + move.name);
    applyEnemyMove(move);
    checkEndConditions();
}

void CombatState::applyCardEffect(const Card& card, const Effect& effect)
{
    switch (effect.type) {
    case EffectType::Damage: {
        const int damage = scaledOutgoingDamage(player_, effect.amount);
        const int dealt = enemy_.receiveDamage(scaledIncomingDamage(enemy_, damage));
        log(card.name + " 造成 " + std::to_string(dealt) + " 点伤害。");
        break;
    }
    case EffectType::Block:
        player_.gainBlock(effect.amount);
        log("获得 " + std::to_string(effect.amount) + " 点格挡。");
        break;
    case EffectType::Draw:
        deck_.drawCards(effect.amount, rng_);
        log("抽 " + std::to_string(effect.amount) + " 张牌。");
        break;
    case EffectType::Strength:
        player_.addStatus(StatusType::Strength, effect.amount);
        log("力量 +" + std::to_string(effect.amount) + "。");
        break;
    case EffectType::Vulnerable:
        enemy_.addStatus(StatusType::Vulnerable, effect.amount);
        log(enemy_.name() + " 获得 " + std::to_string(effect.amount) + " 层易伤。");
        break;
    case EffectType::Weak:
        enemy_.addStatus(StatusType::Weak, effect.amount);
        log(enemy_.name() + " 获得 " + std::to_string(effect.amount) + " 层虚弱。");
        break;
    case EffectType::Heal:
        player_.heal(effect.amount);
        log("回复 " + std::to_string(effect.amount) + " 点生命。");
        break;
    case EffectType::GainEnergy:
        player_.gainEnergy(effect.amount);
        log("获得 " + std::to_string(effect.amount) + " 点能量。");
        break;
    case EffectType::Ritual:
        player_.addStatus(StatusType::Ritual, effect.amount);
        log("获得 " + std::to_string(effect.amount) + " 层仪式。");
        break;
    }
}

void CombatState::applyPotionEffect(const Potion& potion, const Effect& effect)
{
    switch (effect.type) {
    case EffectType::Damage: {
        const int dealt = enemy_.receiveDamage(scaledIncomingDamage(enemy_, effect.amount));
        log(potion.name + " 造成 " + std::to_string(dealt) + " 点伤害。");
        break;
    }
    case EffectType::Block:
        player_.gainBlock(effect.amount);
        log("药水提供 " + std::to_string(effect.amount) + " 点格挡。");
        break;
    case EffectType::Draw:
        deck_.drawCards(effect.amount, rng_);
        log("药水让你抽 " + std::to_string(effect.amount) + " 张牌。");
        break;
    case EffectType::Strength:
        player_.addStatus(StatusType::Strength, effect.amount);
        log("药水让力量 +" + std::to_string(effect.amount) + "。");
        break;
    case EffectType::Vulnerable:
        enemy_.addStatus(StatusType::Vulnerable, effect.amount);
        log(enemy_.name() + " 获得 " + std::to_string(effect.amount) + " 层易伤。");
        break;
    case EffectType::Weak:
        enemy_.addStatus(StatusType::Weak, effect.amount);
        log(enemy_.name() + " 获得 " + std::to_string(effect.amount) + " 层虚弱。");
        break;
    case EffectType::Heal:
        player_.heal(effect.amount);
        log("药水回复 " + std::to_string(effect.amount) + " 点生命。");
        break;
    case EffectType::GainEnergy:
        player_.gainEnergy(effect.amount);
        log("药水提供 " + std::to_string(effect.amount) + " 点能量。");
        break;
    case EffectType::Ritual:
        player_.addStatus(StatusType::Ritual, effect.amount);
        log("药水提供 " + std::to_string(effect.amount) + " 层仪式。");
        break;
    }
}

void CombatState::applyEnemyMove(const EnemyMove& move)
{
    if (move.block > 0) {
        enemy_.gainBlock(move.block);
        log(enemy_.name() + " 获得 " + std::to_string(move.block) + " 点格挡。");
    }
    if (move.strength > 0) {
        enemy_.addStatus(StatusType::Strength, move.strength);
        log(enemy_.name() + " 力量 +" + std::to_string(move.strength) + "。");
    }
    if (move.weak > 0) {
        player_.addStatus(StatusType::Weak, move.weak);
        log("你获得 " + std::to_string(move.weak) + " 层虚弱。");
    }
    if (move.vulnerable > 0) {
        player_.addStatus(StatusType::Vulnerable, move.vulnerable);
        log("你获得 " + std::to_string(move.vulnerable) + " 层易伤。");
    }
    for (int i = 0; i < std::max(1, move.hits); ++i) {
        if (move.damage <= 0) {
            continue;
        }
        const int damage = scaledOutgoingDamage(enemy_, move.damage);
        const int dealt = player_.receiveDamage(scaledIncomingDamage(player_, damage));
        log(enemy_.name() + " 造成 " + std::to_string(dealt) + " 点伤害。");
    }
}

int CombatState::scaledOutgoingDamage(const Creature& attacker, int base) const
{
    int damage = base + attacker.status(StatusType::Strength);
    if (attacker.status(StatusType::Weak) > 0) {
        damage = static_cast<int>(std::floor(damage * 0.75F));
    }
    return clampNonNegative(damage);
}

int CombatState::scaledIncomingDamage(const Creature& defender, int base) const
{
    int damage = base;
    if (defender.status(StatusType::Vulnerable) > 0) {
        damage = static_cast<int>(std::ceil(damage * 1.5F));
    }
    return clampNonNegative(damage);
}

void CombatState::log(std::string text)
{
    events_.push_back(CombatEvent {std::move(text)});
    if (events_.size() > 8) {
        events_.erase(events_.begin());
    }
}

void CombatState::checkEndConditions()
{
    if (enemy_.isDead()) {
        victory_ = true;
        playerTurn_ = false;
        log("胜利！");
    } else if (player_.isDead()) {
        defeat_ = true;
        playerTurn_ = false;
        log("你倒下了。");
    }
}

RunController::RunController()
    : rng_(std::random_device{}())
{
}

void RunController::startNewRun(std::uint32_t seed)
{
    startNewRun(CharacterId::RiftTraveler, seed);
}

void RunController::startNewRun(CharacterId character, std::uint32_t seed)
{
    rng_.seed(seed);
    characterId_ = character;
    const CharacterDefinition& definition = characterDefinition(character);
    player_ = Player {definition.name, definition.maxHp, definition.maxEnergy, definition.startingGold};
    for (const std::string& relic : definition.startingRelics) {
        player_.addRelic(relic);
    }
    deck_ = definition.startingDeck;
    activeNodeId_.reset();
    steps_ = 0;
    level_ = 1;
    active_ = true;
    won_ = false;
    resultRecorded_ = false;
    generateMap();
    unlockInitialNodes();
}

bool RunController::active() const
{
    return active_;
}

bool RunController::won() const
{
    return won_;
}

void RunController::setWon(bool won)
{
    won_ = won;
}

CharacterId RunController::characterId() const
{
    return characterId_;
}

const CharacterDefinition& RunController::character() const
{
    return characterDefinition(characterId_);
}

const Player& RunController::player() const
{
    return player_;
}

Player& RunController::player()
{
    return player_;
}

const std::vector<Card>& RunController::deck() const
{
    return deck_;
}

const std::vector<MapNode>& RunController::map() const
{
    return map_;
}

const std::optional<int>& RunController::activeNodeId() const
{
    return activeNodeId_;
}

int RunController::steps() const
{
    return steps_;
}

int RunController::level() const
{
    return level_;
}

int RunController::maxLevels() const
{
    return maxLevels_;
}

bool RunController::finalLevel() const
{
    return level_ >= maxLevels_;
}

std::string RunController::levelName() const
{
    switch (level_) {
    case 1:
        return "第一层：灰烬地牢";
    case 2:
        return "第二层：晶化温室";
    case 3:
        return "第三层：星钟尖塔";
    default:
        return "未知层";
    }
}

std::vector<int> RunController::availableNodeIds() const
{
    std::vector<int> ids;
    for (const MapNode& node : map_) {
        if (node.available && !node.completed) {
            ids.push_back(node.id);
        }
    }
    return ids;
}

bool RunController::isNodeAvailable(int id) const
{
    const MapNode* node = findNode(id);
    return node != nullptr && node->available && !node->completed;
}

const MapNode* RunController::findNode(int id) const
{
    const auto found = std::find_if(map_.begin(), map_.end(), [id](const MapNode& node) {
        return node.id == id;
    });
    return found == map_.end() ? nullptr : &(*found);
}

MapNode* RunController::findNode(int id)
{
    const auto found = std::find_if(map_.begin(), map_.end(), [id](const MapNode& node) {
        return node.id == id;
    });
    return found == map_.end() ? nullptr : &(*found);
}

bool RunController::selectNode(int id)
{
    if (!isNodeAvailable(id)) {
        return false;
    }
    activeNodeId_ = id;
    for (MapNode& node : map_) {
        if (!node.completed) {
            node.available = node.id == id;
        }
    }
    return true;
}

void RunController::completeActiveNode()
{
    if (!activeNodeId_) {
        return;
    }
    MapNode* node = findNode(*activeNodeId_);
    if (node == nullptr) {
        activeNodeId_.reset();
        return;
    }
    node->completed = true;
    for (MapNode& candidate : map_) {
        if (!candidate.completed) {
            candidate.available = false;
        }
    }
    ++steps_;

    for (int nextId : node->next) {
        if (MapNode* next = findNode(nextId)) {
            next->available = true;
        }
    }
    activeNodeId_.reset();
}

Enemy RunController::makeEnemyForActiveNode()
{
    const MapNode* node = activeNodeId_ ? findNode(*activeNodeId_) : nullptr;
    if (node == nullptr) {
        return makeEnemy(EnemyKind::AshCultist, steps_);
    }

    if (node->type == NodeType::Boss) {
        if (level_ == 1) {
            return makeEnemy(EnemyKind::RootMatriarch, steps_);
        }
        if (level_ == 2) {
            return makeEnemy(EnemyKind::ClockworkDragon, steps_);
        }
        return makeEnemy(EnemyKind::SpireArchitect, steps_);
    }
    if (node->type == NodeType::Elite) {
        if (level_ == 1) {
            return makeEnemy(EnemyKind::IronSentinel, steps_);
        }
        if (level_ == 2) {
            return makeEnemy(EnemyKind::EmberDuelist, steps_);
        }
        return makeEnemy(EnemyKind::ChronoKnight, steps_);
    }

    std::vector<EnemyKind> normal {
        EnemyKind::AshCultist,
        EnemyKind::AcidSlime,
        EnemyKind::BellGuard,
    };
    if (level_ == 2) {
        normal = {EnemyKind::ThornLurker, EnemyKind::CrystalWisp, EnemyKind::BellGuard};
    } else if (level_ >= 3) {
        normal = {EnemyKind::NullPriest, EnemyKind::CrystalWisp, EnemyKind::ThornLurker};
    }
    return makeEnemy(randomElement(normal, rng_), steps_);
}

std::vector<Card> RunController::makeRewards(int count)
{
    std::vector<Card> rewards;
    for (int i = 0; i < count; ++i) {
        rewards.push_back(makeRandomReward());
    }
    return rewards;
}

std::vector<Card> RunController::makeShopCards(int count)
{
    return makeRewards(count);
}

std::vector<std::string> RunController::makeShopRelics(int count)
{
    const std::vector<std::string> pool {
        "晨星羽饰",
        "裂纹罗盘",
        "余烬护符",
        "晶化沙漏",
        "旧塔徽章",
        "静默钟摆",
    };

    std::vector<std::string> relics;
    for (int i = 0; i < count; ++i) {
        relics.push_back(randomElement(pool, rng_));
    }
    return relics;
}

std::vector<Potion> RunController::makeShopPotions(int count)
{
    std::vector<Potion> potions;
    for (int i = 0; i < count; ++i) {
        potions.push_back(makeRandomPotion());
    }
    return potions;
}

Card RunController::makeRandomReward()
{
    const std::vector<Card> pool = cardPool(characterId_);
    return randomElement(pool, rng_);
}

Potion RunController::makeRandomPotion()
{
    const std::vector<Potion> pool = potionPool();
    return randomElement(pool, rng_);
}

void RunController::addCardToDeck(const Card& card)
{
    deck_.push_back(card);
}

PlayResult RunController::removeCardFromDeck(std::size_t index, int cost)
{
    if (index >= deck_.size()) {
        return {false, "没有这张牌。"};
    }
    if (deck_.size() <= 5) {
        return {false, "牌组太薄，不能继续移除。"};
    }
    if (cost < 0 || !player_.spendGold(cost)) {
        return {false, "金币不足。"};
    }

    const std::string removedName = deck_.at(index).name;
    deck_.erase(deck_.begin() + static_cast<std::ptrdiff_t>(index));
    return {true, "移除了卡牌：" + removedName};
}

void RunController::syncPlayerAfterCombat(const Player& player)
{
    const int gold = player_.gold();
    player_ = player;
    player_.setGold(gold);
}

void RunController::startNextLevel()
{
    if (finalLevel()) {
        won_ = true;
        return;
    }
    ++level_;
    activeNodeId_.reset();
    player_.clearBlock();
    player_.clearStatuses();
    generateMap();
    unlockInitialNodes();
}

void RunController::recoverAfterBoss()
{
    player_.clearBlock();
    player_.heal(player_.maxHp());
}

void RunController::rest()
{
    player_.heal(22);
}

void RunController::eventGainGold()
{
    player_.gainGold(45);
}

bool RunController::eventHealAvailable() const
{
    return player_.hp() < player_.maxHp();
}

void RunController::eventHeal()
{
    if (eventHealAvailable()) {
        player_.heal(14);
    }
}

bool RunController::saveToFile(const std::string& path) const
{
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        return false;
    }

    out << "MINISPIRE_SAVE 3\n";
    out << level_ << ' ' << maxLevels_ << ' ' << steps_ << ' ' << (active_ ? 1 : 0) << ' '
        << (won_ ? 1 : 0) << ' ' << activeNodeId_.value_or(-1) << ' ' << characterIdToInt(characterId_) << '\n';
    out << player_.hp() << ' ' << player_.maxHp() << ' ' << player_.gold() << '\n';

    out << deck_.size() << '\n';
    for (const Card& card : deck_) {
        out << card.id << '\n';
    }

    out << player_.relics().size() << '\n';
    for (const std::string& relic : player_.relics()) {
        out << relic << '\n';
    }

    out << player_.potions().size() << '\n';
    for (const std::optional<Potion>& potion : player_.potions()) {
        out << (potion ? potion->id : "-") << '\n';
    }

    out << map_.size() << '\n';
    for (const MapNode& node : map_) {
        out << node.id << ' ' << node.row << ' ' << node.lane << ' ' << static_cast<int>(node.type) << ' '
            << (node.available ? 1 : 0) << ' ' << (node.completed ? 1 : 0) << ' ' << node.next.size();
        for (int nextId : node.next) {
            out << ' ' << nextId;
        }
        out << '\n';
    }

    return static_cast<bool>(out);
}

bool RunController::loadFromFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    std::string header;
    int version = 0;
    if (!(in >> header >> version) || header != "MINISPIRE_SAVE") {
        return false;
    }

    int activeFlag = 0;
    int wonFlag = 0;
    int activeId = -1;
    int characterValue = 0;
    int hp = 0;
    int maxHp = 0;
    int gold = 0;
    if (!(in >> level_ >> maxLevels_ >> steps_ >> activeFlag >> wonFlag >> activeId)) {
        return false;
    }
    if (version >= 3 && !(in >> characterValue)) {
        return false;
    }
    if (!(in >> hp >> maxHp >> gold)) {
        return false;
    }

    characterId_ = characterIdFromInt(characterValue);
    Player loadedPlayer(characterDefinition(characterId_).name, maxHp, characterDefinition(characterId_).maxEnergy, gold);
    loadedPlayer.setMaxHp(maxHp);
    loadedPlayer.setHp(hp);
    loadedPlayer.setGold(gold);

    std::size_t deckCount = 0;
    if (!(in >> deckCount)) {
        return false;
    }
    std::vector<Card> loadedDeck;
    for (std::size_t i = 0; i < deckCount; ++i) {
        std::string cardId;
        in >> cardId;
        if (std::optional<Card> card = findKnownCard(cardId)) {
            loadedDeck.push_back(*card);
        }
    }
    if (loadedDeck.empty()) {
        loadedDeck = starterDeck(characterId_);
    }

    std::size_t relicCount = 0;
    if (!(in >> relicCount)) {
        return false;
    }
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (std::size_t i = 0; i < relicCount; ++i) {
        std::string relic;
        std::getline(in, relic);
        if (!relic.empty()) {
            loadedPlayer.addRelic(relic);
        }
    }

    std::size_t potionCount = 0;
    if (!(in >> potionCount)) {
        return false;
    }
    for (std::size_t i = 0; i < potionCount; ++i) {
        std::string potionId;
        in >> potionId;
        if (potionId != "-") {
            loadedPlayer.setPotion(i, findKnownPotion(potionId));
        }
    }

    std::size_t mapCount = 0;
    if (!(in >> mapCount)) {
        return false;
    }
    std::vector<MapNode> loadedMap;
    for (std::size_t i = 0; i < mapCount; ++i) {
        MapNode node;
        int type = 0;
        int available = 0;
        int completed = 0;
        std::size_t nextCount = 0;
        if (!(in >> node.id >> node.row >> node.lane >> type >> available >> completed >> nextCount)) {
            return false;
        }
        node.type = static_cast<NodeType>(type);
        node.available = available != 0;
        node.completed = completed != 0;
        node.next.clear();
        for (std::size_t nextIndex = 0; nextIndex < nextCount; ++nextIndex) {
            int nextId = 0;
            in >> nextId;
            node.next.push_back(nextId);
        }
        loadedMap.push_back(std::move(node));
    }

    player_ = std::move(loadedPlayer);
    deck_ = std::move(loadedDeck);
    map_ = std::move(loadedMap);
    if (map_.empty()) {
        generateMap();
        unlockInitialNodes();
    }
    activeNodeId_ = activeId >= 0 ? std::optional<int>(activeId) : std::nullopt;
    active_ = activeFlag != 0;
    won_ = wonFlag != 0;
    resultRecorded_ = false;
    rng_.seed(std::random_device{}());
    return true;
}

bool RunController::deleteSaveFile(const std::string& path) const
{
    std::error_code error;
    return std::filesystem::remove(path, error) || !std::filesystem::exists(path);
}

void RunController::recordRunResult(bool won, const std::string& historyPath)
{
    if (resultRecorded_) {
        return;
    }

    RunHistorySummary summary = loadHistory(historyPath);
    ++summary.totalRuns;
    if (won) {
        ++summary.wins;
    }
    summary.recent.insert(summary.recent.begin(),
                          RunHistoryEntry {won, level_, maxLevels_, steps_, static_cast<int>(deck_.size()), player_.hp(), player_.maxHp()});
    if (summary.recent.size() > 5) {
        summary.recent.resize(5);
    }

    std::ofstream out(historyPath, std::ios::trunc);
    if (out) {
        out << "MINISPIRE_HISTORY 1\n";
        out << summary.totalRuns << ' ' << summary.wins << '\n';
        out << summary.recent.size() << '\n';
        for (const RunHistoryEntry& entry : summary.recent) {
            out << (entry.won ? 1 : 0) << ' ' << entry.level << ' ' << entry.maxLevels << ' ' << entry.steps << ' '
                << entry.deckSize << ' ' << entry.hp << ' ' << entry.maxHp << '\n';
        }
    }

    won_ = won;
    active_ = false;
    resultRecorded_ = true;
    deleteSaveFile();
}

bool RunController::hasSaveFile(const std::string& path)
{
    return std::filesystem::exists(path);
}

RunHistorySummary RunController::loadHistory(const std::string& path)
{
    RunHistorySummary summary;
    std::ifstream in(path);
    if (!in) {
        return summary;
    }

    std::string header;
    int version = 0;
    if (!(in >> header >> version) || header != "MINISPIRE_HISTORY") {
        return summary;
    }

    std::size_t count = 0;
    if (!(in >> summary.totalRuns >> summary.wins >> count)) {
        return RunHistorySummary {};
    }
    for (std::size_t i = 0; i < count && i < 5; ++i) {
        int won = 0;
        RunHistoryEntry entry;
        if (!(in >> won >> entry.level >> entry.maxLevels >> entry.steps >> entry.deckSize >> entry.hp >> entry.maxHp)) {
            break;
        }
        entry.won = won != 0;
        summary.recent.push_back(entry);
    }
    return summary;
}

void RunController::generateMap()
{
    if (level_ == 1) {
        map_ = {
            {0, 0, 0, NodeType::Battle, {2, 3}, false, false},
            {1, 0, 1, NodeType::Battle, {3, 4}, false, false},
            {2, 1, 0, NodeType::Event, {5}, false, false},
            {3, 1, 1, NodeType::Battle, {5, 6}, false, false},
            {4, 1, 2, NodeType::Shop, {6}, false, false},
            {5, 2, 0, NodeType::Elite, {7}, false, false},
            {6, 2, 1, NodeType::Rest, {7}, false, false},
            {7, 3, 0, NodeType::Battle, {8}, false, false},
            {8, 4, 0, NodeType::Boss, {}, false, false},
        };
        return;
    }

    if (level_ == 2) {
        map_ = {
            {0, 0, 0, NodeType::Battle, {3, 4}, false, false},
            {1, 0, 1, NodeType::Event, {4, 5}, false, false},
            {2, 0, 2, NodeType::Battle, {5}, false, false},
            {3, 1, 0, NodeType::Shop, {6}, false, false},
            {4, 1, 1, NodeType::Battle, {6, 7}, false, false},
            {5, 1, 2, NodeType::Elite, {7}, false, false},
            {6, 2, 0, NodeType::Battle, {8}, false, false},
            {7, 2, 1, NodeType::Rest, {8}, false, false},
            {8, 3, 0, NodeType::Elite, {9}, false, false},
            {9, 4, 0, NodeType::Boss, {}, false, false},
        };
        return;
    }

    map_ = {
        {0, 0, 0, NodeType::Battle, {3}, false, false},
        {1, 0, 1, NodeType::Elite, {3, 4}, false, false},
        {2, 0, 2, NodeType::Battle, {4}, false, false},
        {3, 1, 0, NodeType::Event, {5, 6}, false, false},
        {4, 1, 1, NodeType::Shop, {6}, false, false},
        {5, 2, 0, NodeType::Elite, {7}, false, false},
        {6, 2, 1, NodeType::Battle, {7, 8}, false, false},
        {7, 3, 0, NodeType::Rest, {9}, false, false},
        {8, 3, 1, NodeType::Elite, {9}, false, false},
        {9, 4, 0, NodeType::Boss, {}, false, false},
    };
}

void RunController::unlockInitialNodes()
{
    for (MapNode& node : map_) {
        node.available = node.row == 0;
    }
}

std::string toString(CardType type)
{
    return cardTypePrefix(type);
}

std::string toString(NodeType type)
{
    switch (type) {
    case NodeType::Battle:
        return "战斗";
    case NodeType::Elite:
        return "精英";
    case NodeType::Boss:
        return "Boss";
    case NodeType::Shop:
        return "商店";
    case NodeType::Rest:
        return "休息";
    case NodeType::Event:
        return "事件";
    }
    return "未知";
}

std::string toString(StatusType type)
{
    switch (type) {
    case StatusType::Strength:
        return "力量";
    case StatusType::Vulnerable:
        return "易伤";
    case StatusType::Weak:
        return "虚弱";
    case StatusType::Ritual:
        return "仪式";
    }
    return "未知";
}

std::string relicDescription(const std::string& relic)
{
    if (relic == "晨星羽饰") {
        return "战斗开局力量 +1。";
    }
    if (relic == "余烬血脉") {
        return "战斗开局力量 +2。";
    }
    if (relic == "晶盾核心") {
        return "战斗开局获得 8 格挡。";
    }
    if (relic == "裂纹罗盘" || relic == "铜质罗盘") {
        return "战斗开局额外抽 1 张牌。";
    }
    if (relic == "余烬护符") {
        return "战斗开局获得 6 格挡。";
    }
    if (relic == "晶化沙漏") {
        return "战斗开局获得 1 能量。";
    }
    if (relic == "旧塔徽章") {
        return "战斗开局回复 2 生命。";
    }
    if (relic == "静默钟摆") {
        return "敌人开局获得 1 虚弱。";
    }
    if (relic.find("Boss 印记") != std::string::npos) {
        return "战斗开局力量 +1。";
    }
    return "神秘的塔内造物，效果仍在记录中。";
}

std::vector<Card> starterDeck(CharacterId character)
{
    std::vector<Card> cards;
    switch (character) {
    case CharacterId::EmberAdept:
        for (int i = 0; i < 5; ++i) {
            cards.push_back(makeCard("strike", "打击", 1, CardType::Attack, "基础", "造成 6 伤害",
                                     {{EffectType::Damage, 6}}));
        }
        for (int i = 0; i < 3; ++i) {
            cards.push_back(makeCard("defend", "防御", 1, CardType::Skill, "基础", "获得 5 格挡",
                                     {{EffectType::Block, 5}}));
        }
        cards.push_back(makeCard("ignite", "点燃", 1, CardType::Attack, "普通", "造成 7 伤害\n力量 +1",
                                 {{EffectType::Damage, 7}, {EffectType::Strength, 1}}));
        cards.push_back(makeCard("break_armor", "破甲", 1, CardType::Attack, "普通", "造成 6 伤害\n施加 2 易伤",
                                 {{EffectType::Damage, 6}, {EffectType::Vulnerable, 2}}));
        cards.push_back(makeCard("shout", "战吼", 1, CardType::Skill, "普通", "力量 +2",
                                 {{EffectType::Strength, 2}}));
        return cards;
    case CharacterId::CrystalWarden:
        for (int i = 0; i < 5; ++i) {
            cards.push_back(makeCard("defend", "防御", 1, CardType::Skill, "基础", "获得 5 格挡",
                                     {{EffectType::Block, 5}}));
        }
        for (int i = 0; i < 3; ++i) {
            cards.push_back(makeCard("strike", "打击", 1, CardType::Attack, "基础", "造成 6 伤害",
                                     {{EffectType::Damage, 6}}));
        }
        cards.push_back(makeCard("steady_guard", "稳固", 1, CardType::Skill, "基础", "获得 8 格挡",
                                 {{EffectType::Block, 8}}));
        cards.push_back(makeCard("shield", "盾墙", 1, CardType::Skill, "普通", "获得 9 格挡",
                                 {{EffectType::Block, 9}}));
        cards.push_back(makeCard("deep_breath", "深呼吸", 0, CardType::Skill, "普通", "回复 3 生命\n获得 3 格挡",
                                 {{EffectType::Heal, 3}, {EffectType::Block, 3}}));
        return cards;
    case CharacterId::RiftTraveler:
        break;
    }

    for (int i = 0; i < 5; ++i) {
        cards.push_back(makeCard("strike", "打击", 1, CardType::Attack, "基础", "造成 6 伤害",
                                 {{EffectType::Damage, 6}}));
        cards.push_back(makeCard("defend", "防御", 1, CardType::Skill, "基础", "获得 5 格挡",
                                 {{EffectType::Block, 5}}));
    }
    cards.push_back(makeCard("spark_cut", "火花斩", 1, CardType::Attack, "基础", "造成 7 伤害\n抽 1 张牌",
                             {{EffectType::Damage, 7}, {EffectType::Draw, 1}}));
    cards.push_back(makeCard("steady_guard", "稳固", 1, CardType::Skill, "基础", "获得 8 格挡",
                             {{EffectType::Block, 8}}));
    return cards;
}

std::vector<Card> starterDeck()
{
    return starterDeck(CharacterId::RiftTraveler);
}

std::vector<Card> cardPool(CharacterId character)
{
    switch (character) {
    case CharacterId::EmberAdept:
        return {
            makeCard("strike_plus", "精准斩", 1, CardType::Attack, "普通", "造成 9 伤害",
                     {{EffectType::Damage, 9}}),
            makeCard("double_stab", "双连刺", 1, CardType::Attack, "普通", "造成 5 伤害\n造成 5 伤害",
                     {{EffectType::Damage, 5}, {EffectType::Damage, 5}}),
            makeCard("shout", "战吼", 1, CardType::Skill, "普通", "力量 +2",
                     {{EffectType::Strength, 2}}),
            makeCard("break_armor", "破甲", 1, CardType::Attack, "普通", "造成 6 伤害\n施加 2 易伤",
                     {{EffectType::Damage, 6}, {EffectType::Vulnerable, 2}}),
            makeCard("cleave", "横扫", 2, CardType::Attack, "普通", "造成 16 伤害",
                     {{EffectType::Damage, 16}}),
            makeCard("ignite", "点燃", 1, CardType::Attack, "普通", "造成 7 伤害\n力量 +1",
                     {{EffectType::Damage, 7}, {EffectType::Strength, 1}}),
            makeCard("overrun", "压制", 2, CardType::Attack, "普通", "造成 14 伤害\n施加 1 易伤",
                     {{EffectType::Damage, 14}, {EffectType::Vulnerable, 1}}),
            makeCard("heavy_lance", "重枪突刺", 2, CardType::Attack, "普通", "造成 18 伤害\n施加 1 易伤",
                     {{EffectType::Damage, 18}, {EffectType::Vulnerable, 1}}),
            makeCard("ritual", "赤心仪式", 1, CardType::Skill, "稀有", "每回合力量 +1\n消耗",
                     {{EffectType::Ritual, 1}}, true),
            makeCard("blood_pact", "血契", 1, CardType::Power, "稀有", "力量 +3\n回复 3 生命",
                     {{EffectType::Strength, 3}, {EffectType::Heal, 3}}),
            makeCard("finisher", "终结", 3, CardType::Attack, "稀有", "造成 28 伤害",
                     {{EffectType::Damage, 28}}),
            makeCard("starfall", "星坠", 3, CardType::Attack, "稀有", "造成 20 伤害\n施加 2 易伤",
                     {{EffectType::Damage, 20}, {EffectType::Vulnerable, 2}}),
            makeCard("sun_spear", "日冕枪", 2, CardType::Attack, "稀有", "造成 12 伤害\n力量 +2",
                     {{EffectType::Damage, 12}, {EffectType::Strength, 2}}),
        };
    case CharacterId::CrystalWarden:
        return {
            makeCard("shield", "盾墙", 1, CardType::Skill, "普通", "获得 9 格挡",
                     {{EffectType::Block, 9}}),
            makeCard("smoke", "烟幕", 1, CardType::Skill, "普通", "获得 6 格挡\n施加 1 虚弱",
                     {{EffectType::Block, 6}, {EffectType::Weak, 1}}),
            makeCard("first_aid", "急救", 1, CardType::Skill, "普通", "回复 6 生命\n消耗",
                     {{EffectType::Heal, 6}}, true),
            makeCard("fortify", "堡垒姿态", 2, CardType::Skill, "稀有", "获得 16 格挡",
                     {{EffectType::Block, 16}}),
            makeCard("storm", "风暴术", 2, CardType::Attack, "稀有", "造成 10 伤害\n施加 2 虚弱",
                     {{EffectType::Damage, 10}, {EffectType::Weak, 2}}),
            makeCard("pulse_guard", "脉冲护盾", 2, CardType::Skill, "稀有", "获得 12 格挡\n抽 1 张牌",
                     {{EffectType::Block, 12}, {EffectType::Draw, 1}}),
            makeCard("cleanse", "净化", 1, CardType::Skill, "普通", "回复 4 生命\n抽 1 张牌",
                     {{EffectType::Heal, 4}, {EffectType::Draw, 1}}),
            makeCard("chain_guard", "锁链防御", 1, CardType::Skill, "普通", "获得 7 格挡\n施加 1 虚弱",
                     {{EffectType::Block, 7}, {EffectType::Weak, 1}}),
            makeCard("iron_skin", "铁肤", 2, CardType::Skill, "普通", "获得 14 格挡\n力量 +1",
                     {{EffectType::Block, 14}, {EffectType::Strength, 1}}),
            makeCard("aegis_bloom", "盾花绽放", 2, CardType::Skill, "稀有", "获得 10 格挡\n回复 8 生命",
                     {{EffectType::Block, 10}, {EffectType::Heal, 8}}),
            makeCard("deep_breath", "深呼吸", 0, CardType::Skill, "普通", "回复 3 生命\n获得 3 格挡",
                     {{EffectType::Heal, 3}, {EffectType::Block, 3}}),
        };
    case CharacterId::RiftTraveler:
        break;
    }

    return {
        makeCard("strike_plus", "精准斩", 1, CardType::Attack, "普通", "造成 9 伤害",
                 {{EffectType::Damage, 9}}),
        makeCard("roll", "战术翻滚", 0, CardType::Skill, "普通", "获得 3 格挡\n抽 1 张牌",
                 {{EffectType::Block, 3}, {EffectType::Draw, 1}}),
        makeCard("focus", "聚能", 0, CardType::Skill, "普通", "获得 1 能量\n抽 1 张牌",
                 {{EffectType::GainEnergy, 1}, {EffectType::Draw, 1}}, true),
        makeCard("echo_blade", "回声剑", 2, CardType::Attack, "稀有", "造成 8 伤害\n抽 2 张牌",
                 {{EffectType::Damage, 8}, {EffectType::Draw, 2}}),
        makeCard("meditate", "冥想", 1, CardType::Skill, "稀有", "抽 3 张牌",
                 {{EffectType::Draw, 3}}),
        makeCard("moon_cut", "月弧斩", 1, CardType::Attack, "普通", "造成 8 伤害\n获得 4 格挡",
                 {{EffectType::Damage, 8}, {EffectType::Block, 4}}),
        makeCard("quick_step", "疾步", 0, CardType::Skill, "普通", "抽 2 张牌\n消耗",
                 {{EffectType::Draw, 2}}, true),
        makeCard("clockwork", "发条连携", 1, CardType::Skill, "稀有", "获得 1 能量\n抽 2 张牌\n消耗",
                 {{EffectType::GainEnergy, 1}, {EffectType::Draw, 2}}, true),
        makeCard("void_mark", "虚空印记", 1, CardType::Attack, "普通", "造成 5 伤害\n施加 2 虚弱",
                 {{EffectType::Damage, 5}, {EffectType::Weak, 2}}),
    };
}

std::vector<Card> cardPool()
{
    return cardPool(CharacterId::RiftTraveler);
}

std::vector<CharacterDefinition> characterDefinitions()
{
    return {
        {CharacterId::RiftTraveler,
         "裂隙旅人",
         "抽牌 / 节奏",
         "额外抽牌与临时能量。\n寻找爆发回合。",
         "sprite_player_rift",
         72,
         3,
         99,
         starterDeck(CharacterId::RiftTraveler),
         cardPool(CharacterId::RiftTraveler),
         {"裂纹罗盘"}},
        {CharacterId::EmberAdept,
         "余烬使徒",
         "力量 / 易伤",
         "生命较低。\n快速叠力量并放大伤害。",
         "sprite_player_ember",
         64,
         3,
         99,
         starterDeck(CharacterId::EmberAdept),
         cardPool(CharacterId::EmberAdept),
         {"余烬血脉"}},
        {CharacterId::CrystalWarden,
         "晶盾守卫",
         "格挡 / 回复",
         "生命更高。\n用格挡、回复和虚弱拖住战斗。",
         "sprite_player_crystal",
         82,
         3,
         85,
         starterDeck(CharacterId::CrystalWarden),
         cardPool(CharacterId::CrystalWarden),
         {"晶盾核心"}},
    };
}

const CharacterDefinition& characterDefinition(CharacterId id)
{
    static const std::vector<CharacterDefinition> definitions = characterDefinitions();
    const auto found = std::find_if(definitions.begin(), definitions.end(), [id](const CharacterDefinition& definition) {
        return definition.id == id;
    });
    return found == definitions.end() ? definitions.front() : *found;
}

bool isCardInCharacterPool(CharacterId character, const std::string& cardId)
{
    const std::vector<Card> pool = cardPool(character);
    return std::any_of(pool.begin(), pool.end(), [&cardId](const Card& card) {
        return card.id == cardId;
    });
}

std::vector<Potion> potionPool()
{
    return {
        {"fire_potion", "火焰药水", "造成 20 伤害", {{EffectType::Damage, 20}}},
        {"guard_potion", "钢肤药水", "获得 14 格挡", {{EffectType::Block, 14}}},
        {"heal_potion", "活力药水", "回复 12 生命", {{EffectType::Heal, 12}}},
        {"energy_potion", "能量药水", "获得 2 能量", {{EffectType::GainEnergy, 2}}},
        {"weak_potion", "虚弱药水", "施加 2 虚弱", {{EffectType::Weak, 2}}},
        {"power_potion", "力量药水", "力量 +2", {{EffectType::Strength, 2}}},
        {"draw_potion", "洞察药水", "抽 3 张牌", {{EffectType::Draw, 3}}},
    };
}

Enemy makeEnemy(EnemyKind kind, int progress)
{
    const int scale = std::max(0, progress);
    switch (kind) {
    case EnemyKind::AshCultist:
        return Enemy {
            kind,
            "灰烬信徒",
            36 + scale * 2,
            {
                {"暗刃", "攻击 7", 7, 1, 0, 0, 0, 0},
                {"吟诵", "力量 +2", 0, 1, 0, 2, 0, 0},
                {"狂热劈砍", "攻击 10", 10, 1, 0, 0, 0, 0},
            },
        };
    case EnemyKind::AcidSlime:
        return Enemy {
            kind,
            "酸蚀史莱姆",
            42 + scale * 2,
            {
                {"喷吐", "虚弱 1", 0, 1, 0, 0, 0, 1},
                {"撞击", "攻击 8", 8, 1, 0, 0, 0, 0},
                {"收缩", "格挡 8", 0, 1, 8, 0, 0, 0},
            },
        };
    case EnemyKind::BellGuard:
        return Enemy {
            kind,
            "钟卫",
            48 + scale * 2,
            {
                {"举盾", "格挡 10", 0, 1, 10, 0, 0, 0},
                {"钟摆连击", "攻击 4x2", 4, 2, 0, 0, 0, 0},
                {"震音", "易伤 1", 5, 1, 0, 0, 1, 0},
            },
        };
    case EnemyKind::ThornLurker:
        return Enemy {
            kind,
            "棘藤伏击者",
            54 + scale * 2,
            {
                {"藤刺", "攻击 8 + 虚弱", 8, 1, 0, 0, 0, 1},
                {"扎根", "格挡 12", 0, 1, 12, 0, 0, 0},
                {"缠绕", "攻击 6x2", 6, 2, 0, 0, 0, 0},
            },
        };
    case EnemyKind::CrystalWisp:
        return Enemy {
            kind,
            "晶尘幽光",
            46 + scale * 2,
            {
                {"折射", "格挡 8 + 力量", 0, 1, 8, 1, 0, 0},
                {"晶刺", "攻击 11", 11, 1, 0, 0, 0, 0},
                {"眩光", "易伤 1 + 虚弱 1", 0, 1, 0, 0, 1, 1},
            },
        };
    case EnemyKind::EmberDuelist:
        return Enemy {
            kind,
            "余烬决斗者",
            88 + scale * 3,
            {
                {"试探", "攻击 8x2", 8, 2, 0, 0, 0, 0},
                {"架势", "格挡 14 + 力量", 0, 1, 14, 2, 0, 0},
                {"破绽斩", "攻击 20 + 易伤", 20, 1, 0, 0, 1, 0},
            },
        };
    case EnemyKind::NullPriest:
        return Enemy {
            kind,
            "虚无祭司",
            66 + scale * 2,
            {
                {"低语", "虚弱 2", 0, 1, 0, 0, 0, 2},
                {"空洞脉冲", "攻击 13", 13, 1, 0, 0, 0, 0},
                {"献祭", "力量 +3 + 格挡", 0, 1, 10, 3, 0, 0},
            },
        };
    case EnemyKind::ChronoKnight:
        return Enemy {
            kind,
            "刻时骑士",
            104 + scale * 3,
            {
                {"秒针突刺", "攻击 6x3", 6, 3, 0, 0, 0, 0},
                {"钟甲", "格挡 18", 0, 1, 18, 0, 0, 0},
                {"终刻", "攻击 24 + 易伤", 24, 1, 0, 0, 2, 0},
            },
        };
    case EnemyKind::IronSentinel:
        return Enemy {
            kind,
            "铁面守卫",
            76 + scale * 3,
            {
                {"盾击", "攻击 11 + 格挡", 11, 1, 8, 0, 0, 0},
                {"蓄力", "力量 +3", 0, 1, 0, 3, 0, 0},
                {"处刑", "攻击 18", 18, 1, 0, 0, 0, 0},
            },
        };
    case EnemyKind::RootMatriarch:
        return Enemy {
            kind,
            "根须主母",
            118 + scale * 2,
            {
                {"荆冠", "格挡 16 + 力量", 0, 1, 16, 2, 0, 0},
                {"根须拍击", "攻击 9x2", 9, 2, 0, 0, 0, 0},
                {"花粉云", "虚弱 2 + 易伤 1", 0, 1, 0, 0, 1, 2},
                {"碾压", "攻击 21", 21, 1, 0, 0, 0, 0},
            },
        };
    case EnemyKind::ClockworkDragon:
        return Enemy {
            kind,
            "发条龙",
            152 + scale * 2,
            {
                {"齿轮吐息", "攻击 8x3", 8, 3, 0, 0, 0, 0},
                {"装甲闭合", "格挡 24", 0, 1, 24, 0, 0, 0},
                {"过载", "力量 +4", 0, 1, 0, 4, 0, 0},
                {"熔芯撕咬", "攻击 26 + 易伤", 26, 1, 0, 0, 1, 0},
            },
        };
    case EnemyKind::SpireArchitect:
        return Enemy {
            kind,
            "尖塔建筑师",
            188 + scale,
            {
                {"校准核心", "格挡 14 + 力量", 0, 1, 14, 2, 0, 0},
                {"光束阵列", "攻击 7x3", 7, 3, 0, 0, 0, 0},
                {"坍缩指令", "攻击 16 + 易伤", 16, 1, 0, 0, 2, 0},
                {"重构护甲", "格挡 22", 0, 1, 22, 0, 0, 0},
            },
        };
    }
    return makeEnemy(EnemyKind::AshCultist, progress);
}

} // namespace minispire
