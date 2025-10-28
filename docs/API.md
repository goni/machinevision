# API 레퍼런스

HikRobot Camera Controller의 상세한 API 문서입니다.

## Python API

### HikRobotCamera 클래스

#### 초기화

```python
HikRobotCamera(sdk_path="/opt/MVS/lib/64/libMvCameraControl.so")
```

**매개변수:**
- `sdk_path` (str): MVS SDK 라이브러리 경로

**예외:**
- `OSError`: SDK 라이브러리를 로드할 수 없을 때

---

#### enumerate_devices()

사용 가능한 모든 HikRobot 카메라를 검색합니다.

```python
device_count = camera.enumerate_devices()
```

**반환값:**
- `int`: 검색된 장치 수

**예제:**
```python
camera = HikRobotCamera()
count = camera.enumerate_devices()
print(f"Found {count} device(s)")
```

---

#### open_device()

카메라 장치를 엽니다.

```python
success = camera.open_device(device_index=0, access_mode=MV_ACCESS_Exclusive)
```

**매개변수:**
- `device_index` (int): 열 장치 인덱스 (기본값: 0)
- `access_mode` (int): 접근 모드 (기본값: MV_ACCESS_Exclusive)
  - `MV_ACCESS_Exclusive` (1): 독점 모드
  - `MV_ACCESS_Control` (3): 제어 모드

**반환값:**
- `bool`: 성공 시 True, 실패 시 False

**예제:**
```python
if camera.open_device(0):
    print("Device opened successfully")
```

---

#### start_grabbing()

이미지 취득을 시작합니다.

```python
success = camera.start_grabbing()
```

**반환값:**
- `bool`: 성공 시 True, 실패 시 False

**예제:**
```python
camera.start_grabbing()
```

---

#### stop_grabbing()

이미지 취득을 중지합니다.

```python
success = camera.stop_grabbing()
```

**반환값:**
- `bool`: 성공 시 True, 실패 시 False

---

#### get_image()

카메라에서 한 프레임을 가져옵니다.

```python
image = camera.get_image(timeout_ms=1000)
```

**매개변수:**
- `timeout_ms` (int): 타임아웃 시간 (밀리초, 기본값: 1000)

**반환값:**
- `numpy.ndarray`: 이미지 데이터, 실패 시 None
  - Mono8: (height, width) 형태의 2D 배열
  - RGB/BGR: (height, width, 3) 형태의 3D 배열

**예제:**
```python
image = camera.get_image(timeout_ms=2000)
if image is not None:
    print(f"Image shape: {image.shape}")
    cv2.imwrite("captured.png", image)
```

---

#### set_float_value()

실수형 파라미터를 설정합니다.

```python
success = camera.set_float_value(param_name, value)
```

**매개변수:**
- `param_name` (str): 파라미터 이름
  - "ExposureTime": 노출 시간 (마이크로초)
  - "Gain": 게인 (dB)
  - "Gamma": 감마 값
- `value` (float): 설정할 값

**반환값:**
- `bool`: 성공 시 True, 실패 시 False

**예제:**
```python
# 노출 시간 10ms 설정
camera.set_float_value("ExposureTime", 10000.0)

# 게인 5dB 설정
camera.set_float_value("Gain", 5.0)
```

---

#### get_float_value()

실수형 파라미터를 조회합니다.

```python
value = camera.get_float_value(param_name)
```

**매개변수:**
- `param_name` (str): 파라미터 이름

**반환값:**
- `float`: 파라미터 값, 실패 시 None

**예제:**
```python
exposure = camera.get_float_value("ExposureTime")
print(f"Current exposure: {exposure} μs")
```

---

#### set_int_value()

정수형 파라미터를 설정합니다.

```python
success = camera.set_int_value(param_name, value)
```

**매개변수:**
- `param_name` (str): 파라미터 이름
  - "Width": 이미지 너비
  - "Height": 이미지 높이
  - "OffsetX": X 오프셋
  - "OffsetY": Y 오프셋
- `value` (int): 설정할 값

**반환값:**
- `bool`: 성공 시 True, 실패 시 False

---

#### get_int_value()

정수형 파라미터를 조회합니다.

```python
value = camera.get_int_value(param_name)
```

**매개변수:**
- `param_name` (str): 파라미터 이름

**반환값:**
- `int`: 파라미터 값, 실패 시 None

---

#### close_device()

카메라 장치를 닫습니다.

```python
success = camera.close_device()
```

**반환값:**
- `bool`: 성공 시 True, 실패 시 False

---

## C++ API

