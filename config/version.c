/**
 * @file version.c
 * @brief 펌웨어 버전 관리 구현
 */

#include "version.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * 정적 변수
 * ============================================================================ */

/** 버전 정보 싱글톤 */
static const version_info_t s_version_info = {
    .major = FW_VERSION_MAJOR,
    .minor = FW_VERSION_MINOR,
    .patch = FW_VERSION_PATCH,
    .release_type = RELEASE_TYPE,
    .version_string = FW_VERSION_FULL,
    .build_date = BUILD_DATE,
    .build_time = BUILD_TIME,
    .git_hash = GIT_COMMIT_HASH,
    .product_name = PRODUCT_NAME,
    .hw_version = HW_VERSION,
};

/* ============================================================================
 * 함수 구현
 * ============================================================================ */

const version_info_t* version_get_info(void)
{
    return &s_version_info;
}

const char* version_get_string(void)
{
    return FW_VERSION_FULL;
}

int version_get_build_info(char* buffer, size_t size)
{
    if (buffer == NULL || size == 0) {
        return -1;
    }

    return snprintf(buffer, size,
        "%s %s (%s %s) [%s]",
        PRODUCT_NAME,
        FW_VERSION_FULL,
        BUILD_DATE,
        BUILD_TIME,
        GIT_COMMIT_HASH);
}

void version_print_info(void)
{
    printf("\r\n");
    printf("========================================\r\n");
    printf(" %s\r\n", PRODUCT_NAME);
    printf("========================================\r\n");
    printf(" Firmware    : %s\r\n", FW_VERSION_FULL);
    printf(" Build Date  : %s %s\r\n", BUILD_DATE, BUILD_TIME);
    printf(" Git Commit  : %s\r\n", GIT_COMMIT_HASH);
    printf(" Hardware    : %s\r\n", HW_VERSION);
    printf(" Manufacturer: %s\r\n", MANUFACTURER);
    printf("========================================\r\n");
    printf("\r\n");
}

int version_compare(uint8_t major, uint8_t minor, uint8_t patch)
{
    uint32_t current = FW_VERSION_NUMBER;
    uint32_t compare = (major << 16) | (minor << 8) | patch;

    if (current > compare) {
        return 1;
    } else if (current < compare) {
        return -1;
    }
    return 0;
}
