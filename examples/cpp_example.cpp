/**
 * @file cpp_example.cpp
 * @brief HikRobot MV-CH120-10GC Camera - C++ Example
 *
 * This example demonstrates basic camera operations:
 * - Device enumeration
 * - Camera connection
 * - Image acquisition
 * - Parameter control
 * - Image saving (with OpenCV)
 */

#include "HikRobotCamera.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <ctime>

// Optional OpenCV support for image saving/display
#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>
#endif

using namespace HikRobot;

/**
 * @brief Save raw image data to PGM file (portable graymap)
 */
bool saveImageAsPGM(const std::string& filename,
                    unsigned char* pData,
                    unsigned int width,
                    unsigned int height)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    // Write PGM header
    file << "P5\n" << width << " " << height << "\n255\n";

    // Write image data
    file.write(reinterpret_cast<char*>(pData), width * height);

    file.close();
    std::cout << "✓ Image saved as: " << filename << std::endl;
    return true;
}

/**
 * @brief Basic example - capture a few frames
 */
void basicExample()
{
    std::cout << "============================================================" << std::endl;
    std::cout << "HikRobot MV-CH120-10GC Camera Control Example (C++)" << std::endl;
    std::cout << "============================================================" << std::endl;

    Camera camera;

    try {
        // Step 1: Enumerate devices
        std::cout << "\n[Step 1] Enumerating devices..." << std::endl;
        unsigned int deviceCount = camera.enumerateDevices();

        if (deviceCount == 0) {
            std::cerr << "\n✗ No cameras found!" << std::endl;
            std::cerr << "  Please check:" << std::endl;
            std::cerr << "  1. Camera is powered on" << std::endl;
            std::cerr << "  2. Network cable is connected" << std::endl;
            std::cerr << "  3. Camera and PC are on the same subnet" << std::endl;
            return;
        }

        // Step 2: Open device
        std::cout << "\n[Step 2] Opening device 0..." << std::endl;
        if (!camera.openDevice(0)) {
            std::cerr << "✗ Failed to open device!" << std::endl;
            return;
        }

        // Step 3: Set camera parameters (optional)
        std::cout << "\n[Step 3] Setting camera parameters..." << std::endl;

        // Set exposure time (in microseconds, e.g., 10000 = 10ms)
        camera.setFloatValue("ExposureTime", 10000.0f);

        // Set gain (0.0 - 24.0 dB typically)
        camera.setFloatValue("Gain", 5.0f);

        // Get current parameters
        float exposure, gain;
        if (camera.getFloatValue("ExposureTime", exposure)) {
            std::cout << "  Current exposure time: " << exposure << " μs" << std::endl;
        }
        if (camera.getFloatValue("Gain", gain)) {
            std::cout << "  Current gain: " << gain << " dB" << std::endl;
        }

        // Step 4: Start grabbing
        std::cout << "\n[Step 4] Starting image acquisition..." << std::endl;
        if (!camera.startGrabbing()) {
            camera.closeDevice();
            return;
        }

        // Step 5: Grab images
        std::cout << "\n[Step 5] Grabbing images (5 frames)..." << std::endl;

        for (int i = 0; i < 5; i++) {
            std::cout << "\n  Grabbing frame " << (i+1) << "/5..." << std::endl;

            unsigned char* pData = nullptr;
            unsigned int dataSize = 0;
            MV_FRAME_OUT_INFO_EX frameInfo;

            if (camera.getImage(pData, dataSize, frameInfo, 2000)) {
                std::cout << "    Image size: " << frameInfo.nWidth << "x"
                          << frameInfo.nHeight << std::endl;
                std::cout << "    Data size: " << dataSize << " bytes" << std::endl;

                // Save first frame
                if (i == 0) {
                    // Get current timestamp for filename
                    auto now = std::chrono::system_clock::now();
                    auto timestamp = std::chrono::system_clock::to_time_t(now);

#ifdef USE_OPENCV
                    // Save using OpenCV if available
                    if (frameInfo.enPixelType == PixelType_Gvsp_Mono8) {
                        cv::Mat image(frameInfo.nHeight, frameInfo.nWidth, CV_8UC1, pData);
                        std::string filename = "captured_image_" + std::to_string(timestamp) + ".png";
                        cv::imwrite(filename, image);
                        std::cout << "    ✓ Image saved as: " << filename << std::endl;

                        // Display image (optional)
                        cv::imshow("HikRobot Camera", image);
                        cv::waitKey(1000); // Show for 1 second
                        cv::destroyAllWindows();
                    }
#else
                    // Save as PGM if OpenCV not available
                    if (frameInfo.enPixelType == PixelType_Gvsp_Mono8) {
                        std::string filename = "captured_image_" + std::to_string(timestamp) + ".pgm";
                        saveImageAsPGM(filename, pData, frameInfo.nWidth, frameInfo.nHeight);
                    }
#endif
                }

                // Free allocated memory
                delete[] pData;
            }

            // Small delay between captures
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Step 6: Stop grabbing
        std::cout << "\n[Step 6] Stopping image acquisition..." << std::endl;
        camera.stopGrabbing();

        // Step 7: Close device
        std::cout << "\n[Step 7] Closing device..." << std::endl;
        camera.closeDevice();

        std::cout << "\n============================================================" << std::endl;
        std::cout << "✓ Example completed successfully!" << std::endl;
        std::cout << "============================================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\n✗ Exception occurred: " << e.what() << std::endl;
    }
}

/**
 * @brief Continuous capture example
 */
void continuousCaptureExample()
{
    std::cout << "============================================================" << std::endl;
    std::cout << "HikRobot Camera - Continuous Capture Mode" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    std::cout << "============================================================" << std::endl;

    Camera camera;

    // Setup camera
    if (camera.enumerateDevices() == 0) {
        std::cerr << "No cameras found!" << std::endl;
        return;
    }

    if (!camera.openDevice(0)) {
        std::cerr << "Failed to open device!" << std::endl;
        return;
    }

    if (!camera.startGrabbing()) {
        camera.closeDevice();
        return;
    }

    std::cout << "\n✓ Continuous capture started..." << std::endl;
    std::cout << "  Press Ctrl+C to stop" << std::endl;

    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();

    // Capture loop (in real application, use signal handler for Ctrl+C)
    for (int i = 0; i < 100; i++) {  // Limited to 100 frames for demo
        unsigned char* pData = nullptr;
        unsigned int dataSize = 0;
        MV_FRAME_OUT_INFO_EX frameInfo;

        if (camera.getImage(pData, dataSize, frameInfo, 1000)) {
            frameCount++;

            // Calculate FPS every 10 frames
            if (frameCount % 10 == 0) {
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    currentTime - startTime).count();
                float fps = (frameCount * 1000.0f) / elapsed;
                std::cout << "  Frame " << frameCount << " - FPS: " << fps << std::endl;
            }

#ifdef USE_OPENCV
            // Display with OpenCV
            if (frameInfo.enPixelType == PixelType_Gvsp_Mono8) {
                cv::Mat image(frameInfo.nHeight, frameInfo.nWidth, CV_8UC1, pData);
                cv::putText(image, "Frame: " + std::to_string(frameCount),
                           cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                           1.0, cv::Scalar(255), 2);
                cv::imshow("HikRobot Camera - Continuous", image);

                char key = cv::waitKey(1);
                if (key == 'q' || key == 27) {  // 'q' or ESC to quit
                    delete[] pData;
                    break;
                } else if (key == 's') {  // 's' to save
                    auto now = std::chrono::system_clock::now();
                    auto timestamp = std::chrono::system_clock::to_time_t(now);
                    std::string filename = "captured_" + std::to_string(timestamp) + ".png";
                    cv::imwrite(filename, image);
                    std::cout << "  ✓ Saved: " << filename << std::endl;
                }
            }
#endif

            delete[] pData;
        }
    }

    camera.stopGrabbing();
    camera.closeDevice();

#ifdef USE_OPENCV
    cv::destroyAllWindows();
#endif

    std::cout << "\n✓ Captured " << frameCount << " frames total" << std::endl;
}

