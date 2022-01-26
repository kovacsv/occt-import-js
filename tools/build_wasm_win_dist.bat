pushd %~dp0\..

call tools\build_wasm_win_release.bat || goto :error
echo Build Succeeded.

call set TEST_CONFIG=Release
call npm run test || goto :error

mkdir dist
copy build_wasm\Release\occt-import-js.js dist\occt-import-js.js || goto :error
copy build_wasm\Release\occt-import-js.wasm dist\occt-import-js.wasm || goto :error
copy assimp\LICENSE dist\license.assimp.txt || goto :error
copy LICENSE.md dist\license.occt-import-js.txt || goto :error
xcopy dist\*.* docs\dist\*.* /K /D /H /Y

popd
echo Distribution Succeeded.

exit /b 0

:error
echo Distribution Failed with Error %errorlevel%.
popd
popd
exit /b 1
