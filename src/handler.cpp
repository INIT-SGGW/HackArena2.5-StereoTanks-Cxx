#include "handler.h"
#include "packet.h"

// Function to convert ResponseVariant to string
std::string Handler::ResponseToString(const ResponseVariant& response, std::string& id) {
	nlohmann::ordered_json jsonResponse;

	std::visit([&jsonResponse, &id](const auto& resp) {
	  using T = std::decay_t<decltype(resp)>;
	  if constexpr (std::is_same_v<T, Rotate>) {
		  jsonResponse["type"] = PacketType::TankRotation;
          if (static_cast<int>(resp.tankRotation) == 2){
              jsonResponse["payload"]["tankRotation"] = nullptr;
          } else jsonResponse["payload"]["tankRotation"] = static_cast<int>(resp.tankRotation);
          if (static_cast<int>(resp.turretRotation) == 2){
              jsonResponse["payload"]["turretRotation"] = nullptr;
          } else jsonResponse["payload"]["turretRotation"] = static_cast<int>(resp.turretRotation);
          jsonResponse["payload"]["gameStateId"] = id;
	  } else if constexpr (std::is_same_v<T, Move>) {
		  jsonResponse["type"] = PacketType::TankMovement;
		  jsonResponse["payload"]["direction"] = static_cast<int>(resp.direction);
          jsonResponse["payload"]["gameStateId"] = id;
	  } else if constexpr (std::is_same_v<T, AbilityUse>) {
		  jsonResponse["type"] = PacketType::AbilityUse;
          jsonResponse["payload"]["abilityType"] = static_cast<int>(resp.type);;
		  jsonResponse["payload"]["gameStateId"] = id;
	  } else if constexpr (std::is_same_v<T, Wait>) {
		  jsonResponse["type"] = PacketType::ResponsePass;
          jsonResponse["payload"]["gameStateId"] = id;
	  } else if constexpr (std::is_same_v<T, CaptureZone>) {
		  jsonResponse["type"] = PacketType::CaptureZone;
		  jsonResponse["payload"]["gameStateId"] = id;
	  } else if constexpr (std::is_same_v<T, GoTo>) {
          jsonResponse["type"] = PacketType::GoTo;

          // Required fields
          jsonResponse["payload"]["x"] = resp.x;
          jsonResponse["payload"]["y"] = resp.y;
          jsonResponse["payload"]["gameStateId"] = id;

          // Optional turret rotation
          if (resp.turretRotation.has_value()) {
              jsonResponse["payload"]["turretRotation"] = static_cast<int>(resp.turretRotation.value());
          } else {
              jsonResponse["payload"]["turretRotation"] = nullptr;
          }

          // Optional costs
          if (resp.costs.has_value()) {
              const auto& costs = resp.costs.value();
              jsonResponse["payload"]["costs"]["forward"] = costs.forward;
              jsonResponse["payload"]["costs"]["backward"] = costs.backward;
              jsonResponse["payload"]["costs"]["rotate"] = costs.rotate;
          }

          // Optional penalties
          if (resp.penalties.has_value()) {
              const auto& penalties = resp.penalties.value();

              // Add each penalty field only if it has a value
              if (penalties.blindly.has_value()) {
                  jsonResponse["payload"]["penalties"]["blindly"] = penalties.blindly.value();
              }
              if (penalties.tank.has_value()) {
                  jsonResponse["payload"]["penalties"]["tank"] = penalties.tank.value();
              }
              if (penalties.bullet.has_value()) {
                  jsonResponse["payload"]["penalties"]["bullet"] = penalties.bullet.value();
              }
              if (penalties.mine.has_value()) {
                  jsonResponse["payload"]["penalties"]["mine"] = penalties.mine.value();
              }
              if (penalties.laser.has_value()) {
                  jsonResponse["payload"]["penalties"]["laser"] = penalties.laser.value();
              }

              // Add per-tile penalties if any
              if (!penalties.perTile.empty()) {
                  nlohmann::json perTileArray = nlohmann::json::array();
                  for (const auto& tile : penalties.perTile) {
                      nlohmann::json tileJson;
                      tileJson["x"] = tile.x;
                      tileJson["y"] = tile.y;
                      tileJson["penalty"] = tile.penalty;
                      perTileArray.push_back(tileJson);
                  }
                  jsonResponse["payload"]["penalties"]["perTile"] = perTileArray;
              }
          }
      }
	}, response);

	return jsonResponse.dump(); // Convert JSON object to string
}

