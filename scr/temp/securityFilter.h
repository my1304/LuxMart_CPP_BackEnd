
#include <string>

// Функция проверки безопасности
bool applySecurityPolicy(const std::string& method, const std::string& uri) {
    // Разрешаем только определенные методы и URI
    if (method == "GET" && uri.find("/cart/") == 0) {
        return true;  // Разрешаем доступ к корзине
    }
    // Пример политики: блокируем доступ к любым другим маршрутам
    return false;
}

#endif