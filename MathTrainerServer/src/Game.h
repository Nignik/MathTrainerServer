#pragma once

#include "Asio.h"
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <queue>

#include "Problem.h"
#include "ProblemGenerator.h"

struct GameData
{
	int minAddition = 1, maxAddition = 10;
	int minMultiplication = 1, maxMultiplication = 10;
};

class Game
{
public:
	Game(GameData gameData);

	void JoinGame(std::shared_ptr<websocket::stream<tcp::socket>> ws, std::string& name);
	[[nodiscard]] bool SubmitAnswer(std::shared_ptr<websocket::stream<tcp::socket>> ws, std::string& answer); // returns true if the answer was correct
	std::string GetQuestion(std::shared_ptr<websocket::stream<tcp::socket>> ws);
	json::object GetLeaderboardMessage();
	std::string GetPlayerName(std::shared_ptr<websocket::stream<tcp::socket>> ws);
	void MessageAll(json::object& obj);

private:
	mutable std::mutex m_mutex{};
	ProblemGenerator m_problemGenerator{1, 10, 1, 10};
	std::vector<Problem> m_problems{};
	std::unordered_map<std::shared_ptr<websocket::stream<tcp::socket>>, int> m_players; // maps players websocket to problem index
	std::unordered_map<std::shared_ptr<websocket::stream<tcp::socket>>, std::string> m_playerNames;
	int m_anonID = 0;
};