// Example of sending the response over WebSocket
void Handler::SendResponse(const ResponseVariant& response, std::string& id) {
	std::string responseString = ResponseToString(response, id);
	// Send the response over the WebSocket
	{
		std::lock_guard<std::mutex> lock(*mtxPtr);
		messagesToSendPtr->push(responseString);
	}
	cvPtr->notify_one();
}

void Handler::HandleGameState(nlohmann::json payload) {
	GameState gameState;

	// Parse playerId and tick (time)
	std::string id = payload["id"].get<std::string>();
	gameState.time = payload["tick"].get<int>();

	if (payload.contains("playerId") && !payload["playerId"].is_null()) {
		gameState.playerId = payload["playerId"].get<std::string>();
	}

	// Parse the teams and players
	for (const auto& teamJson : payload["teams"]) {
		Team team;
		team.name = teamJson["name"].get<std::string>();
		team.color = teamJson["color"].get<uint32_t>();

		// Optional team score
		if (teamJson.contains("score") && !teamJson["score"].is_null()) {
			team.score = teamJson["score"].get<int>();
		}

		// Parse players in each team
		for (const auto& playerJson : teamJson["players"]) {
			Player player;
			player.id = playerJson["id"].get<std::string>();
			player.ping = playerJson["ping"].get<int>();

			// Optional fields
			if (playerJson.contains("score") && !playerJson["score"].is_null()) {
				player.score = playerJson["score"].get<int>();
			}
			if (playerJson.contains("ticksToRegen") && !playerJson["ticksToRegen"].is_null()) {
				player.ticksToRegen = playerJson["ticksToRegen"].get<int>();
			}

			team.players.push_back(player);
		}

		gameState.teams.push_back(team);
	}

	// Parse the map
	const auto& mapJson = payload["map"];

	// Parse zones
	for (const auto& zoneJson : mapJson["zones"]) {
		Zone zone;
		zone.x = zoneJson["x"].get<int>();
		zone.y = zoneJson["y"].get<int>();
		zone.width = zoneJson["width"].get<int>();
		zone.height = zoneJson["height"].get<int>();
		zone.name = zoneJson["index"].get<char>();

		ZoneShares zoneShares;

		if (zoneJson.contains("shares")) {
			auto sharesJson = zoneJson["shares"];

			if (sharesJson.contains("neutral")) {
				zoneShares.neutral = sharesJson["neutral"];
			}

			for (auto& [teamName, share] : sharesJson.items()) {
				if (teamName != "neutral") {
					zoneShares.teamShares[teamName] = share;
				}
			}
		}

		zone.status = zoneShares;
		gameState.map.zones.push_back(zone);
	}

    const auto& layer = payload["map"]["tiles"];

    size_t numRows2 = layer.size();
    size_t numCols2 = layer[0].size(); // Assuming at least one row exists

    gameState.map.tiles.resize(numCols2, std::vector<Tile>(numRows2));

    // Loop through each row in the tiles layer
    for (size_t i = 0; i < numRows2; ++i) {
        const auto& rowJson = layer[i];

        // Loop through each tile in the row
        for (size_t j = 0; j < rowJson.size(); ++j) {
            Tile tile;
            for (const auto& tileJson : rowJson[j]){

                if (tileJson.empty()) {
                    // Handle empty tiles appropriately
                    gameState.map.tiles[j][i] = Tile{};
                    continue;
                }
                TileVariant nextObject;
                std::string type = tileJson["type"].get<std::string>();

                if (type == "wall") {
                	Wall wall;

                	wall.type = tileJson["payload"]["type"].get<WallType>();

                    nextObject = wall;
                }
                else if (type == "tank") {
                    // Parse Tank
                    Tank tank;

					// Check if ownerId exists and is not null
					if (!tileJson["payload"].contains("ownerId") || tileJson["payload"]["ownerId"].is_null()) {
						throw std::runtime_error("Missing or null ownerId in tank payload.");
					}
					tank.ownerId = tileJson["payload"]["ownerId"].get<std::string>();

					// Check if tankType exists and is not null
					if (!tileJson["payload"].contains("type") || tileJson["payload"]["type"].is_null()) {
						throw std::runtime_error("Missing or null tankType in tank payload.");
					}
					tank.tankType = static_cast<TankType>(tileJson["payload"]["type"].get<int>());

					// Check if direction exists and is not null
					if (!tileJson["payload"].contains("direction") || tileJson["payload"]["direction"].is_null()) {
						throw std::runtime_error("Missing or null direction in tank payload.");
					}
					tank.direction = static_cast<Direction>(tileJson["payload"]["direction"].get<int>());

                    // Parse Turret (assuming it is mandatory in Tank)
                    if (!tileJson["payload"].contains("turret") || tileJson["payload"]["turret"].is_null()) {
                        throw std::runtime_error("Missing turret in tank payload.");
                    }

                    const auto& turretJson = tileJson["payload"]["turret"];

                    // Check if turret direction exists and is not null
                    if (!turretJson.contains("direction") || turretJson["direction"].is_null()) {
                        throw std::runtime_error("Missing or null turret direction.");
                    }
                    tank.turret.direction = turretJson["direction"].get<Direction>();

					// Optional bulletCount field for turret
					if (turretJson.contains("bulletCount") && !turretJson["bulletCount"].is_null()) {
						tank.turret.bulletCount = turretJson["bulletCount"].get<int>();
					}

					// Optional ticksToBullet field for turret (previously ticksToRegenBullet)
					if (turretJson.contains("ticksToBullet") && !turretJson["ticksToBullet"].is_null()) {
						tank.turret.ticksToBullet = turretJson["ticksToBullet"].get<int>();
					}

					// Optional ticksToDoubleBullet field for light tanks
					if (turretJson.contains("ticksToDoubleBullet") && !turretJson["ticksToDoubleBullet"].is_null()) {
						tank.turret.ticksToDoubleBullet = turretJson["ticksToDoubleBullet"].get<int>();
					}

					// Optional ticksToLaser field for heavy tanks
					if (turretJson.contains("ticksToLaser") && !turretJson["ticksToLaser"].is_null()) {
						tank.turret.ticksToLaser = turretJson["ticksToLaser"].get<int>();
					}

                	if (turretJson.contains("ticksToHealingBullet") && !turretJson["ticksToHealingBullet"].is_null()) {
                		tank.turret.ticksToHealingBullet = turretJson["ticksToHealingBullet"].get<int>();
                	}

                	if (turretJson.contains("ticksToStunBullet") && !turretJson["ticksToStunBullet"].is_null()) {
                		tank.turret.ticksToHealingBullet = turretJson["ticksToStunBullet"].get<int>();
                	}

					// Optional health field
					if (tileJson["payload"].contains("health") && !tileJson["payload"]["health"].is_null()) {
						tank.health = tileJson["payload"]["health"].get<int>();
					}

					// Tank type specific fields
					if (tank.tankType == TankType::Heavy) {
						// Optional ticksToMine field for heavy tanks
						if (tileJson["payload"].contains("ticksToMine") && !tileJson["payload"]["ticksToMine"].is_null()) {
							tank.ticksToMine = tileJson["payload"]["ticksToMine"].get<int>();
						}
					} else if (tank.tankType == TankType::Light) {
						// Optional ticksToRadar field for light tanks
						if (tileJson["payload"].contains("ticksToRadar") && !tileJson["payload"]["ticksToRadar"].is_null()) {
							tank.ticksToRadar = tileJson["payload"]["ticksToRadar"].get<int>();
						}

						// Optional isUsingRadar field for light tanks
						if (tileJson["payload"].contains("isUsingRadar") && !tileJson["payload"]["isUsingRadar"].is_null()) {
							tank.isUsingRadar = tileJson["payload"]["isUsingRadar"].get<bool>();
						}
					}

                	// Parse visibility (2D array of chars)
                	if (tileJson["payload"].contains("visibility") && !tileJson["payload"]["visibility"].is_null()) {
                		const auto& visibilityJson = tileJson["payload"]["visibility"];
                		size_t numRows = visibilityJson.size();
                		size_t numCols = visibilityJson[0].get<std::string>().size();

                		// FIXED: Check if the optional has a value and initialize it if not
                		if (!tank.visibility.has_value()) {
                			tank.visibility = std::vector<std::vector<char>>();
                		}

                		tank.visibility.value().resize(numCols, std::vector<char>(numRows));
                		for (size_t i = 0; i < numRows; ++i) {
                			std::string row = visibilityJson[i].get<std::string>();
                			for (size_t j = 0; j < numCols; ++j) {
                				tank.visibility.value()[i][j] = row[j];
                			}
                		}
                	}

					nextObject = tank;
				}
                else if (type == "bullet") {
                    // Parse Bullet
                    Bullet bullet{};
                    bullet.id = tileJson["payload"]["id"].get<int>();
                    bullet.speed = tileJson["payload"]["speed"].get<double>();
                    bullet.direction = tileJson["payload"]["direction"].get<Direction>();
                    bullet.type = tileJson["payload"]["type"].get<BulletType>();
                    nextObject = bullet;
                }
                else if (type == "laser") {
                    // Parse Item
                    Laser laser{};
                    laser.id = tileJson["payload"]["id"].get<int>();
                    laser.orientation = tileJson["payload"]["orientation"].get<LaserOrientation>();
                    nextObject = laser;
                }
                else if (type == "mine") {
                    // Parse Item
                    Mine mine;
                    mine.id = tileJson["payload"]["id"].get<int>();
                    if (tileJson["payload"].contains("explosionRemainingTicks") && !tileJson["payload"]["explosionRemainingTicks"].is_null()) {
                        mine.explosionRemainingTicks = tileJson["payload"]["explosionRemainingTicks"].get<int>();
                    }
                    nextObject = mine;
                }
                tile.objects.push_back(nextObject);
            }

            // Use the ParseTileVariant function to parse each tile
            gameState.map.tiles[j][i] = tile; // Access the first item in the tile array
        }
    }

    for (size_t row = 0; row < gameState.map.tiles.size(); ++row) {
        for (size_t col = 0; col < gameState.map.tiles[row].size(); ++col) {

            // Initialize zoneName to '?' indicating no zone
            gameState.map.tiles[row][col].zoneName = '?';

            // Check each zone to see if the tile belongs to it
            for (const auto& zone : gameState.map.zones) {
                if (col >= zone.x && col < zone.x + zone.width &&
                    row >= zone.y && row < zone.y + zone.height) {
                    gameState.map.tiles[row][col].zoneName = zone.name;  // Assign the zone name
                    break;  // Stop once a zone is found
                }
            }
        }
    }

	auto start = std::chrono::high_resolution_clock::now();
	ResponseVariant response = botPtr->NextMove(gameState);
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;

	if(duration.count() < botPtr->skipResponse) SendResponse(response, id);
}

