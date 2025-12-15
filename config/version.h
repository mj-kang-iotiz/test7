/**
 * @file version.h
 * @brief 펌웨어 버전 관리 헤더
 *
 * 이 파일은 펌웨어 버전, 빌드 정보, 제품 정보를 정의합니다.
 * 양산 시 버전 추적 및 A/S 관리에 사용됩니다.
 *
 * @note 릴리즈 전 VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH를 업데이트하세요.
 */

#ifndef VERSION_H
#define VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 펌웨어 버전 정보
 *
 * Semantic Versioning 규칙:
 *   MAJOR: 하위 호환이 안 되는 큰 변경
 *   MINOR: 하위 호환되는 기능 추가
 *   PATCH: 하위 호환되는 버그 수정
 * ============================================================================ */

#define FW_VERSION_MAJOR        1       /**< 주 버전 (Major) */
#define FW_VERSION_MINOR        0       /**< 부 버전 (Minor) */
#define FW_VERSION_PATCH        0       /**< 패치 버전 (Patch) */

/** 버전 숫자 (비교용): 0x010000 = v1.0.0 */
#define FW_VERSION_NUMBER       ((FW_VERSION_MAJOR << 16) | \
                                 (FW_VERSION_MINOR << 8) | \
                                 FW_VERSION_PATCH)

/* 문자열 변환 매크로 */
#define _STR(x)                 #x
#define STR(x)                  _STR(x)

/** 버전 문자열: "v1.0.0" */
#define FW_VERSION_STRING       "v" STR(FW_VERSION_MAJOR) "." \
                                STR(FW_VERSION_MINOR) "." \
                                STR(FW_VERSION_PATCH)

/* ============================================================================
 * 빌드 정보
 *
 * __DATE__, __TIME__은 컴파일 시점에 자동으로 채워집니다.
 * GIT_COMMIT_HASH는 빌드 스크립트에서 -D 옵션으로 주입합니다.
 * ============================================================================ */

/** 빌드 날짜: "Dec 15 2025" */
#define BUILD_DATE              __DATE__

/** 빌드 시간: "14:30:00" */
#define BUILD_TIME              __TIME__

/** Git 커밋 해시 (빌드 스크립트에서 주입, 없으면 "unknown") */
#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH         "unknown"
#endif

/** Git 커밋 해시 (짧은 버전, 7자리) */
#ifndef GIT_COMMIT_SHORT
#define GIT_COMMIT_SHORT        "unknown"
#endif

/* ============================================================================
 * 제품 정보
 * ============================================================================ */

#define PRODUCT_NAME            "GPS_RTK_ROVER"     /**< 제품 이름 */
#define PRODUCT_CODE            "GRR-100"           /**< 제품 코드 */
#define MANUFACTURER            "IOTIZ"             /**< 제조사 */
#define HW_VERSION              "REV_A"             /**< 하드웨어 버전 */

/* ============================================================================
 * 릴리즈 정보
 * ============================================================================ */

/** 릴리즈 타입 */
typedef enum {
    RELEASE_TYPE_DEV = 0,       /**< 개발 버전 */
    RELEASE_TYPE_ALPHA,         /**< 알파 버전 */
    RELEASE_TYPE_BETA,          /**< 베타 버전 */
    RELEASE_TYPE_RC,            /**< Release Candidate */
    RELEASE_TYPE_RELEASE        /**< 정식 릴리즈 */
} release_type_t;

/** 현재 릴리즈 타입 */
#define RELEASE_TYPE            RELEASE_TYPE_RELEASE

/** 릴리즈 타입 문자열 */
#if RELEASE_TYPE == RELEASE_TYPE_DEV
#define RELEASE_TYPE_STRING     "-dev"
#elif RELEASE_TYPE == RELEASE_TYPE_ALPHA
#define RELEASE_TYPE_STRING     "-alpha"
#elif RELEASE_TYPE == RELEASE_TYPE_BETA
#define RELEASE_TYPE_STRING     "-beta"
#elif RELEASE_TYPE == RELEASE_TYPE_RC
#define RELEASE_TYPE_STRING     "-rc"
#else
#define RELEASE_TYPE_STRING     ""
#endif

/** 전체 버전 문자열 (릴리즈 타입 포함): "v1.0.0" 또는 "v1.0.0-beta" */
#define FW_VERSION_FULL         FW_VERSION_STRING RELEASE_TYPE_STRING

/* ============================================================================
 * 버전 정보 구조체
 * ============================================================================ */

/**
 * @brief 버전 정보 구조체
 */
typedef struct {
    uint8_t major;              /**< 주 버전 */
    uint8_t minor;              /**< 부 버전 */
    uint8_t patch;              /**< 패치 버전 */
    release_type_t release_type;/**< 릴리즈 타입 */
    const char* version_string; /**< 버전 문자열 */
    const char* build_date;     /**< 빌드 날짜 */
    const char* build_time;     /**< 빌드 시간 */
    const char* git_hash;       /**< Git 커밋 해시 */
    const char* product_name;   /**< 제품 이름 */
    const char* hw_version;     /**< 하드웨어 버전 */
} version_info_t;

/* ============================================================================
 * 함수 선언
 * ============================================================================ */

/**
 * @brief 버전 정보 구조체 가져오기
 * @return 버전 정보 구조체 포인터
 */
const version_info_t* version_get_info(void);

/**
 * @brief 버전 정보 출력 (UART로 출력)
 */
void version_print_info(void);

/**
 * @brief 버전 문자열 가져오기
 * @return 버전 문자열 (예: "v1.0.0")
 */
const char* version_get_string(void);

/**
 * @brief 빌드 정보 문자열 가져오기
 * @param buffer 결과를 저장할 버퍼
 * @param size 버퍼 크기
 * @return 작성된 문자열 길이
 */
int version_get_build_info(char* buffer, size_t size);

/**
 * @brief 버전 비교
 * @param major 비교할 주 버전
 * @param minor 비교할 부 버전
 * @param patch 비교할 패치 버전
 * @return 현재 버전이 크면 1, 같으면 0, 작으면 -1
 */
int version_compare(uint8_t major, uint8_t minor, uint8_t patch);

#ifdef __cplusplus
}
#endif

#endif /* VERSION_H */
