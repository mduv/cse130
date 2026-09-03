# Operating Systems and Concurrent Programming in C

A collection of systems-programming projects that build from Unix file I/O to a concurrent HTTP/1.1 server and cache-policy simulator.

The repository focuses on the mechanics behind network services: file descriptors, sockets, protocol parsing, partial reads and writes, worker pools, synchronization, resource-level locking, and cache replacement. Each project is implemented in C with a small dependency surface and direct use of POSIX interfaces.

## Projects

| Directory | Project | Core concepts |
| --- | --- | --- |
| [`asgn1/`](asgn1/) | File-memory command interface | Unix file I/O, command parsing, buffered reads and writes |
| [`asgn2/`](asgn2/) | HTTP/1.1 file server | TCP sockets, HTTP parsing, request validation, status codes |
| [`asgn3/`](asgn3/) | Concurrency primitives | Bounded queues, reader-writer locks, mutexes, condition variables |
| [`asgn4/`](asgn4/) | Concurrent HTTP server | Worker pools, producer-consumer dispatch, per-file synchronization |
| [`asgn5/`](asgn5/) | Cache simulator | FIFO, LRU, Clock, compulsory and capacity misses |

## Concurrent HTTP server

The centerpiece is `asgn4`, a multithreaded HTTP/1.1 server supporting `GET` and `PUT` requests against the local filesystem.

```text
                       ┌─────────────────────┐
client connections ───▶ listener/dispatcher │
                       └──────────┬──────────┘
                                  │
                                  ▼
                       ┌─────────────────────┐
                       │ bounded work queue  │
                       └──────────┬──────────┘
                                  │
                         ┌────────┼────────┐
                         ▼        ▼        ▼
                       worker   worker   worker
                         └────────┼────────┘
                                  ▼
                       per-file reader/writer locks
                                  │
                                  ▼
                           local filesystem
```

The main thread accepts connections and places client sockets into a blocking queue. A configurable pool of workers consumes those connections. Each URI is associated with a reader-writer lock, allowing concurrent reads while ensuring writes receive exclusive access to the target file.

### HTTP behavior

| Method | Operation | Success response |
| --- | --- | --- |
| `GET /file` | Return a local file | `200 OK` |
| `PUT /file` | Create or replace a local file | `201 Created` or `200 OK` |

The server validates request lines, headers, URIs, methods, and protocol versions. It maps malformed requests and filesystem failures to HTTP error responses and emits an ordered audit log:

```text
<method>,<uri>,<status-code>,<request-id>
```

## Synchronization primitives

The concurrent server is supported by two reusable components developed in `asgn3`:

- A blocking, fixed-capacity queue implemented with a circular buffer, mutex, and `not_empty`/`not_full` condition variables
- A reader-writer lock supporting reader priority, writer priority, and an N-way policy that bounds how many readers may proceed while writers wait

Together they demonstrate producer-consumer coordination and fine-grained concurrency without busy-waiting.

## Cache simulator

`asgn5` models three cache-replacement strategies:

- **FIFO** — evicts the oldest resident item
- **LRU** — evicts the least recently used item
- **Clock** — approximates LRU with a circular reference-bit scan

The simulator reads an access trace from standard input, reports each access as a hit or miss, and separates compulsory misses from capacity misses.

```bash
make -C asgn5
printf 'a\nb\na\nc\n' | ./asgn5/cacher -N 2 -L
```

## Build and run

### Requirements

- Linux or another POSIX development environment
- `clang`
- `make`
- POSIX threads

Each project has an independent Makefile:

```bash
make -C asgn1
make -C asgn2
make -C asgn3
make -C asgn4
make -C asgn5
```

Run the concurrent server on port `8080` with its default four-worker pool:

```bash
./asgn4/httpserver 8080
```

Select a different worker count with `-t`:

```bash
./asgn4/httpserver -t 8 8080
```

Create and retrieve a file:

```bash
curl -i -X PUT \
  -H 'Content-Length: 13' \
  -H 'Request-Id: 42' \
  --data-binary 'Hello, world!' \
  http://localhost:8080/message.txt

curl -i \
  -H 'Request-Id: 43' \
  http://localhost:8080/message.txt
```

Clean any project with its `clean` target, for example:

```bash
make -C asgn4 clean
```

> [!NOTE]
> The HTTP server uses a bundled helper archive from its original Linux development environment. Other operating systems or architectures require a compatible build of that library. Modern compilers may also surface additional diagnostics because the Makefiles promote warnings to errors.

## Technical focus

- POSIX socket and filesystem APIs
- HTTP/1.1 request parsing and response construction
- robust handling of partial I/O
- producer-consumer queues and worker-thread pools
- mutexes and condition variables
- reader-writer fairness policies
- fine-grained file synchronization
- linked data structures and cache replacement
- memory ownership and explicit resource cleanup

## Scope

These projects were developed to explore operating-systems and concurrency fundamentals. The HTTP implementation intentionally supports a limited protocol surface and is not intended to replace a hardened production web server.
