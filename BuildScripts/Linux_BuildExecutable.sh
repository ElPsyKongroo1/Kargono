#!/bin/bash

# Script to set ExternalCompilerOptions and run premake5 command

# Set the environment variable
export ExternalCompilerOptions=KG_EXPORT_DEFAULT
echo "Environment variable ExternalCompilerOptions set to KG_EXPORT_DEFAULT."

# Navigate to the parent directory
cd ..

# Build the executable
echo "Running the make command"
make

# Check if the command executed successfully
if [ $? -eq 0 ]; then
  echo "Make command ran successfully."
else
  echo "An error occurred while running make command."
  exit 1
fi
