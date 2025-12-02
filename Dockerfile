FROM ubuntu:22.04

# 1. Install base dependencies
RUN apt-get update && apt-get install -y \
    g++ cmake make pkg-config wget curl git \
    libssl-dev libsasl2-dev libicu-dev gnupg && \
    apt-get clean

# 2. Add MongoDB repo for C and C++ drivers
RUN wget -qO - https://www.mongodb.org/static/pgp/server-6.0.asc | gpg --dearmor > /usr/share/keyrings/mongodb.gpg
RUN echo "deb [ arch=amd64 signed-by=/usr/share/keyrings/mongodb.gpg ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/6.0 multiverse" \
    > /etc/apt/sources.list.d/mongodb-org-6.0.list

# 3. Install official MongoDB C++ driver (bsoncxx + mongocxx)
RUN apt-get update && apt-get install -y \
    libmongocxx-dev \
    libbsoncxx-dev

# 4. Copy project
WORKDIR /app
COPY . .

# 5. Build project using your command
RUN g++ -std=c++20 /app/backend/server.cpp \
    -o server \
    $(pkg-config --cflags --libs libmongocxx) \
    -lcrypto -lssl -pthread

# 6. Expose port
EXPOSE 8080

# 7. Start server
CMD ["./server"]
