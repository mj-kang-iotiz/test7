/**
 * @file production_test.h
 * @brief 생산 테스트 (Production Test) 모듈 헤더
 *
 * 이 모듈은 양산 라인에서 보드의 정상 동작 여부를 자동으로 테스트합니다.
 *
 * ## 사용 방법
 *
 * 1. 테스트 모드 진입:
 *    - 특정 버튼을 누른 상태로 전원 ON
 *    - 또는 UART로 "AT+PRODTEST" 명령 전송
 *
 * 2. 테스트 진행:
 *    - 자동으로 모든 하드웨어 모듈 테스트
 *    - 각 테스트 결과를 UART로 출력
 *    - LED 패턴으로 PASS/FAIL 표시
 *
 * 3. 결과 확인:
 *    - 녹색 LED 점등 = 전체 PASS
 *    - 빨간 LED 점멸 = 일부 FAIL
 *
 * ## 테스트 항목
 *
 * | 항목      | 설명                        | 타임아웃 |
 * |-----------|----------------------------|----------|
 * | Flash     | Flash 읽기/쓰기 테스트      | 1초      |
 * | EC25      | LTE 모듈 AT 응답 확인       | 5초      |
 * | RAK3172   | LoRa 모듈 AT 응답 확인      | 3초      |
 * | GPS       | NMEA 데이터 수신 확인       | 10초     |
 * | BLE       | BLE 모듈 AT 응답 확인       | 3초      |
 * | RS485     | RS485 루프백 테스트         | 2초      |
 * | Power     | 전원 전압 ADC 측정          | 1초      |
 *
 * @note 테스트 실패 시 해당 에러 코드를 참조하여 불량 원인을 파악하세요.
 */

#ifndef PRODUCTION_TEST_H
#define PRODUCTION_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * 설정
 * ============================================================================ */

/** 테스트 타임아웃 설정 (밀리초) */
#define PROD_TEST_TIMEOUT_FLASH     1000    /**< Flash 테스트 타임아웃 */
#define PROD_TEST_TIMEOUT_EC25      5000    /**< EC25 테스트 타임아웃 */
#define PROD_TEST_TIMEOUT_LORA      3000    /**< LoRa 테스트 타임아웃 */
#define PROD_TEST_TIMEOUT_GPS       10000   /**< GPS 테스트 타임아웃 */
#define PROD_TEST_TIMEOUT_BLE       3000    /**< BLE 테스트 타임아웃 */
#define PROD_TEST_TIMEOUT_RS485     2000    /**< RS485 테스트 타임아웃 */
#define PROD_TEST_TIMEOUT_POWER     1000    /**< 전원 테스트 타임아웃 */

/** 전원 전압 허용 범위 (mV) */
#define PROD_TEST_VIN_MIN           10500   /**< 입력 전압 최소값 (10.5V) */
#define PROD_TEST_VIN_MAX           14500   /**< 입력 전압 최대값 (14.5V) */
#define PROD_TEST_3V3_MIN           3135    /**< 3.3V 레일 최소값 (-5%) */
#define PROD_TEST_3V3_MAX           3465    /**< 3.3V 레일 최대값 (+5%) */

/* ============================================================================
 * 타입 정의
 * ============================================================================ */

/**
 * @brief 개별 테스트 결과 코드
 */
typedef enum {
    PROD_TEST_OK = 0,               /**< 테스트 성공 */
    PROD_TEST_FAIL,                 /**< 테스트 실패 (일반) */
    PROD_TEST_TIMEOUT,              /**< 타임아웃 */
    PROD_TEST_NO_RESPONSE,          /**< 응답 없음 */
    PROD_TEST_INVALID_RESPONSE,     /**< 잘못된 응답 */
    PROD_TEST_DATA_MISMATCH,        /**< 데이터 불일치 */
    PROD_TEST_OUT_OF_RANGE,         /**< 범위 초과 */
    PROD_TEST_NOT_SUPPORTED,        /**< 지원하지 않는 테스트 */
    PROD_TEST_SKIPPED,              /**< 테스트 건너뜀 */
} prod_test_result_t;

