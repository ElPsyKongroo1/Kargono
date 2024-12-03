#!/bin/bash

# Navigate to the parent directory
cd ..

# Run the premake5 command
echo "Running premake5..."
./Dependencies/premake/bin/premake5 gmake2

# Check if the command executed successfully
if [ $? -eq 0 ]; then
  echo "Premake5 command executed successfully."
else
  echo "An error occurred while running premake5."
  exit 1
fi