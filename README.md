# Database Client-Server Management System

## 1. Project Overview

The **Database Client-Server Management System** is a Linux-based application developed using the C programming language.

The system allows multiple clients to connect to a central server and manage student records. The server receives requests from clients, processes them using worker threads, accesses the shared database safely, and sends responses back to the appropriate clients.

This project demonstrates important **Operating System concepts** such as process management, multithreading, inter-process communication, synchronization, memory mapping, logging, monitoring, and signal handling.

---

## 2. Objectives

* To implement communication between clients and a server using TCP sockets.
* To allow multiple clients to access the server simultaneously.
* To process client requests using worker threads.
* To implement CRUD operations on student records.
* To use mutexes for protecting shared database data.
* To use semaphores for synchronizing request and response queues.
* To use memory mapping for database file access.
* To maintain server and transaction logs.
* To monitor server resources.
* To implement controlled server shutdown using signals.

---

## 3. Technologies Used

| Technology         | Purpose                       |
| ------------------ | ----------------------------- |
| C                  | Application development       |
| Linux / Ubuntu     | Operating environment         |
| GCC                | Compilation                   |
| TCP Sockets        | Client-server communication   |
| POSIX Threads      | Concurrent request processing |
| Mutexes            | Protecting shared data        |
| Semaphores         | Queue synchronization         |
| `mmap()`           | Memory-mapped database access |
| `/proc` filesystem | Server monitoring             |
| Makefile           | Project compilation           |

---

## 4. System Architecture

```text
                    +----------------------+
                    |      Client 1         |
                    +----------+-----------+
                               |
                               |
                    +----------v-----------+
                    |                      |
                    |   CENTRAL SERVER     |
                    |                      |
                    |  Client Threads     |
                    |         |            |
                    |         v            |
                    |  Request Queue       |
                    |         |            |
                    |         v            |
                    |  Worker Threads      |
                    |         |            |
                    |         v            |
                    |  Shared Database     |
                    |         |            |
                    |         v            |
                    |  Response Queue      |
                    |         |            |
                    |         v            |
                    |  Response Thread     |
                    |                      |
                    +----------^-----------+
                               |
                    +----------+-----------+
                    |      Client 2         |
                    +----------------------+
```

### Working Flow

1. A client connects to the server using TCP.
2. The client sends a database request.
3. The server's client thread receives the request.
4. The request is placed into the shared request queue.
5. A worker thread takes the request from the queue.
6. The worker performs the requested database operation.
7. The response is placed into the response queue.
8. The response thread sends the response to the correct client.
9. The operation is recorded in the transaction log.

---

## 5. Project Structure

```text
database_client_server/
│
├── src/
│   ├── server.c          # Main server and socket handling
│   ├── client.c          # Client application
│   ├── database.c        # Student database operations
│   ├── queue.c           # Request and response queues
│   ├── logger.c          # Logging functions
│   ├── config.c          # Configuration loading
│   └── monitor.c         # Server monitoring
│
├── include/
│   ├── common.h          # Common constants and structures
│   ├── database.h        # Database function declarations
│   ├── queue.h           # Queue function declarations
│   ├── logger.h          # Logger function declarations
│   ├── config.h          # Configuration declarations
│   └── monitor.h         # Monitoring declarations
│
├── data/
│   └── students.dat      # Memory-mapped database file
│
├── logs/
│   ├── server.log        # Server activity log
│   └── transaction.log   # Database transaction log
│
├── config.txt            # Server configuration
├── Makefile              # Compilation instructions
└── README.md             # Project documentation
```

---

## 6. Main Features

### 6.1 TCP Client-Server Communication

The system uses TCP sockets for communication between clients and the server.

The default configuration is:

```text
Server IP   : 127.0.0.1
Server Port : 8080
```

The server waits for client connections and receives database requests.

---

### 6.2 Multiple Client Support

Multiple clients can connect to the same server simultaneously.

Each connected client is handled using a separate client thread. This allows different clients to send requests independently.

---

### 6.3 Worker Threads

The server creates worker threads to process requests.

The default configuration uses:

```text
Number of worker threads: 4
```

Instead of processing every request directly in the main server thread, requests are placed into a queue and processed by available worker threads.

---

### 6.4 Request and Response Queues

The project uses two shared queues:

#### Request Queue

Stores requests received from clients.

```text
Client → Request Queue → Worker Thread
```

#### Response Queue

Stores the results generated by worker threads.

```text
Worker Thread → Response Queue → Client
```

These queues follow the **producer-consumer model**.

---

### 6.5 Mutex Synchronization

A mutex is used to protect shared database and queue structures.

Only one thread can enter a protected critical section at a time.

This helps prevent:

* Data corruption
* Simultaneous conflicting updates
* Race conditions

Example:

```c
pthread_mutex_lock(&db_mutex);

/* Database operation */

pthread_mutex_unlock(&db_mutex);
```

---

### 6.6 Semaphore Synchronization

Semaphores are used to control access to empty and full queue slots.

For each queue:

* An empty-slot semaphore tracks available spaces.
* A full-slot semaphore tracks available requests or responses.
* A mutex protects the actual queue structure.

This prevents queue overflow and underflow.

---

### 6.7 CRUD Operations

The system supports the following database operations:

| Operation | Description                            |
| --------- | -------------------------------------- |
| `INSERT`  | Adds a new student record              |
| `SEARCH`  | Searches for a student using ID        |
| `UPDATE`  | Updates marks and attendance           |
| `DELETE`  | Deletes a student record               |
| `DISPLAY` | Displays all student records           |
| `MONITOR` | Displays server monitoring information |
| `EXIT`    | Closes the client connection           |

