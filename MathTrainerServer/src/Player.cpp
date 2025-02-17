#include "Player.h"

#include <iostream>
#include <utility>
//#include <print>

Player::Player(std::shared_ptr<websocket::stream<tcp::socket>> ws)
	: m_ws(ws)
{
}

Player::Player()
{

}

void Player::HandleSession()
{
	using HandlerFunc = std::function<void(json::object&)>;
	std::unordered_map<std::string, HandlerFunc> startHandlers = {
		{"login", [this](json::object& obj) { Login(obj); }},
		{"joinGame", [this](json::object& obj) { JoinGame(obj); }},
		{"createGame", [this](json::object& obj) { CreateGame(obj); }},
	};
	std::unordered_map<std::string, HandlerFunc> gameHandlers = {
		{"questionRequest", [this](json::object& obj) { SendQuestion(obj); }},
		{"playerAnswer", [this](json::object& obj) { CheckAnswer(obj); }},
	};

	for (;;)
	{
		beast::flat_buffer buffer;
		m_ws->read(buffer);
		std::string message = beast::buffers_to_string(buffer.data());

		auto parsedMessage = ParseJson(message);
		if (!parsedMessage)
			continue;

		auto& messageObj = parsedMessage.value();

		std::string type = try_value_to<std::string>(messageObj["type"]).value();
		auto handler = startHandlers.find(type);
		if (handler != startHandlers.end())
			handler->second(messageObj);
		
		handler = gameHandlers.find(type);
		if (handler != gameHandlers.end() && m_game)
			handler->second(messageObj);
	}
}

std::optional<json::object> Player::ParseJson(const std::string& message)
{
	boost::system::error_code ec;
	json::value jv = json::parse(message, ec);
	if (ec)
	{
		std::cerr << "Error parsing JSON: " << ec.message() << '\n';
		return std::nullopt;
	}

	if (!jv.is_object())
	{
		std::cerr << "Parsed JSON is not an object.\n";
		return std::nullopt;
	}

	return jv.as_object();
}

bool Player::IsInGame()
{
	return m_game == nullptr ? false : true;
}

std::string Player::GetGameID()
{
	return m_game->GetID();
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
		m_ws->write(asio::buffer(json::serialize(msg)));
		//std::print("Player: {} tried to join a game that doesn't exist", m_name);
		std::cout << "Player: " << m_name << " tried to join a game that doesn't exist\n";
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
		
		SendLeaderboard();
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
	auto leaderboard = m_game->GetLeaderboardMessage();
	m_game->MessageAll(leaderboard);
}
