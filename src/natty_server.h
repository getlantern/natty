#ifndef NATTY_SAMPLES_NATTY_SERVER_H_
#define NATTY_SAMPLES_CLIENT_PEER_CONNECTION_CLIENT_H_
#pragma once

#include <map>
#include <string>

class NattyServer {

 public:    
  static const int X = 5;

  NattyServer(int n) : num_(n) { }   
  ~NattyServer() { }

 private:
  int num_;

};
#endif  // NATTY_SAMPLES_NATTY_SERVER_H_
