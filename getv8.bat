@echo off

SET repository=https://chromium.googlesource.com/v8/v8.git
SET tag=3.28.50

echo Cloning V8 repository...
git clone %repository%
pushd v8
echo Checking out tag version %tag%
git checkout tags/%tag%
popd

echo Finished.