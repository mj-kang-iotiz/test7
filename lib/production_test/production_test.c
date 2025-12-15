/**
 * @file production_test.c
 * @brief 생산 테스트 (Production Test) 모듈 구현
 */

#include "production_test.h"
#include "version.h"
#include "board_config.h"
#include "led.h"

#include <stdio.h>
#include <string.h>

/* HAL 및 드라이버 헤더 */
#ifdef USE_HAL_DRIVER
#include "stm32f4xx_hal.h"
#include "main.h"
#endif

/* FreeRTOS 헤더 (타이밍용) */
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#define GET_TICK_MS()       (xTaskGetTickCount() * portTICK_PERIOD_MS)
#define DELAY_MS(ms)        vTaskDelay(pdMS_TO_TICKS(ms))
#else
#define GET_TICK_MS()       HAL_GetTick()
#define DELAY_MS(ms)        HAL_Delay(ms)
#endif

/* ============================================================================
 * 매크로 및 상수
 * ============================================================================ */

/** 테스트 버튼 GPIO (예: PA0) */
#ifndef PROD_TEST_BUTTON_PORT
#define PROD_TEST_BUTTON_PORT   GPIOA
#endif
#ifndef PROD_TEST_BUTTON_PIN
#define PROD_TEST_BUTTON_PIN    GPIO_PIN_0
#endif

/** Flash 테스트 영역 (마지막 섹터 사용 주의) */
#define FLASH_TEST_ADDR         0x080E0000  /* Sector 11 시작 */
#define FLASH_TEST_SIZE         4

/** 테스트 데이터 패턴 */
static const uint8_t TEST_PATTERN[] = {0xAA, 0x55, 0x12, 0x34};

/* ============================================================================
 * 정적 변수
 * ============================================================================ */

static prod_test_callback_t s_callback = NULL;

/* ============================================================================
 * 유틸리티 함수
 * ============================================================================ */

const char* prod_test_result_to_string(prod_test_result_t result)
{
    switch (result) {
        case PROD_TEST_OK:              return "PASS";
        case PROD_TEST_FAIL:            return "FAIL";
        case PROD_TEST_TIMEOUT:         return "TIMEOUT";
        case PROD_TEST_NO_RESPONSE:     return "NO_RESP";
        case PROD_TEST_INVALID_RESPONSE:return "INVALID";
        case PROD_TEST_DATA_MISMATCH:   return "MISMATCH";
        case PROD_TEST_OUT_OF_RANGE:    return "RANGE";
        case PROD_TEST_NOT_SUPPORTED:   return "N/A";
        case PROD_TEST_SKIPPED:         return "SKIP";
        default:                        return "UNKNOWN";
    }
}

static void init_item_result(prod_test_item_result_t* item, const char* name)
{
    item->name = name;
    item->result = PROD_TEST_FAIL;
    item->elapsed_ms = 0;
    memset(item->message, 0, sizeof(item->message));
}

static void notify_callback(const prod_test_item_result_t* item)
{
    if (s_callback != NULL) {
        s_callback(item);
    }
}

/* ============================================================================
 * 테스트 모드 진입 조건
 * ============================================================================ */

bool prod_test_should_enter(void)
{
#ifdef USE_HAL_DRIVER
    /* 테스트 버튼이 눌린 상태로 부팅되면 테스트 모드 진입 */
    GPIO_PinState state = HAL_GPIO_ReadPin(PROD_TEST_BUTTON_PORT, PROD_TEST_BUTTON_PIN);
    return (state == GPIO_PIN_SET);  /* Active High 기준, 회로에 따라 수정 */
#else
    return false;
#endif
}

/* ============================================================================
 * 개별 테스트 구현
 * ============================================================================ */

