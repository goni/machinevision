#!/usr/bin/env python3
"""
HikRobot Camera Capture with Automatic ROOT Analysis
Captures images and runs ROOT analysis after each capture
Analysis results are continuously updated (overwritten)
"""

import sys
import os
import ctypes
import numpy as np
import time
import signal
import argparse
import subprocess
from datetime import datetime

print("=== HikRobot Camera Capture with ROOT Analysis ===\n")

# Setup paths
MVS_SDK_PATH = "/opt/MVS"
sys.path.append(f"{MVS_SDK_PATH}/Samples/64/Python/MvImport")
os.environ['LD_LIBRARY_PATH'] = f"{MVS_SDK_PATH}/lib/64:{os.environ.get('LD_LIBRARY_PATH', '')}"

from MvCameraControl_class import *

# Global variables
cam = None
running = True
config = None

def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully"""
    global running
    print("\n\nReceived interrupt signal, shutting down...")
    running = False

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

def run_root_analysis():
    """Run ROOT analysis on latest captured image"""
    global config
    
    try:
        analysis_dir = os.path.join(config.output, "analysis_results")
        os.makedirs(analysis_dir, exist_ok=True)
        
        root_macro = config.root_macro
        
        if not os.path.exists(root_macro):
            print(f"  ⚠ ROOT macro not found: {root_macro}")
            return False
        
        # Use full path to ROOT executable
        root_cmd = "/home/kobra/root/bin/root"
        if not os.path.exists(root_cmd):
            root_cmd = "root"
        
        # Run ROOT macro
        cmd = f'{root_cmd} -l -b -q \'{root_macro}("{config.output}", "{analysis_dir}")\''
        
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=30)
        
        if result.returncode == 0:
            print(f"  ✓ ROOT analysis completed")
            return True
        else:
            print(f"  ⚠ ROOT analysis failed:")
            if result.stderr:
                print(f"  ERROR: {result.stderr[-200:]}")
            return False
            
    except subprocess.TimeoutExpired:
        print("  ⚠ ROOT analysis timeout")
        return False
    except Exception as e:
        print(f"  ⚠ ROOT analysis error: {e}")
        return False

def init_camera():
    """Initialize and open camera"""
    global cam, config
    
    print("1. Enumerating devices...")
    deviceList = MV_CC_DEVICE_INFO_LIST()
    ret = MvCamera.MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, deviceList)
    
    if ret != MV_OK or deviceList.nDeviceNum == 0:
        print("✗ No devices found")
        return False
    
    print(f"✓ Found {deviceList.nDeviceNum} device(s)")
    
    # Find target camera
    print(f"2. Looking for camera at {config.ip}...")
    target_index = -1
    
    for i in range(deviceList.nDeviceNum):
        device_info = ctypes.cast(
            deviceList.pDeviceInfo[i],
            ctypes.POINTER(MV_CC_DEVICE_INFO)
        ).contents
        
        if device_info.nTLayerType == MV_GIGE_DEVICE:
            gige_info = ctypes.cast(
                ctypes.byref(device_info.SpecialInfo),
                ctypes.POINTER(MV_GIGE_DEVICE_INFO)
            ).contents
            
            nIp = gige_info.nCurrentIp
            ip_str = f"{(nIp >> 24) & 0xff}.{(nIp >> 16) & 0xff}.{(nIp >> 8) & 0xff}.{nIp & 0xff}"
            
            if ip_str == config.ip:
                target_index = i
                print(f"✓ Found target camera at index {i}")
                break
    
    if target_index == -1:
        print(f"✗ Camera {config.ip} not found")
        return False
    
    # Create and open camera
    print("3. Creating camera handle...")
    cam = MvCamera()
    device_info = ctypes.cast(
        deviceList.pDeviceInfo[target_index],
        ctypes.POINTER(MV_CC_DEVICE_INFO)
    ).contents
    
    ret = cam.MV_CC_CreateHandle(device_info)
    if ret != MV_OK:
        print(f"✗ Create handle failed: 0x{ret:08x}")
        return False
    
    print("4. Opening camera...")
    ret = cam.MV_CC_OpenDevice(MV_ACCESS_Exclusive)
    if ret != MV_OK:
        print(f"✗ Open device failed: 0x{ret:08x}")
        cam.MV_CC_DestroyHandle()
        return False
    
    # Set parameters
    print("5. Setting parameters...")
    cam.MV_CC_SetEnumValue("TriggerMode", 0)
    cam.MV_CC_SetEnumValue("PixelFormat", PixelType_Gvsp_Mono8)
    print("✓ Trigger mode: OFF, Pixel format: Mono8")
    
    # Set exposure
    ret = cam.MV_CC_SetEnumValue("ExposureAuto", 0)
    if ret == MV_OK:
        ret = cam.MV_CC_SetFloatValue("ExposureTime", config.exposure)
        if ret == MV_OK:
            print(f"✓ Exposure time: {config.exposure} μs ({config.exposure/1000:.1f} ms)")
        else:
            print(f"⚠ Set exposure warning: 0x{ret:08x}")
    
    # Set gain
    ret = cam.MV_CC_SetEnumValue("GainAuto", 0)
    if ret == MV_OK:
        ret = cam.MV_CC_SetFloatValue("Gain", config.gain)
        if ret == MV_OK:
            print(f"✓ Gain: {config.gain}")
        else:
            print(f"⚠ Set gain warning: 0x{ret:08x}")
    
    # Get payload size
    stParam = MVCC_INTVALUE()
    memset(ctypes.byref(stParam), 0, ctypes.sizeof(MVCC_INTVALUE))
    ret = cam.MV_CC_GetIntValue("PayloadSize", stParam)
    
    if ret == MV_OK:
        payload_size = stParam.nCurValue
        print(f"✓ Payload size: {payload_size} bytes")
    
    # Start grabbing
    print("6. Starting acquisition...")
    ret = cam.MV_CC_StartGrabbing()
    if ret != MV_OK:
        print(f"✗ Start grabbing failed: 0x{ret:08x}")
        cam.MV_CC_CloseDevice()
        cam.MV_CC_DestroyHandle()
        return False
    
    print("✓ Camera initialized and ready")
    return True

def capture_and_save(capture_count):
    """Capture one image and save"""
    global cam, config
    
    # Get payload size
    stParam = MVCC_INTVALUE()
    memset(ctypes.byref(stParam), 0, ctypes.sizeof(MVCC_INTVALUE))
    cam.MV_CC_GetIntValue("PayloadSize", stParam)
    payload_size = stParam.nCurValue
    
    # Capture
    data_buf = (ctypes.c_ubyte * payload_size)()
    stFrameInfo = MV_FRAME_OUT_INFO_EX()
    memset(ctypes.byref(stFrameInfo), 0, ctypes.sizeof(stFrameInfo))
    
    ret = cam.MV_CC_GetOneFrameTimeout(
        ctypes.byref(data_buf),
        payload_size,
        stFrameInfo,
        1000
    )
    
    if ret != MV_OK:
        print(f"⚠ Get frame failed: 0x{ret:08x}")
        return False
    
    # Convert to numpy
    image_size = stFrameInfo.nWidth * stFrameInfo.nHeight
    image = np.frombuffer(data_buf, count=image_size, dtype=np.uint8)
    image = image.reshape((stFrameInfo.nHeight, stFrameInfo.nWidth))
    
    # Generate filename with timestamp
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename_base = f"capture_{timestamp}_{capture_count:04d}"
    
    # Save image file
    saved_files = []
    
    if config.format == 'pgm':
        pgm_file = os.path.join(config.output, f"{filename_base}.pgm")
        with open(pgm_file, 'wb') as f:
            f.write(f"P5\n{stFrameInfo.nWidth} {stFrameInfo.nHeight}\n255\n".encode())
            f.write(image.tobytes())
        saved_files.append(pgm_file)
        
        # Update latest symlink
        latest_file = os.path.join(config.output, "latest.pgm")
        if os.path.islink(latest_file) or os.path.exists(latest_file):
            os.remove(latest_file)
        os.symlink(os.path.basename(pgm_file), latest_file)
    
    elif config.format == 'png':
        try:
            import cv2
            png_file = os.path.join(config.output, f"{filename_base}.png")
            cv2.imwrite(png_file, image)
            saved_files.append(png_file)
            
            latest_file = os.path.join(config.output, "latest.png")
            if os.path.islink(latest_file) or os.path.exists(latest_file):
                os.remove(latest_file)
            os.symlink(os.path.basename(png_file), latest_file)
        except ImportError:
            print("⚠ opencv-python not installed, falling back to PGM")
            pgm_file = os.path.join(config.output, f"{filename_base}.pgm")
            with open(pgm_file, 'wb') as f:
                f.write(f"P5\n{stFrameInfo.nWidth} {stFrameInfo.nHeight}\n255\n".encode())
                f.write(image.tobytes())
            saved_files.append(pgm_file)
    
    print(f"[{capture_count:04d}] {timestamp} - {stFrameInfo.nWidth}x{stFrameInfo.nHeight} - "
          f"Mean: {image.mean():.1f}, Min: {image.min()}, Max: {image.max()}")
    
    # Run ROOT analysis
    run_root_analysis()
    
    return True

def cleanup():
    """Clean up camera resources"""
    global cam
    
    if cam is not None:
        print("\nCleaning up...")
        cam.MV_CC_StopGrabbing()
        cam.MV_CC_CloseDevice()
        cam.MV_CC_DestroyHandle()
        print("✓ Camera closed")

def main():
    global running, config
    
    # Parse command line arguments
    parser = argparse.ArgumentParser(
        description='HikRobot Camera Capture with Automatic ROOT Analysis',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('--ip', default='192.168.131.142',
                        help='Camera IP address')
    parser.add_argument('--output', default='./captures',
                        help='Output directory')
    parser.add_argument('--interval', type=float, default=1.0,
                        help='Capture interval in seconds')
    parser.add_argument('--exposure', type=float, default=100000.0,
                        help='Exposure time in microseconds')
    parser.add_argument('--gain', type=float, default=0.0,
                        help='Gain value')
    parser.add_argument('--format', choices=['pgm', 'png'], default='pgm',
                        help='Output file format')
    parser.add_argument('--root-macro', default='./BPM_KoBRA_F0.C',
                        help='Path to ROOT analysis macro')
    
    config = parser.parse_args()
    
    print(f"Configuration:")
    print(f"  Camera IP: {config.ip}")
    print(f"  Output directory: {config.output}")
    print(f"  Capture interval: {config.interval} seconds")
    print(f"  Exposure time: {config.exposure} μs ({config.exposure/1000:.1f} ms)")
    print(f"  Gain: {config.gain}")
    print(f"  Output format: {config.format}")
    print(f"  ROOT macro: {config.root_macro}")
    print(f"  ROOT analysis: ENABLED (automatic)")
    print()
    
    # Create output directory
    os.makedirs(config.output, exist_ok=True)
    os.makedirs(os.path.join(config.output, "analysis_results"), exist_ok=True)
    print(f"✓ Output directories ready\n")
    
    # Initialize camera
    if not init_camera():
        print("\n✗ Camera initialization failed")
        return 1
    
    print("\n" + "="*60)
    print("Starting continuous capture with ROOT analysis")
    print("Press Ctrl+C to stop")
    print("="*60 + "\n")
    
    capture_count = 0
    
    try:
        while running:
            start_time = time.time()
            
            if capture_and_save(capture_count):
                capture_count += 1
            
            # Wait for next capture interval
            elapsed = time.time() - start_time
            sleep_time = max(0, config.interval - elapsed)
            
            if sleep_time > 0 and running:
                time.sleep(sleep_time)
    
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()
    
    finally:
        cleanup()
    
    print(f"\n{'='*60}")
    print(f"Capture session ended")
    print(f"Total captures: {capture_count}")
    print(f"Images saved in: {config.output}")
    print(f"Analysis results in: {config.output}/analysis_results")
    print(f"{'='*60}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
