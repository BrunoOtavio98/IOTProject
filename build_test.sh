#!/bin/bash

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

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Running tests..."
    ./Test/Testing
else
    echo "Build failed!"
fi

cd ..