FROM ubuntu:22.04

# ===== 1. Install system deps =====
RUN apt-get update && apt-get install -y \
    g++ cmake make pkg-config wget curl git \
    libssl-dev libsasl2-dev libicu-dev \
    libbson-dev libmongoc-dev && \
    apt-get clean


# ===== 2. Install MongoDB C++ Driver (mongocxx/bsoncxx) =====
RUN wget https://github.com/mongodb/mongo-cxx-driver/archive/r3.9.0.tar.gz && \
    tar -xzf r3.9.0.tar.gz && \
    cd mongo-cxx-driver-r3.9.0/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBSONCXX_POLY_USE_MNMLSTC=1 && \
    cmake --build . --target install && \
    ldconfig


# ===== 3. Copy your project =====
WORKDIR /app
COPY . .


# ===== 4. Build your server with your EXACT command =====
RUN g++ -std=c++20 \
    /app/backend/server.cpp \
    -o server \
    $(pkg-config --cflags --libs libmongocxx) \
    -lcrypto -lssl -pthread


# ===== 5. Expose port =====
EXPOSE 8080


# ===== 6. Run server =====
CMD ["./server"]
