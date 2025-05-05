#include "pch.h"

#pragma once

/// First received list of players
struct LobbyPlayer {
	std::string id;
	std::string nickname;
	uint32_t color;
};

/// Player received at game end
struct EndGamePlayer {
    std::string id;
    std::string nickname;
    uint32_t color;
    int score;
};

struct EndGameLobby {
    std::vector<EndGamePlayer> players;
};

struct LobbyData {
	std::string myId;
	std::vector<LobbyPlayer> players;
    bool sandboxMode;
    std::optional<std::string> matchName;
	int gridDimension;
	int numberOfPlayers;
	int seed;
    /// how many milliseconds in tick
	int broadcastInterval;
	bool eagerBroadcast;
	std::string version;
};


enum class Direction {
    up = 0,
    right = 1,
    down = 2,
    left = 3
};

/// Turret struct for tanks
struct Turret {
	Direction direction;
    /// Not present in enemies
	std::optional<int> bulletCount;
    /// Not present in enemies
	std::optional<int> ticksToRegenBullet;
};

enum class SecondaryItemType {
    unknown = 0,
    Laser = 1,
    DoubleBullet = 2,
    Radar = 3,
    Mine = 4,
};

/// TankPayload struct
struct Tank {
	std::string ownerId;
    Direction direction;
	Turret turret;
    /// Not present in enemies
	std::optional<int> health;
    std::optional<SecondaryItemType> secondaryItem;
};

enum class BulletType {
    bullet = 0,
    doubleBullet = 1
};

/// BulletPayload struct
struct Bullet {
	int id;
    BulletType type;
	double speed;
    Direction direction;
};

enum class LaserOrientation {
    horizontal = 0,
    vertical = 1
};

struct Laser {
    int id;
    LaserOrientation orientation;
};

struct Mine {
    int id;
    std::optional<int> explosionRemainingTicks;
};

/// ZoneStatus struct to represent various zone states
struct ZoneStatus {
	std::string type;
    /// Used in "beingCaptured" and "beingRetaken"
	std::optional<int> remainingTicks;
    /// Used in "beingCaptured" and "captured"
	std::optional<std::string> playerId;
    /// Used in "beingContested" and "beingRetaken"
	std::optional<std::string> capturedById;
    /// Used in "beingRetaken"
	std::optional<std::string> retakenById;
};

/// Zone struct to represent a zone on the map
struct Zone {
	int x;
	int y;
	int width;
	int height;
	char name;
	ZoneStatus status;
};

// Player struct
struct Player {
	std::string id;
	std::string nickname;
	uint32_t color;
	int ping;
    /// Not present in enemies
	std::optional<int> score;
    /// Optional because it might be null
	std::optional<int> ticksToRegen;
    bool isUsingRadar;
};

struct Wall {};

enum class ItemType {
    unknown = 0,
    laser = 1,
	doubleBullet = 2,
    radar = 3,
    mine = 4
};

struct Item {
    ItemType type;
};

using TileVariant = std::variant<Wall, Tank, Bullet, Mine, Laser, Item>;

struct Tile {
    std::vector<TileVariant> objects;
    bool isVisible;
    char zoneName; // '?' or 63 for no zone
};

/// Map struct:
/// Tiles are stored in a 2D array
/// Inner array represents columns of the map
/// Outer arrays represent rows of the map
/// Item with index [0][0] represents top-left corner of the map
struct Map {
	/// A 2D vector to hold variants of tile objects
	std::vector<std::vector<Tile>> tiles;
	std::vector<Zone> zones;
    /// 2D array of chars ('0' or '1') same as tiles
	std::vector<std::vector<char>> visibility;
};

/// GameState struct
struct GameState {
    /// tick number
	int time;
	std::vector<Player> players;
	Map map;
};

enum class RotationDirection {
	left = 0,
	right = 1,
    none = 2,
};

enum class MoveDirection {
	forward = 0,
	backward = 1
};

struct Rotate {
	RotationDirection tankRotation;
	RotationDirection turretRotation;
};

struct Move {
	MoveDirection direction;
};

enum class AbilityType {
    fireBullet = 0,
    useLaser = 1,
	fireDoubleBullet = 2,
    useRadar = 3,
    dropMine = 4
};

struct AbilityUse {
    AbilityType type;
};

struct Wait {};

using ResponseVariant = std::variant<Rotate, Move, AbilityUse, Wait>;

enum class WarningType {
    CustomWarning,
    PlayerAlreadyMadeActionWarning,
    ActionIgnoredDueToDeadWarning,
    SlowResponseWarning,
};