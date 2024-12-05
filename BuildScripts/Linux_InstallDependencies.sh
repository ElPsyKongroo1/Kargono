#!/bin/bash

# Script to install xorg-dev and build-essential libraries

# Check if the script is run as root
if [ "$(id -u)" -ne 0 ]; then
  echo "This script must be run as root. Please use sudo."
  exit 1
fi

echo "Updating package list..."
apt update -y

# Install GLFW Dependency: xorg-dev
echo "Installing xorg-dev..."
apt install -y xorg-dev
if [ $? -eq 0 ]; then
  echo "xorg-dev installed successfully."
else
  echo "An error occurred while installing xorg-dev."
  exit 1
fi

# Install C++ Compiler: build-essential
echo "Installing build-essential..."
apt install -y build-essential
if [ $? -eq 0 ]; then
  echo "build-essential installed successfully."
else
  echo "An error occurred while installing build-essential."
  exit 1
fi