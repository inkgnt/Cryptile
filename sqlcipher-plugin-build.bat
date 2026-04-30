@echo off
SET BUILD_TYPE=%1
IF "%BUILD_TYPE%"=="" SET BUILD_TYPE=Release

SET INSTALL_PREFIX=%2
IF "%INSTALL_PREFIX%"=="" SET INSTALL_PREFIX=..\install

SET QT_DIR=%3
IF "%QT_DIR%"=="" SET QT_DIR=C:\Qt\6.7.3\msvc2022_64\lib\cmake\Qt6

REM -------------------------------------------------------
REM Директория сборки плагина
REM -------------------------------------------------------
SET BUILD_DIR=%~dp0/sqlcipher-plugin/build
IF NOT EXIST "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cd /d "%BUILD_DIR%"
CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM -------------------------------------------------------
REM Конфигурация
REM -------------------------------------------------------
cmake .. -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%" ^
    -DQt6_DIR="%QT_DIR%" ^
	-DCMAKE_PREFIX_PATH="C:/Qt/6.7.3/msvc2022_64/lib/cmake"

IF ERRORLEVEL 1 (
    echo [ERROR] CMake configure failed
    exit /b 1
)

REM -------------------------------------------------------
REM Сборка
REM -------------------------------------------------------
cmake --build . --config %BUILD_TYPE%
IF ERRORLEVEL 1 (
    echo [ERROR] Build failed
    exit /b 1
)

REM -------------------------------------------------------
REM Установка
REM -------------------------------------------------------
cmake --install . --config %BUILD_TYPE%
IF ERRORLEVEL 1 (
    echo [ERROR] Install failed
    exit /b 1
)

echo [INFO] Plugin build and install finished successfully
pause
