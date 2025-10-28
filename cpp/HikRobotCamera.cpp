/**
 * @file HikRobotCamera.cpp
 * @brief Implementation of HikRobot Camera Controller
 */

#include "HikRobotCamera.h"
#include <cstring>
#include <sstream>

namespace HikRobot {

Camera::Camera()
    : m_handle(nullptr)
    , m_lastError(MV_OK)
    , m_isGrabbing(false)
{
    memset(&m_deviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
}

Camera::~Camera()
{
    if (m_handle != nullptr) {
        closeDevice();
    }
}

unsigned int Camera::enumerateDevices()
{
    memset(&m_deviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // Enumerate GigE devices (nTLayerType = MV_GIGE_DEVICE = 1)
    m_lastError = MV_CC_EnumDevices(MV_GIGE_DEVICE, &m_deviceList);

    if (m_lastError != MV_OK) {
        std::cerr << "✗ Enumerate devices failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return 0;
    }

    std::cout << "✓ Found " << m_deviceList.nDeviceNum << " device(s)" << std::endl;

    // Print device information
    for (unsigned int i = 0; i < m_deviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* pDeviceInfo = m_deviceList.pDeviceInfo[i];
        if (pDeviceInfo == nullptr) {
            continue;
        }

        std::cout << "\n  Device " << i << ":" << std::endl;

        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
            // GigE camera
            MV_GIGE_DEVICE_INFO* pGigEInfo = (MV_GIGE_DEVICE_INFO*)(&pDeviceInfo->SpecialInfo.stGigEInfo);

            std::cout << "    Model: " << pDeviceInfo->SpecialInfo.stGigEInfo.chModelName << std::endl;
            std::cout << "    Serial Number: " << pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber << std::endl;
            std::cout << "    User Defined Name: " << pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName << std::endl;

            // Print IP address
            unsigned int nIp = pGigEInfo->nCurrentIp;
            std::cout << "    Current IP: "
                      << ((nIp & 0xff000000) >> 24) << "."
                      << ((nIp & 0x00ff0000) >> 16) << "."
                      << ((nIp & 0x0000ff00) >> 8) << "."
                      << (nIp & 0x000000ff) << std::endl;
        }
    }

    return m_deviceList.nDeviceNum;
}

bool Camera::getDeviceInfo(unsigned int index,
                           std::string& modelName,
                           std::string& serialNumber,
                           std::string& userDefinedName)
{
    if (index >= m_deviceList.nDeviceNum) {
        std::cerr << "✗ Device index out of range!" << std::endl;
        return false;
    }

    MV_CC_DEVICE_INFO* pDeviceInfo = m_deviceList.pDeviceInfo[index];
    if (pDeviceInfo == nullptr) {
        return false;
    }

    if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
        modelName = std::string((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chModelName);
        serialNumber = std::string((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
        userDefinedName = std::string((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
        return true;
    }

    return false;
}

bool Camera::openDevice(unsigned int index, unsigned int accessMode)
{
    if (index >= m_deviceList.nDeviceNum) {
        std::cerr << "✗ Device index " << index << " out of range!" << std::endl;
        return false;
    }

    // Create handle
    m_lastError = MV_CC_CreateHandle(&m_handle, m_deviceList.pDeviceInfo[index]);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Create handle failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    // Open device
    m_lastError = MV_CC_OpenDevice(m_handle, accessMode, 0);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Open device failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        MV_CC_DestroyHandle(m_handle);
        m_handle = nullptr;
        return false;
    }

    std::cout << "✓ Device " << index << " opened successfully" << std::endl;
    return true;
}

bool Camera::startGrabbing()
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    m_lastError = MV_CC_StartGrabbing(m_handle);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Start grabbing failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    m_isGrabbing = true;
    std::cout << "✓ Started grabbing" << std::endl;
    return true;
}

bool Camera::stopGrabbing()
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    m_lastError = MV_CC_StopGrabbing(m_handle);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Stop grabbing failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    m_isGrabbing = false;
    std::cout << "✓ Stopped grabbing" << std::endl;
    return true;
}

bool Camera::getImageBuffer(unsigned char* pData,
                            unsigned int dataSize,
                            MV_FRAME_OUT_INFO_EX& frameInfo,
                            unsigned int timeoutMs)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    memset(&frameInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));

    m_lastError = MV_CC_GetOneFrameTimeout(m_handle, pData, dataSize, &frameInfo, timeoutMs);

    if (m_lastError != MV_OK) {
        if (m_lastError == MV_E_NODATA) {
            std::cerr << "✗ No data received within " << timeoutMs << "ms" << std::endl;
        } else {
            std::cerr << "✗ Get image failed! Error: 0x"
                      << std::hex << m_lastError << std::dec << std::endl;
        }
        return false;
    }

    std::cout << "✓ Got frame " << frameInfo.nFrameNum << ": "
              << frameInfo.nWidth << "x" << frameInfo.nHeight
              << ", pixel_type=0x" << std::hex << frameInfo.enPixelType << std::dec
              << std::endl;

    return true;
}

bool Camera::getImage(unsigned char*& outData,
                      unsigned int& outSize,
                      MV_FRAME_OUT_INFO_EX& frameInfo,
                      unsigned int timeoutMs)
{
    // Allocate buffer (max 10MB)
    const unsigned int MAX_IMAGE_SIZE = 10 * 1024 * 1024;
    outData = new unsigned char[MAX_IMAGE_SIZE];

    if (!getImageBuffer(outData, MAX_IMAGE_SIZE, frameInfo, timeoutMs)) {
        delete[] outData;
        outData = nullptr;
        outSize = 0;
        return false;
    }

    outSize = frameInfo.nFrameLen;
    return true;
}

bool Camera::setIntValue(const std::string& paramName, int64_t value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    m_lastError = MV_CC_SetIntValueEx(m_handle, paramName.c_str(), value);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Set " << paramName << "=" << value
                  << " failed! Error: 0x" << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    std::cout << "✓ Set " << paramName << "=" << value << std::endl;
    return true;
}

bool Camera::getIntValue(const std::string& paramName, int64_t& value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    MVCC_INTVALUE_EX intValue;
    m_lastError = MV_CC_GetIntValueEx(m_handle, paramName.c_str(), &intValue);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Get " << paramName << " failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    value = intValue.nCurValue;
    return true;
}

bool Camera::setFloatValue(const std::string& paramName, float value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    m_lastError = MV_CC_SetFloatValue(m_handle, paramName.c_str(), value);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Set " << paramName << "=" << value
                  << " failed! Error: 0x" << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    std::cout << "✓ Set " << paramName << "=" << value << std::endl;
    return true;
}

bool Camera::getFloatValue(const std::string& paramName, float& value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    MVCC_FLOATVALUE floatValue;
    m_lastError = MV_CC_GetFloatValue(m_handle, paramName.c_str(), &floatValue);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Get " << paramName << " failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    value = floatValue.fCurValue;
    return true;
}

bool Camera::setEnumValue(const std::string& paramName, unsigned int value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    m_lastError = MV_CC_SetEnumValue(m_handle, paramName.c_str(), value);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Set " << paramName << "=" << value
                  << " failed! Error: 0x" << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    std::cout << "✓ Set " << paramName << "=" << value << std::endl;
    return true;
}

bool Camera::getEnumValue(const std::string& paramName, unsigned int& value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    MVCC_ENUMVALUE enumValue;
    m_lastError = MV_CC_GetEnumValue(m_handle, paramName.c_str(), &enumValue);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Get " << paramName << " failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    value = enumValue.nCurValue;
    return true;
}

bool Camera::setBoolValue(const std::string& paramName, bool value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    m_lastError = MV_CC_SetBoolValue(m_handle, paramName.c_str(), value);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Set " << paramName << "=" << (value ? "true" : "false")
                  << " failed! Error: 0x" << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    std::cout << "✓ Set " << paramName << "=" << (value ? "true" : "false") << std::endl;
    return true;
}

bool Camera::getBoolValue(const std::string& paramName, bool& value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    bool boolValue;
    m_lastError = MV_CC_GetBoolValue(m_handle, paramName.c_str(), &boolValue);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Get " << paramName << " failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    value = boolValue;
    return true;
}

bool Camera::setStringValue(const std::string& paramName, const std::string& value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    m_lastError = MV_CC_SetStringValue(m_handle, paramName.c_str(), value.c_str());
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Set " << paramName << "=" << value
                  << " failed! Error: 0x" << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    std::cout << "✓ Set " << paramName << "=" << value << std::endl;
    return true;
}

bool Camera::getStringValue(const std::string& paramName, std::string& value)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    MVCC_STRINGVALUE stringValue;
    m_lastError = MV_CC_GetStringValue(m_handle, paramName.c_str(), &stringValue);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Get " << paramName << " failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    value = std::string(stringValue.chCurValue);
    return true;
}

