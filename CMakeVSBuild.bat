mkdir vs-build
cd vs-build

cmake .. -G "Visual Studio 16 2019" -DBUILD_DEMO:BOOL=ON -DCMAKE_TOOLCHAIN_FILE=c:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_WIN:BOOL=ON
cmake --build .
