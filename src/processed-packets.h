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

enum class WallType {
	solid = 0,
	penetrable = 1
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

/// ZoneShares struct to represent various zone shares
struct ZoneShares {
	float neutral;
	std::map<std::string, float> teamShares;
};

/// Zone struct to represent a zone on the map
struct Zone {
	int x;
	int y;
	int width;
	int height;
	char name;
	ZoneShares status;
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

struct Wall {
	WallType type;
};

using TileVariant = std::variant<Wall, Tank, Bullet, Mine, Laser>;

struct Tile {
    std::vector<TileVariant> objects;
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

struct CaptureZone {};

// For per-tile penalties
struct PerTilePenalty {
	int x;
	int y;
	float penalty;
};

// Penalties structure - can be null entirely, in which case pathfinding won't apply any penalties
struct GotoPenalties {
	std::optional<float> blindly;   // Can be null, in which case pathfinding won't penalize going blindly
	std::optional<float> tank;      // Can be null, in which case pathfinding won't penalize going through tanks
	std::optional<float> bullet;    // Can be null, in which case pathfinding won't penalize going through bullets
	std::optional<float> mine;      // Can be null, in which case pathfinding won't penalize going through mines
	std::optional<float> laser;     // Can be null, in which case pathfinding won't penalize going through lasers
	std::vector<PerTilePenalty> perTile; // Specific per-tile penalties
};

// Costs structure with default values
struct GotoCosts {
	float forward = 1.0f;       // Default value: 1.0
	float backward = 1.5f;      // Default value: 1.5
	float rotate = 1.5f;        // Default value: 1.5
};

// Complete GoTo structure
struct GoTo {
	int x;                                      // Target x coordinate
	int y;                                      // Target y coordinate
	std::optional<RotationDirection> turretRotation; // Optional turret rotation (left/0, right/1)
	std::optional<GotoCosts> costs;             // Optional movement costs
	std::optional<GotoPenalties> penalties;     // Optional penalties, can be null to disable all penalties
};

using ResponseVariant = std::variant<Rotate, Move, AbilityUse, Wait, GoTo, CaptureZone>;

enum class WarningType {
    CustomWarning,
    PlayerAlreadyMadeActionWarning,
    ActionIgnoredDueToDeadWarning,
    SlowResponseWarning,
};