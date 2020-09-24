#include "Utils.h"
#include "algorithms/BP5Bits.h"

using namespace std;

int main(int argc, char ** argv) {

    uint8_t scale = 2;
    size_t size = scale * 32;

    std::uniform_int_distribution<uint64_t> distr(0, 30);
    auto in = create_array<uint64_t>(size, distr);
    auto out = (uint8_t* ) malloc(size * sizeof(uint64_t) * 2);
    
    print_array(size, in, ' ');
    auto result = bp5bits<uint64_t>((const uint8_t * &) in, size, out);
    std::cout << "\n";
    print_bin((uint64_t *) out, 5,64,'\n');

    return 0;
}