/**
 * @brief 전체 테스트 결과 코드 (비트 플래그)
 */
typedef enum {
    PROD_TEST_ALL_PASS = 0,         /**< 모든 테스트 통과 */
    PROD_TEST_FAIL_FLASH = (1 << 0),/**< Flash 테스트 실패 */
    PROD_TEST_FAIL_EC25 = (1 << 1), /**< EC25 테스트 실패 */
    PROD_TEST_FAIL_LORA = (1 << 2), /**< LoRa 테스트 실패 */
    PROD_TEST_FAIL_GPS = (1 << 3),  /**< GPS 테스트 실패 */
    PROD_TEST_FAIL_BLE = (1 << 4),  /**< BLE 테스트 실패 */
    PROD_TEST_FAIL_RS485 = (1 << 5),/**< RS485 테스트 실패 */
    PROD_TEST_FAIL_POWER = (1 << 6),/**< 전원 테스트 실패 */
    PROD_TEST_FAIL_LED = (1 << 7),  /**< LED 테스트 실패 */
} prod_test_fail_flags_t;

/**
 * @brief 개별 테스트 항목 결과 구조체
 */
typedef struct {
    const char* name;               /**< 테스트 이름 */
    prod_test_result_t result;      /**< 결과 코드 */
    uint32_t elapsed_ms;            /**< 소요 시간 (ms) */
    char message[64];               /**< 상세 메시지 */
} prod_test_item_result_t;

/**
 * @brief 전체 테스트 결과 구조체
 */
typedef struct {
    /* 개별 테스트 결과 */
    prod_test_item_result_t flash;  /**< Flash 테스트 결과 */
    prod_test_item_result_t ec25;   /**< EC25 테스트 결과 */
    prod_test_item_result_t lora;   /**< LoRa 테스트 결과 */
    prod_test_item_result_t gps;    /**< GPS 테스트 결과 */
    prod_test_item_result_t ble;    /**< BLE 테스트 결과 */
    prod_test_item_result_t rs485;  /**< RS485 테스트 결과 */
    prod_test_item_result_t power;  /**< 전원 테스트 결과 */
    prod_test_item_result_t led;    /**< LED 테스트 결과 */

    /* 전체 결과 */
    uint32_t fail_flags;            /**< 실패 플래그 (비트 OR) */
    uint32_t total_elapsed_ms;      /**< 총 소요 시간 */
    uint32_t pass_count;            /**< 통과 항목 수 */
    uint32_t fail_count;            /**< 실패 항목 수 */
    uint32_t skip_count;            /**< 건너뛴 항목 수 */
    bool overall_pass;              /**< 전체 통과 여부 */
} prod_test_result_all_t;

/**
 * @brief 테스트 콜백 함수 타입
 * @param item 현재 테스트 항목 결과
 */
typedef void (*prod_test_callback_t)(const prod_test_item_result_t* item);

/* ============================================================================
 * 함수 선언
 * ============================================================================ */

/**
 * @brief 생산 테스트 모드 진입 조건 확인
 *
 * 특정 조건(예: 버튼 누름)이 만족되면 테스트 모드로 진입합니다.
 *
 * @return true: 테스트 모드 진입, false: 정상 모드
 */
bool prod_test_should_enter(void);

/**
 * @brief 전체 생산 테스트 실행
 *
 * 모든 테스트 항목을 순차적으로 실행하고 결과를 반환합니다.
 *
 * @param[out] result 테스트 결과 저장 구조체
 * @return 전체 통과 시 true, 하나라도 실패 시 false
 *
 * @code
 * prod_test_result_all_t result;
 * if (prod_test_run_all(&result)) {
 *     printf("All tests PASSED!\n");
 * } else {
 *     printf("Some tests FAILED: 0x%08X\n", result.fail_flags);
 * }
 * @endcode
 */