### HikRobot::Camera 클래스

#### 생성자

```cpp
HikRobot::Camera()
```

카메라 컨트롤러 객체를 생성합니다.

---

#### enumerateDevices()

사용 가능한 모든 카메라를 검색합니다.

```cpp
unsigned int enumerateDevices()
```

**반환값:**
- `unsigned int`: 검색된 장치 수

**예제:**
```cpp
HikRobot::Camera camera;
unsigned int count = camera.enumerateDevices();
std::cout << "Found " << count << " device(s)" << std::endl;
```

---

#### openDevice()

카메라 장치를 엽니다.

```cpp
bool openDevice(unsigned int index = 0,
                unsigned int accessMode = MV_ACCESS_Exclusive)
```

**매개변수:**
- `index`: 장치 인덱스 (기본값: 0)
- `accessMode`: 접근 모드 (기본값: MV_ACCESS_Exclusive)

**반환값:**
- `bool`: 성공 시 true

**예제:**
```cpp
if (camera.openDevice(0)) {
    std::cout << "Device opened" << std::endl;
}
```

---

#### startGrabbing()

이미지 취득을 시작합니다.

```cpp
bool startGrabbing()
```

**반환값:**
- `bool`: 성공 시 true

---

#### stopGrabbing()

이미지 취득을 중지합니다.

```cpp
bool stopGrabbing()
```

**반환값:**
- `bool`: 성공 시 true

---

#### getImage()

카메라에서 한 프레임을 가져옵니다.

```cpp
bool getImage(unsigned char*& outData,
              unsigned int& outSize,
              MV_FRAME_OUT_INFO_EX& frameInfo,
              unsigned int timeoutMs = 1000)
```

**매개변수:**
- `outData`: 출력 이미지 데이터 포인터 (호출자가 delete[] 해야 함)
- `outSize`: 출력 이미지 크기
- `frameInfo`: 출력 프레임 정보
- `timeoutMs`: 타임아웃 (밀리초)

**반환값:**
- `bool`: 성공 시 true

**예제:**
```cpp
unsigned char* pData = nullptr;
unsigned int dataSize = 0;
MV_FRAME_OUT_INFO_EX frameInfo;

if (camera.getImage(pData, dataSize, frameInfo, 1000)) {
    std::cout << "Image: " << frameInfo.nWidth << "x"
              << frameInfo.nHeight << std::endl;
    // 이미지 처리...
    delete[] pData;
}
```

---

#### setFloatValue()

실수형 파라미터를 설정합니다.

```cpp
bool setFloatValue(const std::string& paramName, float value)
```

**매개변수:**
- `paramName`: 파라미터 이름
- `value`: 설정할 값

**반환값:**
- `bool`: 성공 시 true

**예제:**
```cpp
camera.setFloatValue("ExposureTime", 10000.0f);
camera.setFloatValue("Gain", 5.0f);
```

---

#### getFloatValue()

실수형 파라미터를 조회합니다.

```cpp
bool getFloatValue(const std::string& paramName, float& value)
```

**매개변수:**
- `paramName`: 파라미터 이름
- `value`: 출력 파라미터 값

**반환값:**
- `bool`: 성공 시 true

**예제:**
```cpp
float exposure;
if (camera.getFloatValue("ExposureTime", exposure)) {
    std::cout << "Exposure: " << exposure << " μs" << std::endl;
}
```

---

#### setIntValue()

정수형 파라미터를 설정합니다.

```cpp
bool setIntValue(const std::string& paramName, int64_t value)
```

---

#### getIntValue()

정수형 파라미터를 조회합니다.

```cpp
bool getIntValue(const std::string& paramName, int64_t& value)
```

---

#### setEnumValue()

열거형 파라미터를 설정합니다.

```cpp
bool setEnumValue(const std::string& paramName, unsigned int value)
```

**예제:**
```cpp
// 트리거 모드 설정
camera.setEnumValue("TriggerMode", 1);  // On
camera.setEnumValue("TriggerSource", 7); // Software
```

---

#### executeCommand()

명령을 실행합니다.

```cpp
bool executeCommand(const std::string& commandName)
```

**매개변수:**
- `commandName`: 실행할 명령 이름
  - "TriggerSoftware": 소프트웨어 트리거 실행
  - "AcquisitionStart": 취득 시작
  - "AcquisitionStop": 취득 중지

**반환값:**
- `bool`: 성공 시 true

**예제:**
```cpp
// 소프트웨어 트리거 실행
camera.executeCommand("TriggerSoftware");
```

---

#### closeDevice()

