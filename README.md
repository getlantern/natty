# natty

Standalone WebRTC-based NAT traversal

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
