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

	bool IsInGame() const;
	std::string GetGameID() const;
	std::string GetName() const;

private:
	void Login(boost::json::object& obj); // Gives player a username
	void CreateGame(boost::json::object& obj);
	void JoinGame(boost::json::object& obj);
	void LeaveGame();

	void SendQuestion(boost::json::object& obj);
	void SendLeaderboard();
	void NotifyAnswerCorrect();
	void NotifyJoinedGame();

	void CheckAnswer(boost::json::object& obj);

	std::shared_ptr<websocket::stream<tcp::socket>> m_ws{};
	std::shared_ptr<Game> m_game{};
	std::string m_name = "anonymous";
};