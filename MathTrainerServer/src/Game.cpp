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
	m_players[ws].problemIdx = 0;
	m_players[ws].start = std::chrono::steady_clock::now();
	if (name == "anonymous")
		m_players[ws].name = std::format("anonymous ({})", std::to_string(m_anonID++));
	else
		m_players[ws].name = name;
}

void Game::QuitGame(std::shared_ptr<websocket::stream<tcp::socket>> ws)
{
	{
		std::scoped_lock<std::mutex> lock(m_mutex);

		if (m_players.contains(ws))
		{
			std::cout << "Player " << m_players[ws].name << ": left the game\n";
			m_players.erase(ws);
		}
	}
	
	auto leaderboard = GetLeaderboardMessage();
	MessageAll(leaderboard);
}

std::string Game::GetPlayerName(std::shared_ptr<websocket::stream<tcp::socket>> ws) const
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	auto player = m_players.find(ws);
	assert(player != m_playerNames.end());

	return player->second.name;
}

bool Game::SubmitAnswer(std::shared_ptr<websocket::stream<tcp::socket>> ws, std::string& answer)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	// TODO: Generate new questions or something
	auto player = m_players.find(ws);
	assert(player != m_players.end());

	auto& playerData = player->second;
	if (answer == m_problems[playerData.problemIdx].answer)
	{
		playerData.problemIdx++;
		auto now = std::chrono::steady_clock::now();
		playerData.playTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - playerData.start);
		if (playerData.problemIdx >= m_problems.size())
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

	return m_problems[player->second.problemIdx].question;
}

json::object Game::GetLeaderboardMessage() const
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	json::object leaderboard;
	leaderboard["type"] = "leaderboard";

	std::vector<std::pair<uint32_t, PlayerData>> sortedPlayers;
	sortedPlayers.reserve(m_players.size());
	for (auto& [ws, player] : m_players)
	{
		sortedPlayers.emplace_back(player.problemIdx, player);
	}
	std::sort(sortedPlayers.rbegin(), sortedPlayers.rend(), [](const std::pair<uint32_t, PlayerData>& a, const std::pair<uint32_t, PlayerData>& b) {
		return a.first < b.first ? true : false;
	});

	json::array arr;
	arr.reserve(m_players.size());
	for (auto& [score, player] : sortedPlayers)
	{
		json::object entry;
		entry["name"] = player.name;
		entry["score"] = score;
		entry["playTime"] = player.playTime.count();
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
	for (auto& [ws, player] : m_players)
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
