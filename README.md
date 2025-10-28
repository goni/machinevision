# HikRobot Machine Vision Camera Controller

HikRobot MV-CH120-10GC 카메라를 Ubuntu 환경에서 Python과 C++로 제어하기 위한 SDK wrapper 라이브러리입니다.

## 목차

- [기능](#기능)
- [요구사항](#요구사항)
- [설치](#설치)
- [사용법](#사용법)
  - [Python 사용법](#python-사용법)
  - [C++ 사용법](#c-사용법)
- [예제](#예제)
- [문제 해결](#문제-해결)
- [라이센스](#라이센스)

## 기능

- ✓ 카메라 장치 검색 및 열거
- ✓ 카메라 연결/해제
- ✓ 실시간 이미지 취득
- ✓ 카메라 파라미터 설정 (노출, 게인 등)
- ✓ 연속 캡처 모드
- ✓ 소프트웨어 트리거 모드
- ✓ Python/C++ 지원
- ✓ OpenCV 통합 (선택사항)

## 요구사항

### 하드웨어
- HikRobot MV-CH120-10GC 카메라 (또는 호환 모델)
- GigE 네트워크 인터페이스
- Ubuntu 18.04 / 20.04 / 22.04

### 소프트웨어
- HikRobot MVS SDK (Machine Vision Software)
- Python 3.6+ (Python 사용 시)
- GCC 7.5+ (C++ 사용 시)
- CMake 3.10+ (C++ 빌드 시)
- OpenCV 3.0+ (선택사항, 이미지 디스플레이/저장용)

## 설치

### 1. HikRobot MVS SDK 설치

1. HikRobot 공식 웹사이트에서 MVS SDK 다운로드:
   - https://www.hikrobotics.com/en/machinevision/service/download

2. Linux용 SDK 다운로드 (예: MVS_STD_GML_V2.1.2_231116.zip)

3. SDK 설치:
```bash
# 다운로드한 파일 압축 해제
unzip MVS_STD_GML_V2.1.2_231116.zip
cd MVS_STD_GML_V2.1.2_231116

# 설치 스크립트 실행
sudo chmod +x setup.sh
sudo ./setup.sh
```

4. 설치 확인:
```bash
# SDK가 /opt/MVS에 설치되었는지 확인
ls -la /opt/MVS
```

### 2. 네트워크 설정

카메라와 PC가 같은 서브넷에 있어야 합니다:

```bash
# 네트워크 인터페이스 확인
ip addr

# 카메라 IP 대역에 맞게 PC IP 설정
# 예: 카메라가 192.168.1.64이면 PC를 192.168.1.100으로 설정
sudo ip addr add 192.168.1.100/24 dev eth0

# 또는 GUI 설정 사용:
nm-connection-editor
```

### 3. Python 환경 설정

```bash
# 필요한 패키지 설치
pip3 install numpy

# OpenCV 설치 (선택사항)
pip3 install opencv-python
```

### 4. C++ 빌드 설정

```bash
# 프로젝트 디렉토리로 이동
cd /path/to/machinevision

# 빌드 디렉토리 생성
mkdir build && cd build

# CMake 실행
cmake ..

# 컴파일
make -j$(nproc)

# 설치 (선택사항)
sudo make install
```

## 사용법

### Python 사용법

#### 기본 사용 예제

```python
from hikvision_camera import HikRobotCamera

# 카메라 인스턴스 생성
camera = HikRobotCamera()

# 장치 검색
device_count = camera.enumerate_devices()

# 첫 번째 장치 열기
camera.open_device(0)

# 파라미터 설정
camera.set_float_value("ExposureTime", 10000)  # 10ms
camera.set_float_value("Gain", 5.0)             # 5dB

# 이미지 취득 시작
camera.start_grabbing()

# 이미지 캡처
image = camera.get_image(timeout_ms=1000)
if image is not None:
    print(f"Image shape: {image.shape}")
    # OpenCV를 사용한 이미지 처리/저장
    # cv2.imwrite("captured.png", image)

# 취득 중지
camera.stop_grabbing()

# 장치 닫기
camera.close_device()
```

#### 예제 실행

```bash
# 기본 예제 (5프레임 캡처)
python3 examples/python_example.py

# 연속 캡처 모드
python3 examples/python_example.py --continuous
```

### C++ 사용법

#### 기본 사용 예제

```cpp
#include "HikRobotCamera.h"

using namespace HikRobot;

int main() {
    Camera camera;

    // 장치 검색
    camera.enumerateDevices();

    // 첫 번째 장치 열기
    camera.openDevice(0);

    // 파라미터 설정
    camera.setFloatValue("ExposureTime", 10000.0f);
    camera.setFloatValue("Gain", 5.0f);

    // 이미지 취득 시작
    camera.startGrabbing();

    // 이미지 캡처
    unsigned char* pData = nullptr;
    unsigned int dataSize = 0;
    MV_FRAME_OUT_INFO_EX frameInfo;

    if (camera.getImage(pData, dataSize, frameInfo, 1000)) {
        std::cout << "Image: " << frameInfo.nWidth << "x"
                  << frameInfo.nHeight << std::endl;
        delete[] pData;
    }

    // 취득 중지
    camera.stopGrabbing();

    // 장치 닫기
    camera.closeDevice();

    return 0;
}
```

#### 예제 실행

```bash
# 빌드 후 실행 파일 경로로 이동
cd build

# 기본 예제 (5프레임 캡처)
./cpp_example

# 연속 캡처 모드
./cpp_example --continuous

# 소프트웨어 트리거 모드
./cpp_example --trigger
```

## 예제

### Python 예제

#### 1. 단일 이미지 캡처
```python
camera = HikRobotCamera()
camera.enumerate_devices()
camera.open_device(0)
camera.start_grabbing()

image = camera.get_image()
if image is not None:
    cv2.imwrite("snapshot.png", image)

camera.stop_grabbing()
camera.close_device()
```

#### 2. 연속 캡처
```python
camera = HikRobotCamera()
camera.enumerate_devices()
camera.open_device(0)
camera.start_grabbing()

try:
    while True:
        image = camera.get_image(timeout_ms=1000)
        if image is not None:
            cv2.imshow("Camera", image)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
except KeyboardInterrupt:
    pass

camera.stop_grabbing()
camera.close_device()
cv2.destroyAllWindows()
```

### C++ 예제

#### 1. 파라미터 조회
```cpp
Camera camera;
camera.openDevice(0);

// 정수 값 조회
int64_t width, height;
camera.getIntValue("Width", width);
camera.getIntValue("Height", height);
std::cout << "Resolution: " << width << "x" << height << std::endl;

// 실수 값 조회
float exposure, gain;
camera.getFloatValue("ExposureTime", exposure);
camera.getFloatValue("Gain", gain);
std::cout << "Exposure: " << exposure << " μs" << std::endl;
std::cout << "Gain: " << gain << " dB" << std::endl;
```

#### 2. 소프트웨어 트리거
```cpp
Camera camera;
camera.openDevice(0);

// 트리거 모드 설정
camera.setEnumValue("TriggerMode", 1);     // On
camera.setEnumValue("TriggerSource", 7);   // Software

camera.startGrabbing();

// 트리거 실행
camera.executeCommand("TriggerSoftware");

// 이미지 취득
unsigned char* pData = nullptr;
unsigned int dataSize = 0;
MV_FRAME_OUT_INFO_EX frameInfo;
camera.getImage(pData, dataSize, frameInfo, 2000);

delete[] pData;
camera.stopGrabbing();
camera.closeDevice();
```

## 주요 파라미터

| 파라미터 이름 | 타입 | 설명 | 범위 |
|-------------|------|------|------|
| ExposureTime | Float | 노출 시간 (μs) | 모델에 따라 다름 |
| Gain | Float | 게인 (dB) | 0.0 - 24.0 |
| Width | Int | 이미지 너비 | 카메라 해상도에 따름 |
| Height | Int | 이미지 높이 | 카메라 해상도에 따름 |
| TriggerMode | Enum | 트리거 모드 | 0=Off, 1=On |
| TriggerSource | Enum | 트리거 소스 | 7=Software |
| PixelFormat | Enum | 픽셀 포맷 | Mono8, RGB8 등 |

## 문제 해결

### 카메라를 찾을 수 없음

```bash
# 1. 카메라 전원 확인
# 2. 네트워크 케이블 연결 확인

# 3. 네트워크 인터페이스 확인
ip addr

# 4. 카메라와 같은 서브넷인지 확인
# 카메라 기본 IP: 192.168.1.64 (일반적)
ping 192.168.1.64

# 5. 방화벽 설정 확인
sudo ufw status
sudo ufw allow from 192.168.1.0/24
```

### SDK 라이브러리를 찾을 수 없음

```bash
# 1. SDK 설치 확인
ls -la /opt/MVS/lib/64/libMvCameraControl.so

# 2. 환경 변수 설정
export LD_LIBRARY_PATH=/opt/MVS/lib/64:$LD_LIBRARY_PATH

# 3. .bashrc에 추가 (영구적)
echo 'export LD_LIBRARY_PATH=/opt/MVS/lib/64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### Python에서 SDK를 로드할 수 없음

```python
# SDK 경로 확인 및 수정
camera = HikRobotCamera(sdk_path="/opt/MVS/lib/64/libMvCameraControl.so")
```

### 권한 오류

```bash
# 사용자를 dialout 그룹에 추가
sudo usermod -a -G dialout $USER

# 재로그인 필요
logout
```

### 이미지 타임아웃

```python
# 타임아웃 시간 증가
image = camera.get_image(timeout_ms=5000)  # 5초

# 네트워크 MTU 크기 확인 및 조정
ip link show
sudo ip link set eth0 mtu 9000  # Jumbo frames 활성화
```

## 프로젝트 구조

```
machinevision/
├── CMakeLists.txt          # CMake 빌드 설정
├── README.md               # 본 문서
├── include/                # C++ 헤더 파일
│   └── HikRobotCamera.h
├── cpp/                    # C++ 구현 파일
│   └── HikRobotCamera.cpp
├── python/                 # Python 모듈
│   └── hikvision_camera.py
├── examples/               # 예제 코드
│   ├── python_example.py
│   └── cpp_example.cpp
└── docs/                   # 추가 문서
```

## 참고 자료

- [HikRobot 공식 웹사이트](https://www.hikrobotics.com/)
- [MVS SDK 다운로드](https://www.hikrobotics.com/en/machinevision/service/download)
- [GigE Vision 표준](https://www.emva.org/standards-technology/genicam/)

## 라이센스

이 프로젝트는 MIT 라이센스 하에 배포됩니다.

## 기여

버그 리포트, 기능 요청, Pull Request는 언제든지 환영합니다!

## 연락처

문의사항이 있으시면 Issue를 등록해주세요.
