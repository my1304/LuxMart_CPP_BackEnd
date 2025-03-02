#ifndef JWT_CONFIG_H
#define JWT_CONFIG_H

#include <string>

// ��������� ����� ��� ������� JWT
const std::string ACCESS_TOKEN_SECRET = "X30INcpL2Yzf80ArITEdwLegdAFT2dkeq1hxlR7waOk =";
const std::string REFRESH_TOKEN_SECRET = "u1vxX56P6a + 4tWS2RLt0qIG9yF8DbW6d1pv4JUQI + dk =";

// ����� ����� ������� � ��������
const int ACCESS_TOKEN_EXPIRATION = 3600;  // 1 ���
const int REFRESH_TOKEN_EXPIRATION = 604800;  // 7 ����

#endif // JWT_CONFIG_H