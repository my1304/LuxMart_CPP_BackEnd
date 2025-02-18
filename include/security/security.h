#ifndef SECURITY_H
#define SECURITY_H

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

json readConfig();
void closeSocket(boost::asio::ip::tcp::socket& socket);
void do_session(boost::asio::ip::tcp::socket socket);

#endif // SECURITY_H