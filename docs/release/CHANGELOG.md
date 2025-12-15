# Changelog (변경 이력)

> GPS RTK Rover 펌웨어 버전별 변경 내역

이 문서는 [Semantic Versioning](https://semver.org/lang/ko/) 및 [Keep a Changelog](https://keepachangelog.com/ko/) 형식을 따릅니다.

---

## [Unreleased] - 개발 중

### Added (추가)
- 생산 테스트 기능 추가 (`production_test.c/h`)
- 시리얼 번호 관리 기능 추가 (`serial_number.c/h`)
- 펌웨어 버전 관리 시스템 (`version.h`)

### Changed (변경)
- (예정)

### Fixed (수정)
- (예정)

### Deprecated (지원 중단 예정)
- (없음)

### Removed (제거)
- (없음)

### Security (보안)
- (없음)

---

## [1.0.0] - 2024-12-15

### 개요

첫 번째 양산 버전 릴리즈.
GPS RTK 기지국/로버 시스템의 기본 기능을 포함합니다.

### Added (추가)

#### 핵심 기능
- **GPS RTK 지원**
  - u-blox F9P 모듈 지원
  - UniCore UM982 모듈 지원
  - NMEA 및 UBX 프로토콜 파싱
  - RTCM 3.x 메시지 처리

- **LTE 통신 (EC25)**
  - AT 명령어 기반 제어
  - TCP/IP 소켓 통신 (최대 12개)
  - NTRIP 클라이언트 기능
  - DMA 기반 고속 데이터 수신

- **LoRa P2P 통신 (RAK3172)**
  - 920.9 MHz P2P 모드
  - RTCM 프래그먼트 전송/수신
  - CRC24Q 검증
  - 10-15km 통신 거리

- **기타**
  - BLE 모듈 지원
  - RS485 통신 지원
  - Flash 파라미터 저장
  - LED 상태 표시

#### 보드 지원
- `BOARD_TYPE_BASE_F9P`: F9P 기지국
- `BOARD_TYPE_BASE_UM982`: UM982 기지국
- `BOARD_TYPE_ROVER_UBLOX`: u-blox 로버
- `BOARD_TYPE_ROVER_UNICORE`: UniCore 로버

### 하드웨어 요구사항

- MCU: STM32F405RG
- PCB: REV_A 이상
- 입력 전압: DC 10.5V ~ 14.5V

### 알려진 이슈

1. **GPS Cold Start 지연**
   - 증상: 최초 부팅 시 GPS 위성 획득까지 최대 60초 소요
   - 영향: 실내 테스트 시 GPS 테스트 타임아웃 가능
   - 해결: 테스트 환경에서 GPS 안테나 연결 필요

2. **LTE 연결 재시도**
   - 증상: 네트워크 상태에 따라 LTE 연결 시 간헐적 재시도 필요
   - 영향: 초기 연결 시간 증가 (최대 30초)
   - 해결: 자동 재시도 로직으로 복구됨

3. **LoRa 수신 감도**
   - 증상: SF7 설정 시 장거리 통신 시 패킷 손실 가능
   - 영향: 10km 이상 거리에서 수신율 저하
   - 해결: SF 값 조정 고려 (SF9 권장)

### 업그레이드 지침

- 이전 버전 없음 (최초 릴리즈)

---

## 버전 번호 규칙

```
MAJOR.MINOR.PATCH (예: 1.2.3)

MAJOR: 하위 호환이 안 되는 큰 변경
       - 프로토콜 변경
       - API 변경
       - 하드웨어 변경 대응

MINOR: 하위 호환되는 기능 추가
       - 새로운 기능
       - 성능 개선
       - 새 하드웨어 지원

PATCH: 하위 호환되는 버그 수정
       - 버그 수정
       - 안정성 개선
       - 문서 수정
```

---

## 릴리즈 프로세스

1. `develop` 브랜치에서 기능 개발 완료
2. 버전 번호 업데이트 (`version.h`)
3. CHANGELOG 업데이트
4. `release/vX.Y.Z` 브랜치 생성
5. QA 테스트 진행
6. `main` 브랜치에 머지
7. Git 태그 생성 (`vX.Y.Z`)
8. 릴리즈 바이너리 생성
9. 릴리즈 노트 발행

---

## 링크

- [1.0.0]: 최초 릴리즈
- [Unreleased]: 개발 중
