#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from setuptools import setup, find_packages

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

with open("requirements.txt", "r", encoding="utf-8") as fh:
    requirements = [line.strip() for line in fh if line.strip() and not line.startswith("#")]

setup(
    name="hikrobot-camera",
    version="1.0.0",
    author="Your Name",
    author_email="your.email@example.com",
    description="HikRobot Machine Vision Camera Controller for Python",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/yourusername/machinevision",
    packages=find_packages(),
    package_dir={"": "python"},
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Topic :: Scientific/Engineering :: Image Recognition",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Operating System :: POSIX :: Linux",
    ],
    python_requires=">=3.6",
    install_requires=requirements,
    extras_require={
        "dev": [
            "pytest>=6.0",
            "pytest-cov>=2.0",
            "flake8>=3.8",
            "black>=20.8b1",
        ],
    },
    entry_points={
        "console_scripts": [
            "hikrobot-camera=hikvision_camera:main",
        ],
    },
)
