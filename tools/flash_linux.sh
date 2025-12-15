#!/bin/bash
# ============================================================================
#  GPS RTK Rover - Production Flashing Tool (Linux/macOS)
# ============================================================================
#
#  이 스크립트는 STM32CubeProgrammer 또는 st-flash를 사용하여 펌웨어를 굽습니다.
#
#  사용법:
#    ./flash_linux.sh                      - 기본 펌웨어 플래싱
#    ./flash_linux.sh firmware.bin         - 특정 펌웨어 플래싱
#    ./flash_linux.sh firmware.bin 0001    - 펌웨어 + 시리얼 번호
#
#  필요 조건:
#    - STM32CubeProgrammer 또는 stlink-tools 설치
#    - ST-Link 연결
#
#  설치 (Ubuntu):
#    sudo apt install stlink-tools
#  또는
#    STM32CubeProgrammer 다운로드
#
# ============================================================================

set -e  # 에러 발생 시 중단

# === 색상 정의 ===
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# === 설정 ===
PRODUCT_NAME="GPS_RTK_ROVER"
FW_VERSION="v1.0.0"

# 기본 펌웨어 파일
DEFAULT_FIRMWARE="../build/gugu_system_rover.bin"

# Flash 주소
FLASH_ADDR=0x08000000

# 날짜 코드 (시리얼 번호용)
DATE_CODE=$(date +%y%m%d)

# === 함수 정의 ===

print_header() {
    echo ""
    echo "============================================================"
    echo -e "  ${BLUE}${PRODUCT_NAME}${NC} - Production Flashing Tool"
    echo "============================================================"
    echo "  Firmware Version : ${FW_VERSION}"
    echo "  Firmware File    : ${FIRMWARE}"
    echo "  Date Code        : ${DATE_CODE}"
    if [ -n "$SERIAL_SEQ" ]; then
        echo "  Serial Number    : GRR-${DATE_CODE}-${SERIAL_SEQ}"
    fi
    echo "============================================================"
    echo ""
}

print_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_step() {
    echo ""
    echo -e "${YELLOW}[$1]${NC} $2"
}

# === 프로그래머 찾기 ===
find_programmer() {
    # STM32CubeProgrammer CLI 확인
    if command -v STM32_Programmer_CLI &> /dev/null; then
        PROGRAMMER="STM32_Programmer_CLI"
        PROGRAMMER_TYPE="cube"
        return 0
    fi

    # 일반적인 설치 경로 확인
    CUBE_PATHS=(
        "/opt/stm32cubeprog/bin/STM32_Programmer_CLI"
        "$HOME/STM32CubeProgrammer/bin/STM32_Programmer_CLI"
        "/usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI"
    )

    for path in "${CUBE_PATHS[@]}"; do
        if [ -x "$path" ]; then
            PROGRAMMER="$path"
            PROGRAMMER_TYPE="cube"
            return 0
        fi
    done

    # st-flash 확인 (stlink-tools)
    if command -v st-flash &> /dev/null; then
        PROGRAMMER="st-flash"
        PROGRAMMER_TYPE="stlink"
        return 0
    fi

    return 1
}

# === 플래싱 함수 (STM32CubeProgrammer) ===
flash_with_cube() {
    print_step "1/4" "Connecting to target..."
    if ! $PROGRAMMER -c port=SWD mode=NORMAL reset=HWrst > /dev/null 2>&1; then
        print_error "Cannot connect to target!"
        echo ""
        echo "    Checklist:"
        echo "    [ ] ST-Link connected?"
        echo "    [ ] Target powered?"
        echo "    [ ] SWD pins connected correctly?"
        echo "    [ ] Permissions (try: sudo)?"
        echo ""
        exit 1
    fi
    print_success "Connected successfully."

    print_step "2/4" "Erasing flash memory..."
    if ! $PROGRAMMER -c port=SWD -e all > /dev/null 2>&1; then
        print_error "Flash erase failed!"
        exit 1
    fi
    print_success "Flash erased."

    print_step "3/4" "Programming firmware..."
    if ! $PROGRAMMER -c port=SWD -w "$FIRMWARE" $FLASH_ADDR -v; then
        print_error "Programming failed!"
        exit 1
    fi
    print_success "Programming complete."

    print_step "4/4" "Resetting target..."
    $PROGRAMMER -c port=SWD -rst > /dev/null 2>&1 || true
    print_success "Target reset."
}

# === 플래싱 함수 (st-flash) ===
flash_with_stlink() {
    print_step "1/3" "Erasing flash memory..."
    if ! st-flash erase > /dev/null 2>&1; then
        print_error "Flash erase failed!"
        echo ""
        echo "    Try: sudo st-flash erase"
        echo ""
        exit 1
    fi
    print_success "Flash erased."

    print_step "2/3" "Programming firmware..."
    if ! st-flash --reset write "$FIRMWARE" $FLASH_ADDR; then
        print_error "Programming failed!"
        exit 1
    fi
    print_success "Programming complete."

    print_step "3/3" "Verifying..."
    # st-flash는 자동 검증하므로 여기서 완료
    print_success "Verification complete."
}

# === 메인 ===

# 인자 처리
FIRMWARE="${1:-$DEFAULT_FIRMWARE}"
SERIAL_SEQ="$2"

# 헤더 출력
print_header

# 프로그래머 찾기
if ! find_programmer; then
    print_error "No programmer found!"
    echo ""
    echo "    Please install one of the following:"
    echo ""
    echo "    Option 1: STM32CubeProgrammer (recommended)"
    echo "              https://www.st.com/en/development-tools/stm32cubeprog.html"
    echo ""
    echo "    Option 2: stlink-tools"
    echo "              Ubuntu: sudo apt install stlink-tools"
    echo "              macOS:  brew install stlink"
    echo ""
    exit 1
fi

print_info "Using programmer: $PROGRAMMER ($PROGRAMMER_TYPE)"

# 펌웨어 파일 확인
if [ ! -f "$FIRMWARE" ]; then
    print_error "Firmware file not found: $FIRMWARE"
    exit 1
fi

print_info "Firmware size: $(du -h "$FIRMWARE" | cut -f1)"

# 플래싱 실행
if [ "$PROGRAMMER_TYPE" = "cube" ]; then
    flash_with_cube
else
    flash_with_stlink
fi

# 결과 출력
echo ""
echo "============================================================"
echo ""
echo -e "  ${GREEN}********************************************${NC}"
echo -e "  ${GREEN}*                                          *${NC}"
echo -e "  ${GREEN}*      FLASHING COMPLETED SUCCESSFULLY     *${NC}"
echo -e "  ${GREEN}*                                          *${NC}"
echo -e "  ${GREEN}********************************************${NC}"
echo ""
echo "  Firmware: $FIRMWARE"
echo "  Address : $FLASH_ADDR"
if [ -n "$SERIAL_SEQ" ]; then
    echo "  Serial  : GRR-${DATE_CODE}-${SERIAL_SEQ}"
fi
echo ""
echo "============================================================"
echo ""

# 로그 기록
echo "$(date '+%Y-%m-%d %H:%M:%S') - $FIRMWARE - SUCCESS" >> flash_log.txt
if [ -n "$SERIAL_SEQ" ]; then
    echo "$(date '+%Y-%m-%d %H:%M:%S') - GRR-${DATE_CODE}-${SERIAL_SEQ}" >> serial_log.txt
fi

exit 0
