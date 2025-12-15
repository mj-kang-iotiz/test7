/**
 * @file serial_number.h
 * @brief 시리얼 번호 관리 모듈 헤더
 *
 * 이 모듈은 제품의 고유 시리얼 번호를 관리합니다.
 * 시리얼 번호는 Flash의 특정 영역에 한 번만 기록되며, 이후 변경할 수 없습니다.
 *
 * ## 시리얼 번호 형식
 *
 * ```
 * GRR-YYMMDD-XXXX
 * │   │      │
 * │   │      └── 일련번호 (0001~9999)
 * │   └───────── 생산일자 (241215 = 2024년 12월 15일)
 * └───────────── 제품 코드 (GPS RTK Rover)
 * ```
 *
 * ## 사용 예시
 *
 * @code
 * // 시리얼 번호 확인
 * if (serial_number_is_set()) {
 *     char sn[32];
 *     serial_number_get(sn, sizeof(sn));
 *     printf("Serial: %s\n", sn);
 * }
 *
 * // 시리얼 번호 설정 (생산 라인에서 1회만)
 * serial_number_set("GRR-241215-0001");
 * @endcode
 *
 * ## Flash 메모리 레이아웃
 *
 * ```
 * 0x080FF000 - 0x080FFFFF (4KB): Device Info 영역
 * ├── Serial Number (32 bytes)
 * ├── Production Date (16 bytes)
 * ├── Hardware Version (8 bytes)
 * ├── Factory Code (8 bytes)
 * ├── MAC Address (8 bytes)
 * ├── Reserved (180 bytes)
 * └── CRC32 (4 bytes)
 * ```
 *
 * @warning 시리얼 번호는 한 번 설정하면 변경할 수 없습니다!
 */

#ifndef SERIAL_NUMBER_H
#define SERIAL_NUMBER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * 설정
 * ============================================================================ */

/** 시리얼 번호 저장 Flash 주소 */
#define SERIAL_NUMBER_FLASH_ADDR    0x080FF000

/** 시리얼 번호 최대 길이 */
#define SERIAL_NUMBER_MAX_LEN       32

/** 생산일자 최대 길이 */
#define PRODUCTION_DATE_MAX_LEN     16

/** 하드웨어 버전 최대 길이 */
#define HW_VERSION_MAX_LEN          8

/** 공장 코드 최대 길이 */
#define FACTORY_CODE_MAX_LEN        8

/** MAC 주소 길이 */
#define MAC_ADDRESS_LEN             6

/** 프로그래밍 되지 않은 Flash 값 */
#define FLASH_ERASED_VALUE          0xFF

/** 시리얼 번호 접두사 */
#define SERIAL_NUMBER_PREFIX        "GRR"

/* ============================================================================
 * 타입 정의
 * ============================================================================ */

/**
 * @brief 결과 코드
 */
typedef enum {
    SERIAL_OK = 0,              /**< 성공 */
    SERIAL_ERROR,               /**< 일반 에러 */
    SERIAL_ALREADY_SET,         /**< 이미 설정됨 */
    SERIAL_INVALID_FORMAT,      /**< 잘못된 형식 */
    SERIAL_FLASH_ERROR,         /**< Flash 에러 */
    SERIAL_NOT_SET,             /**< 설정되지 않음 */
    SERIAL_CRC_ERROR,           /**< CRC 불일치 */
} serial_number_result_t;

/**
 * @brief 디바이스 정보 구조체
 *
 * Flash에 저장되는 구조체 (256 bytes 정렬)
 */
typedef struct __attribute__((packed)) {
    char serial_number[SERIAL_NUMBER_MAX_LEN];      /**< 시리얼 번호 */
    char production_date[PRODUCTION_DATE_MAX_LEN];  /**< 생산일자 (YYYY-MM-DD) */
    char hw_version[HW_VERSION_MAX_LEN];            /**< 하드웨어 버전 */
    char factory_code[FACTORY_CODE_MAX_LEN];        /**< 공장 코드 */
    uint8_t mac_address[MAC_ADDRESS_LEN];           /**< MAC 주소 (BLE용) */
    uint8_t reserved[2];                            /**< 예약 (정렬용) */
    uint32_t production_timestamp;                  /**< 생산 타임스탬프 (Unix) */
    uint32_t first_boot_timestamp;                  /**< 최초 부팅 타임스탬프 */
    uint8_t _reserved[180];                         /**< 예약 영역 */
    uint32_t crc32;                                 /**< CRC32 체크섬 */
} device_info_t;

