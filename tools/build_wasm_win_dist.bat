pushd %~dp0\..

call tools\build_wasm_win_release.bat || goto :error
echo Build Succeeded.

call npm run test || goto :error

mkdir dist
copy build\wasm\Release\occt-import-js.js dist\occt-import-js.js || goto :error
copy build\wasm\Release\occt-import-js.wasm dist\occt-import-js.wasm || goto :error
copy occt\LICENSE_LGPL_21.txt dist\license.occt.txt || goto :error
copy LICENSE.md dist\license.occt-import-js.txt || goto :error

popd
echo Distribution Succeeded.

exit /b 0

:error
echo Distribution Failed with Error %errorlevel%.
popd
exit /b 1
