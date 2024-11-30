pushd %~dp0\..

call cmake -B build -G "Visual Studio 17 2022" . || goto :error
call cmake --build build --config RelWithDebInfo || goto :error

popd
exit /b 0

:error
echo Build Failed with Error %errorlevel%.
popd
exit /b 1
