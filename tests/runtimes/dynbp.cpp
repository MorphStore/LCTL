/*
 * Compile with 
 * g++ -O3 -I/$TVL -DSCALAR -DCOMPRESSEDBASEBITSIZE=8 -DBASE=16 -DUPPER=31 -o statbp statbp.cpp
 */
#include "testcase.h"

using namespace std;
using namespace LCTL;



int main(int argc, char ** argv) {
  //if (argc <= 1) {
  //  cout << "No log file name entered. Exiting...";
  //  return -1;
  //} else {
    
    const size_t countInLog = 2;
    
    string formatstring = "dynamicbp<";
    formatstring.append(PROCESSINGSTYLESTRING);
    formatstring.append(", ");
    
    formatstring.append("64, uint");
    formatstring.append(std::to_string(BASEBITSIZE));
    formatstring.append("_t>");
    
    std::cout << "Start Testcase\n";
    testcaseRuntimes < String < decltype("DynamicBP"_tstr) >, 1024*100000, 1024*1000, dynbp <PROCESSINGSTYLE, 1, BASE> >::apply();
  //}
  return 0;
}
