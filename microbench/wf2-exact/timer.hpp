#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <chrono>
#include <iostream>
#include <string>

namespace wf2 {

class Timer {
 std::chrono::time_point<std::chrono::high_resolution_clock> t0; 
 std::chrono::time_point<std::chrono::high_resolution_clock> t1; 

public:
  Timer() : t0(std::chrono::high_resolution_clock::now()) {}

  void start() {
    t0 = std::chrono::high_resolution_clock::now();
  }

  void lap(std::string str) {
    using namespace std;
    t1 = chrono::high_resolution_clock::now();
    auto diff_ns = chrono::duration<uint64_t, nano>(t1-t0);
    uint64_t diff = chrono::duration_cast<chrono::milliseconds>(diff_ns).count();
    uint64_t diff_sec_part = diff / 1000;
    uint64_t diff_ms_part = diff % 1000;

    cout << str << " time: " << diff_sec_part << "." << diff_ms_part << " seconds" << endl;
    t0 = t1;
  }
};

}

#endif
