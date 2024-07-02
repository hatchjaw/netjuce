# Console application

This test application was created prior to development of the WFS plugin.

This application's sampling rate is governed by the system; for interoperability
with Teensy-based clients, whose sampling rate is fixed at 44.1 kHz[^1], make 
sure your audio system is running at that rate. E.g., via PipeWire:

```shell
pw-metadata -n settings 0 clock.force-rate 44100
```

[^1]: The sampling rate of Teensy's synchronous audio interface is modifiable, 
but the Teensy Audio Library assumes a rate of 44.1 kHz and uses that number in 
various audio-related calculations.

## Usage

Run `njConsole` from the command line. Use the `-s` flag (one or more times) to 
specify an audio source to be transmitted to the network, e.g.:

```shell
./njConsole -s /path/to/my/44100Hz-wav-file.wav -s /path/to/another/file.wav
```

Connect a client and `njConsole` will tell you when it detects a stream of 
returning audio packets from that client/peer:

```
Peer 192.168.48.234 connected.
```

And if the stream is interrupted for over a second:

```
Peer 192.168.48.234 disconnected.
```
