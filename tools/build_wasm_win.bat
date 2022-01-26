pushd %~dp0\..

call emsdk\emsdk_env.bat
call emcmake cmake -B build_wasm -G "Unix Makefiles" -DEMSCRIPTEN=1 -DCMAKE_MAKE_PROGRAM=mingw32-make -DCMAKE_BUILD_TYPE=%1 . || goto :error
call emmake mingw32-make -C build_wasm || goto :error
popd
echo Build Succeeded.

popd
exit /b 0

:error
echo Build Failed with Error %errorlevel%.
popd
popd
exit /b 1