bool prod_test_run_all(prod_test_result_all_t* result);

/**
 * @brief 테스트 진행 콜백 설정
 *
 * 각 테스트 항목이 완료될 때마다 콜백이 호출됩니다.
 *
 * @param callback 콜백 함수 (NULL이면 비활성화)
 */
void prod_test_set_callback(prod_test_callback_t callback);

/* ============================================================================
 * 개별 테스트 함수
 * ============================================================================ */

/**
 * @brief Flash 메모리 테스트
 *
 * 테스트 영역에 데이터를 쓰고 읽어서 비교합니다.
 *
 * @param[out] result 테스트 결과
 * @return 성공 시 PROD_TEST_OK
 */
prod_test_result_t prod_test_flash(prod_test_item_result_t* result);

/**
 * @brief EC25 LTE 모듈 테스트
 *
 * AT 명령어를 보내고 응답을 확인합니다.
 *
 * @param[out] result 테스트 결과
 * @return 성공 시 PROD_TEST_OK
 */
prod_test_result_t prod_test_ec25(prod_test_item_result_t* result);

/**
 * @brief RAK3172 LoRa 모듈 테스트
 *
 * AT 명령어를 보내고 응답을 확인합니다.
 *
 * @param[out] result 테스트 결과
 * @return 성공 시 PROD_TEST_OK
 */
prod_test_result_t prod_test_lora(prod_test_item_result_t* result);

/**
 * @brief GPS 모듈 테스트
 *
 * NMEA 데이터 수신 여부를 확인합니다.
 *
 * @param[out] result 테스트 결과
 * @return 성공 시 PROD_TEST_OK
 */
prod_test_result_t prod_test_gps(prod_test_item_result_t* result);

/**
 * @brief BLE 모듈 테스트
 *
 * AT 명령어를 보내고 응답을 확인합니다.
 *
 * @param[out] result 테스트 결과
 * @return 성공 시 PROD_TEST_OK
 */
prod_test_result_t prod_test_ble(prod_test_item_result_t* result);

/**
 * @brief RS485 테스트
 *
 * 루프백 테스트 또는 통신 테스트를 수행합니다.
 *
 * @param[out] result 테스트 결과
 * @return 성공 시 PROD_TEST_OK
 */
prod_test_result_t prod_test_rs485(prod_test_item_result_t* result);

/**
 * @brief 전원 전압 테스트
 *
 * ADC로 전원 전압을 측정하고 범위를 확인합니다.
 *
 * @param[out] result 테스트 결과
 * @return 성공 시 PROD_TEST_OK
 */
prod_test_result_t prod_test_power(prod_test_item_result_t* result);

/**
 * @brief LED 테스트
 *
 * 모든 LED를 순차적으로 점멸합니다 (육안 확인용).
 *
 * @param[out] result 테스트 결과
 * @return 항상 PROD_TEST_OK (육안 확인)
 */
prod_test_result_t prod_test_led(prod_test_item_result_t* result);

/* ============================================================================
 * 유틸리티 함수
 * ============================================================================ */

/**
 * @brief 테스트 결과 코드를 문자열로 변환
 *
 * @param result 결과 코드
 * @return 결과 문자열 (예: "OK", "FAIL", "TIMEOUT")
 */
const char* prod_test_result_to_string(prod_test_result_t result);

/**
 * @brief 테스트 결과 요약 출력
 *
 * @param result 전체 테스트 결과
 */
void prod_test_print_summary(const prod_test_result_all_t* result);

/**
 * @brief 테스트 결과에 따른 LED 패턴 설정
 *
 * @param pass true: 성공 패턴 (녹색), false: 실패 패턴 (빨강 점멸)
 */
void prod_test_set_led_result(bool pass);

#ifdef __cplusplus
}
#endif

#endif /* PRODUCTION_TEST_H */
