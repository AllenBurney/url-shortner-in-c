# URL Shortener

A minimal HTTP URL shortener server written in C, with no external dependencies beyond the standard POSIX/BSD socket API.

## Project Structure

```
.
├── db.h        # URLMap struct definition and DB function declarations
├── db.c        # In-memory storage: save and lookup short→long URL mappings
└── server.c    # TCP server: HTTP request parsing, short URL generation, routing
```

## How It Works

1. The server listens on port **8080** for raw TCP connections.
2. Each incoming HTTP request is parsed with a regex to extract the method and path.
3. **POST** requests shorten a URL; **GET** requests redirect to the original.

The in-memory database holds up to **100** entries (configurable via `MAX_ENTRIES` in `db.h`). All data is lost when the server stops.

## API

### Shorten a URL
```
POST /
Content-Type: application/x-www-form-urlencoded

url=https://example.com/some/long/path
```
**Response:** `200 OK` with the short code in the body (e.g. `b`).

### Redirect via short URL
```
GET /<short_code>
```
**Response:** `302 Found` with a `Location` header pointing to the original URL, or `404 Not Found` if the code doesn't exist.

## Build & Run

```bash
gcc server.c db.c -o urlshortener
./urlshortener
```

The server starts on `http://localhost:8080`.

## Quick Test with curl

```bash
# Shorten a URL
curl -X POST -d "url=https://example.com/very/long/url" http://localhost:8080/

# Follow the redirect (returns the short code, e.g. "b")
curl -L http://localhost:8080/b
```

## Limitations

- **In-memory only** — no persistence across restarts.
- **Single-threaded** — handles one request at a time.
- **Max 100 entries** — set by `MAX_ENTRIES` in `db.h`.
- Short codes max out at **9 characters** (`short_url[10]` in the struct); with base-62 encoding this supports ~3.5 trillion unique URLs before overflow.
- No input validation on the submitted URL (any string is accepted).
