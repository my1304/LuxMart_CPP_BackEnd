#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp> // ���������� ����������

// ��� �������� ���������� ���������
using json = nlohmann::json;

int main() {
    // ��������� ���� config.json
    std::ifstream config_file("config.json");
    if (!config_file.is_open()) {
        std::cerr << "������: �� ������� ������� config.json" << std::endl;
        return 1;
    }

    // ������ JSON �� �����
    json config;
    try {
        config_file >> config; // ��������� ������ �� ����� � ������ json
    }
    catch (const std::exception& e) {
        std::cerr << "������ ��� �������� JSON: " << e.what() << std::endl;
        return 1;
    }

    // ��������� ������
    int server_port = config["server"]["port"];
    std::string server_host = config["server"]["host"];

    std::string db_host = config["database"]["host"];
    int db_port = config["database"]["port"];
    std::string db_username = config["database"]["username"];
    std::string db_password = config["database"]["password"];

    // ������� ������
    std::cout << "Server config:\n"
        << "  Host: " << server_host << "\n"
        << "  Port: " << server_port << "\n";

    std::cout << "Database config:\n"
        << "  Host: " << db_host << "\n"
        << "  Port: " << db_port << "\n"
        << "  Username: " << db_username << "\n"
        << "  Password: " << db_password << "\n";

    return 0;
}