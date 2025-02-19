#include "Game.h"

#include <iostream>

Game::Game(std::string& gameID, GameData gameData)
	: m_ID(gameID),
	m_problemGenerator(gameData.minAddition, gameData.maxAddition, gameData.minMultiplication, gameData.maxMultiplication)
{	
	AppendProblems(100);
}

void Game::JoinGame(std::shared_ptr<websocket::stream<tcp::socket>> ws, std::string& name)
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	m_players[ws] = 0;
	if (name == "anonymous")
		m_playerNames[ws] = std::format("anonymous ({})", std::to_string(m_anonID++));
	else
		m_playerNames[ws] = name;
}

void Game::QuitGame(std::shared_ptr<websocket::stream<tcp::socket>> ws)
{
	{
		std::scoped_lock<std::mutex> lock(m_mutex);

		if (m_players.contains(ws))
			m_players.erase(ws);

		std::cout << "Player " << m_playerNames[ws] << ": left the game\n";
		if (m_playerNames.contains(ws))
			m_playerNames.erase(ws);
	}
	
	auto leaderboard = GetLeaderboardMessage();
	MessageAll(leaderboard);
}

std::string Game::GetPlayerName(std::shared_ptr<websocket::stream<tcp::socket>> ws) const
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	auto player = m_playerNames.find(ws);
	assert(player != m_playerNames.end());
	return player->second;
}

bool Game::SubmitAnswer(std::shared_ptr<websocket::stream<tcp::socket>> ws, std::string& answer)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	// TODO: Generate new questions or something
	auto player = m_players.find(ws);
	assert(player != m_players.end());

	if (answer == m_problems[player->second].answer)
	{
		player->second++;
		if (player->second >= m_problems.size())
		{
			AppendProblems(100);
		}
			
		return true;
	}
	
	return false;
}

std::string Game::GetQuestion(std::shared_ptr<websocket::stream<tcp::socket>> ws) const
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	auto player = m_players.find(ws);
	assert(player != m_players.end());

	return m_problems[player->second].question;
}

json::object Game::GetLeaderboardMessage() const
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	json::object leaderboard;
	leaderboard["type"] = "leaderboard";

	std::vector<std::pair<int, std::string>> sortedPlayers;
	sortedPlayers.reserve(m_players.size());
	for (auto& [key, val] : m_players)
	{
		sortedPlayers.emplace_back(val, m_playerNames.find(key)->second);
	}
	std::sort(sortedPlayers.rbegin(), sortedPlayers.rend());

	json::array arr;
	arr.reserve(m_players.size());
	for (auto& [score, name] : sortedPlayers)
	{
		json::object entry;
		entry["name"] = name;
		entry["score"] = score;
		arr.push_back(std::move(entry));
	}

	leaderboard["leaderboard"] = std::move(arr);

	return leaderboard;
}

uint32_t Game::GetNumberOfPlayers() const
{
	return m_players.size();
}

std::string Game::GetID() const
{
	return m_ID;
}

void Game::MessageAll(json::object& obj) const
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	std::string jsonStr = json::serialize(obj);
	auto msg = asio::buffer(jsonStr);
	for (auto& [ws, score] : m_players)
	{
		ws->write(msg);
	}
}

void Game::AppendProblems(uint32_t amount)
{
	m_problems.reserve(m_problems.size() + amount);
	for (int i = 0; i < 100; i++)
	{
		m_problems.push_back(m_problemGenerator.GenerateProblem());
	}
}
