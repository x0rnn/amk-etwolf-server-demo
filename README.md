# Enemy Territory server side demos

This is a library for recording players from server.

Compatible with vanilla **etded 2.60b linux-i386**. Any mod should work as this is an engine extension.

Recorded demos are identical to standard client-side demos, so there's no need for a replay server or a patched client.

## Installation

The library needs to be preloaded into the `etded` binary, like this:

~~~
LD_PRELOAD=/path/to/libetwolf_server_demo.so etded +set dedicated 2 +...
~~~

## Usage

Recording on demand:

~~~
record <clientNum>
~~~

Stopping the recording:

~~~
stoprecord <clientNum>
~~~

All demos will be written to `%mod%/demos` folder.

### Configuration

- `svr_autoRecord` Automatically record all players
    - `0` disabled (default)
    - `1` enabled

- `svr_demoName` Demo name format
    - default: `%T-%m-%g-%n`, outputs: `2021-06-24-143520-goldrush-a4e12ef3-et_player`

| Character | Description                                                               |
|-----------|---------------------------------------------------------------------------|
| `%T`      | Map startup time                                                          |
| `%t`      | Demo creation time                                                        |
| `%G`      | Full GUID (32 hexadecimal characters)                                     |
| `%g`      | Short GUID (8 last characters)                                            |
| `%n`      | Player name                                                               |
| `%p`      | Client number                                                             |
| `%c`      | Demo counter (4 characters long zero padded number advanced on each demo) |
| `%m`      | Map name                                                                  |

- `svr_compress` GZip compress demo files
  - `0` disabled (default)
  - `1` enabled