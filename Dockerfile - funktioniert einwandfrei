FROM rockylinux:9

# Устанавливаем зависимости
RUN dnf install -y epel-release && \
    dnf groupinstall -y "Development Tools" && \
    dnf install -y cmake gcc-c++ make wget openssl-devel boost-devel \
    postgresql-devel

#  install Boost 1.86 в Dockerfile
RUN wget https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.bz2 -O /tmp/boost_1_86_0.tar.bz2 && \
    file /tmp/boost_1_86_0.tar.bz2 && \
    tar -xjf /tmp/boost_1_86_0.tar.bz2 -C /opt && \
    cd /opt/boost_1_86_0 && \
    ./bootstrap.sh && \
    ./b2 install

# Устанавливаем OpenSSL
RUN dnf install -y openssl-devel

# Копируем исходники
WORKDIR /app
COPY . .

# Создаем папку сборки
WORKDIR /app/build

# Указываем CMake, где искать PostgreSQL
RUN rm -rf /app/build/CMakeCache.txt && \
    cmake .. -DPOSTGRESQL_INCLUDE_DIR=/usr/include/pgsql -DPOSTGRESQL_LIBRARY=/usr/lib64/pgsql/libpq.so && \
    make

# Проверяем файлы в папке сборки
RUN ls -l /app/build
# Запуск сервера (замени "luxmart_server" на свой исполняемый файл)
CMD ["./LuxMart_CPP_BackEnd"]