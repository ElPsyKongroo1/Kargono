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

# Add LunarG Vulkan SDK repository
echo "Adding LunarG Vulkan SDK repository..."
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | tee /etc/apt/trusted.gpg.d/lunarg.asc
if [ $? -ne 0 ]; then
  echo "Failed to add LunarG signing key."
  exit 1
fi

OS_VERSION=$(lsb_release -cs)
echo "Detected OS version: $OS_VERSION"

if [[ "$OS_VERSION" == "noble" || "$OS_VERSION" == "jammy" ]]; then
  echo "Adding LunarG Vulkan SDK sources list for $OS_VERSION..."
  wget -qO /etc/apt/sources.list.d/lunarg-vulkan-${OS_VERSION}.list http://packages.lunarg.com/vulkan/lunarg-vulkan-${OS_VERSION}.list
  if [ $? -ne 0 ]; then
    echo "Failed to add LunarG Vulkan sources list for $OS_VERSION."
    exit 1
  fi
else
  echo "Unsupported OS version for LunarG Vulkan SDK."
  exit 1
fi

# Update and install Vulkan SDK
echo "Updating package list..."
apt update -y
if [ $? -ne 0 ]; then
  echo "Failed to update package list."
  exit 1
fi

echo "Installing Vulkan SDK..."
apt install -y vulkan-sdk
if [ $? -eq 0 ]; then
  echo "Vulkan SDK installed successfully."
else
  echo "An error occurred while installing Vulkan SDK."
  exit 1
fi