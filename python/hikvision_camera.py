#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HikRobot Machine Vision Camera Controller (MV-CH120-10GC)
Python wrapper for MVS (Machine Vision Software) SDK

This module provides a Python interface to control HikRobot cameras
using the MVS SDK library.
"""

import ctypes
import numpy as np
import sys
import os
from ctypes import *


# MVS SDK Constants
MV_OK = 0x00000000
MV_E_HANDLE = 0x80000000
MV_E_SUPPORT = 0x80000001
MV_E_BUFOVER = 0x80000002
MV_E_CALLORDER = 0x80000003
MV_E_PARAMETER = 0x80000004
MV_E_RESOURCE = 0x80000005
MV_E_NODATA = 0x80000006
MV_E_UNKNOW = 0x800000FF

# GigE Camera Access Mode
MV_ACCESS_Exclusive = 1
MV_ACCESS_ExclusiveWithSwitch = 2
MV_ACCESS_Control = 3
MV_ACCESS_ControlWithSwitch = 4

# Pixel Types
PixelType_Gvsp_Mono8 = 0x01080001
PixelType_Gvsp_RGB8_Packed = 0x02180014
PixelType_Gvsp_BGR8_Packed = 0x02180015
PixelType_Gvsp_BayerRG8 = 0x01080009


class MV_CC_DEVICE_INFO(Structure):
    """Camera device information structure"""
    _fields_ = [
        ("nMajorVer", c_ushort),
        ("nMinorVer", c_ushort),
        ("nMacAddrHigh", c_uint),
        ("nMacAddrLow", c_uint),
        ("nTLayerType", c_uint),
        ("Reserved", c_ubyte * 4),
        ("chSerialNumber", c_char * 16),
        ("chModelName", c_char * 32),
        ("chDeviceVersion", c_char * 32),
        ("chManufacturerName", c_char * 32),
        ("chUserDefinedName", c_char * 32),
        ("SpecialInfo", c_ubyte * 256),
    ]


class MV_CC_DEVICE_INFO_LIST(Structure):
    """Device list structure"""
    _fields_ = [
        ("nDeviceNum", c_uint),
        ("pDeviceInfo", POINTER(POINTER(MV_CC_DEVICE_INFO)) * 256),
    ]


class MV_FRAME_OUT_INFO_EX(Structure):
    """Image output information structure"""
    _fields_ = [
        ("nWidth", c_ushort),
        ("nHeight", c_ushort),
        ("enPixelType", c_uint),
        ("nFrameNum", c_uint),
        ("nDevTimeStampHigh", c_uint),
        ("nDevTimeStampLow", c_uint),
        ("nReserved0", c_uint),
        ("nHostTimeStamp", c_longlong),
        ("nFrameLen", c_uint),
        ("nLostPacket", c_uint),
        ("nReserved", c_uint * 2),
    ]


class MV_FRAME_OUT(Structure):
    """Image data output structure"""
    _fields_ = [
        ("pBufAddr", POINTER(c_ubyte)),
        ("stFrameInfo", MV_FRAME_OUT_INFO_EX),
        ("nReserved", c_uint * 16),
    ]


class HikRobotCamera:
    """
    HikRobot Camera Controller Class

    Usage:
        camera = HikRobotCamera()
        camera.enumerate_devices()
        camera.open_device(0)
        camera.start_grabbing()
        image = camera.get_image()
        camera.stop_grabbing()
        camera.close_device()
    """

    def __init__(self, sdk_path="/opt/MVS/lib/64/libMvCameraControl.so"):
        """
        Initialize the camera controller

        Args:
            sdk_path: Path to the MVS SDK library
        """
        self.sdk_path = sdk_path
        self.handle = None
        self.device_list = MV_CC_DEVICE_INFO_LIST()

        # Load MVS SDK library
        try:
            self.mvs_lib = ctypes.CDLL(sdk_path)
            print(f"✓ MVS SDK loaded from: {sdk_path}")
        except OSError as e:
            print(f"✗ Failed to load MVS SDK from {sdk_path}")
            print(f"  Error: {e}")
            print(f"  Please install HikRobot MVS SDK and update the path")
            raise

        self._setup_functions()

    def _setup_functions(self):
        """Setup function prototypes for MVS SDK"""

        # MV_CC_EnumDevices
        self.mvs_lib.MV_CC_EnumDevices.argtype = [c_uint, POINTER(MV_CC_DEVICE_INFO_LIST)]
        self.mvs_lib.MV_CC_EnumDevices.restype = c_uint

        # MV_CC_CreateHandle
        self.mvs_lib.MV_CC_CreateHandle.argtype = [POINTER(c_void_p), POINTER(MV_CC_DEVICE_INFO)]
        self.mvs_lib.MV_CC_CreateHandle.restype = c_uint

        # MV_CC_OpenDevice
        self.mvs_lib.MV_CC_OpenDevice.argtype = [c_void_p, c_uint, c_ushort]
        self.mvs_lib.MV_CC_OpenDevice.restype = c_uint

        # MV_CC_StartGrabbing
        self.mvs_lib.MV_CC_StartGrabbing.argtype = [c_void_p]
        self.mvs_lib.MV_CC_StartGrabbing.restype = c_uint

        # MV_CC_StopGrabbing
        self.mvs_lib.MV_CC_StopGrabbing.argtype = [c_void_p]
        self.mvs_lib.MV_CC_StopGrabbing.restype = c_uint

        # MV_CC_GetOneFrameTimeout
        self.mvs_lib.MV_CC_GetOneFrameTimeout.argtype = [c_void_p, POINTER(c_ubyte), c_uint,
                                                          POINTER(MV_FRAME_OUT_INFO_EX), c_uint]
        self.mvs_lib.MV_CC_GetOneFrameTimeout.restype = c_uint

        # MV_CC_CloseDevice
        self.mvs_lib.MV_CC_CloseDevice.argtype = [c_void_p]
        self.mvs_lib.MV_CC_CloseDevice.restype = c_uint

        # MV_CC_DestroyHandle
        self.mvs_lib.MV_CC_DestroyHandle.argtype = [c_void_p]
        self.mvs_lib.MV_CC_DestroyHandle.restype = c_uint

    def enumerate_devices(self):
        """
        Enumerate all available HikRobot cameras

        Returns:
            int: Number of devices found
        """
        # MV_GIGE_DEVICE = 1, enumerate GigE devices
        ret = self.mvs_lib.MV_CC_EnumDevices(1, byref(self.device_list))

        if ret != MV_OK:
            print(f"✗ Enumerate devices failed! Error code: 0x{ret:08x}")
            return 0

        device_count = self.device_list.nDeviceNum
        print(f"\n✓ Found {device_count} device(s)")

        for i in range(device_count):
            device_info = cast(self.device_list.pDeviceInfo[i], POINTER(MV_CC_DEVICE_INFO)).contents
            print(f"\n  Device {i}:")
            print(f"    Model: {device_info.chModelName.decode('utf-8')}")
            print(f"    Serial Number: {device_info.chSerialNumber.decode('utf-8')}")
            print(f"    User Defined Name: {device_info.chUserDefinedName.decode('utf-8')}")

        return device_count

    def open_device(self, device_index=0, access_mode=MV_ACCESS_Exclusive):
        """
        Open camera device

        Args:
            device_index: Index of the device to open (default: 0)
            access_mode: Access mode (default: MV_ACCESS_Exclusive)

        Returns:
            bool: True if successful, False otherwise
        """
        if device_index >= self.device_list.nDeviceNum:
            print(f"✗ Device index {device_index} out of range!")
            return False

        # Create handle
        device_info = cast(self.device_list.pDeviceInfo[device_index],
                          POINTER(MV_CC_DEVICE_INFO))
        self.handle = c_void_p()

        ret = self.mvs_lib.MV_CC_CreateHandle(byref(self.handle), device_info)
        if ret != MV_OK:
            print(f"✗ Create handle failed! Error code: 0x{ret:08x}")
            return False

        # Open device
        ret = self.mvs_lib.MV_CC_OpenDevice(self.handle, access_mode, 0)
        if ret != MV_OK:
            print(f"✗ Open device failed! Error code: 0x{ret:08x}")
            self.mvs_lib.MV_CC_DestroyHandle(self.handle)
            self.handle = None
            return False

        print(f"✓ Device {device_index} opened successfully")
        return True

    def start_grabbing(self):
        """
        Start image acquisition

        Returns:
            bool: True if successful, False otherwise
        """
        if not self.handle:
            print("✗ Device not opened!")
            return False

        ret = self.mvs_lib.MV_CC_StartGrabbing(self.handle)
        if ret != MV_OK:
            print(f"✗ Start grabbing failed! Error code: 0x{ret:08x}")
            return False

        print("✓ Started grabbing")
        return True

    def stop_grabbing(self):
        """
        Stop image acquisition

        Returns:
            bool: True if successful, False otherwise
        """
        if not self.handle:
            print("✗ Device not opened!")
            return False

        ret = self.mvs_lib.MV_CC_StopGrabbing(self.handle)
        if ret != MV_OK:
            print(f"✗ Stop grabbing failed! Error code: 0x{ret:08x}")
            return False

        print("✓ Stopped grabbing")
        return True

    def get_image(self, timeout_ms=1000):
        """
        Get one frame from camera

        Args:
            timeout_ms: Timeout in milliseconds (default: 1000)

        Returns:
            numpy.ndarray: Image data, or None if failed
        """
        if not self.handle:
            print("✗ Device not opened!")
            return None

        # Allocate buffer for image (max 10MB)
        buf_size = 10 * 1024 * 1024
        buf = (c_ubyte * buf_size)()
        frame_info = MV_FRAME_OUT_INFO_EX()

        ret = self.mvs_lib.MV_CC_GetOneFrameTimeout(
            self.handle,
            buf,
            buf_size,
            byref(frame_info),
            timeout_ms
        )

        if ret != MV_OK:
            if ret == MV_E_NODATA:
                print(f"✗ No data received within {timeout_ms}ms")
            else:
                print(f"✗ Get image failed! Error code: 0x{ret:08x}")
            return None

        # Convert buffer to numpy array
        width = frame_info.nWidth
        height = frame_info.nHeight
        pixel_type = frame_info.enPixelType

        if pixel_type == PixelType_Gvsp_Mono8:
            # Mono8 format
            image = np.frombuffer(buf, dtype=np.uint8, count=width*height)
            image = image.reshape((height, width))
        elif pixel_type in [PixelType_Gvsp_RGB8_Packed, PixelType_Gvsp_BGR8_Packed]:
            # RGB/BGR format
            image = np.frombuffer(buf, dtype=np.uint8, count=width*height*3)
            image = image.reshape((height, width, 3))
        else:
            print(f"⚠ Unsupported pixel type: 0x{pixel_type:08x}")
            # Return raw data
            image = np.frombuffer(buf, dtype=np.uint8, count=frame_info.nFrameLen)

        print(f"✓ Got frame {frame_info.nFrameNum}: {width}x{height}, "
              f"pixel_type=0x{pixel_type:08x}")

        return image

    def set_int_value(self, param_name, value):
        """
        Set integer parameter

        Args:
            param_name: Parameter name (e.g., "Width", "Height", "ExposureTime")
            value: Integer value

        Returns:
            bool: True if successful, False otherwise
        """
        if not self.handle:
            print("✗ Device not opened!")
            return False

        ret = self.mvs_lib.MV_CC_SetIntValue(
            self.handle,
            param_name.encode('utf-8'),
            value
        )

        if ret != MV_OK:
            print(f"✗ Set {param_name}={value} failed! Error code: 0x{ret:08x}")
            return False

        print(f"✓ Set {param_name}={value}")
        return True

    def get_int_value(self, param_name):
        """
        Get integer parameter

        Args:
            param_name: Parameter name

        Returns:
            int: Parameter value, or None if failed
        """
        if not self.handle:
            print("✗ Device not opened!")
            return None

        value = c_uint()
        ret = self.mvs_lib.MV_CC_GetIntValue(
            self.handle,
            param_name.encode('utf-8'),
            byref(value)
        )

        if ret != MV_OK:
            print(f"✗ Get {param_name} failed! Error code: 0x{ret:08x}")
            return None

        return value.value

    def set_float_value(self, param_name, value):
        """
        Set float parameter

        Args:
            param_name: Parameter name (e.g., "Gain", "ExposureTime")
            value: Float value

        Returns:
            bool: True if successful, False otherwise
        """
        if not self.handle:
            print("✗ Device not opened!")
            return False

        ret = self.mvs_lib.MV_CC_SetFloatValue(
            self.handle,
            param_name.encode('utf-8'),
            c_float(value)
        )

        if ret != MV_OK:
            print(f"✗ Set {param_name}={value} failed! Error code: 0x{ret:08x}")
            return False

        print(f"✓ Set {param_name}={value}")
        return True

    def get_float_value(self, param_name):
        """
        Get float parameter

        Args:
            param_name: Parameter name

        Returns:
            float: Parameter value, or None if failed
        """
        if not self.handle:
            print("✗ Device not opened!")
            return None

        value = c_float()
        ret = self.mvs_lib.MV_CC_GetFloatValue(
            self.handle,
            param_name.encode('utf-8'),
            byref(value)
        )

        if ret != MV_OK:
            print(f"✗ Get {param_name} failed! Error code: 0x{ret:08x}")
            return None

        return value.value

    def close_device(self):
        """
        Close camera device

        Returns:
            bool: True if successful, False otherwise
        """
        if not self.handle:
            print("✗ Device not opened!")
            return False

        # Close device
        ret = self.mvs_lib.MV_CC_CloseDevice(self.handle)
        if ret != MV_OK:
            print(f"✗ Close device failed! Error code: 0x{ret:08x}")

        # Destroy handle
        ret = self.mvs_lib.MV_CC_DestroyHandle(self.handle)
        if ret != MV_OK:
            print(f"✗ Destroy handle failed! Error code: 0x{ret:08x}")
            return False

        self.handle = None
        print("✓ Device closed")
        return True

    def __del__(self):
        """Destructor - ensure device is closed"""
        if self.handle:
            self.close_device()


if __name__ == "__main__":
    # Simple test
    print("HikRobot Camera Controller - Test Mode")
    print("=" * 50)

    camera = HikRobotCamera()

    # Enumerate devices
    device_count = camera.enumerate_devices()

    if device_count > 0:
        print("\nTo use the camera:")
        print("  camera.open_device(0)")
        print("  camera.start_grabbing()")
        print("  image = camera.get_image()")
        print("  camera.stop_grabbing()")
        print("  camera.close_device()")