카메라 장치를 닫습니다.

```cpp
bool closeDevice()
```

**반환값:**
- `bool`: 성공 시 true

---

## 주요 상수 및 열거형

### 접근 모드

```python
# Python
MV_ACCESS_Exclusive = 1          # 독점 접근
MV_ACCESS_Control = 3            # 제어 접근
```

```cpp
// C++
MV_ACCESS_Exclusive = 1          // 독점 접근
MV_ACCESS_Control = 3            // 제어 접근
```

### 픽셀 타입

```python
# Python
PixelType_Gvsp_Mono8 = 0x01080001      # Mono 8-bit
PixelType_Gvsp_RGB8_Packed = 0x02180014 # RGB 8-bit
PixelType_Gvsp_BGR8_Packed = 0x02180015 # BGR 8-bit
```

### 오류 코드

```python
# Python
MV_OK = 0x00000000              # 성공
MV_E_HANDLE = 0x80000000        # 잘못된 핸들
MV_E_PARAMETER = 0x80000004     # 잘못된 파라미터
MV_E_NODATA = 0x80000006        # 데이터 없음
```

## 일반적인 파라미터

### 이미지 관련

| 파라미터 | 타입 | 설명 | 범위 |
|---------|------|------|------|
| Width | Int | 이미지 너비 | 카메라 의존 |
| Height | Int | 이미지 높이 | 카메라 의존 |
| OffsetX | Int | X 오프셋 | 0 ~ (MaxWidth-Width) |
| OffsetY | Int | Y 오프셋 | 0 ~ (MaxHeight-Height) |
| PixelFormat | Enum | 픽셀 포맷 | Mono8, RGB8, etc. |

### 노출 및 게인

| 파라미터 | 타입 | 설명 | 범위 |
|---------|------|------|------|
| ExposureTime | Float | 노출 시간 (μs) | 30 ~ 1000000 |
| ExposureAuto | Enum | 자동 노출 | 0=Off, 1=Once, 2=Continuous |
| Gain | Float | 게인 (dB) | 0.0 ~ 24.0 |
| GainAuto | Enum | 자동 게인 | 0=Off, 1=Once, 2=Continuous |

### 트리거

| 파라미터 | 타입 | 설명 | 값 |
|---------|------|------|------|
| TriggerMode | Enum | 트리거 모드 | 0=Off, 1=On |
| TriggerSource | Enum | 트리거 소스 | 0=Line0, 7=Software |
| TriggerActivation | Enum | 트리거 활성화 | 0=RisingEdge, 1=FallingEdge |

## 예제 코드

### 완전한 Python 예제

```python
from hikvision_camera import HikRobotCamera
import cv2

# 카메라 초기화
camera = HikRobotCamera()

# 장치 검색 및 열기
if camera.enumerate_devices() > 0:
    camera.open_device(0)

    # 파라미터 설정
    camera.set_float_value("ExposureTime", 10000)
    camera.set_float_value("Gain", 5.0)

    # 이미지 취득
    camera.start_grabbing()

    for i in range(10):
        image = camera.get_image()
        if image is not None:
            cv2.imwrite(f"image_{i}.png", image)

    camera.stop_grabbing()
    camera.close_device()
```

### 완전한 C++ 예제

```cpp
#include "HikRobotCamera.h"

int main() {
    HikRobot::Camera camera;

    // 장치 검색 및 열기
    if (camera.enumerateDevices() > 0) {
        camera.openDevice(0);

        // 파라미터 설정
        camera.setFloatValue("ExposureTime", 10000.0f);
        camera.setFloatValue("Gain", 5.0f);

        // 이미지 취득
        camera.startGrabbing();

        for (int i = 0; i < 10; i++) {
            unsigned char* pData = nullptr;
            unsigned int dataSize = 0;
            MV_FRAME_OUT_INFO_EX frameInfo;

            if (camera.getImage(pData, dataSize, frameInfo)) {
                // 이미지 처리...
                delete[] pData;
            }
        }

        camera.stopGrabbing();
        camera.closeDevice();
    }

    return 0;
}
```

## 참고 사항

1. **메모리 관리**: C++에서 `getImage()`로 받은 데이터는 반드시 `delete[]`로 해제해야 합니다.

2. **스레드 안전성**: 하나의 Camera 객체는 단일 스레드에서만 사용하세요.

3. **오류 처리**: 모든 함수는 성공/실패를 반환하므로 항상 확인하세요.

4. **타임아웃**: 네트워크 상태에 따라 적절한 타임아웃 값을 설정하세요.
