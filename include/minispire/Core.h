#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <map>
#include <optional>
#include <random>
#include <string>
#include <vector>

namespace minispire {

enum class CardType {
    Attack,
    Skill,
    Power
};

enum class EffectType {
    Damage,
    Block,
    Draw,
    Strength,
    Vulnerable,
    Weak,
    Heal,
    GainEnergy,
    Ritual
};

enum class StatusType {
    Strength,
    Vulnerable,
    Weak,
    Ritual
};

enum class EnemyKind {
    AshCultist,
    AcidSlime,
    BellGuard,
    ThornLurker,
    CrystalWisp,
    EmberDuelist,
    NullPriest,
    ChronoKnight,
    IronSentinel,
    RootMatriarch,
    ClockworkDragon,
    SpireArchitect
};

enum class NodeType {
    Battle,
    Elite,
    Boss,
    Shop,
    Rest,
    Event
};

enum class CharacterId {
    RiftTraveler,
    EmberAdept,
    CrystalWarden
};

struct Effect {
    EffectType type {};
    int amount {};
};

struct Card {
    std::string id;
    std::string name;
    int cost {};
    CardType type {};
    std::string rarity;
    std::string description;
    std::vector<Effect> effects;
    bool exhaust {false};
};

struct Potion {
    std::string id;
    std::string name;
    std::string description;
    std::vector<Effect> effects;
};

struct CharacterDefinition {
    CharacterId id {};
    std::string name;
    std::string subtitle;
    std::string description;
    std::string spriteKey;
    int maxHp {72};
    int maxEnergy {3};
    int startingGold {99};
    std::vector<Card> startingDeck;
    std::vector<Card> cardPool;
    std::vector<std::string> startingRelics;
};

struct CombatEvent {
    std::string text;
};

class Creature {
public:
    Creature() = default;
    Creature(std::string name, int maxHp);

    const std::string& name() const;
    int hp() const;
    int maxHp() const;
    int block() const;
    bool isAlive() const;
    bool isDead() const;

    int status(StatusType type) const;
    void addStatus(StatusType type, int amount);
    void setStatus(StatusType type, int amount);
    void reduceStatus(StatusType type, int amount);
    const std::map<StatusType, int>& statuses() const;

    void gainBlock(int amount);
    int receiveDamage(int amount);
    void heal(int amount);
    void clearBlock();
    void clearStatuses();
    void setHp(int hp);
    void setMaxHp(int maxHp);

private:
    std::string name_;
    int maxHp_ {1};
    int hp_ {1};
    int block_ {0};
    std::map<StatusType, int> statuses_;
};

class Player : public Creature {
public:
    Player();
    Player(std::string name, int maxHp, int maxEnergy, int gold);

    int energy() const;
    int maxEnergy() const;
    int gold() const;
    const std::vector<std::string>& relics() const;
    const std::array<std::optional<Potion>, 2>& potions() const;

    void beginTurn();
    bool spendEnergy(int amount);
    void gainEnergy(int amount);
    void gainGold(int amount);
    bool spendGold(int amount);
    void addRelic(std::string relic);
    bool addPotion(Potion potion);
    std::optional<Potion> takePotion(std::size_t slot);
    bool discardPotion(std::size_t slot);
    void setPotion(std::size_t slot, std::optional<Potion> potion);
    void setGold(int gold);
    void setMaxEnergy(int maxEnergy);

private:
    int maxEnergy_ {3};
    int energy_ {3};
    int gold_ {99};
    std::vector<std::string> relics_;
    std::array<std::optional<Potion>, 2> potions_ {};
};

struct EnemyMove {
    std::string name;
    std::string intentText;
    int damage {0};
    int hits {1};
    int block {0};
    int strength {0};
    int vulnerable {0};
    int weak {0};
};

class Enemy : public Creature {
public:
    Enemy() = default;
    Enemy(EnemyKind kind, std::string name, int maxHp, std::vector<EnemyMove> moves);

    EnemyKind kind() const;
    EnemyMove previewMove() const;
    EnemyMove takeMove();
    void onHalfHealth();
    bool halfHealthTriggered() const;

private:
    EnemyKind kind_ {EnemyKind::AshCultist};
    std::vector<EnemyMove> moves_;
    std::size_t moveIndex_ {0};
    bool halfHealthTriggered_ {false};
};

class Deck {
public:
    static constexpr std::size_t MaxHandSize = 10;

    void reset(std::vector<Card> cards, std::mt19937& rng);
    void drawCards(int count, std::mt19937& rng);
    bool hasPlayableCard(int energy) const;

    const std::vector<Card>& hand() const;
    std::vector<Card>& hand();
    std::size_t drawCount() const;
    std::size_t discardCount() const;
    std::size_t exhaustCount() const;
    std::size_t totalCount() const;

    Card removeFromHand(std::size_t index);
    void discard(Card card);
    void exhaust(Card card);
    void discardHand();

private:
    void reshuffleDiscardIntoDraw(std::mt19937& rng);

