#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HikRobot MV-CH120-10GC Camera - Python Example

This script demonstrates basic camera operations:
- Device enumeration
- Camera connection
- Image acquisition
- Parameter control
- Image saving
"""

import sys
import os
import time

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'python'))

from hikvision_camera import HikRobotCamera

try:
    import cv2
    HAS_OPENCV = True
except ImportError:
    HAS_OPENCV = False
    print("⚠ OpenCV not installed. Image display and saving will be limited.")


def main():
    print("=" * 60)
    print("HikRobot MV-CH120-10GC Camera Control Example")
    print("=" * 60)

    # Create camera instance
    camera = HikRobotCamera()

    try:
        # Step 1: Enumerate devices
        print("\n[Step 1] Enumerating devices...")
        device_count = camera.enumerate_devices()

        if device_count == 0:
            print("\n✗ No cameras found!")
            print("  Please check:")
            print("  1. Camera is powered on")
            print("  2. Network cable is connected")
            print("  3. Camera and PC are on the same subnet")
            return

        # Step 2: Open device
        print("\n[Step 2] Opening device 0...")
        if not camera.open_device(0):
            print("✗ Failed to open device!")
            return

        # Step 3: Set camera parameters (optional)
        print("\n[Step 3] Setting camera parameters...")

        # Set exposure time (in microseconds, e.g., 10000 = 10ms)
        camera.set_float_value("ExposureTime", 10000)

        # Set gain (0.0 - 24.0 dB typically)
        camera.set_float_value("Gain", 5.0)

        # Get current parameters
        exposure = camera.get_float_value("ExposureTime")
        gain = camera.get_float_value("Gain")
        if exposure is not None:
            print(f"  Current exposure time: {exposure} μs")
        if gain is not None:
            print(f"  Current gain: {gain} dB")

        # Step 4: Start grabbing
        print("\n[Step 4] Starting image acquisition...")
        if not camera.start_grabbing():
            camera.close_device()
            return

        # Step 5: Grab images
        print("\n[Step 5] Grabbing images (5 frames)...")
        for i in range(5):
            print(f"\n  Grabbing frame {i+1}/5...")
            image = camera.get_image(timeout_ms=2000)

            if image is not None:
                print(f"    Image shape: {image.shape}")
                print(f"    Image dtype: {image.dtype}")

                # Save image if OpenCV is available
                if HAS_OPENCV and i == 0:  # Save only the first frame
                    filename = f"captured_image_{int(time.time())}.png"
                    cv2.imwrite(filename, image)
                    print(f"    ✓ Image saved as: {filename}")

                    # Display image (optional - press any key to continue)
                    print(f"    Displaying image... (press any key to continue)")
                    cv2.imshow('HikRobot Camera', image)
                    cv2.waitKey(1000)  # Show for 1 second
                    cv2.destroyAllWindows()

            time.sleep(0.1)  # Small delay between captures

        # Step 6: Stop grabbing
        print("\n[Step 6] Stopping image acquisition...")
        camera.stop_grabbing()

        # Step 7: Close device
        print("\n[Step 7] Closing device...")
        camera.close_device()

        print("\n" + "=" * 60)
        print("✓ Example completed successfully!")
        print("=" * 60)

    except KeyboardInterrupt:
        print("\n\n⚠ Interrupted by user")
        if camera.handle:
            camera.stop_grabbing()
            camera.close_device()
    except Exception as e:
        print(f"\n✗ Error occurred: {e}")
        import traceback
        traceback.print_exc()
        if camera.handle:
            try:
                camera.stop_grabbing()
            except:
                pass
            try:
                camera.close_device()
            except:
                pass


def continuous_capture():
    """
    Continuous capture mode - grab images continuously until Ctrl+C
    """
    print("=" * 60)
    print("HikRobot Camera - Continuous Capture Mode")
    print("Press Ctrl+C to stop")
    print("=" * 60)

    camera = HikRobotCamera()

    try:
        # Setup camera
        if camera.enumerate_devices() == 0:
            print("No cameras found!")
            return

        if not camera.open_device(0):
            print("Failed to open device!")
            return

        if not camera.start_grabbing():
            camera.close_device()
            return

        print("\n✓ Continuous capture started...")
        frame_count = 0

        while True:
            image = camera.get_image(timeout_ms=1000)

            if image is not None:
                frame_count += 1

                if HAS_OPENCV:
                    # Display FPS on image
                    display_image = image.copy() if len(image.shape) == 3 else cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)
                    cv2.putText(display_image, f"Frame: {frame_count}",
                               (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                    cv2.imshow('HikRobot Camera - Continuous', display_image)

                    key = cv2.waitKey(1)
                    if key == ord('q') or key == 27:  # 'q' or ESC to quit
                        break
                    elif key == ord('s'):  # 's' to save
                        filename = f"captured_{int(time.time())}.png"
                        cv2.imwrite(filename, image)
                        print(f"  ✓ Saved: {filename}")
                else:
                    print(f"  Frame {frame_count}: {image.shape}")

    except KeyboardInterrupt:
        print("\n⚠ Stopping...")
    finally:
        if camera.handle:
            camera.stop_grabbing()
            camera.close_device()
        if HAS_OPENCV:
            cv2.destroyAllWindows()

        print(f"\n✓ Captured {frame_count} frames total")


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='HikRobot Camera Python Example')
    parser.add_argument('--continuous', '-c', action='store_true',
                       help='Continuous capture mode')
    args = parser.parse_args()

    if args.continuous:
        continuous_capture()
    else:
        main()
