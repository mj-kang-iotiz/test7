# 보드 양산 준비 종합 가이드

> 이 문서는 GPS RTK 로버 시스템 (STM32F405RG) 보드의 양산을 위해 필요한 모든 소프트웨어 작업과 문서를 설명합니다.

---

## 목차

1. [양산이란?](#1-양산이란)
2. [펌웨어 버전 관리 시스템](#2-펌웨어-버전-관리-시스템)
3. [생산 테스트 펌웨어](#3-생산-테스트-펌웨어-self-test)
4. [부트로더와 OTA 업데이트](#4-부트로더와-ota-업데이트)
5. [시리얼 번호 관리](#5-시리얼-번호-관리)
6. [플래싱 자동화](#6-플래싱-자동화)
7. [필요한 문서 목록](#7-필요한-문서-목록)
8. [실제 양산 프로세스 흐름](#8-실제-양산-프로세스-흐름)

---

## 1. 양산이란?

### 1.1 개념

**양산(量産, Mass Production)** = 제품을 대량으로 생산하는 것

```
개발 단계:  보드 1~5개 만들어서 테스트
    ↓
시제품 단계: 보드 10~50개 만들어서 필드 테스트
    ↓
양산 단계:  보드 100개, 1000개, 10000개... 생산
```

### 1.2 왜 준비가 필요한가?

| 개발할 때 | 양산할 때 |
|-----------|-----------|
| 내가 직접 펌웨어 굽는다 | 공장 작업자가 굽는다 |
| 문제 생기면 내가 디버깅 | 작업자는 디버깅 못함 |
| 보드 1개씩 천천히 | 하루에 100개씩 빠르게 |
| 버전 관리 대충 | 어떤 버전인지 정확히 알아야 함 |
| 불량나면 내가 고침 | 불량률 관리, A/S 대응 필요 |

### 1.3 양산 준비의 목표

```
✅ 누구나 쉽게 펌웨어를 굽을 수 있어야 함
✅ 보드가 정상인지 빠르게 판별할 수 있어야 함
✅ 문제 발생 시 원인 추적이 가능해야 함
✅ 나중에 펌웨어 업데이트가 가능해야 함
✅ 모든 보드를 구분할 수 있어야 함 (시리얼 번호)
```

---

## 2. 펌웨어 버전 관리 시스템

### 2.1 왜 필요한가?

```
고객: "제 보드가 안 돼요"
나:   "펌웨어 버전이 뭐에요?"
고객: "몰라요"
나:   "..."  ← 이러면 안 됨!
```

버전이 있으면:
- 어떤 기능이 들어간 펌웨어인지 알 수 있음
- 버그가 있는 버전을 추적할 수 있음
- 업데이트가 필요한 보드를 식별할 수 있음

### 2.2 버전 번호 체계 (Semantic Versioning)

```
v1.2.3
 │ │ │
 │ │ └── Patch: 버그 수정 (하위 호환)
 │ └──── Minor: 기능 추가 (하위 호환)
 └────── Major: 큰 변경 (하위 호환 안 될 수 있음)
```

**예시:**
```
v1.0.0 → 첫 양산 버전
v1.0.1 → 버그 수정
v1.1.0 → LoRa 범위 개선 기능 추가
v2.0.0 → 프로토콜 변경 (이전 버전과 호환 안 됨)
```

### 2.3 구현 방법

**version.h 파일 만들기:**

```c
// config/version.h

#ifndef VERSION_H
#define VERSION_H

// ============================================
// 펌웨어 버전 정보
// ============================================

#define FW_VERSION_MAJOR    1       // 주 버전
#define FW_VERSION_MINOR    0       // 부 버전
#define FW_VERSION_PATCH    0       // 패치 버전

// 버전 문자열 (자동 생성)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define FW_VERSION_STRING   "v" STR(FW_VERSION_MAJOR) "." \
                            STR(FW_VERSION_MINOR) "." \
                            STR(FW_VERSION_PATCH)

// ============================================
// 빌드 정보 (빌드할 때 자동으로 채워짐)
// ============================================

#define BUILD_DATE          __DATE__    // "Dec 15 2025"
#define BUILD_TIME          __TIME__    // "14:30:00"

// Git commit hash (빌드 스크립트에서 주입)
#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH     "unknown"
#endif

// ============================================
// 하드웨어/보드 정보
// ============================================

#define HW_VERSION          "REV_A"     // PCB 버전
#define PRODUCT_NAME        "GPS_RTK_ROVER"
#define MANUFACTURER        "IOTIZ"

#endif // VERSION_H
```

**사용 예시:**

```c
// main.c 또는 init에서 버전 출력

#include "version.h"

void print_system_info(void) {
    printf("\n");
    printf("========================================\n");
    printf(" %s\n", PRODUCT_NAME);
    printf("========================================\n");
    printf(" Firmware : %s\n", FW_VERSION_STRING);
    printf(" Build    : %s %s\n", BUILD_DATE, BUILD_TIME);
    printf(" Git      : %s\n", GIT_COMMIT_HASH);
    printf(" Hardware : %s\n", HW_VERSION);
    printf("========================================\n");
    printf("\n");
}

// 출력 결과:
// ========================================
//  GPS_RTK_ROVER
// ========================================
//  Firmware : v1.0.0
//  Build    : Dec 15 2025 14:30:00
//  Git      : a1b2c3d
//  Hardware : REV_A
// ========================================
```

### 2.4 버전 조회 명령어 추가

UART나 BLE로 버전을 조회할 수 있게 만들기:

```c
// AT+VERSION 명령어 처리
if (strcmp(cmd, "AT+VERSION") == 0) {
    printf("+VERSION:%s,%s,%s\r\n",
           FW_VERSION_STRING,
           BUILD_DATE,
           GIT_COMMIT_HASH);
    printf("OK\r\n");
}
```

---

## 3. 생산 테스트 펌웨어 (Self-Test)

### 3.1 왜 필요한가?

양산 라인에서:
```
보드 조립 완료
    ↓
펌웨어 굽기
    ↓
테스트 ← 이게 "생산 테스트"
    ↓
PASS → 출하 / FAIL → 불량 처리
```

사람이 일일이 테스트하면:
- 시간이 오래 걸림
- 테스트 누락 가능
- 사람마다 기준이 다름

**자동화된 Self-Test가 필요!**

### 3.2 테스트 항목

우리 보드 기준으로 테스트해야 할 것들:

```
┌─────────────────────────────────────────────────────────┐
│                    생산 테스트 항목                       │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  1. MCU 기본 테스트                                      │
│     ├── Flash 읽기/쓰기                                  │
│     ├── RAM 테스트                                       │
│     └── Clock 확인 (168MHz)                             │
│                                                         │
│  2. LED 테스트                                           │
│     └── 모든 LED 순차 점멸 (육안 확인)                    │
│                                                         │
│  3. 통신 모듈 테스트                                     │
│     ├── EC25 (LTE) - AT 응답 확인                       │
│     ├── RAK3172 (LoRa) - AT 응답 확인                   │
│     ├── GPS 모듈 - NMEA 수신 확인                        │
│     ├── BLE 모듈 - AT 응답 확인                         │
│     └── RS485 - 루프백 테스트                           │
│                                                         │
│  4. 전원 테스트                                          │
│     ├── 입력 전압 확인 (ADC)                            │
│     └── 각 전원 레일 확인                                │
│                                                         │
│  5. 파라미터 저장 테스트                                 │
│     └── Flash에 쓰고 읽어서 비교                         │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### 3.3 구현 예시

```c
// production_test.h

#ifndef PRODUCTION_TEST_H
#define PRODUCTION_TEST_H

#include <stdint.h>
#include <stdbool.h>

// 테스트 결과 코드
typedef enum {
    TEST_PASS = 0,
    TEST_FAIL_FLASH,
    TEST_FAIL_EC25,
    TEST_FAIL_LORA,
    TEST_FAIL_GPS,
    TEST_FAIL_BLE,
    TEST_FAIL_RS485,
    TEST_FAIL_POWER,
} TestResult;

// 전체 테스트 결과 구조체
typedef struct {
    bool flash_ok;
    bool ec25_ok;
    bool lora_ok;
    bool gps_ok;
    bool ble_ok;
    bool rs485_ok;
    bool power_ok;
    TestResult overall_result;
} ProductionTestResult;

// 테스트 실행
ProductionTestResult run_production_test(void);

// 개별 테스트 함수
bool test_flash_memory(void);
bool test_ec25_module(void);
bool test_lora_module(void);
bool test_gps_module(void);
bool test_ble_module(void);
bool test_rs485(void);
bool test_power_rails(void);

#endif
```

```c
// production_test.c

#include "production_test.h"
#include "gsm.h"
#include "lora.h"
#include "gps.h"

// EC25 LTE 모듈 테스트
bool test_ec25_module(void) {
    printf("[TEST] EC25 LTE Module...\n");

    // AT 명령어 보내기
    if (!gsm_send_at_command("AT", 1000)) {
        printf("[FAIL] EC25 not responding\n");
        return false;
    }

    // 모듈 정보 확인
    if (!gsm_send_at_command("ATI", 1000)) {
        printf("[FAIL] EC25 ATI failed\n");
        return false;
    }

    // SIM 카드 확인 (옵션)
    // gsm_send_at_command("AT+CPIN?", 1000);

    printf("[PASS] EC25 OK\n");
    return true;
}

// LoRa 모듈 테스트
bool test_lora_module(void) {
    printf("[TEST] RAK3172 LoRa Module...\n");

    // AT 명령어 테스트
    if (!lora_send_at_command("AT", 1000)) {
        printf("[FAIL] RAK3172 not responding\n");
        return false;
    }

    // 버전 확인
    if (!lora_send_at_command("AT+VER=?", 1000)) {
        printf("[FAIL] RAK3172 version check failed\n");
        return false;
    }

    printf("[PASS] RAK3172 OK\n");
    return true;
}

// GPS 모듈 테스트
bool test_gps_module(void) {
    printf("[TEST] GPS Module...\n");

    // 5초 동안 NMEA 데이터 수신 대기
    uint32_t start = HAL_GetTick();
    bool nmea_received = false;

    while ((HAL_GetTick() - start) < 5000) {
        if (gps_is_data_available()) {
            nmea_received = true;
            break;
        }
        HAL_Delay(100);
    }

    if (!nmea_received) {
        printf("[FAIL] GPS no NMEA data\n");
        return false;
    }

    printf("[PASS] GPS OK\n");
    return true;
}

// Flash 메모리 테스트
bool test_flash_memory(void) {
    printf("[TEST] Flash Memory...\n");

    // 테스트 데이터
    uint8_t test_data[] = {0xAA, 0x55, 0x12, 0x34};
    uint8_t read_data[4];

    // Flash에 쓰기
    if (!flash_write_test_area(test_data, sizeof(test_data))) {
        printf("[FAIL] Flash write error\n");
        return false;
    }

    // Flash에서 읽기
    if (!flash_read_test_area(read_data, sizeof(read_data))) {
        printf("[FAIL] Flash read error\n");
        return false;
    }

    // 비교
    if (memcmp(test_data, read_data, sizeof(test_data)) != 0) {
        printf("[FAIL] Flash data mismatch\n");
        return false;
    }

    printf("[PASS] Flash OK\n");
    return true;
}

// 전체 생산 테스트 실행
ProductionTestResult run_production_test(void) {
    ProductionTestResult result = {0};

    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║      PRODUCTION TEST START           ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("\n");

    // LED 테스트 (육안 확인용)
    printf("[TEST] LED Blink Test - Check visually\n");
    for (int i = 0; i < 3; i++) {
        led_all_on();
        HAL_Delay(300);
        led_all_off();
        HAL_Delay(300);
    }
    printf("[INFO] LED test done\n\n");

    // 각 모듈 테스트
    result.flash_ok = test_flash_memory();
    result.ec25_ok = test_ec25_module();
    result.lora_ok = test_lora_module();
    result.gps_ok = test_gps_module();
    // result.ble_ok = test_ble_module();
    // result.rs485_ok = test_rs485();

    // 전체 결과 판정
    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║         TEST RESULT SUMMARY          ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  Flash  : %s                        ║\n", result.flash_ok ? "PASS" : "FAIL");
    printf("║  EC25   : %s                        ║\n", result.ec25_ok ? "PASS" : "FAIL");
    printf("║  LoRa   : %s                        ║\n", result.lora_ok ? "PASS" : "FAIL");
    printf("║  GPS    : %s                        ║\n", result.gps_ok ? "PASS" : "FAIL");
    printf("╠══════════════════════════════════════╣\n");

    bool all_pass = result.flash_ok && result.ec25_ok &&
                    result.lora_ok && result.gps_ok;

    if (all_pass) {
        result.overall_result = TEST_PASS;
        printf("║  ★★★ OVERALL: PASS ★★★            ║\n");
        // 성공 LED 패턴 (녹색 점등)
        led_set_pattern(LED_PATTERN_PASS);
    } else {
        result.overall_result = TEST_FAIL_FLASH; // 실패 원인에 따라 변경
        printf("║  ✗✗✗ OVERALL: FAIL ✗✗✗            ║\n");
        // 실패 LED 패턴 (빨강 점멸)
        led_set_pattern(LED_PATTERN_FAIL);
    }

    printf("╚══════════════════════════════════════╝\n");
    printf("\n");

    return result;
}
```

### 3.4 테스트 모드 진입 방법

생산 테스트는 **특별한 조건에서만** 실행되어야 함:

```c
// main.c

int main(void) {
    HAL_Init();
    SystemClock_Config();

    // 버튼이 눌린 상태로 부팅하면 테스트 모드
    if (is_test_button_pressed()) {
        run_production_test();
        while(1) {
            // 테스트 모드에서 대기
            // 결과 확인 후 리셋
        }
    }

    // 정상 동작 모드
    normal_operation();
}
```

또는 **특별한 펌웨어**로 분리:

```
firmware_production_test.bin  ← 공장에서 테스트용
firmware_release.bin          ← 실제 출하용
```

---

## 4. 부트로더와 OTA 업데이트

### 4.1 왜 필요한가?

```
현재 상황: 펌웨어 업데이트하려면...
    1. ST-Link 연결
    2. PC에서 플래싱
    3. 현장에 가서 작업해야 함 😱

부트로더가 있으면:
    1. 새 펌웨어를 서버에 올림
    2. 보드가 LTE로 다운로드
    3. 자동 업데이트 😊
```

### 4.2 부트로더 개념

```
┌────────────────────────────────────────────────────────────┐
│                    STM32 Flash Memory (1MB)                │
├────────────────────────────────────────────────────────────┤
│                                                            │
│  0x08000000 ┌──────────────────────┐                      │
│             │    Bootloader        │  32KB                │
│             │    (부트로더)         │  - 절대 안 바뀜      │
│  0x08008000 ├──────────────────────┤                      │
│             │    Application       │  480KB               │
│             │    (메인 펌웨어)      │  - 업데이트 대상     │
│  0x08080000 ├──────────────────────┤                      │
│             │    Backup/Update     │  480KB               │
│             │    (백업 영역)        │  - 새 펌웨어 저장    │
│  0x080F8000 ├──────────────────────┤                      │
│             │    Parameters        │  32KB                │
│             │    (설정 저장)        │  - 시리얼번호 등     │
│  0x08100000 └──────────────────────┘                      │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

### 4.3 부트로더 동작 흐름

```
전원 ON
   │
   ▼
┌─────────────────┐
│   Bootloader    │
│   시작          │
└────────┬────────┘
         │
         ▼
    업데이트 플래그
    설정되어 있나?
        │
    ┌───┴───┐
    │       │
   YES      NO
    │       │
    ▼       ▼
┌─────────┐ ┌─────────────┐
│새 펌웨어│ │메인 펌웨어로│
│복사&설치│ │점프         │
└────┬────┘ └──────┬──────┘
     │             │
     ▼             ▼
  검증 후     ┌─────────────┐
  재부팅      │ Application │
              │ 실행        │
              └─────────────┘
```

### 4.4 OTA 업데이트 흐름

```
┌──────────┐         ┌──────────┐         ┌──────────┐
│  서버    │         │  보드    │         │ Flash    │
└────┬─────┘         └────┬─────┘         └────┬─────┘
     │                    │                    │
     │   1. 버전 확인 요청  │                    │
     │<───────────────────│                    │
     │                    │                    │
     │   2. 새 버전 있음   │                    │
     │───────────────────>│                    │
     │   (v1.0.0 → v1.1.0)│                    │
     │                    │                    │
     │   3. 펌웨어 다운로드 │                    │
     │───────────────────>│   4. Backup 영역에 │
     │   (480KB 청크로)    │──────────────────>│
     │                    │      저장          │
     │                    │                    │
     │                    │   5. CRC 검증      │
     │                    │<──────────────────│
     │                    │                    │
     │                    │   6. 업데이트 플래그│
     │                    │──────────────────>│
     │                    │      설정          │
     │                    │                    │
     │                    │   7. 리부팅        │
     │                    │       │            │
     │                    │       ▼            │
     │                    │   [Bootloader]     │
     │                    │   새 펌웨어 설치    │
     │                    │                    │
```

### 4.5 부트로더 구현 시 주의사항

```
⚠️  중요한 것들:

1. 무결성 검증
   - 다운로드한 펌웨어가 손상되지 않았는지 CRC/SHA로 확인
   - 손상되면 업데이트 취소

2. 롤백 (Rollback)
   - 새 펌웨어가 부팅 안 되면 이전 버전으로 복구
   - "부팅 성공" 플래그로 판단

3. 전원 차단 대응
   - 업데이트 중 전원 꺼져도 벽돌 안 되게
   - Backup 영역에 먼저 쓰고, 검증 후 복사

4. 보안
   - 펌웨어 서명 검증 (선택사항)
   - 암호화된 전송 (HTTPS)
```

### 4.6 양산 초기에는...

> 💡 **팁**: 부트로더는 복잡하므로, 양산 초기에는 없어도 됨!
>
> 1차 양산: ST-Link로 직접 플래싱
> 2차 양산: 부트로더 추가

---

## 5. 시리얼 번호 관리

### 5.1 왜 필요한가?

```
1000대 출하 후...

고객: "보드가 고장났어요"
나:   "보드 시리얼 번호가 뭐에요?"
고객: "SN-2024-001234"
나:   "아, 그 보드는 2024년 1월 생산분이고,
       v1.0.0 펌웨어가 들어갔네요.
       해당 배치에서 문제가 좀 있었어요.
       새 펌웨어 보내드릴게요."
```

시리얼 번호로 할 수 있는 것:
- **추적성(Traceability)**: 언제, 어디서 생산됐는지
- **A/S 관리**: 보증 기간, 수리 이력
- **불량 분석**: 특정 배치의 불량률
- **재고 관리**: 입출고 추적

### 5.2 시리얼 번호 체계 설계

```
SN-YYMMDD-XXXX
│  │      │
│  │      └── 일련번호 (0001~9999)
│  └───────── 생산일자 (241215 = 2024년 12월 15일)
└──────────── 접두사

예시:
SN-241215-0001  ← 2024년 12월 15일 생산, 1번째
SN-241215-0002  ← 2024년 12월 15일 생산, 2번째
SN-241216-0001  ← 2024년 12월 16일 생산, 1번째
```

더 복잡한 체계:
```
IOTIZ-RTK-A-241215-0001
│     │   │ │      │
│     │   │ │      └── 일련번호
│     │   │ └───────── 생산일자
│     │   └─────────── 하드웨어 버전 (A, B, C...)
│     └─────────────── 제품 모델
└───────────────────── 제조사
```

### 5.3 구현 방법

**Flash의 특정 영역에 저장:**

```c
// serial_number.h

#ifndef SERIAL_NUMBER_H
#define SERIAL_NUMBER_H

#define SERIAL_NUMBER_FLASH_ADDR    0x080FF000  // Flash 마지막 4KB
#define SERIAL_NUMBER_MAX_LEN       32

typedef struct {
    char serial_number[SERIAL_NUMBER_MAX_LEN];  // "SN-241215-0001"
    char production_date[12];                    // "2024-12-15"
    char hardware_version[8];                    // "REV_A"
    char factory_code[8];                        // "FAC01"
    uint32_t crc;                                // 무결성 검증
} DeviceInfo;

// 시리얼 번호 읽기
bool get_serial_number(char* buffer, size_t len);

// 시리얼 번호 쓰기 (생산 시 1회만)
bool set_serial_number(const char* serial);

// 시리얼 번호 설정 여부 확인
bool is_serial_number_set(void);

#endif
```

```c
// serial_number.c

#include "serial_number.h"
#include "flash.h"

static DeviceInfo* device_info = (DeviceInfo*)SERIAL_NUMBER_FLASH_ADDR;

bool get_serial_number(char* buffer, size_t len) {
    // Flash에서 직접 읽기
    if (device_info->serial_number[0] == 0xFF) {
        // 프로그래밍 안 됨
        strncpy(buffer, "NOT_PROGRAMMED", len);
        return false;
    }

    strncpy(buffer, device_info->serial_number, len);
    return true;
}

bool set_serial_number(const char* serial) {
    // 이미 설정되어 있으면 실패 (1회만 쓰기 가능)
    if (is_serial_number_set()) {
        return false;
    }

    DeviceInfo new_info = {0};
    strncpy(new_info.serial_number, serial, SERIAL_NUMBER_MAX_LEN - 1);
    // ... 다른 필드 설정
    new_info.crc = calculate_crc(&new_info, sizeof(new_info) - 4);

    // Flash에 쓰기
    return flash_write(SERIAL_NUMBER_FLASH_ADDR, &new_info, sizeof(new_info));
}
```

### 5.4 생산 라인에서 시리얼 번호 주입

```bash
# 플래싱 스크립트에서

# 1. 펌웨어 굽기
st-flash write firmware.bin 0x08000000

# 2. 시리얼 번호 주입 (별도 도구 또는 UART 명령)
python inject_serial.py --port COM3 --serial "SN-241215-0001"

# 3. 시리얼 번호 확인
python verify_serial.py --port COM3
```

---

## 6. 플래싱 자동화

### 6.1 왜 필요한가?

```
수동 플래싱 (개발할 때):
1. STM32CubeIDE 열기
2. 프로젝트 열기
3. Build
4. ST-Link 연결
5. Flash
6. 결과 확인

→ 10분 소요, 실수 가능

자동 플래싱 (양산할 때):
1. 스크립트 더블클릭
2. 끝

→ 30초, 실수 없음
```

### 6.2 ST-Link 커맨드라인 도구

```bash
# STM32CubeProgrammer CLI 사용

# 펌웨어 굽기
STM32_Programmer_CLI -c port=SWD -w firmware.bin 0x08000000 -v

# 옵션 설명:
# -c port=SWD      : ST-Link 연결 (SWD 모드)
# -w firmware.bin  : 쓸 파일
# 0x08000000       : 시작 주소
# -v               : 쓴 후 검증 (verify)
```

### 6.3 플래싱 스크립트 예시

**Windows 배치 파일 (flash.bat):**

```batch
@echo off
echo ============================================
echo   GPS RTK Rover - Production Flashing Tool
echo ============================================
echo.

set PROGRAMMER="C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
set FIRMWARE="firmware_v1.0.0.bin"
set ADDRESS=0x08000000

echo [1/3] Connecting to target...
%PROGRAMMER% -c port=SWD

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Cannot connect to target!
    echo         Check ST-Link connection.
    pause
    exit /b 1
)

echo.
echo [2/3] Flashing firmware...
%PROGRAMMER% -c port=SWD -w %FIRMWARE% %ADDRESS% -v

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Flashing failed!
    pause
    exit /b 1
)

echo.
echo [3/3] Resetting target...
%PROGRAMMER% -c port=SWD -rst

echo.
echo ============================================
echo   FLASHING COMPLETE - SUCCESS
echo ============================================
echo.
pause
```

**Linux/Mac 쉘 스크립트 (flash.sh):**

```bash
#!/bin/bash

PROGRAMMER="STM32_Programmer_CLI"
FIRMWARE="firmware_v1.0.0.bin"
ADDRESS="0x08000000"

echo "============================================"
echo "  GPS RTK Rover - Production Flashing Tool"
echo "============================================"
echo ""

# 연결 확인
echo "[1/3] Connecting to target..."
$PROGRAMMER -c port=SWD > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "[ERROR] Cannot connect to target!"
    exit 1
fi

# 플래싱
echo "[2/3] Flashing firmware..."
$PROGRAMMER -c port=SWD -w $FIRMWARE $ADDRESS -v
if [ $? -ne 0 ]; then
    echo "[ERROR] Flashing failed!"
    exit 1
fi

# 리셋
echo "[3/3] Resetting target..."
$PROGRAMMER -c port=SWD -rst

echo ""
echo "============================================"
echo "  FLASHING COMPLETE - SUCCESS"
echo "============================================"
```

### 6.4 대량 플래싱 시스템

```
┌─────────────────────────────────────────────────────────┐
│                대량 플래싱 시스템 구성                    │
├─────────────────────────────────────────────────────────┤
│                                                         │
│   ┌─────────┐    ┌─────────┐    ┌─────────┐           │
│   │ ST-Link │    │ ST-Link │    │ ST-Link │           │
│   │   #1    │    │   #2    │    │   #3    │   ...     │
│   └────┬────┘    └────┬────┘    └────┬────┘           │
│        │              │              │                 │
│        └──────────────┼──────────────┘                 │
│                       │                                │
│                  ┌────┴────┐                           │
│                  │ USB Hub │                           │
│                  └────┬────┘                           │
│                       │                                │
│                  ┌────┴────┐                           │
│                  │   PC    │                           │
│                  │ 플래싱  │                           │
│                  │ 소프트웨어│                          │
│                  └─────────┘                           │
│                                                         │
│   → 여러 보드를 동시에 플래싱 가능                       │
│   → 자동 시리얼 번호 부여                               │
│   → 테스트 결과 로깅                                    │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## 7. 필요한 문서 목록

### 7.1 생산 테스트 절차서 (Production Test Procedure)

**목적**: 공장 작업자가 보고 따라할 수 있는 테스트 순서

**내용**:
```
1. 준비물
   - 테스트 지그
   - ST-Link
   - 테스트 PC
   - 전원 공급기

2. 테스트 순서
   Step 1: 보드를 지그에 장착
   Step 2: 전원 연결 (12V)
   Step 3: ST-Link 연결
   Step 4: 플래싱 프로그램 실행
   Step 5: 자동 테스트 대기 (약 30초)
   Step 6: 결과 확인
          - 녹색 LED = PASS → 양품 박스로
          - 빨간 LED = FAIL → 불량 박스로

3. 불량 유형별 대처
   - EC25 FAIL: LTE 모듈 납땜 확인
   - GPS FAIL: GPS 모듈 납땜 확인
   - ...
```

### 7.2 플래싱 가이드 (Flashing Guide)

**목적**: 펌웨어를 굽는 정확한 방법

**내용**:
```
1. 필요한 도구 설치
   - STM32CubeProgrammer 다운로드/설치

2. ST-Link 드라이버 설치

3. 펌웨어 파일 위치
   - firmware_v1.0.0.bin

4. 플래싱 순서
   (스크린샷 포함)

5. 문제 해결
   - "Cannot connect" 에러 → ...
   - "Verification failed" 에러 → ...
```

### 7.3 QC 체크리스트 (Quality Control Checklist)

**목적**: 품질 검사 항목과 합격 기준

**예시**:
```
┌────┬─────────────────────────┬──────────────┬────────┐
│ No │ 검사 항목                │ 합격 기준     │ 결과   │
├────┼─────────────────────────┼──────────────┼────────┤
│ 1  │ 외관 검사               │ 스크래치 없음 │ □PASS  │
│ 2  │ LED 점등                │ 3개 모두 점등 │ □PASS  │
│ 3  │ LTE 연결                │ AT 응답 OK   │ □PASS  │
│ 4  │ LoRa 연결               │ AT 응답 OK   │ □PASS  │
│ 5  │ GPS 수신                │ NMEA 출력    │ □PASS  │
│ 6  │ 전원 전압               │ 3.3V ± 5%   │ □PASS  │
│ 7  │ 소비 전류               │ < 500mA     │ □PASS  │
├────┴─────────────────────────┴──────────────┴────────┤
│ 최종 판정: □ 합격  □ 불합격                           │
│ 검사자: ____________  날짜: ____________             │
└──────────────────────────────────────────────────────┘
```

### 7.4 릴리즈 노트 (Release Note)

**목적**: 각 버전의 변경 사항 기록

**예시**:
```markdown
# Release Note - v1.0.0

## 릴리즈 정보
- 버전: v1.0.0
- 릴리즈 날짜: 2024-12-15
- 빌드 번호: 100

## 새로운 기능
- 초기 양산 버전
- LTE/LoRa/GPS 기본 기능

## 버그 수정
- 해당 없음 (초기 버전)

## 알려진 이슈
- GPS Cold Start 시 최대 60초 소요
- LTE 연결 시 간헐적 재시도 필요

## 하드웨어 요구사항
- PCB Rev.A 이상
- EC25 모듈 펌웨어 v2.0 이상

## 업그레이드 주의사항
- 해당 없음 (초기 버전)
```

### 7.5 Troubleshooting Guide (문제 해결 가이드)

**목적**: 자주 발생하는 문제와 해결 방법

**예시**:
```
문제: EC25 테스트 FAIL

증상:
- "EC25 not responding" 메시지

원인 및 해결:
1. EC25 모듈 납땜 상태 확인
   → 납땜 불량 시 재작업

2. EC25 전원 확인 (3.8V)
   → 전압 이상 시 전원부 점검

3. UART 연결 확인
   → TX/RX 핀 연결 상태 확인

4. EC25 모듈 자체 불량
   → 모듈 교체
```

---

## 8. 실제 양산 프로세스 흐름

### 8.1 전체 흐름도

```
┌─────────────────────────────────────────────────────────────┐
│                    양산 프로세스 흐름                         │
└─────────────────────────────────────────────────────────────┘

[SMT 공정]
    │
    ▼
┌─────────────┐
│ PCB + 부품   │
│ 실장 완료    │
└──────┬──────┘
       │
       ▼
[AOI 검사] ──FAIL──→ [재작업]
    │                    │
   PASS                  │
    │                    │
    ▼                    │
┌─────────────┐          │
│ ICT 검사    │←─────────┘
│ (회로 검사)  │
└──────┬──────┘
       │
      PASS
       │
       ▼
┌─────────────────────────────────────────┐
│           펌웨어 플래싱                  │
├─────────────────────────────────────────┤
│  1. ST-Link 연결                        │
│  2. Production Test FW 굽기             │
│  3. 자동 테스트 실행                     │
│  4. 테스트 PASS 시 → Release FW 굽기    │
│  5. 시리얼 번호 주입                     │
│  6. 시리얼 번호 라벨 출력/부착           │
└──────────────────┬──────────────────────┘
                   │
                  PASS
                   │
                   ▼
            ┌─────────────┐
            │ 기능 테스트  │
            │ (샘플링)     │
            └──────┬──────┘
                   │
                  PASS
                   │
                   ▼
            ┌─────────────┐
            │ 포장        │
            │ 출하        │
            └─────────────┘
```

### 8.2 플래싱 스테이션 구성

```
┌──────────────────────────────────────────────────────────┐
│                  플래싱 스테이션                          │
├──────────────────────────────────────────────────────────┤
│                                                          │
│   ┌──────────────┐    ┌──────────────────────────────┐  │
│   │              │    │                              │  │
│   │  테스트 지그  │    │       모니터                 │  │
│   │              │    │    ┌─────────────────────┐   │  │
│   │  ┌────────┐  │    │    │ 플래싱 프로그램     │   │  │
│   │  │ 보드   │  │    │    │                     │   │  │
│   │  │ 장착   │  │    │    │ [상태: 대기중]      │   │  │
│   │  └────────┘  │    │    │                     │   │  │
│   │              │    │    │ [시작] [결과보기]   │   │  │
│   │  ST-Link     │    │    └─────────────────────┘   │  │
│   │  연결됨      │    │                              │  │
│   │              │    └──────────────────────────────┘  │
│   └──────────────┘                                      │
│                                                          │
│   ┌──────────┐  ┌──────────┐  ┌──────────┐             │
│   │ 양품 박스│  │ 불량 박스│  │바코드스캐너│            │
│   │          │  │          │  │          │             │
│   └──────────┘  └──────────┘  └──────────┘             │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

---

## 체크리스트: 양산 준비 완료 여부

```
소프트웨어 준비:
□ 펌웨어 버전 관리 시스템 구현
□ 생산 테스트 펌웨어 개발
□ 부트로더 구현 (옵션)
□ 시리얼 번호 관리 시스템
□ 플래싱 스크립트/도구

문서 준비:
□ 생산 테스트 절차서
□ 플래싱 가이드
□ QC 체크리스트
□ 릴리즈 노트
□ Troubleshooting 가이드

기타:
□ 테스트 지그 설계/제작
□ 바코드/라벨 시스템
□ 불량 관리 체계
□ A/S 프로세스 정립
```

---

## 다음 단계

이 문서를 기반으로 다음을 진행하면 됩니다:

1. **우선순위 결정**: 위 체크리스트에서 필요한 것 선택
2. **구현/작성**: 하나씩 진행
3. **테스트**: 실제 보드로 검증
4. **문서화**: 결과물 정리

추가 도움이 필요하면 각 항목별로 더 자세한 구현을 요청해 주세요!
