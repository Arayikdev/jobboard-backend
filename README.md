# Junior Job Board Backend

A C++ backend for a job board application, using `httplib` for the API and `mongo-cxx-driver` for the database.

## Prerequisites

- C++17 compiler (GCC/Clang)
- CMake 3.10+
- MongoDB installed and running
- MongoDB C++ Driver (`libmongocxx` and `libbsoncxx`)
- OpenSSL

## Setup

1.  **Clone the repository**
2.  **Configure Environment**
    Create a `.env` file in the root directory (see `.env.example` or just use defaults):
    ```
    MONGODB_URI=mongodb://localhost:27017
    DB_NAME=testdb
    ```

## Build

```bash
mkdir build
cd build
cmake ..
make
```

## Run

```bash
./server
```

The server will start on port `8080`.

## API Endpoints

### Auth
- `POST /auth/register/user` - Register a candidate
- `POST /auth/register/company` - Register a company
- `POST /auth/login` - Login (returns JWT)

### Jobs
- `GET /jobs` - List jobs (filters: category, location, etc.)
- `POST /jobs` - Create a job (Company only)
- `PUT /jobs/:id` - Update a job
- `DELETE /jobs/:id` - Delete a job

### Applications
- `POST /applications` - Apply for a job (User only)
- `GET /applications/user/:id` - Get my applications
