#pragma once
#define BOOST_ASIO_SEPARATE_COMPILATION

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace json = boost::json;
namespace asio = boost::asio;
namespace beast = boost::beast;