:: Get V8 and tools to build it
::
@SETLOCAL
@ECHO off

:: V8 version to checkout
SET tag=3.27.34.21

:: drop custom python vars if any
SET PYTHONHOME=
SET PYTHONPATH=
SET PYTHON=

PATH %PATH%;depot_tools

ECHO updating gclient
CALL gclient

ECHO fetching v8
CALL fetch v8

ECHO checking out tag: %tag%
pushd v8
CALL git checkout %tag%
popd

ECHO now you can build v8

ENDLOCAL
