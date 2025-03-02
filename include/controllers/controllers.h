#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include <boost/asio.hpp>

namespace http = boost::beast::http;
using json = nlohmann::json;

void controllers(const json& parsed_body, http::response<http::string_body>& res, const http::request<http::string_body>& req);

#endif // CONTROLLERS_H