@echo off
REM ============================================================================
REM  GPS RTK Rover - Production Flashing Tool (Windows)
REM ============================================================================
REM
REM  이 스크립트는 STM32CubeProgrammer를 사용하여 펌웨어를 굽습니다.
REM
REM  사용법:
REM    flash_windows.bat                    - 기본 펌웨어 플래싱
REM    flash_windows.bat firmware.bin       - 특정 펌웨어 플래싱
REM    flash_windows.bat firmware.bin 0001  - 펌웨어 + 시리얼 번호
REM
REM  필요 조건:
REM    - STM32CubeProgrammer 설치
REM    - ST-Link 드라이버 설치
REM    - ST-Link 연결
REM
REM ============================================================================

setlocal enabledelayedexpansion

REM === 설정 ===
set PRODUCT_NAME=GPS_RTK_ROVER
set FW_VERSION=v1.0.0

REM STM32CubeProgrammer 경로 (설치 위치에 따라 수정)
set PROGRAMMER=C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe

REM 기본 펌웨어 파일
set DEFAULT_FIRMWARE=..\build\gugu_system_rover.bin

REM Flash 주소
set FLASH_ADDR=0x08000000

REM 날짜 (시리얼 번호용)
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set datetime=%%a
set YEAR=%datetime:~2,2%
set MONTH=%datetime:~4,2%
set DAY=%datetime:~6,2%
set DATE_CODE=%YEAR%%MONTH%%DAY%

REM === 인자 처리 ===
if "%~1"=="" (
    set FIRMWARE=%DEFAULT_FIRMWARE%
) else (
    set FIRMWARE=%~1
)

set SERIAL_SEQ=%~2

REM === 헤더 출력 ===
echo.
echo ============================================================
echo   %PRODUCT_NAME% - Production Flashing Tool
echo ============================================================
echo   Firmware Version : %FW_VERSION%
echo   Firmware File    : %FIRMWARE%
echo   Date Code        : %DATE_CODE%
if not "%SERIAL_SEQ%"=="" (
    echo   Serial Number    : GRR-%DATE_CODE%-%SERIAL_SEQ%
)
echo ============================================================
echo.

REM === STM32CubeProgrammer 확인 ===
if not exist "%PROGRAMMER%" (
    echo [ERROR] STM32CubeProgrammer not found!
    echo         Expected: %PROGRAMMER%
    echo.
    echo         Please install STM32CubeProgrammer from:
    echo         https://www.st.com/en/development-tools/stm32cubeprog.html
    echo.
    pause
    exit /b 1
)

REM === 펌웨어 파일 확인 ===
if not exist "%FIRMWARE%" (
    echo [ERROR] Firmware file not found: %FIRMWARE%
    echo.
    pause
    exit /b 1
)

REM === 플래싱 시작 ===
echo [1/4] Connecting to target...
"%PROGRAMMER%" -c port=SWD mode=NORMAL reset=HWrst >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Cannot connect to target!
    echo.
    echo         Checklist:
    echo         [ ] ST-Link connected?
    echo         [ ] Target powered?
    echo         [ ] SWD pins connected correctly?
    echo.
    pause
    exit /b 1
)
echo         Connected successfully.

echo.
echo [2/4] Erasing flash memory...
"%PROGRAMMER%" -c port=SWD -e all >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Flash erase failed!
    pause
    exit /b 1
)
echo         Flash erased.

echo.
echo [3/4] Programming firmware...
"%PROGRAMMER%" -c port=SWD -w "%FIRMWARE%" %FLASH_ADDR% -v
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Programming failed!
    pause
    exit /b 1
)
echo         Programming complete.

echo.
echo [4/4] Resetting target...
"%PROGRAMMER%" -c port=SWD -rst >nul 2>&1
echo         Target reset.

REM === 결과 출력 ===
echo.
echo ============================================================
echo.
echo   ********************************************
echo   *                                          *
echo   *      FLASHING COMPLETED SUCCESSFULLY     *
echo   *                                          *
echo   ********************************************
echo.
echo   Firmware: %FIRMWARE%
echo   Address : %FLASH_ADDR%
if not "%SERIAL_SEQ%"=="" (
    echo   Serial  : GRR-%DATE_CODE%-%SERIAL_SEQ%
)
echo.
echo ============================================================
echo.

REM === 로그 기록 ===
echo %DATE% %TIME% - %FIRMWARE% - SUCCESS >> flash_log.txt
if not "%SERIAL_SEQ%"=="" (
    echo %DATE% %TIME% - GRR-%DATE_CODE%-%SERIAL_SEQ% >> serial_log.txt
)

pause
exit /b 0
