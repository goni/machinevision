# 설치 가이드

이 문서는 HikRobot MV-CH120-10GC 카메라 제어를 위한 상세한 설치 가이드입니다.

## 목차

1. [시스템 요구사항](#시스템-요구사항)
2. [HikRobot MVS SDK 설치](#hikrobot-mvs-sdk-설치)
3. [네트워크 설정](#네트워크-설정)
4. [Python 환경 설정](#python-환경-설정)
5. [C++ 빌드 환경 설정](#c-빌드-환경-설정)
6. [설치 확인](#설치-확인)

## 시스템 요구사항

### 하드웨어
- **카메라**: HikRobot MV-CH120-10GC (GigE Vision 호환)
- **네트워크**: GigE 네트워크 인터페이스 카드
- **RAM**: 최소 4GB (8GB 권장)
- **저장공간**: 최소 500MB

### 소프트웨어
- **OS**: Ubuntu 18.04 / 20.04 / 22.04 LTS
- **Python**: 3.6 이상 (Python 사용 시)
- **GCC**: 7.5 이상 (C++ 사용 시)
- **CMake**: 3.10 이상 (C++ 빌드 시)

## HikRobot MVS SDK 설치

### 1. SDK 다운로드

1. HikRobot 공식 웹사이트 방문:
   - https://www.hikrobotics.com/en/machinevision/service/download

2. "Machine Vision Industrial Cameras" → "Software" 선택

3. "MVS (Linux)" 다운로드:
   - 최신 버전 권장 (예: MVS_STD_GML_V2.1.2)
   - 파일 형식: `.zip` 또는 `.tar.gz`

### 2. SDK 압축 해제

```bash
# zip 파일인 경우
unzip MVS_STD_GML_V2.1.2_231116.zip
cd MVS_STD_GML_V2.1.2_231116

# tar.gz 파일인 경우
tar -xzvf MVS_STD_GML_V2.1.2_231116.tar.gz
cd MVS_STD_GML_V2.1.2_231116
```

### 3. 설치 스크립트 실행

```bash
# 실행 권한 부여
chmod +x setup.sh

# 설치 (관리자 권한 필요)
sudo ./setup.sh
```

설치 과정에서 다음을 확인하세요:
- ✓ 설치 경로: `/opt/MVS`
- ✓ 라이브러리 경로: `/opt/MVS/lib/64`
- ✓ 헤더 파일 경로: `/opt/MVS/include`

### 4. 환경 변수 설정

```bash
# 현재 세션에서만 적용
export LD_LIBRARY_PATH=/opt/MVS/lib/64:$LD_LIBRARY_PATH

# 영구적으로 적용 (.bashrc에 추가)
echo 'export LD_LIBRARY_PATH=/opt/MVS/lib/64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### 5. SDK 설치 확인

```bash
# 라이브러리 파일 확인
ls -la /opt/MVS/lib/64/libMvCameraControl.so

# 헤더 파일 확인
ls -la /opt/MVS/include/MvCameraControl.h

# 예제 프로그램 확인 (있는 경우)
ls -la /opt/MVS/Samples
```

## 네트워크 설정

GigE Vision 카메라는 이더넷을 통해 연결되므로 올바른 네트워크 설정이 필요합니다.

### 1. 카메라 기본 IP 확인

일반적인 HikRobot 카메라 기본 설정:
- **IP 주소**: 192.168.1.64
- **서브넷 마스크**: 255.255.255.0
- **게이트웨이**: 192.168.1.1

### 2. PC 네트워크 설정

#### 방법 1: 명령줄 (임시)

```bash
# 네트워크 인터페이스 확인
ip addr
# 또는
ifconfig

# 예: eth0 인터페이스에 IP 설정
sudo ip addr add 192.168.1.100/24 dev eth0
sudo ip link set eth0 up

# MTU 크기 설정 (Jumbo Frames - 선택사항)
sudo ip link set eth0 mtu 9000
```

#### 방법 2: NetworkManager GUI (영구적)

```bash
# NetworkManager 설정 도구 실행
nm-connection-editor
```

설정:
1. 카메라가 연결된 이더넷 인터페이스 선택
2. "IPv4 Settings" 탭 선택
3. Method: "Manual" 선택
4. "Add" 클릭하여 다음 입력:
   - **Address**: 192.168.1.100
   - **Netmask**: 255.255.255.0
   - **Gateway**: 192.168.1.1
5. "Save" 클릭

#### 방법 3: Netplan (Ubuntu 18.04+)

```bash
# Netplan 설정 파일 생성/수정
sudo nano /etc/netplan/01-camera-network.yaml
```

파일 내용:
```yaml
network:
  version: 2
  renderer: networkd
  ethernets:
    eth0:  # 카메라가 연결된 인터페이스
      addresses:
        - 192.168.1.100/24
      nameservers:
        addresses: [8.8.8.8, 8.8.4.4]
      # Jumbo Frames (선택사항)
      mtu: 9000
```

적용:
```bash
sudo netplan apply
```

### 3. 연결 테스트

```bash
# 카메라 Ping 테스트
ping 192.168.1.64

# 성공 시 출력:
# 64 bytes from 192.168.1.64: icmp_seq=1 ttl=64 time=0.5 ms
```

### 4. 방화벽 설정

```bash
# UFW 상태 확인
sudo ufw status

# 카메라 서브넷 허용
sudo ufw allow from 192.168.1.0/24

# 또는 특정 포트만 허용 (GigE Vision은 일반적으로 UDP 3956)
sudo ufw allow 3956/udp
```

## Python 환경 설정

### 1. Python 버전 확인

```bash
python3 --version
# Python 3.6 이상 필요
```

### 2. pip 설치/업그레이드

```bash
sudo apt update
sudo apt install python3-pip

# pip 업그레이드
pip3 install --upgrade pip
```

### 3. 필수 패키지 설치

```bash
# 프로젝트 디렉토리로 이동
cd /path/to/machinevision

# requirements.txt에서 설치
pip3 install -r requirements.txt

# 또는 개별 설치
pip3 install numpy
pip3 install opencv-python  # 선택사항
```

### 4. 가상 환경 사용 (권장)

```bash
# 가상 환경 생성
python3 -m venv venv

# 가상 환경 활성화
source venv/bin/activate

# 패키지 설치
pip install -r requirements.txt

# 사용 후 비활성화
deactivate
```

## C++ 빌드 환경 설정

### 1. 빌드 도구 설치

```bash
sudo apt update
sudo apt install build-essential cmake git
```

### 2. OpenCV 설치 (선택사항)

#### 방법 1: APT를 통한 설치 (간단)

```bash
sudo apt install libopencv-dev python3-opencv
```

#### 방법 2: 소스에서 빌드 (최신 버전)

```bash
# 의존성 설치
sudo apt install cmake git libgtk2.0-dev pkg-config \
    libavcodec-dev libavformat-dev libswscale-dev

# OpenCV 다운로드
git clone https://github.com/opencv/opencv.git
cd opencv
git checkout 4.8.0  # 원하는 버전

# 빌드
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc)
sudo make install
```

### 3. 프로젝트 빌드

```bash
# 프로젝트 디렉토리로 이동
cd /path/to/machinevision

# 빌드 디렉토리 생성
mkdir build && cd build

# CMake 실행
cmake ..

# 또는 OpenCV 없이 빌드
cmake -DUSE_OPENCV=OFF ..

# 컴파일
make -j$(nproc)

# 성공 시 생성되는 파일:
# - libhikrobot_camera.so (공유 라이브러리)
# - cpp_example (예제 실행 파일)
```

### 4. 설치 (선택사항)

```bash
# 시스템 전역 설치
sudo make install

# 설치 위치:
# - /usr/local/lib/libhikrobot_camera.so
# - /usr/local/bin/cpp_example
# - /usr/local/include/HikRobotCamera.h
```

## 설치 확인

### 1. SDK 라이브러리 확인

```bash
# 라이브러리 로드 테스트
ldd /opt/MVS/lib/64/libMvCameraControl.so

# 심볼 확인
nm -D /opt/MVS/lib/64/libMvCameraControl.so | grep MV_CC
```

### 2. Python 모듈 테스트

```bash
cd /path/to/machinevision

# Python 모듈 임포트 테스트
python3 -c "import sys; sys.path.insert(0, 'python'); from hikvision_camera import HikRobotCamera; print('✓ Python module loaded successfully')"
```

### 3. C++ 빌드 테스트

```bash
cd /path/to/machinevision/build

# 예제 실행 파일 확인
./cpp_example --help

# 라이브러리 의존성 확인
ldd ./cpp_example
```

### 4. 카메라 연결 테스트

```bash
# Python 예제 실행
python3 examples/python_example.py

# C++ 예제 실행
./build/cpp_example

# 성공 시 출력 예:
# ✓ MVS SDK loaded from: /opt/MVS/lib/64/libMvCameraControl.so
# ✓ Found 1 device(s)
#   Device 0:
#     Model: MV-CH120-10GC
#     Serial Number: 00A12345678
```

## 문제 해결

### SDK를 찾을 수 없음

```bash
# SDK 경로 확인
ls -la /opt/MVS

# 없으면 재설치
sudo ./setup.sh
```

### 라이브러리 로드 오류

```bash
# 동적 링커 캐시 갱신
sudo ldconfig

# LD_LIBRARY_PATH 확인
echo $LD_LIBRARY_PATH
```

### 카메라를 찾을 수 없음

1. 물리적 연결 확인
2. 네트워크 설정 확인
3. Ping 테스트
4. 방화벽 설정 확인

### 권한 오류

```bash
# 사용자를 필요한 그룹에 추가
sudo usermod -a -G dialout,video $USER

# 로그아웃 후 재로그인
```

## 다음 단계

설치가 완료되었으면:
1. [README.md](../README.md)의 사용법 섹션 참조
2. [examples/](../examples/) 디렉토리의 예제 코드 실행
3. 카메라 파라미터 조정 및 최적화

## 추가 리소스

- [HikRobot 공식 문서](https://www.hikrobotics.com/)
- [GigE Vision 표준](https://www.emva.org/)
- [Ubuntu 네트워크 설정 가이드](https://ubuntu.com/server/docs/network-configuration)
