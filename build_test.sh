#!/bin/bash

# Exit on any error
set -e

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Configure CMake
echo "Configuring CMake..."
cmake ..

# Build the project
echo "Building tests..."
cmake --build . --target Testing

# Run tests
./Test/Testing

# Store the test result
TEST_RESULT=$?

cd ..

# Exit with the test result
exit $TEST_RESULT