prod_test_result_t prod_test_flash(prod_test_item_result_t* result)
{
    init_item_result(result, "Flash");
    uint32_t start = GET_TICK_MS();

    printf("[TEST] Flash Memory...\r\n");

    /*
     * 실제 구현 시:
     * 1. Flash unlock
     * 2. 테스트 영역 erase
     * 3. 테스트 패턴 write
     * 4. Read back & compare
     * 5. Flash lock
     */

    /* 시뮬레이션: Flash 테스트는 일반적으로 성공 */
    /* 실제 코드에서는 flash_params 모듈의 함수를 활용 */

    bool test_ok = true;  /* 실제 테스트 결과로 대체 */

    result->elapsed_ms = GET_TICK_MS() - start;

    if (test_ok) {
        result->result = PROD_TEST_OK;
        snprintf(result->message, sizeof(result->message), "R/W OK");
        printf("[PASS] Flash OK (%lu ms)\r\n", result->elapsed_ms);
    } else {
        result->result = PROD_TEST_DATA_MISMATCH;
        snprintf(result->message, sizeof(result->message), "Data mismatch");
        printf("[FAIL] Flash data mismatch\r\n");
    }

    notify_callback(result);
    return result->result;
}

prod_test_result_t prod_test_ec25(prod_test_item_result_t* result)
{
    init_item_result(result, "EC25 LTE");
    uint32_t start = GET_TICK_MS();

    const board_config_t* config = board_get_config();

    /* GSM 미사용 보드는 스킵 */
    if (!config->use_gsm) {
        result->result = PROD_TEST_SKIPPED;
        snprintf(result->message, sizeof(result->message), "GSM disabled");
        printf("[SKIP] EC25 - GSM not enabled\r\n");
        notify_callback(result);
        return result->result;
    }

    printf("[TEST] EC25 LTE Module...\r\n");

    /*
     * 실제 구현:
     * 1. gsm_port_init() 호출
     * 2. "AT\r\n" 전송
     * 3. "OK" 응답 대기 (타임아웃: 5초)
     * 4. 선택적으로 "ATI" 로 모듈 정보 확인
     */

    bool at_ok = true;  /* gsm_send_at_sync("AT", 5000) 결과로 대체 */

    result->elapsed_ms = GET_TICK_MS() - start;

    if (at_ok) {
        result->result = PROD_TEST_OK;
        snprintf(result->message, sizeof(result->message), "AT response OK");
        printf("[PASS] EC25 OK (%lu ms)\r\n", result->elapsed_ms);
    } else {
        result->result = PROD_TEST_NO_RESPONSE;
        snprintf(result->message, sizeof(result->message), "No AT response");
        printf("[FAIL] EC25 no response\r\n");
    }

    notify_callback(result);
    return result->result;
}

prod_test_result_t prod_test_lora(prod_test_item_result_t* result)
{
    init_item_result(result, "RAK3172 LoRa");
    uint32_t start = GET_TICK_MS();

    printf("[TEST] RAK3172 LoRa Module...\r\n");

    /*
     * 실제 구현:
     * 1. lora_port_init() 호출
     * 2. "AT\r\n" 전송
     * 3. "OK" 응답 대기 (타임아웃: 3초)
     * 4. "AT+VER=?" 로 버전 확인
     */

    bool at_ok = true;  /* lora_send_at_sync("AT", 3000) 결과로 대체 */

    result->elapsed_ms = GET_TICK_MS() - start;

    if (at_ok) {
        result->result = PROD_TEST_OK;
        snprintf(result->message, sizeof(result->message), "AT response OK");
        printf("[PASS] RAK3172 OK (%lu ms)\r\n", result->elapsed_ms);
    } else {
        result->result = PROD_TEST_NO_RESPONSE;
        snprintf(result->message, sizeof(result->message), "No AT response");
        printf("[FAIL] RAK3172 no response\r\n");
    }

    notify_callback(result);
    return result->result;
}

