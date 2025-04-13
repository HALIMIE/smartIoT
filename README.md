# 🏠 스마트 IoT 홈 시스템 | Smart IoT Home System

> 다양한 센서와 모듈을 기반으로 **조도·온습도 모니터링**, **자동 제어**,  
> **모바일 연동**, **실시간 데이터 시각화**가 가능한 **IoT 기반 스마트 홈 시스템**입니다.

---

## 📌 프로젝트 개요

본 프로젝트는 사용자의 주거 환경을 보다 편리하고 효율적으로 만들기 위한  
**IoT 자동 제어 스마트 홈 시스템**으로, 다양한 센서 데이터를 실시간으로 수집하고  
조건에 따라 조명을 제어하거나 가전을 자동으로 동작시킵니다.  
수집된 데이터는 **MariaDB에 저장**되며, **Android 앱 및 LCD**를 통해 시각화됩니다.

---

## 🛠️ 사용 기술 스택

| 분류           | 상세 내용 |
|----------------|----------|
| 언어/프로그래밍 | C, Python, PHP, HTML, JavaScript |
| 하드웨어        | STM32, Arduino MEGA, Raspberry Pi |
| 통신 방식       | Bluetooth, Wi-Fi, UART, I2C |
| 센서 및 모듈    | 조도센서(CdS), 온습도센서, 서보모터, LCD |
| DB 및 서버      | MariaDB, Raspberry Pi Web Server |
| 앱 연동         | Android 앱과 Wi-Fi 통신 기반 실시간 연동 |
| UI              | LCD 출력 / 안드로이드 앱 시각화 / 웹 인터페이스 |

---

## 🧩 시스템 구조
[ Arduino: 센서 수집 & Bluetooth 전송 ] ↕ Bluetooth [ STM32: LCD 시각화 + 모터 제어 ] ↕ Wi-Fi [ Raspberry Pi: 데이터 수신, 저장, Android 연동, MariaDB 처리 ]

## 🎯 핵심 기능

- ✅ **센서 기반 자동 제어**
  - 조도 100Lux 이하 → 전등 자동 ON  
  - 온도 28℃ 이상 → 에어컨 작동  
  - 습도 70% 이상 → 제습기 작동

- ✅ **실시간 센서 데이터 시각화**
  - STM32 LCD에 현재 조도, 온도, 습도 표시  
  - Android 앱에서 실시간 상태 확인

- ✅ **자동문 출입 제어**
  - 사용자 인증 후 서보모터를 통한 출입문 자동 개폐

- ✅ **Raspberry Pi 서버 기능**
  - 센서 데이터 수신 및 MariaDB 저장  
  - PHP/HTML로 웹 대시보드 구현  
  - Android 앱과 데이터 송수신

---

## ⚠️ 문제점 및 해결 방법

| 문제 | 해결 방법 |
|------|-----------|
| Wi-Fi 모듈 불안정으로 STM32 연결 실패 | → Bluetooth 모듈로 대체하여 통신 안정성 확보 |
| 다양한 모듈 연결 시 회로 및 명령어 충돌 | → 개별 모듈 별 테스트 후 통합 / 명령어 구분 정리 |
| 네트워크 부하로 인한 데이터 지연 및 리셋 현상 | → Raspberry Pi 서버 리소스 최적화 / 데이터 전송 주기 조정 |
| 센서 값 정밀도 부족 및 간헐적 오류 | → 값 필터링 및 오차 범위 설정으로 정제된 데이터 처리 |

---

## 배운 점

- 하드웨어 제어 + 네트워크 통신 + 데이터베이스 + 프론트 UI까지  
  **IoT 시스템의 전체 흐름을 설계하고 구현하는 경험**을 쌓을 수 있었습니다.

- **블루투스와 Wi-Fi 통신의 특성 차이**,  
  **센서 오류 처리 방법**, **데이터 정규화 및 시각화**,  
  **실시간 IoT 기기 상태 모니터링의 중요성**을 직접 체감했습니다.

- **서버-클라이언트 구조** 및 **통신 병목 해결 방식**,  
  **하드웨어 간 통합 시 발생하는 충돌 문제 해결 경험**을 통해  
  실전적인 임베디드 시스템 구축 능력을 키웠습니다.



---

## 📈 향후 발전 방향

- MQTT 기반 통신으로 통신 안정성 향상  
- 다양한 IoT 디바이스 연결 확장 (에너지 관리, 보안 장치 등)  
- AI 기반 사용자 패턴 예측 및 자동 제어 알고리즘 추가  
- 모바일 앱 UX 개선 및 음성 제어 연동 시도

---

> ⚙️ **센서 → 통신 → 처리 → 제어 → 시각화**까지  
> 하나의 시스템 흐름을 직접 구현하며 **IoT 시스템의 핵심 구조를 완성한 경험**이었습니다.
