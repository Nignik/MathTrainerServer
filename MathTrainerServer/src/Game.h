#pragma once

#include "Asio.h"
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <cstdint>

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
	void QuitGame(std::shared_ptr<websocket::stream<tcp::socket>> ws);
	[[nodiscard]] bool SubmitAnswer(std::shared_ptr<websocket::stream<tcp::socket>> ws, std::string& answer); // returns true if the answer was correct
	std::string GetQuestion(std::shared_ptr<websocket::stream<tcp::socket>> ws) const;
	json::object GetLeaderboardMessage() const;
	std::string GetPlayerName(std::shared_ptr<websocket::stream<tcp::socket>> ws) const;
	std::string GetID() const;
	int GetNumberOfPlayers() const;
	void MessageAll(json::object& obj) const;

private:
	void AppendProblems(uint32_t amount);

	mutable std::mutex m_mutex{};
	std::string m_ID{};
	ProblemGenerator m_problemGenerator{1, 10, 1, 10};
	std::vector<Problem> m_problems{};
	std::unordered_map<std::shared_ptr<websocket::stream<tcp::socket>>, int> m_players{}; // maps players websocket to problem index
	std::unordered_map<std::shared_ptr<websocket::stream<tcp::socket>>, std::string> m_playerNames{};
	int m_anonID = 0;
};