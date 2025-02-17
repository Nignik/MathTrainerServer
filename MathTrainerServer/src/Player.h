#pragma once

#include "Asio.h"
#include <memory>
#include <vector>

#include "Game.h"
#include "GameManager.h"

class Player
{
public:
	Player();
	Player(std::shared_ptr<websocket::stream<tcp::socket>> ws);

	void HandleSession();
	std::optional<json::object> ParseJson(const std::string& message);
	bool IsInGame();
	std::string GetGameID();

private:
	void Login(boost::json::object& obj);
	void JoinGame(boost::json::object& obj);
	void CreateGame(boost::json::object& obj);
	void SendQuestion(boost::json::object& obj);
	void CheckAnswer(boost::json::object& obj);

	void SendJoinGameMessage();
	void SendLeaderboard();

	std::shared_ptr<websocket::stream<tcp::socket>> m_ws{};
	std::shared_ptr<Game> m_game{};
	std::string m_name = "anonymous";
};