void Handler::HandleGameEnded(nlohmann::json payload) {
	EndGameLobby endGameLobby;

	// Extract teams array and populate the teams vector
	for (const auto& team : payload.at("teams")) {
		EndGameTeam endGameTeam;
		endGameTeam.name = team.at("name").get<std::string>();
		endGameTeam.color = team.at("color").get<uint32_t>();
		endGameTeam.score = team.at("score").get<int>();

		// Extract players array for this team
		for (const auto& player : team.at("players")) {
			EndGamePlayer endGamePlayer;
			endGamePlayer.id = player.at("id").get<std::string>();
			endGamePlayer.kills = player.at("kills").get<int>();
			endGamePlayer.tankType = player.at("tankType").get<TankType>();

			// Add player to the team
			endGameTeam.players.push_back(endGamePlayer);
		}

		// Add team to the lobby
		endGameLobby.teams.push_back(endGameTeam);
	}

    botPtr->OnGameEnded(endGameLobby);
}

void Handler::HandleLobbyData(nlohmann::json payload) {
	LobbyData lobbyData;

	// Extract the playerId
	lobbyData.myId = payload.at("playerId").get<std::string>();
	lobbyData.teamName = payload.at("teamName").get<std::string>();

	// Extract teams array and populate the teams vector
	for (const auto& teamJson : payload.at("teams")) {
		LobbyTeams lobbyTeam;
		lobbyTeam.name = teamJson.at("name").get<std::string>();
		lobbyTeam.color = teamJson.at("color").get<uint32_t>();

		// Extract players in this team
		for (const auto& playerJson : teamJson.at("players")) {
			LobbyPlayer lobbyPlayer;
			lobbyPlayer.id = playerJson.at("id").get<std::string>();

			// Parse tank type
			if (playerJson.contains("tankType") && !playerJson["tankType"].is_null()) {
				lobbyPlayer.tankType = static_cast<TankType>(playerJson.at("tankType").get<int>());
			}

			lobbyTeam.players.push_back(lobbyPlayer);
		}

		lobbyData.teams.push_back(lobbyTeam);
	}

	// Extract server settings from the nested object
	const auto& serverSettings = payload.at("serverSettings");


    if (serverSettings.contains("matchName") && !serverSettings["matchName"].is_null()) {
        lobbyData.matchName = serverSettings.at("matchName").get<std::string>();
    }
    lobbyData.sandboxMode = serverSettings.at("sandboxMode").get<bool>();
	lobbyData.gridDimension = serverSettings.at("gridDimension").get<int>();
	lobbyData.numberOfPlayers = serverSettings.at("numberOfPlayers").get<int>();
	lobbyData.seed = serverSettings.at("seed").get<int>();
	lobbyData.broadcastInterval = serverSettings.at("broadcastInterval").get<int>();
	lobbyData.eagerBroadcast = serverSettings.at("eagerBroadcast").get<bool>();
	lobbyData.version = serverSettings.at("version").get<std::string>();

	// Initialize the bot with the parsed lobby data
	botPtr->Init(lobbyData);

    if (lobbyData.sandboxMode) HandleGameStarting();
}

Handler::Handler(Bot *botPtr, std::queue<std::string> *messagesToSendPtr, std::mutex *mtxPtr,std::condition_variable *cvPtr)
: botPtr(botPtr), messagesToSendPtr(messagesToSendPtr), mtxPtr(mtxPtr), cvPtr(cvPtr) {}

void Handler::OnWarningReceived(WarningType warningType, std::optional<std::string> message) {
    botPtr->OnWarningReceived(warningType, message);
}

void Handler::HandleGameStarting() {
    botPtr->OnGameStarting();
    try {
        // Serialize Packet to JSON
        nlohmann::json jsonResponse;
        jsonResponse["type"] = static_cast<uint64_t>(PacketType::ReadyToReceiveGameState);

        std::string responseString = jsonResponse.dump();

        // Send the response over the WebSocket
        std::lock_guard<std::mutex> lock(*mtxPtr);
        messagesToSendPtr->push(responseString);
        cvPtr->notify_one();
    } catch (const std::exception& e) {
        std::cerr << "Error responding to GameStarting: " << e.what() << std::endl << std::flush;
    }
}
