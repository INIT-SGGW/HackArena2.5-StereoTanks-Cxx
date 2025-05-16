#include "bot.h"
#include <utility>
#include <random>

void Bot::PrintMap(const std::vector<std::vector<Tile>>& tiles) {
    auto rows = tiles.size();
    auto cols = tiles[0].size();

    std::vector<std::vector<std::string>> map(rows, std::vector<std::string>(cols, " "));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            const Tile& tile = tiles[i][j];

            // Check for tile objects
            bool hasObject = false;
            for (const TileVariant& object : tile.objects) {
                if (std::holds_alternative<Wall>(object)) {
                    map[i][j] = '#';
                    hasObject = true;
                    break; // Found wall, no need to check further
                } else if (std::holds_alternative<Tank>(object)) {
                    if (auto tankPtr = std::get_if<Tank>(&object)) {
                        auto tankId = tankPtr->ownerId;
                        // Use '@' for player's tank, 'T' for enemy tanks
                        std::string tankSymbol = (tankId != myId) ? "T" : "@";
                        tankSymbol += (tankPtr->direction == Direction::up) ? "^" :
                                      (tankPtr->direction == Direction::down) ? "v" :
                                      (tankPtr->direction == Direction::left) ? "<" : ">";
                        map[i][j] = tankSymbol;
                        hasObject = true;
                        break; // Found tank, no need to check further
                    }
                } else if (std::holds_alternative<Mine>(object)) {
                    map[i][j] = 'X';
                    hasObject = true;
                } else if (std::holds_alternative<Laser>(object)) {
                    if (auto laserPtr = std::get_if<Laser>(&object)) {
                        map[i][j] = (laserPtr->orientation == LaserOrientation::horizontal) ? '-' : '!';
                        hasObject = true;
                    }
                } else if (std::holds_alternative<Bullet>(object)) {
                    if (auto bulletPtr = std::get_if<Bullet>(&object)) {
                        if (bulletPtr->type == BulletType::basic) {
                            map[i][j] = (bulletPtr->direction == Direction::up) ? "^" :
                                        (bulletPtr->direction == Direction::down) ? "v" :
                                        (bulletPtr->direction == Direction::left) ? "<" : ">";
                        } else {
                            map[i][j] = (bulletPtr->direction == Direction::up) ? "^^" :
                                        (bulletPtr->direction == Direction::down) ? "vv" :
                                        (bulletPtr->direction == Direction::left) ? "<<" : ">>";
                        }
                        hasObject = true;
                    }
                }
                break; // Stop checking objects after the first found
            }

            // If no object found, check for None tile properties
            if (!hasObject) {
                if (tile.zoneName != '?') {
                    map[i][j] = tile.zoneName; // Print zone name
                } else {
                    map[i][j] = ' '; // Not visible, so leave it as empty space
                }
            }
        }
    }

    // Print the map
    for (const auto& row : map) {
        bool firstTile = true; // Flag to handle spacing
        for (const auto& tile : row) {
            // Only print space before tile if it is not the first tile
            if (firstTile) {
                firstTile = false; // Reset the flag after the first tile is printed
            } else {
                // Check the length of the last printed tile
                if (tile.length() == 1) {
                    std::cout << ' '; // Print space for single character tiles
                }
            }
            std::cout << tile; // Print the tile character(s)
        }
        std::cout << '\n'; // New line after each row
    }
    std::cout << "---------------------------------------------------" << std::endl;
}

Bot::Bot() = default;
void Bot::Init(const LobbyData& lobbyData) { myId = lobbyData.myId; skipResponse = lobbyData.broadcastInterval - 1;}
ResponseVariant Bot::NextMove(const GameState& gameState) {
	PrintMap(gameState.map.tiles);

    // Create a random device and generator
    std::random_device rd;
    std::mt19937 gen(rd());

    // Define the distributions for each random selection
    std::uniform_int_distribution<int> actionDist(0, 3); // 4 possible actions
    std::uniform_int_distribution<int> directionDist(0, 1); // for MoveDirection and RotationDirection
    std::uniform_int_distribution<int> abilityDist(0, 4); // 5 possible abilities

    // Randomly choose the type of action
    int action = actionDist(gen);

    switch (action) {
        case 0: { // Rotate
            RotationDirection tankRot = static_cast<RotationDirection>(directionDist(gen));
            RotationDirection turretRot = static_cast<RotationDirection>(directionDist(gen));
            return Rotate{tankRot, turretRot};
        }
        case 1: { // Move
            MoveDirection moveDir = static_cast<MoveDirection>(directionDist(gen));
            return Move{moveDir};
        }
        case 2: { // AbilityUse
            AbilityType ability = static_cast<AbilityType>(abilityDist(gen));
            return AbilityUse{ability};
        }
        case 3: // Wait
        default:
            return Wait{};
    }
}

void Bot::OnWarningReceived(WarningType warningType, std::optional<std::string> &message) {}
void Bot::OnGameStarting() {}
void Bot::OnGameEnded(const EndGameLobby& endGameLobby) {}
