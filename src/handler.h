#pragma once

#include "pch.h"
#include "bot/bot.h"
#include "processed-packets.h"
#include "packet.h"

class Handler {
 public:
	Handler(Bot *botPtr, std::queue<std::string> *messagesToSendPtr, std::mutex *mtxPtr, std::condition_variable *cvPtr);
	void HandleLobbyData(nlohmann::json payload);
	void HandleGameState(nlohmann::json payload);
	void HandleGameEnded(nlohmann::json payload);
    void HandleGameStarting();
    void OnWarningReceived(WarningType warningType, std::optional<std::string> message);

 private:
	static std::string ResponseToString(const ResponseVariant& response, std::string& id);
	void SendResponse(const ResponseVariant& response, std::string& id);
	Bot *botPtr;
	std::queue<std::string> *messagesToSendPtr;
	std::mutex *mtxPtr;
	std::condition_variable *cvPtr;
};

