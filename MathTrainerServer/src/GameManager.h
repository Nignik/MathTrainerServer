#pragma once

#include "Game.h"
#include <unordered_map>
#include <string>
#include <iostream>

class GameManager
{
public:
	static GameManager& GetInstance()
	{
		static GameManager instance;
		return instance;
	}

	[[nodiscard]] std::shared_ptr<Game> CreateGame(std::string& gameID, GameData gameData)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);
		
		if (m_games.contains(gameID))
		{
			std::cout << "Player requested to create a game with an already taken id." << std::endl;
			return nullptr;
		}

		m_games[gameID] = std::make_shared<Game>(gameID, gameData);
		return m_games[gameID];
	}

	[[nodiscard]] std::shared_ptr<Game> JoinGame(std::string& gameID)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);

		if (!m_games.contains(gameID))
		{
			std::cout << "Player requested to join a game that doesn't exist." << std::endl;
			return nullptr;
		}

		return m_games[gameID];
	}

	void QuitGame(std::string& gameID, std::shared_ptr<websocket::stream<tcp::socket>> ws)
	{
		if (!m_games.contains(gameID))
			return;

		m_games[gameID]->QuitGame(ws);
		if (m_games[gameID]->GetNumberOfPlayers() == 0)
		{
			m_games.erase(gameID);
			std::cout << "Game: " << gameID << "has been closed\n";
		}
	}

	bool DoesGameExist(std::string& gameID) const
	{
		return m_games.contains(gameID);
	}

	GameManager(const GameManager&) = delete;
	GameManager& operator=(const GameManager&) = delete;

private:
	GameManager() = default;

	std::mutex m_mutex{};
	std::unordered_map<std::string, std::shared_ptr<Game>> m_games{};
};