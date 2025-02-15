#include "Player.h"

#include <iostream>
#include <utility>

Player::Player(std::shared_ptr<websocket::stream<tcp::socket>> ws)
	: m_ws(ws)
{
}

Player::Player()
{

}

void Player::HandleSession()
{
	for (;;)
	{
		beast::flat_buffer buffer;
		m_ws->read(buffer);
		std::string message = beast::buffers_to_string(buffer.data());

		boost::system::error_code ec;
		json::value jv = json::parse(message, ec);
		if (ec)
		{
			std::cerr << "Error parsing JSON: " << ec.message() << '\n';
			continue;
		}

		if (!jv.is_object())
		{
			std::cerr << "Parsed JSON is not an object.\n";
			continue;
		}

		json::object obj = jv.as_object();
		if (!obj.contains("type"))
		{
			std::cerr << "Received message does't have type specified.\n";
			continue;
		}

		if (obj["type"] == "login")
		{
			try
			{
				Login(obj);
			}
			catch (std::exception ex)
			{
				std::cerr << "Error logging in: " << ex.what() << '\n';
			}
		}
		else if (obj["type"] == "joinGame")
		{
			try
			{
				JoinGame(obj);
			}
			catch (std::exception ex)
			{
				std::cerr << "Error joining game: " << ex.what() << '\n';
			}
		}
		else if (obj["type"] == "createGame")
		{
			try
			{	
				CreateGame(obj);
			}
			catch (std::exception ex)
			{
				std::cerr << "Error creating game: " << ex.what() << '\n';
			}
		}
		else if (obj["type"] == "questionRequest")
		{
			try
			{
				SendQuestion(obj);
			}
			catch (std::exception ex)
			{
				std::cerr << "Error sending question: " << ex.what() << '\n';
			}
		}
		else if (obj["type"] == "playerAnswer")
		{
			try
			{
				CheckAnswer(obj);
			}
			catch (std::exception ex)
			{
				std::cerr << "Error checking player answer: " << ex.what() << '\n';
			}
		}
		else
		{
			std::cerr << "Message contains invalid type\n";
		}
	}
}

void Player::Login(boost::json::object& obj)
{
	m_name = try_value_to<std::string>(obj["username"]).value();
	std::cout << m_name << " has logged in.\n";
}

void Player::JoinGame(boost::json::object& obj)
{
	auto& gameManager = GameManager::GetInstance();

	std::string gameID = try_value_to<std::string>(obj["gameID"]).value();

	json::object msg;
	if (!gameManager.DoesGameExist(gameID))
	{
		msg["type"] = "error";
		msg["message"] = "Game id doesn't exist";
		m_ws->write(asio::buffer("The game you tried to join doesn't exist."));
		return;
	}
	m_game = gameManager.JoinGame(gameID);
	m_game->JoinGame(m_ws, m_name);
	SendJoinGameMessage();
	std::cout << m_name << " has joined the game.\n";

	SendLeaderboard();
}

void Player::CreateGame(boost::json::object& obj)
{
	auto& gameManager = GameManager::GetInstance();

	std::string gameID = json::try_value_to<std::string>(obj["gameID"]).value();

	json::object msg;
	if (gameManager.DoesGameExist(gameID))
	{
		msg["type"] = "error";
		msg["message"] = "Game id is already taken";
		m_ws->write(asio::buffer(json::serialize(msg)));
		return;
	}

	GameData gameData = {
		.minAddition = json::try_value_to<int>(obj["minAddition"]).value(),
		.maxAddition = json::try_value_to<int>(obj["maxAddition"]).value(),
		.minMultiplication = json::try_value_to<int>(obj["minMultiplication"]).value(),
		.maxMultiplication = json::try_value_to<int>(obj["maxMultiplication"]).value(),
	};

	m_game = gameManager.CreateGame(gameID, gameData);
	msg["type"] = "gameCreated";
	m_ws->write(asio::buffer(json::serialize(msg)));
	std::cout << m_name << " has created a game.\n";

	m_game->JoinGame(m_ws, m_name);
	SendJoinGameMessage();
	std::cout << m_name << " has joined a game.\n";

	SendLeaderboard();
}

void Player::SendQuestion(boost::json::object& obj)
{
	json::object msg;
	msg["type"] = "question";
	msg["question"] = m_game->GetQuestion(m_ws);
	m_ws->write(asio::buffer(json::serialize(msg)));
}

void Player::CheckAnswer(boost::json::object& obj)
{
	std::string answer = try_value_to<std::string>(obj["answer"]).value();
	if (m_game->SubmitAnswer(m_ws, answer))
	{
		json::object msg;
		msg["type"] = "answerWasCorrect";
		m_ws->write(asio::buffer(json::serialize(msg)));
		
		auto leaderboard = m_game->GetLeaderboardMessage();
		m_game->MessageAll(leaderboard);
	}
}

void Player::SendJoinGameMessage()
{
	json::object msg;
	msg["type"] = "joinedGame";
	m_ws->write(asio::buffer(json::serialize(msg)));
}

void Player::SendLeaderboard()
{
	json::object msg;
	m_ws->write(asio::buffer(json::serialize(m_game->GetLeaderboardMessage())));
}
