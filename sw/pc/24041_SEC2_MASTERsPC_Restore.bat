@ECHO OFF
CLS

REM Set a unique title for this command window
TITLE RestoreCommandWindow

ECHO ========== Closing Open Programs ============
ECHO.
TASKLIST | FIND "mplab_ide64.exe" > NUL
IF %errorlevel% == 0 (
    ECHO Closing MPLABX IDE...
    TASKKILL /F /IM mplab_ide64.exe
)
TASKLIST | FIND "Acrobat.exe" > NUL
IF %errorlevel% == 0 (
    ECHO Closing Adobe...
    TASKKILL /F /IM Acrobat.exe
)
TASKLIST | FIND "ttermpro.exe" > NUL
IF %errorlevel% == 0 (
    ECHO Closing Tera Term...
    TASKKILL /F /IM ttermpro.exe
)
FOR /F "tokens=2 delims=," %%i IN ('TASKLIST /FI "WINDOWTITLE eq venv" /FO CSV /NH') DO (
    ECHO Closing venv...
    TASKKILL /PID %%i /F
)

TIMEOUT /T 5 /NOBREAK > NUL

SETLOCAL ENABLEDELAYEDEXPANSION

REM Get the PID of the current command window based on the title
FOR /F "tokens=2 delims=," %%i IN ('TASKLIST /FI "WINDOWTITLE eq RestoreCommandWindow" /FO CSV /NH') DO (
    SET currentPID=%%i
)
ECHO Current command window PID is !currentPID!

REM PowerShell command to close all windows except the current one and exclude critical processes
ECHO About to execute PowerShell command
POWERSHELL -NoProfile -Command "$currentPID = !currentPID!; Write-Host 'Current PID:' $currentPID; $processes = Get-Process | Where-Object { $_.MainWindowHandle -ne 0 -and $_.Id -ne [int]$currentPID -and $_.ProcessName -ne 'winlogon' -and $_.ProcessName -ne 'taskhostw' -and $_.ProcessName -ne 'ApplicationFrameHost' -and $_.ProcessName -ne 'ShellExperienceHost' -and $_.ProcessName -ne 'SearchUI' }; foreach ($process in $processes) { Write-Host 'Found process:' $process.Id $process.ProcessName; if ($process.ProcessName -eq 'chrome' -or $process.ProcessName -eq 'explorer') { $process.Kill() | Out-Null } else { $process.CloseMainWindow() | Out-Null }; Start-Sleep -Seconds 1; }; Write-Host 'PowerShell command executed successfully';"
ECHO PowerShell command executed

ECHO.
ECHO All windows on the taskbar except the current one have been processed.
ECHO.

ECHO ========== Removing 24041_SEC2 working directory contents ============
ECHO.
RMDIR /S /Q "C:\MASTERs\24041_SEC2"
ECHO.

ECHO ========== Creating 24041_SEC2 working directory ============
ECHO.
MKDIR "C:\MASTERs\24041_SEC2"
ECHO.

ECHO ========== Finding the first ZIP file in the directory ============
ECHO.

REM Enable Delayed Expansion
SETLOCAL ENABLEDELAYEDEXPANSION

REM Set the directory to search for ZIP files
SET "searchDir=C:\Backup\24041_SEC2"

REM Initialize the variable to hold the first ZIP file found
SET "zipFile="

REM Loop through the files in the directory and find the first ZIP file
FOR %%F IN ("%searchDir%\*.zip") DO (
    SET "zipFile=%%F"
    ECHO Found ZIP file: !zipFile!
    GOTO :foundZip
)

:foundZip
IF NOT DEFINED zipFile (
    ECHO No ZIP file found in the directory.
    PAUSE
    ENDLOCAL
    EXIT /B
)

ECHO ========== Unzipping !zipFile! to temporary directory ============
ECHO.
POWERSHELL -Command "Expand-Archive -Path '!zipFile!' -DestinationPath 'C:\Temp\24041_SEC2'"
ECHO.

ECHO Unzipping completed.

ECHO ========== Moving unzipped files to working directory ============
ECHO.
REM Move all files and directories from the extracted first level directory to the target directory
FOR /D %%D IN ("C:\Temp\24041_SEC2\*") DO (
    ECHO Moving contents of %%D to C:\MASTERs\24041_SEC2
    ROBOCOPY "%%D" "C:\MASTERs\24041_SEC2" /E /MOVE /NFL /NDL /NJH /NJS
)
ECHO.

ECHO ========== Cleaning up temporary directory ============
RMDIR /S /Q "C:\Temp\24041_SEC2"
ECHO.

ECHO ==================== UNZIPPING AND MOVING COMPLETE ==========================
ECHO.

ECHO ========== Creating Python virtual environment ============
ECHO.
CD "C:\MASTERs\24041_SEC2\sw\pc"
CALL "python_venv_setup.bat"
ECHO.

ECHO ========== Opening Programs ============
ECHO.
START "" "C:\Program Files\Adobe\Acrobat DC\Acrobat\Acrobat.exe" "C:\MASTERs\24041_SEC2\doc\LabManual.pdf"
START "" "C:\Program Files (x86)\teraterm\ttermpro.exe" /F="C:\MASTERs\24041_SEC2\sw\pc\TERATERM.INI"
START "" "C:\Program Files\Microchip\MPLABX\v6.20\mplab_platform\bin\mplab_ide64.exe" "C:\MASTERs\24041_SEC2\sw\badge_firmware\firmware\badge.X" --console new
ECHO.

ECHO.