---

## 7. Student Record Format

Each student record contains:

| Field      | Description                            |
| ---------- | -------------------------------------- |
| ID         | Unique student identifier              |
| Name       | Student name                           |
| Course     | Student course or department           |
| Marks      | Student marks                          |
| Attendance | Student attendance percentage          |
| Active     | Indicates whether the record is active |

---

## 8. Supported Commands

### Insert a Student

```text
INSERT id name course marks attendance
```

Example:

```text
INSERT 10 StudentA CSE 80 85
```

### Search for a Student

```text
SEARCH id
```

Example:

```text
SEARCH 10
```

### Update a Student

```text
UPDATE id marks attendance
```

Example:

```text
UPDATE 10 95 90
```

### Delete a Student

```text
DELETE id
```

Example:

```text
DELETE 10
```

### Display All Students

```text
DISPLAY
```

The student records are displayed in the server terminal.

### Monitor the Server

```text
MONITOR
```

This displays the server process information in the server terminal.

### Exit the Client

```text
EXIT
```

---

## 9. Memory Mapping

The database file is accessed using Linux memory mapping.

The `mmap()` function maps the database file into the process's memory space.

This allows the program to access database records through memory.

When the database is closed, `munmap()` releases the mapped memory.

```text
Database File → mmap() → Process Memory
Process Memory → munmap() → Memory Released
```

The database file is stored at:

```text
data/students.dat
```

---

## 10. Logging

The system maintains two log files.

### Server Log

File:

```text
logs/server.log
```

It records activities such as:

* Logger initialization
* Server startup
* Worker thread creation
* Client connection
* Client disconnection
* Server shutdown

### Transaction Log

File:

```text
logs/transaction.log
```

It records database operations such as:

* Successful insertion
* Successful search
* Successful update
* Successful deletion
* Failed searches
* Failed database operations

Logging helps in tracking server activity and database transactions.

---

## 11. Monitoring

The monitoring module uses the Linux `/proc` filesystem to display server information.

It displays:

* Server process ID
* Resident memory usage
* Number of threads

Example:

```text
========== SERVER MONITOR ==========
Server PID: 7031
VmRSS:      2204 kB
Threads:        7
====================================
```

The exact values may change depending on the system and number of active threads.

---

## 12. Signal Handling

The server handles the following signals:

| Signal    | Purpose                                                             |
| --------- | ------------------------------------------------------------------- |
| `SIGINT`  | Handles Ctrl+C shutdown                                             |
| `SIGTERM` | Handles controlled termination                                      |
| `SIGPIPE` | Prevents unwanted termination when sending to a disconnected client |

### Controlled Shutdown

When the server receives `SIGINT` or `SIGTERM`:

1. The server stops accepting new connections.
2. The server socket is closed.
3. Worker and response threads are stopped.
4. Queues are destroyed.
5. The database is closed.
6. Log files are closed.
7. The server exits safely.

Example output:

```text
Shutting down server...
Server stopped successfully.
```

---

## 13. Configuration

The server configuration is loaded from `config.txt`.

Example configuration:

```text
Port         : 8080
Workers      : 4
Max Clients  : 20
Log Level    : 1
```

The configuration controls:

* Server port
* Number of worker threads
* Maximum client backlog
* Logging level

---

## 14. Compilation

Make sure GCC and the required Linux development tools are installed.

Compile the project using:

```bash
make clean
make
```

This creates two executable files:

```text
server
client
```

---

## 15. Execution

### Step 1: Start the Server

Open Terminal 1:

```bash
cd ~/database_client_server
./server
```

The server will display its configuration and wait for clients.

### Step 2: Start a Client

Open Terminal 2:

```bash
cd ~/database_client_server
./client
```

### Step 3: Start Additional Clients

Open another terminal:

```bash
cd ~/database_client_server
./client
```

Multiple clients can connect to the same server.

---

## 16. Testing Performed

The following tests were performed successfully:

* Server startup and configuration loading
* TCP client-server connection
* Student insertion
* Student searching
* Student updating
* Student deletion
* Displaying student records
* Invalid command handling
* Searching for a non-existing student
* Updating a deleted/non-existing student
* Multiple-client access
* Request and response queue implementation
* Mutex synchronization verification
* Semaphore synchronization verification
* Memory mapping verification
* Server log verification
* Transaction log verification
* Server process monitoring
* SIGINT controlled shutdown
* SIGTERM controlled shutdown

### Example Test Results

```text
INSERT 10 StudentA CSE 80 85
SUCCESS: Student inserted

SEARCH 10
SUCCESS: ID=10 Name=StudentA Course=CSE Marks=80.00 Attendance=85.00

UPDATE 10 95 90
SUCCESS: Student updated

SEARCH 10
SUCCESS: ID=10 Name=StudentA Course=CSE Marks=95.00 Attendance=90.00

DELETE 10
SUCCESS: Student deleted

SEARCH 10
ERROR: Student not found
```

---

## 17. Conclusion

The Database Client-Server Management System demonstrates how operating system concepts can be combined to build a concurrent database application.

The project uses TCP sockets for communication, POSIX threads for concurrency, queues for request processing, mutexes and semaphores for synchronization, memory mapping for database access, logging for activity tracking, monitoring for resource information, and signals for controlled shutdown.

This project provides practical experience with Linux system programming and concurrent application development.
