@echo off
SET VENV_PATH=%cd%\venv
SET TARGET_DIR=%cd%
:: Set this path to where our script lives

REM Check if the directory exists
IF NOT EXIST "%VENV_PATH%" (
    echo Directory %VENV_PATH% does not exist.
    echo Creating directory %VENV_PATH%...
    mkdir "%VENV_PATH%"
    
    echo Creating virtual environment in %VENV_PATH%...
    python -m venv "%VENV_PATH%"
    
    echo Virtual environment created.
) ELSE (
    echo Directory %VENV_PATH% already exists.
)

echo Activating virtual environment...
call "%VENV_PATH%\Scripts\activate.bat"


REM Check if pyserial is installed
pip show pyserial >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo pyserial is not installed.
    echo Installing pyserial...
    pip install pyserial
    echo pyserial installed.
) ELSE (
    echo pyserial is already installed.
)

call "%VENV_PATH%\Scripts\deactivate.bat"

start cmd.exe /k "TITLE venv & %VENV_PATH%\Scripts\activate.bat & cd /d %TARGET_DIR% & echo Virtual environment activated."

echo Virtual environment activated.