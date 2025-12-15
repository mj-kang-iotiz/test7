# Release Note - vX.Y.Z

> GPS RTK Rover 펌웨어 릴리즈 노트 템플릿

---

## 릴리즈 정보

| 항목 | 내용 |
|------|------|
| **버전** | vX.Y.Z |
| **릴리즈 날짜** | YYYY-MM-DD |
| **빌드 번호** | XXX |
| **Git Commit** | abc1234 |
| **릴리즈 타입** | 정식 / RC / Beta / Alpha |

---

## 요약

(1-2문장으로 이번 릴리즈의 주요 내용 요약)

예시:
> 이번 릴리즈는 LTE 연결 안정성을 개선하고, 새로운 GPS 모듈을 지원합니다.

---

## 새로운 기능 (New Features)

### 기능 1: (기능 이름)

- **설명**: (기능에 대한 설명)
- **사용 방법**: (사용 방법 또는 설정 방법)
- **관련 설정**: (관련 설정 파라미터)

### 기능 2: (기능 이름)

- **설명**:
- **사용 방법**:
- **관련 설정**:

---

## 개선 사항 (Improvements)

- [ ] (개선 내용 1)
- [ ] (개선 내용 2)
- [ ] (개선 내용 3)

---

## 버그 수정 (Bug Fixes)

### BUG-001: (버그 제목)

- **증상**: (버그로 인해 발생하던 증상)
- **원인**: (버그의 원인)
- **수정 내용**: (수정 방법)
- **영향받는 버전**: vX.Y.Z 이전

### BUG-002: (버그 제목)

- **증상**:
- **원인**:
- **수정 내용**:
- **영향받는 버전**:

---

## 알려진 이슈 (Known Issues)

### ISSUE-001: (이슈 제목)

- **증상**: (발생하는 증상)
- **영향**: (이 이슈로 인한 영향)
- **회피 방법**: (임시 해결 방법, 있다면)
- **해결 예정**: (해결 예정 버전)

### ISSUE-002: (이슈 제목)

- **증상**:
- **영향**:
- **회피 방법**:
- **해결 예정**:

---

## 호환성 (Compatibility)

### 하드웨어 요구사항

| 항목 | 요구사항 |
|------|----------|
| MCU | STM32F405RG |
| PCB 버전 | REV_A 이상 |
| GPS 모듈 | F9P / UM982 |
| LTE 모듈 | EC25 펌웨어 v2.0+ |
| LoRa 모듈 | RAK3172 펌웨어 v1.0+ |

### 소프트웨어 호환성

| 항목 | 호환 버전 |
|------|----------|
| 이전 펌웨어 | vX.Y.Z 이상 |
| 설정 파라미터 | 호환 / 재설정 필요 |
| NTRIP 프로토콜 | v1.0, v2.0 |

### 하위 호환성

- [ ] 이전 버전과 완전 호환
- [ ] 설정 마이그레이션 필요
- [ ] 재플래싱 필요

---

## 업그레이드 지침 (Upgrade Instructions)

### 사전 요구사항

1. 현재 펌웨어 버전 확인: vX.Y.Z 이상
2. 설정 백업 (필요시)
3. ST-Link 또는 OTA 업데이트 환경

### 업그레이드 절차

#### 방법 1: ST-Link 플래싱

```bash
# 1. 펌웨어 다운로드
# firmware_vX.Y.Z.bin

# 2. 플래싱
./flash_linux.sh firmware_vX.Y.Z.bin
```

#### 방법 2: OTA 업데이트 (지원 시)

```
AT+FWUPDATE=<서버URL>
```

### 업그레이드 후 확인

1. 버전 확인: `AT+VERSION`
2. 기능 테스트
3. 설정 복원 (필요시)

### 롤백 방법

문제 발생 시 이전 버전으로 롤백:

```bash
./flash_linux.sh firmware_vX.Y.Z_previous.bin
```

---

## 테스트 결과 (Test Results)

### 테스트 환경

| 항목 | 내용 |
|------|------|
| 테스트 보드 | 10대 |
| 테스트 기간 | YYYY-MM-DD ~ YYYY-MM-DD |
| 테스터 | (담당자) |

### 테스트 항목

| 테스트 항목 | 결과 | 비고 |
|------------|------|------|
| 부팅 테스트 | PASS | |
| LTE 연결 | PASS | |
| LoRa 통신 | PASS | |
| GPS 수신 | PASS | |
| 장시간 안정성 | PASS | 72시간 |
| 온도 테스트 | PASS | -10~60°C |

---

## 변경된 파일 (Changed Files)

<details>
<summary>변경된 파일 목록 (클릭하여 펼치기)</summary>

```
M config/version.h
M lib/gsm/gsm.c
M lib/lora/lora.c
A lib/production_test/production_test.c
A lib/production_test/production_test.h
```

</details>

---

## 빌드 정보 (Build Information)

```
Toolchain: ARM GCC 10.3.1
IDE: STM32CubeIDE 1.12.0
HAL: STM32Cube_FW_F4 V1.27.1
FreeRTOS: V10.4.6

Binary Size:
  .text   : XXX KB
  .data   : XXX KB
  .bss    : XXX KB
  Total   : XXX KB (XX% of 1MB Flash)
```

---

## 다운로드 (Downloads)

| 파일 | 설명 | 크기 | SHA256 |
|------|------|------|--------|
| firmware_vX.Y.Z.bin | 펌웨어 바이너리 | XXX KB | abc123... |
| firmware_vX.Y.Z.hex | HEX 파일 | XXX KB | def456... |
| release_notes_vX.Y.Z.pdf | 릴리즈 노트 PDF | XXX KB | - |

---

## 문의

- **기술 지원**: support@example.com
- **버그 리포트**: https://github.com/xxx/issues

---

## 승인

| 역할 | 이름 | 서명 | 날짜 |
|------|------|------|------|
| 개발 | | | |
| QA | | | |
| 승인 | | | |

---

*이 릴리즈 노트는 자동 생성된 템플릿입니다. 각 섹션을 실제 내용으로 채워주세요.*
