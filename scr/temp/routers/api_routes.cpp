# Маршруты(endpoints) API

curl - X OPTIONS http ://localhost:8088/api/auth/login -H "Origin: http://localhost:3000" -H "Access-Control-Request-Method: POST" -H "Access-Control-Request-Headers: content-type"

curl - X POST http ://localhost:8088/api/auth/login -H "Content-Type: application/json" -H "Origin: http://localhost:3000" -d '{"email": "slav@mail.com", "password": "Qwerty!123"}'