bool Camera::executeCommand(const std::string& commandName)
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    m_lastError = MV_CC_SetCommandValue(m_handle, commandName.c_str());
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Execute command " << commandName
                  << " failed! Error: 0x" << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    std::cout << "✓ Executed command: " << commandName << std::endl;
    return true;
}

bool Camera::closeDevice()
{
    if (m_handle == nullptr) {
        std::cerr << "✗ Device not opened!" << std::endl;
        return false;
    }

    // Stop grabbing if still grabbing
    if (m_isGrabbing) {
        stopGrabbing();
    }

    // Close device
    m_lastError = MV_CC_CloseDevice(m_handle);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Close device failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
    }

    // Destroy handle
    m_lastError = MV_CC_DestroyHandle(m_handle);
    if (m_lastError != MV_OK) {
        std::cerr << "✗ Destroy handle failed! Error: 0x"
                  << std::hex << m_lastError << std::dec << std::endl;
        return false;
    }

    m_handle = nullptr;
    std::cout << "✓ Device closed" << std::endl;
    return true;
}

std::string Camera::errorToString(unsigned int errorCode)
{
    std::stringstream ss;
    ss << "0x" << std::hex << errorCode;

    switch (errorCode) {
        case MV_OK:
            return "Success";
        case MV_E_HANDLE:
            return "Error: Invalid handle";
        case MV_E_SUPPORT:
            return "Error: Not supported";
        case MV_E_BUFOVER:
            return "Error: Buffer overflow";
        case MV_E_CALLORDER:
            return "Error: Invalid call order";
        case MV_E_PARAMETER:
            return "Error: Invalid parameter";
        case MV_E_RESOURCE:
            return "Error: Resource allocation failed";
        case MV_E_NODATA:
            return "Error: No data";
        case MV_E_UNKNOW:
            return "Error: Unknown error";
        default:
            return "Error code: " + ss.str();
    }
}

} // namespace HikRobot
