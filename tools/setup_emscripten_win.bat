pushd %~dp0\..

call git clone https://github.com/emscripten-core/emsdk.git
call cd emsdk
call emsdk install latest
call emsdk activate latest
call emsdk install mingw-4.6.2-32bit
call emsdk activate mingw-4.6.2-32bit
call cd ..

popd
