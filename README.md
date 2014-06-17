# natty

Standalone WebRTC-based NAT traversal

With peer-to-peer communcation, a set of tools to directly connect two endpoints is employed to bypass the intermediary barriers of NAT/firewall devices. WebRTC, a new project that enables real-time communication capabilities in web browsers, uses a trio of NAT traversal standards--ICE, STUN, and TURN--to help a peer discover the topology between itself and the peer it wishes to communicate with. These standards help to determine the best route possible, if it exists, through a given network topology.

Natty extracts these NAT traversal tools as implemented in the WebRTC source.
In keeping fashion with the standard itself, natty remains agnostic to
signaling specifics. It forwards messages between a caller and signaler,
facilitating an offer/answer exchange, and concentrates entirely on the NAT
traversal process.
Natty relies almost entirely on the ICE framework to conducts NAT traversals. ICE works, for a given peer, by gathering a prioritized list of possible network interface and port candidates. This list of candidates is accumulated by natty and forwarded to the signaling intermediary. Once a full set of candidate pairs is available at a specific peer, natty uses ICE to perform a series of connectivity checks.
When/if a connectivity check succeeds, and a connection is successfully established, natty outputs the resultant 5-tuple(s) on both sides. Natty, by default, consumes and returns JSON messages. After multiple rounds of tests, if no pair is found, natty assumes no working candidate pair for connecting the peers exists and returns an error message, assuming some fallback mechanism is subsequently necessary.

### Usage
```bash
   ./natty
    -offer (used on the initiator side)
    -debug (output debugging info)
    -out (re-direct output to file; assumes stdout by default)
```
Here's an example session:
```
./natty -offer -out test
cat test
{"sdp":"v=0\r\no=- 4652263964728009613 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE audio\r\na=msid-semantic: WMS\r\nm=audio 1 RTP/SAVPF 111 103 104 9 102 0 8 106 105 13 127 126\r\nc=IN IP4 0.0.0.0\r\na=rtcp:1 IN IP4 0.0.0.0\r\na=ice-ufrag:f3py6nfbZqDyjaub\r\na=ice-pwd:RqN17HNmLH6aQRLB+bsffxH9\r\na=ice-options:google-ice\r\na=mid:audio\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=recvonly\r\na=rtcp-mux\r\na=crypto:0 AES_CM_128_HMAC_SHA1_32 inline:jTq/Ml/M7Rf0+nDbgoMIDrAo+YCN1iwLvFrd8VoK\r\na=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:KrEcsYGkiLyoWRz6NIKk3MOkvIg7znU1dY31IptC\r\na=rtpmap:111 opus/48000/2\r\na=fmtp:111 minptime=10\r\na=rtpmap:103 ISAC/16000\r\na=rtpmap:104 ISAC/32000\r\na=rtpmap:9 G722/16000\r\na=rtpmap:102 ILBC/8000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:106 CN/32000\r\na=rtpmap:105 CN/16000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:127 red/8000\r\na=rtpmap:126 telephone-event/8000\r\na=maxptime:60\r\n","type":"offer"}
{"candidate":"a=candidate:3090910651 1 udp 2122194687 10.171.165.178 54357 typ host generation 0\r\n","sdpMLineIndex":0,"sdpMid":"audio"}
{"candidate":"a=candidate:3090910651 2 udp 2122194687 10.171.165.178 54357 typ host generation 0\r\n","sdpMLineIndex":0,"sdpMid":"audio"}
{"candidate":"a=candidate:4139282763 1 tcp 1518214911 10.171.165.178 50947 typ host generation 0\r\n","sdpMLineIndex":0,"sdpMid":"audio"}
{"candidate":"a=candidate:4139282763 2 tcp 1518214911 10.171.165.178 50947 typ host generation 0\r\n","sdpMLineIndex":0,"sdpMid":"audio"}
{"candidate":"a=candidate:1958130216 1 udp 1685987071 70.209.197.25 4791 typ srflx raddr 10.171.165.178 rport 54357 generation 0\r\n","sdpMLineIndex":0,"sdpMid":"audio"}
{"candidate":"a=candidate:1958130216 2 udp 1685987071 70.209.197.25 4791 typ srflx raddr 10.171.165.178 rport 54357 generation 0\r\n","sdpMLineIndex":0,"sdpMid":"audio"}

./natty -out resp < test
{"sdp":"v=0\r\no=- 3252870593939442268 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE audio\r\na=msid-semantic: WMS\r\nm=audio 1 RTP/SAVPF 111 103 104 9 102 0 8 106 105 13 127 126\r\nc=IN IP4 0.0.0.0\r\na=rtcp:1 IN IP4 0.0.0.0\r\na=ice-ufrag:rav4zT6QOjPTw1AO\r\na=ice-pwd:ejQCrilVxI6zapOclGyIfiIK\r\na=mid:audio\r\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\r\na=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\r\na=sendonly\r\na=rtcp-mux\r\na=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:zezEDJ85fa61k7u0HVDfUBcWNuwoOUGz1lUAig1o\r\na=rtpmap:111 opus/48000/2\r\na=fmtp:111 minptime=10\r\na=rtpmap:103 ISAC/16000\r\na=rtpmap:104 ISAC/32000\r\na=rtpmap:9 G722/16000\r\na=rtpmap:102 ILBC/8000\r\na=rtpmap:0 PCMU/8000\r\na=rtpmap:8 PCMA/8000\r\na=rtpmap:106 CN/32000\r\na=rtpmap:105 CN/16000\r\na=rtpmap:13 CN/8000\r\na=rtpmap:127 red/8000\r\na=rtpmap:126 telephone-event/8000\r\na=maxptime:60\r\n","type":"answer"}
{"candidate":"a=candidate:1526242907 1 udp 2122194687 107.170.244.214 34487 typ host generation 0\r\n","sdpMLineIndex":0,"sdpMid":"audio"}
{"candidate":"a=candidate:343630507 1 tcp 1518214911 107.170.244.214 39301 typ host generation 0\r\n","sdpMLineIndex":0,"sdpMid":"audio"}

A full file transfer test using natty is available [here](https://github.com/getlantern/natty-java-xmpp)```

