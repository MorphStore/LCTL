/*
 * Compile with 
 * g++ -O3 -I/$TVL -DSCALAR -DCOMPRESSEDBASEBITSIZE=8 -DBASE=16 -DUPPER=31 -o statbp statbp.cpp
 */
#include "testcase.h"

using namespace std;
using namespace LCTL;

int main(int argc, char ** argv) {
  if (argc <= 1) {
    cout << "No log file name entered. Exiting...";
    return -1;
  } else {
    
    const size_t countInLog = 2;
    
    string formatstring = "statbp<";
    formatstring.append(PROCESSINGSTYLESTRING);
    formatstring.append(", ");
    
    formatstring.append(std::to_string(BIT_WIDTH));
    formatstring.append(", uint");
    formatstring.append(std::to_string(BASEBITSIZE));
    formatstring.append("_t>");
    
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, UPPER, sizeof(COMPRESSEDBASE) * 8 * countInLog, false, statbp <PROCESSINGSTYLE, BIT_WIDTH, BASE> >::apply(
            argv,
            formatstring);
  }
  return 0;
}
