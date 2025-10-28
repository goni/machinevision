/**
 * @file HikRobotCamera.h
 * @brief HikRobot Machine Vision Camera Controller (MV-CH120-10GC)
 *
 * C++ wrapper class for HikRobot MVS SDK
 * Provides easy-to-use interface for camera control and image acquisition
 */

#ifndef HIKROBOT_CAMERA_H
#define HIKROBOT_CAMERA_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "MvCameraControl.h"

namespace HikRobot {

/**
 * @class Camera
 * @brief Main camera controller class
 *
 * Usage:
 *   Camera cam;
 *   cam.enumerateDevices();
 *   cam.openDevice(0);
 *   cam.startGrabbing();
 *   cv::Mat image = cam.getImage();
 *   cam.stopGrabbing();
 *   cam.closeDevice();
 */
class Camera {
public:
    /**
     * @brief Constructor
     */
    Camera();

    /**
     * @brief Destructor - automatically closes device if open
     */
    ~Camera();

    /**
     * @brief Enumerate all available HikRobot cameras
     * @return Number of devices found
     */
    unsigned int enumerateDevices();

    /**
     * @brief Get device information
     * @param index Device index
     * @param modelName Output: Model name
     * @param serialNumber Output: Serial number
     * @param userDefinedName Output: User defined name
     * @return true if successful
     */
    bool getDeviceInfo(unsigned int index,
                       std::string& modelName,
                       std::string& serialNumber,
                       std::string& userDefinedName);

    /**
     * @brief Open camera device
     * @param index Device index (0-based)
     * @param accessMode Access mode (default: exclusive)
     * @return true if successful
     */
    bool openDevice(unsigned int index = 0,
                    unsigned int accessMode = MV_ACCESS_Exclusive);

    /**
     * @brief Start image acquisition
     * @return true if successful
     */
    bool startGrabbing();

    /**
     * @brief Stop image acquisition
     * @return true if successful
     */
    bool stopGrabbing();

    /**
     * @brief Get one frame from camera
     * @param pData Output buffer for image data
     * @param dataSize Size of output buffer
     * @param frameInfo Output: Frame information
     * @param timeoutMs Timeout in milliseconds (default: 1000)
     * @return true if successful
     */
    bool getImageBuffer(unsigned char* pData,
                        unsigned int dataSize,
                        MV_FRAME_OUT_INFO_EX& frameInfo,
                        unsigned int timeoutMs = 1000);

    /**
     * @brief Get one frame as allocated buffer
     * @param outData Output: Pointer to image data (must be freed by caller)
     * @param outSize Output: Size of image data
     * @param frameInfo Output: Frame information
     * @param timeoutMs Timeout in milliseconds (default: 1000)
     * @return true if successful
     */
    bool getImage(unsigned char*& outData,
                  unsigned int& outSize,
                  MV_FRAME_OUT_INFO_EX& frameInfo,
                  unsigned int timeoutMs = 1000);

    /**
     * @brief Set integer parameter
     * @param paramName Parameter name (e.g., "Width", "Height")
     * @param value Parameter value
     * @return true if successful
     */
    bool setIntValue(const std::string& paramName, int64_t value);

    /**
     * @brief Get integer parameter
     * @param paramName Parameter name
     * @param value Output: Parameter value
     * @return true if successful
     */
    bool getIntValue(const std::string& paramName, int64_t& value);

    /**
     * @brief Set float parameter
     * @param paramName Parameter name (e.g., "Gain", "ExposureTime")
     * @param value Parameter value
     * @return true if successful
     */
    bool setFloatValue(const std::string& paramName, float value);

    /**
     * @brief Get float parameter
     * @param paramName Parameter name
     * @param value Output: Parameter value
     * @return true if successful
     */
    bool getFloatValue(const std::string& paramName, float& value);

    /**
     * @brief Set enum parameter
     * @param paramName Parameter name
     * @param value Parameter value
     * @return true if successful
     */
    bool setEnumValue(const std::string& paramName, unsigned int value);

    /**
     * @brief Get enum parameter
     * @param paramName Parameter name
     * @param value Output: Parameter value
     * @return true if successful
     */
    bool getEnumValue(const std::string& paramName, unsigned int& value);

    /**
     * @brief Set boolean parameter
     * @param paramName Parameter name
     * @param value Parameter value
     * @return true if successful
     */
    bool setBoolValue(const std::string& paramName, bool value);

    /**
     * @brief Get boolean parameter
     * @param paramName Parameter name
     * @param value Output: Parameter value
     * @return true if successful
     */
    bool getBoolValue(const std::string& paramName, bool& value);

    /**
     * @brief Set string parameter
     * @param paramName Parameter name
     * @param value Parameter value
     * @return true if successful
     */
    bool setStringValue(const std::string& paramName, const std::string& value);

    /**
     * @brief Get string parameter
     * @param paramName Parameter name
     * @param value Output: Parameter value
     * @return true if successful
     */
    bool getStringValue(const std::string& paramName, std::string& value);

    /**
     * @brief Execute command
     * @param commandName Command name (e.g., "TriggerSoftware")
     * @return true if successful
     */
    bool executeCommand(const std::string& commandName);

    /**
     * @brief Close camera device
     * @return true if successful
     */
    bool closeDevice();

    /**
     * @brief Check if device is open
     * @return true if device is open
     */
    bool isOpen() const { return m_handle != nullptr; }

    /**
     * @brief Get last error code
     * @return Error code
     */
    unsigned int getLastError() const { return m_lastError; }

    /**
     * @brief Convert error code to string
     * @param errorCode Error code
     * @return Error description
     */
    static std::string errorToString(unsigned int errorCode);

private:
    void* m_handle;                          ///< Camera handle
    MV_CC_DEVICE_INFO_LIST m_deviceList;    ///< Device list
    unsigned int m_lastError;                ///< Last error code
    bool m_isGrabbing;                       ///< Grabbing state

    // Disable copy
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
};

} // namespace HikRobot

#endif // HIKROBOT_CAMERA_H