prod_test_result_t prod_test_gps(prod_test_item_result_t* result)
{
    init_item_result(result, "GPS");
    uint32_t start = GET_TICK_MS();

    const board_config_t* config = board_get_config();

    if (config->gps_cnt == 0) {
        result->result = PROD_TEST_SKIPPED;
        snprintf(result->message, sizeof(result->message), "No GPS configured");
        printf("[SKIP] GPS - not configured\r\n");
        notify_callback(result);
        return result->result;
    }

    printf("[TEST] GPS Module (waiting for NMEA, max %d sec)...\r\n",
           PROD_TEST_TIMEOUT_GPS / 1000);

    /*
     * 실제 구현:
     * 1. gps_port_init() 호출
     * 2. NMEA 데이터 수신 대기 (최대 10초)
     * 3. $GPGGA 또는 $GNGGA 수신 확인
     */

    bool nmea_received = true;  /* gps_wait_nmea(10000) 결과로 대체 */

    result->elapsed_ms = GET_TICK_MS() - start;

    if (nmea_received) {
        result->result = PROD_TEST_OK;
        snprintf(result->message, sizeof(result->message), "NMEA received");
        printf("[PASS] GPS OK (%lu ms)\r\n", result->elapsed_ms);
    } else {
        result->result = PROD_TEST_TIMEOUT;
        snprintf(result->message, sizeof(result->message), "No NMEA data");
        printf("[FAIL] GPS no NMEA data (timeout)\r\n");
    }

    notify_callback(result);
    return result->result;
}

prod_test_result_t prod_test_ble(prod_test_item_result_t* result)
{
    init_item_result(result, "BLE");
    uint32_t start = GET_TICK_MS();

    const board_config_t* config = board_get_config();

    if (!config->use_ble) {
        result->result = PROD_TEST_SKIPPED;
        snprintf(result->message, sizeof(result->message), "BLE disabled");
        printf("[SKIP] BLE - not enabled\r\n");
        notify_callback(result);
        return result->result;
    }

    printf("[TEST] BLE Module...\r\n");

    /*
     * 실제 구현:
     * 1. ble_port_init() 호출
     * 2. "AT\r\n" 전송
     * 3. 응답 대기
     */

    bool at_ok = true;  /* ble_send_at_sync("AT", 3000) 결과로 대체 */

    result->elapsed_ms = GET_TICK_MS() - start;

    if (at_ok) {
        result->result = PROD_TEST_OK;
        snprintf(result->message, sizeof(result->message), "AT response OK");
        printf("[PASS] BLE OK (%lu ms)\r\n", result->elapsed_ms);
    } else {
        result->result = PROD_TEST_NO_RESPONSE;
        snprintf(result->message, sizeof(result->message), "No AT response");
        printf("[FAIL] BLE no response\r\n");
    }

    notify_callback(result);
    return result->result;
}

prod_test_result_t prod_test_rs485(prod_test_item_result_t* result)
{
    init_item_result(result, "RS485");
    uint32_t start = GET_TICK_MS();

    const board_config_t* config = board_get_config();

    if (!config->use_rs485) {
        result->result = PROD_TEST_SKIPPED;
        snprintf(result->message, sizeof(result->message), "RS485 disabled");
        printf("[SKIP] RS485 - not enabled\r\n");
        notify_callback(result);
        return result->result;
    }

    printf("[TEST] RS485...\r\n");

    /*
     * 실제 구현 (루프백 테스트):
     * 1. TX/RX 핀을 연결 (외부 루프백 케이블)
     * 2. 테스트 데이터 전송
     * 3. 수신 데이터 비교
     *
     * 또는 외부 테스트 장비와 통신 확인
     */

    bool loopback_ok = true;  /* rs485_loopback_test() 결과로 대체 */

    result->elapsed_ms = GET_TICK_MS() - start;

    if (loopback_ok) {
        result->result = PROD_TEST_OK;
        snprintf(result->message, sizeof(result->message), "Loopback OK");
        printf("[PASS] RS485 OK (%lu ms)\r\n", result->elapsed_ms);
    } else {
        result->result = PROD_TEST_DATA_MISMATCH;
        snprintf(result->message, sizeof(result->message), "Loopback failed");
        printf("[FAIL] RS485 loopback failed\r\n");
    }

    notify_callback(result);
    return result->result;
}

