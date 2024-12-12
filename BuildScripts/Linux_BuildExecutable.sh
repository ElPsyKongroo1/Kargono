#!/bin/bash

# Set environment variables
./Linux_SetEnvironmentVariables.sh

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
