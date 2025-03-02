cmake_minimum_required(VERSION 3.10)

# Название проекта
project(LuxMart_CPP_BackEnd)

# Укажите стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Найти OpenSSL
find_package(OpenSSL REQUIRED)
if(NOT OpenSSL_FOUND)
  message(FATAL_ERROR "OpenSSL not found!")
endif()

# Указать пути к заголовочным файлам Boost, OpenSSL и проекта
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

# Добавить PostgreSQL
find_package(PostgreSQL REQUIRED)
if(NOT PostgreSQL_FOUND)
  message(FATAL_ERROR "PostgreSQL not found!")
endif()
include_directories(${PostgreSQL_INCLUDE_DIRS})
link_directories(${PostgreSQL_LIBRARY_DIRS})

# Добавляем исполняемый файл
add_executable(LuxMart_CPP_BackEnd
    "scr/main.cpp"
    "scr/security/security.cpp"
    "scr/security/secController/authController.cpp"
    "scr/security/secConfig/jwtService.cpp"
    "scr/controllers/controllers.cpp"
    "scr/database/dbOperations.cpp"
    "scr/services/functions.cpp"
    "include/bcrypt/bcrypt.cpp"
    "include/bcrypt/blowfish.cpp"
)

# Убедитесь, что Boost установлен
find_package(Boost 1.86 REQUIRED COMPONENTS system filesystem)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found!")
endif()
# Линкуйте библиотеки Boost
target_link_libraries(LuxMart_CPP_BackEnd ${Boost_LIBRARIES})

# Линкуем PostgreSQL
target_link_libraries(LuxMart_CPP_BackEnd ${PostgreSQL_LIBRARIES})

# Линкуем OpenSSL
target_link_libraries(LuxMart_CPP_BackEnd OpenSSL::SSL OpenSSL::Crypto)

# Логи
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "PostgreSQL include dirs: ${PostgreSQL_INCLUDE_DIRS}")
message(STATUS "OpenSSL include dirs: ${OPENSSL_INCLUDE_DIR}")