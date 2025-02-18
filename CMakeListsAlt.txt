
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("LuxMart_CPP_BackEnd")

# Найти Boost
find_package(Boost 1.86 REQUIRED COMPONENTS system filesystem)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found!")
endif()

# Найти OpenSSL
find_package(OpenSSL REQUIRED)
if(NOT OpenSSL_FOUND)
  message(FATAL_ERROR "OpenSSL not found!")
endif()

# Указываем пути к заголовочным файлам Boost и OpenSSL
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

# Указываем путь к библиотекам и заголовочным файлам PostgreSQL
include_directories("C:/Program Files/PostgreSQL/16/include")
link_directories("C:/Program Files/PostgreSQL/16/lib")

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

# Линкуем Boost библиотеки
target_link_libraries(LuxMart_CPP_BackEnd ${Boost_LIBRARIES})

# Линкуем библиотеку PostgreSQL (libpq)
target_link_libraries(LuxMart_CPP_BackEnd libpq)

# Линкуем библиотеку OpenSSL
target_link_libraries(LuxMart_CPP_BackEnd OpenSSL::SSL OpenSSL::Crypto)

# Указываем стандарт C++
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LuxMart_CPP_BackEnd PROPERTY CXX_STANDARD 20)
endif()