if not exists build mkdir build
cd build
cmake -S ../ -B . -G "MinGW Makefiles"
mingw32-make.exe && mingw32-make.exe Shders
cd ..