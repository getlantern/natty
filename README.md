# natty

Standalone WebRTC-based NAT traversal

With peer-to-peer communcation, a set of tools to directly connect two endpoints is employed to bypass the intermediary barriers of NAT/firewall devices. WebRTC, a new project that enables real-time communication capabilities in web browsers, uses a trio of NAT traversal standards--ICE, STUN, and TURN--to help a peer discover the topology between itself and the peer it wishes to communicate with. These standards help to determine the best route possible, if it exists, through a given network topology.

Natty extracts these NAT traversal tools as implemented in the WebRTC source.
In keeping fashion with the standard itself, natty remains agnostic to
signaling specifics. It forwards messages between a caller and signaler,
facilitating an offer/answer exchange, and concentrates entirely on the NAT
traversal process.
Natty relies almost entirely on the ICE framework to perform NAT traversals. ICE works, for a given peer, by gathering a prioritized list of possible IP address and port candidates. This list of candidates is accumulated by natty and forwarded to the signaling intermediary. Once a full set of candidate pairs is available on a specific endpoint, natty relies on ICE to perform a series of connectivity checks.
When/if a connectivity check succeeds, and a connection is successfully established, natty outputs the resultant 5-tuple(s) on both sides. Natty, by default, consumes and returns JSON messages. After multiple rounds of tests, if no pair is found, natty assumes no working candidate pair for connecting the peers exists and returns an error message, assuming some fallback mechanism is subsequently necessary.

## Installation
These instructions are also available in the webrtc-setup file. 

First, checkout and build the WebRTC source:
```bash
mkdir build
cd build
gclient config --name src 'git+https://chromium.googlesource.com/external/webrtc'
gclient sync -j200
cd src
git svn init --prefix=origin/ https://webrtc.googlecode.com/svn -T/branches/3.55/webrtc@6541 --rewrite-root=http://webrtc.googlecode.com/svn
git svn fetch
git checkout master
```

Copy over natty source and *.gyp files. Then generate build files:
```bash
cp ../../src/webrtc/channel.cc build/src
cp -r src/natty build/src
cp gyp/* build/src
```

Building natty
```
ninja -C build/src/out/Release
```

If all goes well, the natty binary should be available in the build/src/out/Release directory.

### Usage
You communicate with a natty process over stdin/stdout.

```bash
   ./natty
    -offer (used on the initiator side)
    -debug (output debugging info)
    -out (re-direct output to file; assumes stdout by default)
```
Here's an example session:
```
./natty -offer -out offerer
cat test
{"sdp":"v=0\r\no=- 4872732101493451958 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=msid-semantic: WMS\r\nm=application 9 DTLS/SCTP 5000\r\nc=IN IP4 0.0.0.0\r\na=ice-ufrag:t/JZ+l7sDBx99Zcy\r\na=ice-pwd:FWg9xOKBe51/2IMxWjkqqXYg\r\na=ice-options:google-ice\r\na=fingerprint:sha-1 46:BC:06:FE:C4:EC:D5:0B:CE:B9:FC:BE:55:3E:65:EB:85:28:26:06\r\na=setup:actpass\r\na=mid:data\r\na=sctpmap:5000 webrtc-datachannel 1024\r\n","type":"offer"}
{"candidate":"candidate:2085243720 1 udp 2122063615 192.168.1.70 59631 typ host generation 0","sdpMLineIndex":0,"sdpMid":"datae}
{"candidate":"candidate:852080568 1 tcp 1518083839 192.168.1.70 53788 typ host tcptype passive generation 0","sdpMLineIndex":0,"sdpMid":"data"}
{"candidate":"candidate:2321167004 1 udp 1685855999 107.201.128.213 59631 typ srflx raddr 192.168.1.70 rport 59631 generation 0","sdpMLineIndex":0,"sdpMid":"data"}

./natty -out answerer < offerer
{"sdp":"v=0\r\no=- 3470342631269636907 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=msid-semantic: WMS\r\nm=application 9 DTLS/SCTP 5000\r\nc=IN IP4 0.0.0.0\r\nb=AS:30\r\na=ice-ufrag:kAk9nHxBSAnIU5w9\r\na=ice-pwd:SgPPuhcyl/qZu/cdZiJTQyJq\r\na=fingerprint:sha-1 05:60:24:81:2D:8F:B6:10:C6:EE:20:0E:59:CA:F9:5E:E2:19:42:D5\r\na=setup:active\r\na=mid:data\r\na=sctpmap:5000 webrtc-datachannel 1024\r\n","type":"answer"}
{"candidate":"candidate:2085243720 1 udp 2122063615 192.168.1.70 64492 typ host generation 0","sdpMLineIndex":0,"sdpMid":"data"}
{"candidate":"candidate:852080568 1 tcp 1518083839 192.168.1.70 53790 typ host tcptype passive generation 0","sdpMLineIndex":0,"sdpMid":"data"}
{"candidate":"candidate:2321167004 1 udp 1685855999 107.201.128.213 64492 typ srflx raddr 192.168.1.70 rport 64492 generation 0","sdpMLineIndex":0,"sdpMid":"data"}
```
On both sides, if the NAT traversal succeeds, after a series of connectivity checks, you should see five tuples emitted on both the offerer and answerer side 
```
offer -> answer: {"local":"192.168.1.70:65410","proto":"udp","remote":"192.168.1.70:50746","type":"5-tuple"}

answer -> offer: {"local":"192.168.1.70:50746","proto":"udp","remote":"192.168.1.70:65410","type":"5-tuple"}
```

## Download
[Windows](https://s3.amazonaws.com/bifurcate/windows/natty.exe), Linux [32](https://s3.amazonaws.com/bifurcate/linux/i386/natty)/[64](https://s3.amazonaws.com/bifurcate/linux/x86_64/natty), and [OS X](https://s3.amazonaws.com/bifurcate/osx/natty)/[PGP sig](https://s3.amazonaws.com/bifurcate/osx/natty.asc)
                                                
A full demo using natty is available [here](https://github.com/getlantern/go-natty) using the [waddell](https://github.com/getlantern/waddell) signaling server.
