#pragma once
#define BOOST_ASIO_SEPARATE_COMPILATION
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace json = boost::json;
namespace asio = boost::asio;
namespace beast = boost::beast;