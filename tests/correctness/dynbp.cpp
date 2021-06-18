#include "testcase.h"

int main(int argc, char ** argv) {
  if (argc == 0) {
    cout << "No log file name entered. Exiting...";
    return -1;
  } else {
    
      
    const size_t countInLog = 2;
    
    string formatstring = "dynbp<";
    formatstring.append(PROCESSINGSTYLESTRING);
    formatstring.append(", ");
    
    formatstring.append(std::to_string(BIT_WIDTH));
    formatstring.append(", uint");
    formatstring.append(std::to_string(BASEBITSIZE));
    formatstring.append("_t>");
    
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, UPPER, sizeof(COMPRESSEDBASE) * 8 * countInLog * SCALE, false, dynbp <PROCESSINGSTYLE, SCALE, BASE> >::apply(
            argv,
            formatstring);
  }
  return 0;
}