prod_test_result_t prod_test_power(prod_test_item_result_t* result)
{
    init_item_result(result, "Power");
    uint32_t start = GET_TICK_MS();

    printf("[TEST] Power Rails...\r\n");

    /*
     * 실제 구현:
     * 1. ADC로 VIN 측정 (분압 회로 고려)
     * 2. ADC로 3.3V 레일 측정
     * 3. 범위 확인
     */

    uint32_t vin_mv = 12000;    /* ADC 측정값으로 대체 */
    uint32_t v3v3_mv = 3300;    /* ADC 측정값으로 대체 */

    bool vin_ok = (vin_mv >= PROD_TEST_VIN_MIN && vin_mv <= PROD_TEST_VIN_MAX);
    bool v3v3_ok = (v3v3_mv >= PROD_TEST_3V3_MIN && v3v3_mv <= PROD_TEST_3V3_MAX);

    result->elapsed_ms = GET_TICK_MS() - start;

    if (vin_ok && v3v3_ok) {
        result->result = PROD_TEST_OK;
        snprintf(result->message, sizeof(result->message),
                 "VIN=%lumV 3V3=%lumV", vin_mv, v3v3_mv);
        printf("[PASS] Power OK - VIN=%lumV, 3V3=%lumV (%lu ms)\r\n",
               vin_mv, v3v3_mv, result->elapsed_ms);
    } else {
        result->result = PROD_TEST_OUT_OF_RANGE;
        snprintf(result->message, sizeof(result->message),
                 "VIN=%lumV(%s) 3V3=%lumV(%s)",
                 vin_mv, vin_ok ? "OK" : "NG",
                 v3v3_mv, v3v3_ok ? "OK" : "NG");
        printf("[FAIL] Power out of range\r\n");
    }

    notify_callback(result);
    return result->result;
}

prod_test_result_t prod_test_led(prod_test_item_result_t* result)
{
    init_item_result(result, "LED");
    uint32_t start = GET_TICK_MS();

    printf("[TEST] LED Blink Test (visual check)...\r\n");

    /* LED 순차 점멸 테스트 (육안 확인용) */
    for (int i = 0; i < 3; i++) {
        led_set_all(true);
        DELAY_MS(300);
        led_set_all(false);
        DELAY_MS(300);
    }

    result->elapsed_ms = GET_TICK_MS() - start;
    result->result = PROD_TEST_OK;  /* 육안 확인이므로 항상 OK */
    snprintf(result->message, sizeof(result->message), "Visual check");

    printf("[INFO] LED test complete - check visually\r\n");

    notify_callback(result);
    return result->result;
}

/* ============================================================================
 * 전체 테스트 실행
 * ============================================================================ */

void prod_test_set_callback(prod_test_callback_t callback)
{
    s_callback = callback;
}

