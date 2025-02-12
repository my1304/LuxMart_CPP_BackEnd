cd /share/backend/LuxMart_CPP_BackEnd
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

cmake --build build --clean-first