    std::vector<Card> drawPile_;
    std::vector<Card> discardPile_;
    std::vector<Card> hand_;
    std::vector<Card> exhaustPile_;
};

struct PlayResult {
    bool accepted {false};
    std::string message;
};

class CombatState {
public:
    CombatState(Player player, Enemy enemy, std::vector<Card> deckCards, std::uint32_t seed);

    void start();
    PlayResult playCard(std::size_t handIndex);
    PlayResult usePotion(std::size_t slot);
    PlayResult discardPotion(std::size_t slot);
    void endPlayerTurn();

    const Player& player() const;
    Player& player();
    const Enemy& enemy() const;
    Enemy& enemy();
    const Deck& deck() const;
    Deck& deck();
    const std::vector<CombatEvent>& events() const;

    bool isPlayerTurn() const;
    bool victory() const;
    bool defeat() const;
    bool finished() const;
    int turn() const;

private:
    void beginPlayerTurn();
    void applyRelicsAtCombatStart();
    void runEnemyTurn();
    void applyCardEffect(const Card& card, const Effect& effect);
    void applyPotionEffect(const Potion& potion, const Effect& effect);
    void applyEnemyMove(const EnemyMove& move);
    int scaledOutgoingDamage(const Creature& attacker, int base) const;
    int scaledIncomingDamage(const Creature& defender, int base) const;
    void log(std::string text);
    void checkEndConditions();

    Player player_;
    Enemy enemy_;
    Deck deck_;
    std::mt19937 rng_;
    std::vector<CombatEvent> events_;
    bool started_ {false};
    bool playerTurn_ {false};
    bool victory_ {false};
    bool defeat_ {false};
    int turn_ {0};
};

struct MapNode {
    int id {};
    int row {};
    int lane {};
    NodeType type {};
    std::vector<int> next;
    bool available {false};
    bool completed {false};
};

struct RunHistoryEntry {
    bool won {false};
    int level {1};
    int maxLevels {3};
    int steps {0};
    int deckSize {0};
    int hp {0};
    int maxHp {0};
};

struct RunHistorySummary {
    int totalRuns {0};
    int wins {0};
    std::vector<RunHistoryEntry> recent;
};

class RunController {
public:
    RunController();

    void startNewRun(std::uint32_t seed = std::random_device{}());
    void startNewRun(CharacterId character, std::uint32_t seed = std::random_device{}());
    bool active() const;
    bool won() const;
    void setWon(bool won);
    CharacterId characterId() const;
    const CharacterDefinition& character() const;

    const Player& player() const;
    Player& player();
    const std::vector<Card>& deck() const;
    const std::vector<MapNode>& map() const;
    const std::optional<int>& activeNodeId() const;
    int steps() const;
    int level() const;
    int maxLevels() const;
    bool finalLevel() const;
    std::string levelName() const;

    std::vector<int> availableNodeIds() const;
    bool isNodeAvailable(int id) const;
    const MapNode* findNode(int id) const;
    MapNode* findNode(int id);
    bool selectNode(int id);
    void completeActiveNode();

    Enemy makeEnemyForActiveNode();
    std::vector<Card> makeRewards(int count);
    std::vector<Card> makeShopCards(int count);
    std::vector<std::string> makeShopRelics(int count);
    std::vector<Potion> makeShopPotions(int count);
    Card makeRandomReward();
    Potion makeRandomPotion();
    void addCardToDeck(const Card& card);
    PlayResult removeCardFromDeck(std::size_t index, int cost);
    void syncPlayerAfterCombat(const Player& player);
    void startNextLevel();
    void recoverAfterBoss();
    void rest();
    void eventGainGold();
    bool eventHealAvailable() const;
    void eventHeal();
    bool saveToFile(const std::string& path = "mini_spire_save.txt") const;
    bool loadFromFile(const std::string& path = "mini_spire_save.txt");
    bool deleteSaveFile(const std::string& path = "mini_spire_save.txt") const;
    void recordRunResult(bool won, const std::string& historyPath = "mini_spire_history.txt");

    static bool hasSaveFile(const std::string& path = "mini_spire_save.txt");
    static RunHistorySummary loadHistory(const std::string& path = "mini_spire_history.txt");

private:
    void generateMap();
    void unlockInitialNodes();

    Player player_;
    std::vector<Card> deck_;
    std::vector<MapNode> map_;
    std::optional<int> activeNodeId_;
    int steps_ {0};
    int level_ {1};
    int maxLevels_ {3};
    bool active_ {false};
    bool won_ {false};
    bool resultRecorded_ {false};
    CharacterId characterId_ {CharacterId::RiftTraveler};
    std::mt19937 rng_;
};

std::string toString(CardType type);
std::string toString(NodeType type);
std::string toString(StatusType type);
std::string relicDescription(const std::string& relic);

std::vector<CharacterDefinition> characterDefinitions();
const CharacterDefinition& characterDefinition(CharacterId id);
bool isCardInCharacterPool(CharacterId character, const std::string& cardId);
std::vector<Card> starterDeck();
std::vector<Card> starterDeck(CharacterId character);
std::vector<Card> cardPool();
std::vector<Card> cardPool(CharacterId character);
std::vector<Potion> potionPool();
Enemy makeEnemy(EnemyKind kind, int progress);

} // namespace minispire
