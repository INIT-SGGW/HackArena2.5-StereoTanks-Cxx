#include "pch.h"

#pragma once

enum class TankType {
	Light = 0,
	Heavy = 1
};

/// First received list of players
struct LobbyPlayer {
	std::string id;
	TankType tankType;
};

/// First received list of teams
struct LobbyTeams {
	std::string name;
	uint32_t color;
	std::vector<LobbyPlayer> players;
};

/// Player received at game end
struct EndGamePlayer {
    std::string id;
    int kills;
    TankType tankType;
};

struct EndGameTeam {
	std::string name;
	uint32_t color;
	int score;
	std::vector<EndGamePlayer> players;
};

struct EndGameLobby {
    std::vector<EndGameTeam> teams;
};

struct LobbyData {
	std::string myId;
	std::string teamName;
	std::vector<LobbyTeams> teams;
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
	std::optional<int> ticksToBullet;
	/// Only in light tanks, not present in enemies
	std::optional<int> ticksToDoubleBullet;
	/// Only in heavy tanks, not present in enemies
	std::optional<int> ticksToLaser;
	std::optional<int> ticksToHealingBullet;
	std::optional<int> ticksToStunBullet;
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
	TankType tankType;
    Direction direction;
	Turret turret;
    /// Not present in enemies
	std::optional<int> health;
	/// Only in heavy tanks, not present in enemies
	std::optional<int> ticksToMine;
	/// Only in light tanks, not present in enemies
	std::optional<int> ticksToRadar;
	/// Only in light tanks
	std::optional<bool> isUsingRadar;
	/// 2D array of chars ('0' or '1') same as tiles
	std::optional<std::vector<std::vector<char>>> visibility;
};

enum class BulletType {
    basic = 0,
    doubleBullet = 1,
	healing = 2,
	stun = 3
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
	int ping;
    /// Not present in enemies
	std::optional<int> score;
    /// Optional because it might be null (it is present only if you are this player)
	std::optional<int> ticksToRegen;
};

struct Team {
	std::string name;
	uint32_t color;
	std::optional<int> score;
	std::vector<Player> players;
};

struct Wall {};

using TileVariant = std::variant<Wall, Tank, Bullet, Mine, Laser>;

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
};

/// GameState struct
struct GameState {
    /// tick number
	int time;
	std::vector<Team> teams;
	std::optional<std::string> playerId;
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
    dropMine = 4,
	fireHealingBullet = 5,
	fireStunBullet = 6,
};

struct AbilityUse {
    AbilityType type;
};

struct Wait {};

// For per-tile penalties
struct PerTilePenalty {
	int x;
	int y;
	float penalty;
};

// Penalties structure
struct GotoPenalties {
	float blindly = 5.0f;       // Default value
	float bullet = 20.0f;       // Default value
	float mine = 20.0f;         // Default value
	float laser = 20.0f;        // Default value
	std::vector<PerTilePenalty> perTile;
};

// Costs structure
struct GotoCosts {
	float forward = 1.0f;       // Default value
	float backward = 1.5f;      // Default value
	float rotate = 1.5f;        // Default value
};

// Complete GoTo structure
struct GoTo {
	int x;
	int y;
	std::optional<RotationDirection> turretRotation;
	std::optional<GotoCosts> costs;
	std::optional<GotoPenalties> penalties;
};

using ResponseVariant = std::variant<Rotate, Move, AbilityUse, Wait, GoTo>;

enum class WarningType {
    CustomWarning,
    PlayerAlreadyMadeActionWarning,
    ActionIgnoredDueToDeadWarning,
    SlowResponseWarning,
};