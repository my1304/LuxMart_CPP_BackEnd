
#include <string>

// ������� �������� ������������
bool applySecurityPolicy(const std::string& method, const std::string& uri) {
    // ��������� ������ ������������ ������ � URI
    if (method == "GET" && uri.find("/cart/") == 0) {
        return true;  // ��������� ������ � �������
    }
    // ������ ��������: ��������� ������ � ����� ������ ���������
    return false;
}

#endif