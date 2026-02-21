#!/bin/bash

# Exit on any error
set -e

# Default action: build and run
ACTION="${1:-a}"

# Validate input
if [[ ! "$ACTION" =~ ^[rba]$ ]]; then
    echo "Usage: $0 [r|b|a]"
    echo "  r - Run tests only"
    echo "  b - Build tests only"
    echo "  a - Build and run tests (default)"
    exit 1
fi

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Configure and build if needed
if [[ "$ACTION" == "b" ]] || [[ "$ACTION" == "a" ]]; then
    echo "Configuring CMake..."
    cmake ..
    
    echo "Building tests..."
    cmake --build . --target Testing
fi

# Run tests if needed
if [[ "$ACTION" == "r" ]] || [[ "$ACTION" == "a" ]]; then
    echo "Running tests..."
    ./Test/Testing
fi

# Store the test result
TEST_RESULT=$?

cd ..

# Exit with the test result
exit $TEST_RESULT