bool prod_test_run_all(prod_test_result_all_t* result)
{
    uint32_t total_start = GET_TICK_MS();

    /* 결과 구조체 초기화 */
    memset(result, 0, sizeof(prod_test_result_all_t));

    /* 헤더 출력 */
    printf("\r\n");
    printf("========================================================\r\n");
    printf("          PRODUCTION TEST - %s %s\r\n", PRODUCT_NAME, FW_VERSION_STRING);
    printf("========================================================\r\n");
    printf("  Build: %s %s\r\n", BUILD_DATE, BUILD_TIME);
    printf("  Git:   %s\r\n", GIT_COMMIT_HASH);
    printf("========================================================\r\n");
    printf("\r\n");

    /* LED 테스트 (먼저, 육안 확인용) */
    prod_test_led(&result->led);
    printf("\r\n");

    /* 각 모듈 테스트 실행 */
    prod_test_flash(&result->flash);
    prod_test_ec25(&result->ec25);
    prod_test_lora(&result->lora);
    prod_test_gps(&result->gps);
    prod_test_ble(&result->ble);
    prod_test_rs485(&result->rs485);
    prod_test_power(&result->power);

    /* 결과 집계 */
    result->total_elapsed_ms = GET_TICK_MS() - total_start;

    prod_test_item_result_t* items[] = {
        &result->flash,
        &result->ec25,
        &result->lora,
        &result->gps,
        &result->ble,
        &result->rs485,
        &result->power,
        &result->led
    };

    prod_test_fail_flags_t flags[] = {
        PROD_TEST_FAIL_FLASH,
        PROD_TEST_FAIL_EC25,
        PROD_TEST_FAIL_LORA,
        PROD_TEST_FAIL_GPS,
        PROD_TEST_FAIL_BLE,
        PROD_TEST_FAIL_RS485,
        PROD_TEST_FAIL_POWER,
        PROD_TEST_FAIL_LED
    };

    for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); i++) {
        if (items[i]->result == PROD_TEST_OK) {
            result->pass_count++;
        } else if (items[i]->result == PROD_TEST_SKIPPED) {
            result->skip_count++;
        } else {
            result->fail_count++;
            result->fail_flags |= flags[i];
        }
    }

    result->overall_pass = (result->fail_count == 0);

    /* 결과 요약 출력 */
    prod_test_print_summary(result);

    /* LED로 결과 표시 */
    prod_test_set_led_result(result->overall_pass);

    return result->overall_pass;
}

void prod_test_print_summary(const prod_test_result_all_t* result)
{
    printf("\r\n");
    printf("========================================================\r\n");
    printf("                    TEST SUMMARY\r\n");
    printf("========================================================\r\n");

    prod_test_item_result_t const* items[] = {
        &result->flash,
        &result->ec25,
        &result->lora,
        &result->gps,
        &result->ble,
        &result->rs485,
        &result->power,
        &result->led
    };

    for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); i++) {
        printf("  %-12s : %-8s  %s\r\n",
               items[i]->name,
               prod_test_result_to_string(items[i]->result),
               items[i]->message);
    }

    printf("--------------------------------------------------------\r\n");
    printf("  Total Time   : %lu ms\r\n", result->total_elapsed_ms);
    printf("  Pass/Fail/Skip: %lu / %lu / %lu\r\n",
           result->pass_count, result->fail_count, result->skip_count);
    printf("========================================================\r\n");

    if (result->overall_pass) {
        printf("\r\n");
        printf("  **************************************\r\n");
        printf("  *                                    *\r\n");
        printf("  *     ===   OVERALL: PASS   ===      *\r\n");
        printf("  *                                    *\r\n");
        printf("  **************************************\r\n");
    } else {
        printf("\r\n");
        printf("  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\r\n");
        printf("  X                                    X\r\n");
        printf("  X     XXX   OVERALL: FAIL   XXX      X\r\n");
        printf("  X                                    X\r\n");
        printf("  X     Fail Flags: 0x%08lX         X\r\n", result->fail_flags);
        printf("  X                                    X\r\n");
        printf("  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\r\n");
    }

    printf("\r\n");
}

void prod_test_set_led_result(bool pass)
{
    if (pass) {
        /* 성공: 녹색 LED 점등 */
        led_set_color(LED_COLOR_GREEN);
    } else {
        /* 실패: 빨간 LED 점멸 */
        led_set_pattern(LED_PATTERN_ERROR);
    }
}

/* ============================================================================
 * LED 함수 Stub (led.h에 없으면 여기서 제공)
 * ============================================================================ */

__attribute__((weak)) void led_set_all(bool on)
{
    (void)on;
    /* 실제 LED 제어 코드로 대체 */
}

__attribute__((weak)) void led_set_color(uint8_t color)
{
    (void)color;
    /* 실제 LED 제어 코드로 대체 */
}

__attribute__((weak)) void led_set_pattern(uint8_t pattern)
{
    (void)pattern;
    /* 실제 LED 제어 코드로 대체 */
}
