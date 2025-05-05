#pragma once

#include "../processed-packets.h"

class Bot {
 public:
    /// DO NOT DELETE
    /// time in milliseconds after which the NextMove() answer is not sent to server, CAN BE CHANGED WHENEVER YOU WANT
    int skipResponse = 99;
	std::string myId;

	Bot();
	void Init(const LobbyData& lobbyData);
	ResponseVariant NextMove(const GameState& gameState);
	void OnGameEnded(const EndGameLobby& endGameLobby);
    void OnWarningReceived(WarningType warningType, std::optional<std::string>& message);
    void OnGameStarting();
	void PrintMap(const std::vector<std::vector<Tile>>& tiles);
};
