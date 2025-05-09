# HTTP Server

## Build Instructions

```bash
make
```

To clean:

```bash
make clean
```

---

## How to Run Instructions

Default (port 80):

```bash
./http_server
```

Custom port:

```bash
./http_server -p 8080
```

---

## Features

- Serve static files via `/static/filename`
- Perform calculations via `/calc/add/num1/num2`, `/calc/mul/num1/num2`, `/calc/div/num1/num2`

---

## Example Usage

Serve a static file:

```
http://localhost:8080/static/images/example.png
```

Perform a calculation:

```
http://localhost:8080/calc/add/5/7
```
Returns:
```
12
```
---
