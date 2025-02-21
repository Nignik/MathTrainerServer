#pragma once

#include "Asio.h"
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <cstdint>
#include <chrono>

#include "Problem.h"
#include "ProblemGenerator.h"

struct GameData
{
	int minAddition = 1, maxAddition = 10;
	int minMultiplication = 1, maxMultiplication = 10;
};

struct PlayerData
{
	uint32_t problemIdx{};
	std::string name{};
	std::chrono::time_point<std::chrono::steady_clock> start{};
	std::chrono::duration<float> playTime{};
};

class Game
{
public:
	Game(std::string& gameID, GameData gameData);

	void JoinGame(std::shared_ptr<websocket::stream<tcp::socket>> ws, std::string& name);
	void QuitGame(std::shared_ptr<websocket::stream<tcp::socket>> ws);
	[[nodiscard]] bool SubmitAnswer(std::shared_ptr<websocket::stream<tcp::socket>> ws, std::string& answer); // returns true if the answer was correct

	std::string GetPlayerName(std::shared_ptr<websocket::stream<tcp::socket>> ws) const;
	std::string GetQuestion(std::shared_ptr<websocket::stream<tcp::socket>> ws) const;
	json::object GetLeaderboardMessage() const;
	uint32_t GetNumberOfPlayers() const;
	std::string GetID() const;

	void MessageAll(json::object& obj) const;

private:
	void AppendProblems(uint32_t amount);

	mutable std::mutex m_mutex{};
	std::string m_ID{};
	ProblemGenerator m_problemGenerator{1, 10, 1, 10};
	std::vector<Problem> m_problems{};
	std::unordered_map<std::shared_ptr<websocket::stream<tcp::socket>>, PlayerData> m_players{}; // maps players websocket to problem index
	uint32_t m_anonID = 0;
};