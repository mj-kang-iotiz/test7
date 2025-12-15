/**
 * @file serial_number.c
 * @brief 시리얼 번호 관리 모듈 구현
 */

#include "serial_number.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef USE_HAL_DRIVER
#include "stm32f4xx_hal.h"
#endif

/* ============================================================================
 * 내부 상수 및 변수
 * ============================================================================ */

/** CRC32 다항식 */
#define CRC32_POLYNOMIAL    0xEDB88320

/** Flash에 저장된 디바이스 정보 포인터 */
static const device_info_t* s_device_info =
    (const device_info_t*)SERIAL_NUMBER_FLASH_ADDR;

/** 캐시된 디바이스 정보 (유효성 검사 후) */
static device_info_t s_cached_info;
static bool s_info_valid = false;
static bool s_info_loaded = false;

/* ============================================================================
 * 내부 함수
 * ============================================================================ */

/**
 * @brief CRC32 계산
 */
static uint32_t calculate_crc32(const void* data, size_t length)
{
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++) {
        crc ^= bytes[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            } else {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}

/**
 * @brief Flash 영역이 빈 상태인지 확인
 */
static bool is_flash_empty(const void* addr, size_t size)
{
    const uint8_t* bytes = (const uint8_t*)addr;
    for (size_t i = 0; i < size; i++) {
        if (bytes[i] != FLASH_ERASED_VALUE) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Flash 쓰기 (내부 함수)
 */
static bool flash_write_device_info(const device_info_t* info)
{
#ifdef USE_HAL_DRIVER
    HAL_StatusTypeDef status;

    /* Flash unlock */
    status = HAL_FLASH_Unlock();
    if (status != HAL_OK) {
        return false;
    }

    /* Sector 11 erase (0x080E0000 ~ 0x080FFFFF, 128KB) */
    /* 주의: 다른 데이터가 있으면 백업 필요 */
    FLASH_EraseInitTypeDef erase_init = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Sector = FLASH_SECTOR_11,
        .NbSectors = 1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3,
    };

    uint32_t sector_error = 0;
    status = HAL_FLASHEx_Erase(&erase_init, &sector_error);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }

    /* Word 단위로 쓰기 */
    const uint32_t* src = (const uint32_t*)info;
    uint32_t dest_addr = SERIAL_NUMBER_FLASH_ADDR;
    size_t words = sizeof(device_info_t) / 4;

    for (size_t i = 0; i < words; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                   dest_addr + (i * 4),
                                   src[i]);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
    }

    /* Flash lock */
    HAL_FLASH_Lock();

    /* 검증 */
    if (memcmp((void*)SERIAL_NUMBER_FLASH_ADDR, info, sizeof(device_info_t)) != 0) {
        return false;
    }

    return true;
#else
    (void)info;
    return false;
#endif
}

/* ============================================================================
 * 공개 함수 구현
 * ============================================================================ */

serial_number_result_t serial_number_init(void)
{
    s_info_loaded = true;

    /* Flash가 비어있으면 설정 안 됨 */
    if (is_flash_empty(s_device_info, sizeof(device_info_t))) {
        s_info_valid = false;
        return SERIAL_NOT_SET;
    }

    /* CRC 검증 */
    uint32_t calc_crc = calculate_crc32(s_device_info,
                                        sizeof(device_info_t) - sizeof(uint32_t));

    if (calc_crc != s_device_info->crc32) {
        s_info_valid = false;
        return SERIAL_CRC_ERROR;
    }

    /* 캐시에 복사 */
    memcpy(&s_cached_info, s_device_info, sizeof(device_info_t));
    s_info_valid = true;

    return SERIAL_OK;
}

bool serial_number_is_set(void)
{
    if (!s_info_loaded) {
        serial_number_init();
    }
    return s_info_valid;
}

serial_number_result_t serial_number_get(char* buffer, size_t size)
{
    if (buffer == NULL || size == 0) {
        return SERIAL_ERROR;
    }

    if (!serial_number_is_set()) {
        strncpy(buffer, "NOT_PROGRAMMED", size - 1);
        buffer[size - 1] = '\0';
        return SERIAL_NOT_SET;
    }

    strncpy(buffer, s_cached_info.serial_number, size - 1);
    buffer[size - 1] = '\0';

    return SERIAL_OK;
}

serial_number_result_t serial_number_set(const char* serial_number)
{
    if (serial_number == NULL) {
        return SERIAL_ERROR;
    }

    /* 이미 설정되어 있으면 실패 */
    if (serial_number_is_set()) {
        return SERIAL_ALREADY_SET;
    }

    /* 형식 검증 */
    if (!serial_number_validate_format(serial_number)) {
        return SERIAL_INVALID_FORMAT;
    }

    /* 디바이스 정보 구조체 준비 */
    device_info_t info = {0};
    strncpy(info.serial_number, serial_number, SERIAL_NUMBER_MAX_LEN - 1);

    /* CRC 계산 */
    info.crc32 = calculate_crc32(&info, sizeof(device_info_t) - sizeof(uint32_t));

    /* Flash에 쓰기 */
    if (!flash_write_device_info(&info)) {
        return SERIAL_FLASH_ERROR;
    }

    /* 캐시 업데이트 */
    memcpy(&s_cached_info, &info, sizeof(device_info_t));
    s_info_valid = true;

    return SERIAL_OK;
}

