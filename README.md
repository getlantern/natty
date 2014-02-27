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