/**
 * @brief Software trigger example
 */
void softwareTriggerExample()
{
    std::cout << "============================================================" << std::endl;
    std::cout << "HikRobot Camera - Software Trigger Mode" << std::endl;
    std::cout << "============================================================" << std::endl;

    Camera camera;

    if (camera.enumerateDevices() == 0) {
        std::cerr << "No cameras found!" << std::endl;
        return;
    }

    if (!camera.openDevice(0)) {
        std::cerr << "Failed to open device!" << std::endl;
        return;
    }

    // Set trigger mode
    std::cout << "\n✓ Setting trigger mode..." << std::endl;
    camera.setEnumValue("TriggerMode", 1);  // 1 = On
    camera.setEnumValue("TriggerSource", 7); // 7 = Software

    if (!camera.startGrabbing()) {
        camera.closeDevice();
        return;
    }

    std::cout << "\n✓ Capturing 5 frames with software trigger..." << std::endl;

    for (int i = 0; i < 5; i++) {
        std::cout << "\n  Triggering frame " << (i+1) << "..." << std::endl;

        // Send software trigger command
        camera.executeCommand("TriggerSoftware");

        // Get image
        unsigned char* pData = nullptr;
        unsigned int dataSize = 0;
        MV_FRAME_OUT_INFO_EX frameInfo;

        if (camera.getImage(pData, dataSize, frameInfo, 2000)) {
            std::cout << "    ✓ Received frame: " << frameInfo.nWidth << "x"
                      << frameInfo.nHeight << std::endl;
            delete[] pData;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Turn off trigger mode
    camera.setEnumValue("TriggerMode", 0);  // 0 = Off

    camera.stopGrabbing();
    camera.closeDevice();

    std::cout << "\n✓ Software trigger example completed!" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc > 1) {
        std::string mode(argv[1]);
        if (mode == "--continuous" || mode == "-c") {
            continuousCaptureExample();
        } else if (mode == "--trigger" || mode == "-t") {
            softwareTriggerExample();
        } else {
            std::cout << "Usage: " << argv[0] << " [--continuous|-c] [--trigger|-t]" << std::endl;
            std::cout << "  --continuous, -c : Continuous capture mode" << std::endl;
            std::cout << "  --trigger, -t    : Software trigger mode" << std::endl;
            std::cout << "  (no arguments)   : Basic example (capture 5 frames)" << std::endl;
            return 1;
        }
    } else {
        basicExample();
    }

    return 0;
}
