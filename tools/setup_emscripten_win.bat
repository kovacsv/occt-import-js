pushd %~dp0\..

call git clone https://github.com/emscripten-core/emsdk.git
call cd emsdk
call emsdk install 3.1.58
call emsdk activate 3.1.58
call emsdk install mingw-7.1.0-64bit
call emsdk activate mingw-7.1.0-64bit
call cd ..

popd