## Build instructions
 
    # Download webrtc trunk
    ./webrtc-setup

    # build latest code with debugging enabled
    ninja -C build/trunk/out/Debug

    # build natty
    ninja -f natty.ninja

    # execute natty
    ./natty

    # clean up
    ninja -f natty.ninja -t clean
    
    # sample sessions
    #
    Initializing session description
    Session id 8864502416870081173
    Creating port allocator session
    ice sid HCV6ayfcZsugQjaX:bJgo20cB0leKTV1Th1ibJx6/
    Creating port allocator session
    Cce sid HCV6ayfcZsugQjaX:bJgo20cB0leKTV1Th1ibJx6/
    Ice candidate 4079541279:1:udp:2122194687:192.168.1.74:51478:local::0:
                  HCV6ayfcZsugQjaX:bJgo20cB0leKTV1Th1ibJx6
    -> 192.168.1.74:51478
    Found port Port[audio:1:0::Net[en0:192.168.1.0/24]]
    Found port Port[audio:1:0::Net[en0:192.168.1.0/24]]
    Ice candidate 2182108911:1:tcp:1518214911:192.168.1.74:64346:local::0:
                  HCV6ayfcZsugQjaX:bJgo20cB0leKTV1Th1ibJx6
    -> 192.168.1.74:64346
    Found port Port[audio:1:0:local:Net[en0:192.168.1.0/24]]
    Found port Port[audio:1:0:local:Net[en0:192.168.1.0/24]]
    Ice candidate 87598539:1:udp:1685987071:107.201.128.213:51478:stun:192.168.1.74:51478:
                  HCV6ayfcZsugQjaX:bJgo20cB0leKTV1Th1ibJx6
    -> 107.201.128.213:51478

    Initializing session description
    Session id 658569486909023361
    Creating port allocator session
    Ice sid ailLdeH/UkVxqSG2:+eUJYFBUeZSZ2yuIHZa2S7lV
    Creating port allocator session muxer
    Creating port allocator session
    Ice sid ailLdeH/UkVxqSG2:+eUJYFBUeZSZ2yuIHZa2S7lV
    Ice candidate 833690215:1:udp:2122194687:192.168.1.138:60868:local::0:
                  ailLdeH/UkVxqSG2:+eUJYFBUeZSZ2yuIHZa2S7lV
    -> 192.168.1.138:60868
    Found port Port[audio:1:0::Net[en0:192.168.1.0/24]]
    Found port Port[audio:1:0::Net[en0:192.168.1.0/24]]
    Ice candidate 2969115859:1:udp:1685987071:12.167.51.34:38249:stun:192.168.1.138:60868:
                  ailLdeH/UkVxqSG2:+eUJYFBUeZSZ2yuIHZa2S7lV
    -> 12.167.51.34:38249
    Ice candidate 2134042263:1:tcp:1518214911:192.168.1.138:52894:local::0:
                  ailLdeH/UkVxqSG2:+eUJYFBUeZSZ2yuIHZa2S7lV
    -> 192.168.1.138:52894
