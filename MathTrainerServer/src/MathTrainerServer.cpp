#include "Asio.h"

#include <iostream>
#include <unordered_map>
#include <memory>
#include <thread>
#include <mutex>
#include <vector>

#include "Game.h"
#include "Player.h"

std::unordered_map<std::shared_ptr<websocket::stream<tcp::socket>>, std::shared_ptr<Player>> players;
std::vector<Game> games;
std::mutex players_mutex;

void handle_session(tcp::socket socket)
{
	auto ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
	try
	{
		ws->accept();

		{
			std::scoped_lock<std::mutex> lock(players_mutex);
			players[ws] = std::make_shared<Player>(ws);
		}

		players[ws]->HandleSession();
	}
	catch (const std::exception& e)
	{
		std::cerr << "WebSocket error: " << e.what() << std::endl;
	}

	{
		std::scoped_lock<std::mutex> lock(players_mutex);
		players.erase(ws);
	}
}

int main()
{
	try
	{
		asio::io_context ioc;
		tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 3000));

		std::cout << "WebSocket server listening on port 3000..." << std::endl;

		for (;;)
		{
			tcp::socket socket(ioc);
			acceptor.accept(socket);
			std::cout << "New websocket accpeted" << std::endl;
			std::thread(handle_session, std::move(socket)).detach();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Server error: " << e.what() << std::endl;
	}
	return 0;
}
