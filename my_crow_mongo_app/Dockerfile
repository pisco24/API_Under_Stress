# Stage 1: Build the MongoDB C++ Driver
FROM ubuntu:22.04 as mongocxx-builder
WORKDIR /lib

# Install MongoDB C Driver dependencies
RUN apt-get update && apt-get install -y \
    libboost-all-dev \
    g++ \
    make \
    cmake \
    pkg-config \
    libssl-dev \
    libsasl2-dev \
    git \
    wget

# Environment variables to define versions
ENV MONGO_CXX_DRIVER_VERSION=3.10.1

# Download and extract the MongoDB C++ driver source code
RUN wget https://github.com/mongodb/mongo-cxx-driver/releases/download/r${MONGO_CXX_DRIVER_VERSION}/mongo-cxx-driver-r${MONGO_CXX_DRIVER_VERSION}.tar.gz && \
    tar -xzf mongo-cxx-driver-r${MONGO_CXX_DRIVER_VERSION}.tar.gz && \
    cd mongo-cxx-driver-r${MONGO_CXX_DRIVER_VERSION}/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release \
            -DBSONCXX_POLY_USE_IMPLS=ON \
            -DCMAKE_INSTALL_PREFIX=/usr/local && \
    cmake --build . && \
    make install

# Optional: Compile application with the MongoDB C++ Driver
# RUN c++ --std=c++11 <input>.cpp $(pkg-config --cflags --libs libmongocxx)

# Stage 2: Build Crow Application
# Use Ubuntu as the base image
FROM ubuntu:22.04

# Set the working directory
WORKDIR /app

# Install C++ compiler, CMake, and other necessary tools along with Boost
RUN apt-get update && \
    apt-get install -y \
    g++ \
    cmake \
    pkg-config \
    libssl-dev \
    libsasl2-dev \
    libboost-all-dev \
    libasio-dev \
    make \
    git \
    wget \
    curl

# Assuming we still need MongoDB C++ Driver installed
COPY --from=mongocxx-builder /usr/local /usr/local

ENV LD_LIBRARY_PATH /usr/local/lib:$LD_LIBRARY_PATH

# Download SimpleJSON
RUN curl -o /usr/local/include/json.hpp https://raw.githubusercontent.com/nbsdx/SimpleJSON/master/json.hpp

# Clone Crow repository
RUN git clone https://github.com/CrowCpp/Crow.git && \
    cd Crow && mkdir build && cd build && \
    cmake .. -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF && \
    make install

# Clone UUID v4 repo
RUN git clone https://github.com/crashoz/uuid_v4 && \
    cp uuid_v4/uuid_v4.h uuid_v4/endianness.h /usr/local/include/    

# Copy source and include files into the image
COPY ./src /app/src
COPY ./inc /app/inc

# Compile app
RUN g++ -std=c++17 -o main /app/src/main.cpp -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/bsoncxx/v_noabi -lboost_system -lpthread -lcrypto \
-lssl -lmongocxx -lbsoncxx -mavx2

# Expose the port required for Crow framework
EXPOSE 18080

# Run web server
CMD ["./main"]
