# Team02 - NIGHT HUNT

> **비대칭 멀티플레이 공포 서바이벌 게임**  
> 2명의 생존자 vs 1명의 살인마

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.5-313131?style=flat-square)
![C++](https://img.shields.io/badge/C++-00599C?style=flat-square)
![Multiplayer](https://img.shields.io/badge/Multiplayer-3P-red?style=flat-square)

---

## 📌 프로젝트 개요

**Dead by Daylight** 영감의 3인 비대칭 PvP 게임

- **생존자 목표**: 6개 키 수집 → 포탈 활성화 → 120초 내 탈출
- **살인마 목표**: 제한 시간 내 모든 생존자 처치
- **특징**: 어두운 시야, 손전등 기반 공포 연출

---

## 👥 팀 구성

| 이름 | 역할 | 담당 |
|------|------|------|
| 김원종 | Leader | UI, GameMode |
| 박준현 | Deputy | Killer 시스템 |
| 권익환 | Member | Survivor 시스템 |

---

## 🎮 핵심 기능

### 생존자 (Survivor)
- 🔦 **손전등**: 배터리 기반 시야 확보
- 🎒 **인벤토리**: 5칸 (키/포션/배터리)
- 🤝 **상호작용**: 홀드 방식 아이템 수집
- ❤️ **체력**: HP 100

### 살인마 (Killer)
- ⚔️ **공격**: 40 데미지, Box Sweep 판정
- 🏃 **대쉬**: 5초 쿨다운, FOV 전환 효과
- 🪤 **함정**: 15초 쿨다운, 속도↓ + 위치 노출
- 👁️ **비전**: 디버프 걸린 생존자 아우라 표시

### 게임 흐름
```
게임 시작 → 역할 랜덤 배정 (1K + 2S)
         ↓
키 6개 수집 (진행도 UI)
         ↓
포탈 생성 (120초 제한)
         ↓
탈출 성공/실패 → 결과 화면 → 타이틀
```

---

## 🏗️ 기술 스택

### 아키텍처
```
Listen Server (P2P)
- Host: Server + Client
- Players: Client 1, Client 2
```

### 핵심 클래스
```cpp
// 게임 모드
AT2GameModeBase        // 타이틀/로비
AT2PlayGameMod         // 게임플레이 (역할 배정, 승패 판정)

// 캐릭터
AT2BaseCharacter
  ├─ AT2KillerCharacter    // 공격, 대쉬, 함정
  └─ AT2PlayerCharacter    // 손전등, 상호작용, 인벤토리

// 상태 관리
AT2PlayerState           // 역할 (Killer/Survivor)
ASurvivorPlayerState     // HP, 인벤토리, 디버프
AT2PlayGameState         // 키 카운트, 포탈 상태, 매치 결과

// UI
AT2BaseController        // UI 생성/관리, 관전 모드
```

---

## 💡 주요 구현

### 1️⃣ Survivor - 상호작용 시스템
```cpp
// 2단계 감지
Overlap (범위)  →  LineTrace (정확도)
                   ↓
              홀드형 수집 (E키)
                   ↓
            서버 RPC → 인벤토리 추가
```

**동시 수집 방지**:
- `InteractingPC` 변수로 점유 관리
- 서버에서만 획득 판정

### 2️⃣ Killer - 공격 시스템
```cpp
Client Input  →  Server RPC (bIsAttacking = true)
                      ↓
              Multicast: 애니메이션 재생
                      ↓
          AnimNotify: Box Sweep 판정 (서버)
                      ↓
              HitActors 배열로 중복 방지
```

**핵심**: Socket Trace → Box Sweep으로 정확도 개선

### 3️⃣ UI - 역할별 HUD
```cpp
// Controller에서 관리 (Dedicated Server 대응)
OnRep_PlayerRole() → UpdateHUDForRole()
                          ↓
              Killer HUD / Survivor HUD 전환
```

### 4️⃣ GameMode - 역할 배정
```cpp
AssignRolesIfReady() {
    // 1. 플레이어 셔플
    // 2. 첫 번째 → Killer, 나머지 → Survivor
    // 3. 스폰 지점 태그로 구분 (KillerSpawn/SurvivorSpawn)
    // 4. Pawn 생성 및 빙의
}
```

---

## 🐛 트러블슈팅

| 파트 | 문제 | 해결 |
|------|------|------|
| **Survivor** | 서버 Crouch 실행X | 입력(클라) + 승인(서버 RPC) 분리 |
| **Killer** | 공격 다단히트 | `HitActors` 배열로 중복 체크 |
| **Killer** | 함정 중복 발동 | `bIsExploded` 플래그 추가 |
| **UI** | GameMode에서 UI 안 보임 | Controller에서 UI 관리로 변경 |
| **GameMode** | OpenLevel 시 클라 이탈 | `ServerTravel`로 변경 |

**핵심 교훈**:
- 서버/클라 권한 구조를 항상 먼저 고려
- 멀티플레이는 `ServerTravel/ClientTravel` 필수
- UI는 Controller, 로직은 GameMode/PlayerState

---

## 🚀 실행 방법

### PIE 멀티플레이 테스트
1. **Edit → Editor Preferences → Play**
2. **Number of Players: 3**
3. **Alt + P** 실행

### 조작법
| 키 | 생존자 | 살인마 |
|----|--------|--------|
| **WASD** | 이동 | 이동 |
| **E** | 상호작용 | - |
| **F** | 손전등 | - |
| **1~5** | 아이템 사용 | - |
| **클릭** | - | 공격 |
| **Space** | - | 대쉬 |
| **Q** | - | 함정 |

---

## 📁 프로젝트 구조

```
Source/Team02/
├── Character/
│   ├── KillerCharacter/      # 공격, 대쉬, 함정
│   └── PlayerCharacter/       # 손전등, 상호작용
├── Component/
│   ├── FlashlightComponent    # 손전등 로직
│   ├── T2CooldownComponent    # 쿨다운 관리
│   └── InventoryComponent     # 인벤토리 시스템
├── GameMode/
│   ├── T2GameModeBase         # 타이틀/로비
│   └── T2PlayGameMod          # 게임플레이
├── PlayerState/
│   └── SurvivorPlayerState    # HP, 디버프
├── Gimmick/
│   ├── ItemBase               # 아이템 베이스
│   └── Portal/PortalActor     # 탈출 포탈
└── UI/
    ├── UW_KillerHUD
    └── UW_SurvivorHUD
```

---

<div align="center">

**Built with ❤️ using Unreal Engine 5.5**

</div>
