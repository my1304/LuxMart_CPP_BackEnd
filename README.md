# Описание проекта
cmake -G "Visual Studio 17 2022" #Выполнить эту команду для настройки CMake
cmake --build

curl -v http://localhost:8080/

** JSON Web Tokens (JWT) и криптографическими функциями






#add_executable (LuxMart_CPP_BackEnd "scr/LuxMart_CPP_BackEnd.cpp" "include/LuxMart_CPP_BackEnd.h")
#add_executable(LuxMart_CPP_BackEnd "scr/LuxMart_CPP_BackEnd.cpp" "scr/controllers/cartController.cpp" "scr/security/secConfig/webConfig.cpp")

CMD: curl -X POST http://localhost:8088/api/auth/login -H "Content-Type: application/json" -d "{\"email\": \"slav@mail.com\", \"password\": \"Qwerty!123\"}"
	 curl -X GET http://localhost:8088/api/cart/1 -H "Content-Type: application/json" 


curl -X OPTIONS http://localhost:8088/api/auth/login -H "Origin: http://localhost:3000" -H "Access-Control-Request-Method: POST" -H "Access-Control-Request-Headers: content-type"

curl -X POST http://localhost:8088/api/auth/login -H "Content-Type: application/json" -H "Origin: http://localhost:3000" -d "{\"email\":\"slav@mail.com\",\"password\":\"Qwerty!123\"}"

const result = await dispatch(loginUser({ "slav@mail.com", "Qwerty!123" }));