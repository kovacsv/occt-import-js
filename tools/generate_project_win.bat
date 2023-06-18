pushd %~dp0\..

call cmake -B build -G "Visual Studio 17 2022" .

popd