/* 구조체 크기 검증 */
_Static_assert(sizeof(device_info_t) == 256, "device_info_t must be 256 bytes");

/* ============================================================================
 * 함수 선언
 * ============================================================================ */

/**
 * @brief 시리얼 번호 모듈 초기화
 *
 * Flash에서 디바이스 정보를 읽고 유효성을 검사합니다.
 *
 * @return SERIAL_OK: 성공, SERIAL_NOT_SET: 설정 안 됨, SERIAL_CRC_ERROR: 손상됨
 */
serial_number_result_t serial_number_init(void);

/**
 * @brief 시리얼 번호 설정 여부 확인
 *
 * @return true: 설정됨, false: 설정 안 됨
 */
bool serial_number_is_set(void);

/**
 * @brief 시리얼 번호 가져오기
 *
 * @param[out] buffer 시리얼 번호를 저장할 버퍼
 * @param[in] size 버퍼 크기
 * @return SERIAL_OK: 성공, SERIAL_NOT_SET: 설정 안 됨
 *
 * @code
 * char sn[32];
 * if (serial_number_get(sn, sizeof(sn)) == SERIAL_OK) {
 *     printf("SN: %s\n", sn);
 * }
 * @endcode
 */
serial_number_result_t serial_number_get(char* buffer, size_t size);

/**
 * @brief 시리얼 번호 설정 (1회만 가능)
 *
 * @param[in] serial_number 설정할 시리얼 번호 (예: "GRR-241215-0001")
 * @return SERIAL_OK: 성공, SERIAL_ALREADY_SET: 이미 설정됨
 *
 * @warning 한 번 설정하면 변경할 수 없습니다!
 *
 * @code
 * serial_number_result_t result = serial_number_set("GRR-241215-0001");
 * if (result == SERIAL_ALREADY_SET) {
 *     printf("Already programmed!\n");
 * }
 * @endcode
 */
serial_number_result_t serial_number_set(const char* serial_number);

/**
 * @brief 전체 디바이스 정보 설정 (1회만 가능)
 *
 * @param[in] info 설정할 디바이스 정보
 * @return SERIAL_OK: 성공, SERIAL_ALREADY_SET: 이미 설정됨
 */
serial_number_result_t serial_number_set_full(const device_info_t* info);

/**
 * @brief 디바이스 정보 가져오기
 *
 * @return 디바이스 정보 구조체 포인터 (설정 안 됐으면 NULL)
 */
const device_info_t* serial_number_get_device_info(void);

/**
 * @brief 생산일자 가져오기
 *
 * @param[out] buffer 생산일자를 저장할 버퍼
 * @param[in] size 버퍼 크기
 * @return SERIAL_OK: 성공
 */
serial_number_result_t serial_number_get_production_date(char* buffer, size_t size);

/**
 * @brief MAC 주소 가져오기
 *
 * @param[out] mac MAC 주소를 저장할 버퍼 (6 bytes)
 * @return SERIAL_OK: 성공
 */
serial_number_result_t serial_number_get_mac_address(uint8_t mac[MAC_ADDRESS_LEN]);

/**
 * @brief 디바이스 정보 출력
 */
void serial_number_print_info(void);

/* ============================================================================
 * 유틸리티 함수
 * ============================================================================ */

/**
 * @brief 시리얼 번호 형식 검증
 *
 * 형식: XXX-YYMMDD-NNNN (예: GRR-241215-0001)
 *
 * @param[in] serial_number 검증할 시리얼 번호
 * @return true: 유효, false: 무효
 */
bool serial_number_validate_format(const char* serial_number);

/**
 * @brief 시리얼 번호 생성 (유틸리티)
 *
 * @param[out] buffer 결과를 저장할 버퍼
 * @param[in] size 버퍼 크기
 * @param[in] date 생산일자 (YYMMDD 형식, 예: "241215")
 * @param[in] sequence 일련번호 (1~9999)
 * @return 생성된 시리얼 번호 길이
 *
 * @code
 * char sn[32];
 * serial_number_generate(sn, sizeof(sn), "241215", 1);
 * // sn = "GRR-241215-0001"
 * @endcode
 */
int serial_number_generate(char* buffer, size_t size,
                          const char* date, uint16_t sequence);

/**
 * @brief 결과 코드를 문자열로 변환
 *
 * @param[in] result 결과 코드
 * @return 결과 문자열
 */
const char* serial_number_result_to_string(serial_number_result_t result);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_NUMBER_H */
