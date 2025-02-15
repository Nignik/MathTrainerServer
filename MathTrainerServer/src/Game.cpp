#include "Game.h"

Game::Game(GameData gameData)
	: m_problemGenerator{gameData.minAddition, gameData.maxAddition, gameData.minMultiplication, gameData.maxMultiplication}
{	
	m_problems.reserve(100);
	for (int i = 0; i < 100; i++)
	{
		m_problems.push_back(m_problemGenerator.GenerateProblem());
	}
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

bool Game::SubmitAnswer(std::shared_ptr<websocket::stream<tcp::socket>> ws, std::string& answer)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	// TODO: Generate new questions or something
	if (answer == m_problems[m_players[ws]].answer)
	{
		m_players[ws]++;
		if (m_players[ws] >= 100)
			m_players[ws] = 99;
		return true;
	}
	
	return false;
}

std::string Game::GetQuestion(std::shared_ptr<websocket::stream<tcp::socket>> ws)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	return m_problems[m_players[ws]].question;
}

json::object Game::GetLeaderboardMessage()
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	json::object leaderboard;
	leaderboard["type"] = "leaderboard";

	std::vector<std::pair<int, std::string>> sortedPlayers;
	sortedPlayers.reserve(m_players.size());
	for (auto& [key, val] : m_players)
	{
		sortedPlayers.emplace_back(val, m_playerNames[key]);
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

std::string Game::GetPlayerName(std::shared_ptr<websocket::stream<tcp::socket>> ws)
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	return m_playerNames[ws];
}

void Game::MessageAll(json::object& obj)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	std::string jsonStr = json::serialize(obj);
	auto msg = asio::buffer(jsonStr);
	for (auto& [ws, score] : m_players)
	{
		ws->write(msg);
	}
}
