#ifndef SECURITY_H
#define SECURITY_H

#include <boost/asio.hpp>
#include <string>
#include <set>  // std::set

class CorsConfig {
public:
    bool handleCors(boost::asio::ip::tcp::socket& socket, const std::string& uri);
};

#endif // SECURITY_H