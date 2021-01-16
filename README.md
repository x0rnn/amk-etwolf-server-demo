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

### Autorecording

You can record everyone automatically by setting:

~~~
svr_autorecord 1
~~~