serial_number_result_t serial_number_set_full(const device_info_t* info)
{
    if (info == NULL) {
        return SERIAL_ERROR;
    }

    /* 이미 설정되어 있으면 실패 */
    if (serial_number_is_set()) {
        return SERIAL_ALREADY_SET;
    }

    /* 시리얼 번호 형식 검증 */
    if (!serial_number_validate_format(info->serial_number)) {
        return SERIAL_INVALID_FORMAT;
    }

    /* CRC 계산 포함된 복사본 생성 */
    device_info_t info_copy;
    memcpy(&info_copy, info, sizeof(device_info_t));
    info_copy.crc32 = calculate_crc32(&info_copy,
                                      sizeof(device_info_t) - sizeof(uint32_t));

    /* Flash에 쓰기 */
    if (!flash_write_device_info(&info_copy)) {
        return SERIAL_FLASH_ERROR;
    }

    /* 캐시 업데이트 */
    memcpy(&s_cached_info, &info_copy, sizeof(device_info_t));
    s_info_valid = true;

    return SERIAL_OK;
}

const device_info_t* serial_number_get_device_info(void)
{
    if (!serial_number_is_set()) {
        return NULL;
    }
    return &s_cached_info;
}

serial_number_result_t serial_number_get_production_date(char* buffer, size_t size)
{
    if (buffer == NULL || size == 0) {
        return SERIAL_ERROR;
    }

    if (!serial_number_is_set()) {
        buffer[0] = '\0';
        return SERIAL_NOT_SET;
    }

    strncpy(buffer, s_cached_info.production_date, size - 1);
    buffer[size - 1] = '\0';

    return SERIAL_OK;
}

serial_number_result_t serial_number_get_mac_address(uint8_t mac[MAC_ADDRESS_LEN])
{
    if (mac == NULL) {
        return SERIAL_ERROR;
    }

    if (!serial_number_is_set()) {
        memset(mac, 0, MAC_ADDRESS_LEN);
        return SERIAL_NOT_SET;
    }

    memcpy(mac, s_cached_info.mac_address, MAC_ADDRESS_LEN);

    return SERIAL_OK;
}

void serial_number_print_info(void)
{
    printf("\r\n");
    printf("========================================\r\n");
    printf("          DEVICE INFORMATION\r\n");
    printf("========================================\r\n");

    if (!serial_number_is_set()) {
        printf("  Status: NOT PROGRAMMED\r\n");
        printf("========================================\r\n");
        return;
    }

    printf("  Serial Number : %s\r\n", s_cached_info.serial_number);
    printf("  Production    : %s\r\n", s_cached_info.production_date);
    printf("  HW Version    : %s\r\n", s_cached_info.hw_version);
    printf("  Factory       : %s\r\n", s_cached_info.factory_code);
    printf("  MAC Address   : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           s_cached_info.mac_address[0],
           s_cached_info.mac_address[1],
           s_cached_info.mac_address[2],
           s_cached_info.mac_address[3],
           s_cached_info.mac_address[4],
           s_cached_info.mac_address[5]);
    printf("========================================\r\n");
}

/* ============================================================================
 * 유틸리티 함수
 * ============================================================================ */

bool serial_number_validate_format(const char* serial_number)
{
    if (serial_number == NULL) {
        return false;
    }

    size_t len = strlen(serial_number);

    /* 최소 길이 확인: XXX-YYMMDD-NNNN = 15자 */
    if (len < 15 || len >= SERIAL_NUMBER_MAX_LEN) {
        return false;
    }

    /* 접두사 확인 (선택적) */
    /* if (strncmp(serial_number, SERIAL_NUMBER_PREFIX, 3) != 0) {
        return false;
    } */

    /* 형식 확인: XXX-YYMMDD-NNNN */
    if (serial_number[3] != '-' || serial_number[10] != '-') {
        return false;
    }

    /* 날짜 부분이 숫자인지 확인 */
    for (int i = 4; i < 10; i++) {
        if (!isdigit((unsigned char)serial_number[i])) {
            return false;
        }
    }

    /* 일련번호 부분이 숫자인지 확인 */
    for (size_t i = 11; i < len; i++) {
        if (!isdigit((unsigned char)serial_number[i])) {
            return false;
        }
    }

    return true;
}

int serial_number_generate(char* buffer, size_t size,
                          const char* date, uint16_t sequence)
{
    if (buffer == NULL || size == 0 || date == NULL) {
        return -1;
    }

    if (sequence < 1 || sequence > 9999) {
        return -1;
    }

    return snprintf(buffer, size, "%s-%s-%04u",
                    SERIAL_NUMBER_PREFIX, date, sequence);
}

const char* serial_number_result_to_string(serial_number_result_t result)
{
    switch (result) {
        case SERIAL_OK:             return "OK";
        case SERIAL_ERROR:          return "ERROR";
        case SERIAL_ALREADY_SET:    return "ALREADY_SET";
        case SERIAL_INVALID_FORMAT: return "INVALID_FORMAT";
        case SERIAL_FLASH_ERROR:    return "FLASH_ERROR";
        case SERIAL_NOT_SET:        return "NOT_SET";
        case SERIAL_CRC_ERROR:      return "CRC_ERROR";
        default:                    return "UNKNOWN";
    }
}
