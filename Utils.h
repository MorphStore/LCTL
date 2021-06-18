//
// Created by André Berthold
//

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <random>
#include <queue>
#include <ctime>
#include <iostream>
#include <cstring>
#include <math.h> 
#include <array>

#ifndef UTILS_H
#define UTILS_H

namespace LCTL {

  template <size_t u, size_t v>
  struct gcd {
    enum { value = gcd<v, u % v>::value };
  };

  template <size_t u>
  struct gcd<u, 0> {
    enum { value = u };
  };

  template <>
  struct gcd<0, 0> {
    enum { value = -1 };
  };

  template <size_t a, size_t b>
  struct lcm {
    enum { value = a * b / (gcd<a, b>::value) };
  };

template<typename T, typename DISTR>
T * create_array(size_t size, DISTR distribution) {
  std::default_random_engine generator;
  generator.seed(time(nullptr));
  auto ptr = (T *) malloc(size * sizeof (T));
  for (size_t i = 0; i < size; ++i) {
      ptr[i] = distribution(generator);
  }

  return ptr;
}

template<typename T>
T * dup_array(size_t size, T * array) {
  auto ptr = (T *) malloc(size * sizeof (T));
  memcpy((void *) ptr, (void *) array, size * sizeof (T));
  return ptr;
}

template<typename T>
int equals(std::vector<T> v1, std::vector<T> v2) {
  if (v1.size() != v2.size()) return 0;
  else {
    for (int i = 0; i < v1.size(); i++) {
      if (v1[i] != v2[i])
        return 0;
    }
    return 1;
  }
}

template<typename T, typename U>
int equals(T v1, U v2) {
  std::cerr << "Incomparable types" << std::endl;
  return 0;
}

template<typename T>
T * create_c_array(size_t size, T fill) {
  auto array = (T *) malloc(sizeof (T) * size);
  for (size_t i = 0; i < size; ++i) {
    array[i] = fill;
  }
  return array;
}

template<typename T>
T * rand_num_arr(size_t size, T min, T max) {
  auto array = (T *) malloc(sizeof (T) * size);

  srand(time(0));
  for (size_t i = 0; i < size; ++i) {
    array[i] = (rand() % (max - min)) + min;
  }
  return array;
}

template<typename T>
void print_array(size_t size, T * array, char separator) {
  for (size_t i = 0; i < size; i++) {
    if (i != 0 && i % 32 == 0) std::cout << std::endl;
    std::cout << (uint64_t) * array << separator;
    array++;

  }
  std::cout << std::endl;
}

template<typename T>
void print_array_tokenize(size_t size, T * array, char separator, int token_size) {
  int counter = 0;
  for (size_t i = 0; i < size; ++i) {
    std::cout << array[i] << separator;
    ++counter;
    if (counter >= token_size) {
      std::cout << std::endl;
      counter = 0;
    }
  }
  std::cout << std::endl;
}

template<typename T>
void print_vector(std::vector<T> vector, char separator) {
  for (size_t i = 0; i < vector.size(); ++i) {
      std::cout << vector[i] << separator;
  }
  std::cout << std::endl;

}

template <typename T, size_t SIZE = sizeof (T) * 8 >
char * to_binary(const T num) {

  T num_cpy = num;
  char * str = (char *) malloc((SIZE + 1) * sizeof (char)); //null terminator
  str[SIZE] = 0x00;
  for (size_t i = 0; i < SIZE; ++i) {

    str[(SIZE - 1) - i] = num_cpy % 2 == 0 ? '0' : '1';
    num_cpy = num_cpy >> 1;
  }
    return str;
}

template<typename T>
void print_vector_bin(std::vector<T> vector, char separator) {
    for (size_t i = 0; i < vector.size(); ++i) {
        std::cout << to_binary(vector[i]) << separator;
    }
    std::cout << std::endl;
}

template<typename T>
T* print_min(size_t size, T * array) {
    T * min = array;
    T * max = array;
    for (size_t i = 0; i < size; ++i) {
        array[i] < *min ? min = &array[i] : min = min;
        array[i] > *max ? max = &array[i] : max = max;
    }
    std::cout << "min:" << *min << std::endl;
    std::cout << "max:" << *max << std::endl;
    std::cout << "bw: " << ceil(log2(*max - *min + 1)) << std::endl;
    return min;
}

template<typename T>
void print_array_for(size_t size, T * array, T ref, char separator) {
    for (size_t i = 0; i < size; ++i) {
        std::cout << array[i] - ref << separator;
    }
    std::cout << std::endl;
}

template<typename T>
void print_bin(T* in, size_t length, size_t printnumbits, std::ostream& s = std::cout) {
    if (length <= 0) return;
    for (int i = 0; i < length; i++) {
        s << "(" << i << ")\t" << (uint64_t*) ((T*) in + i) << "\t";
        for (int j = std::min(sizeof (T)*8 - 1, printnumbits - 1); j >= 0; --j)
            s << ((*(in + i) >> j) & 1);
        s << "\t" << (uint64_t) *(in + i) << "\n";
    }
    s << "\n";
}

template<typename T>
void print_compare(T* in, T* in2, size_t length, size_t printnumbits, std::ostream& s = std::cout) {
    if (length <= 0) return;
    size_t cnt = length - 1;
    while (*(in + cnt) == 0 && *(in2 + cnt) == 0 && cnt >= 0) {
        cnt--;
    }
    for (int i = 0; i <= cnt; i++) {
        s << "(" << i << ")\t";
        for (int j = std::min(sizeof (T)*8 - 1, printnumbits - 1); j >= 0; --j)
            s << ((*(in + i) >> j) & 1);
        s << "\t" << (uint64_t) *(in + i) << "\t" << (uint64_t *) (in + i) << "\t";
        for (int j = std::min(sizeof (T)*8 - 1, printnumbits - 1); j >= 0; --j)
            if (((*(in2 + i) >> j)&1) == ((*(in + i) >> j)&1))
                s << (uint64_t) ((*(in2 + i) >> j) & 1);
            else (&s == &std::cout)? s << "\033[1;31m" << (uint64_t) ((*(in2 + i) >> j) & 1) << "\033[0m" : s << (uint64_t) ((*(in2 + i) >> j) & 1);
        if (*(in2 + i) == *(in + i)) s << "\t" << (uint64_t) *(in2 + i) << "\t" << (uint64_t*) (in2 + i) <<"\n";
        else
            (&s == &std::cout)?s << "\t\033[1;31m" << (uint64_t) *(in2 + i) << "\033[0m" << "\t" << (uint64_t*) (in2 + i) <<"\n": s << (uint64_t) *(in2 + i) << "\t" << (uint64_t*) (in2 + i) <<"\n";
    }
    s << "\n";
}

/*
 * Erase all Occurrences of given substring from main string.
 */
void eraseAllSubStr(std::string & mainStr, const std::string & toErase) {
    size_t pos = std::string::npos;
    // Search for the substring in string in a loop untill nothing is found
    while ((pos = mainStr.find(toErase)) != std::string::npos) {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
    }
}

void printTree(std::string str) {
    std::size_t found = str.find_first_of("<,>");
    int cnt = 0;
    while (found != std::string::npos) {
        if (str[found] == '<') {
            cnt++;
            str.insert(found + 1, "\n" + std::string(cnt, ' '));
            found = str.find_first_of("<,>", found + cnt + 1);
        } else if (str[found] == ',') {
            str.insert(found + 1, "\n" + std::string(cnt - 1, ' '));
            found = str.find_first_of("<,>", found + cnt);
        } else if (str[found] == '>') {
            cnt--;
            str.insert(found, "\n" + std::string(cnt, ' '));
            found = str.find_first_of("<,>", found + cnt + 2);
        }

    }
    std::cout << str << "\n";
}

std::string execCommand(const std::string cmd, int& out_exitStatus) {
    out_exitStatus = 0;
    auto pPipe = ::popen(cmd.c_str(), "r");
    if(pPipe == nullptr) {
        throw std::runtime_error("Cannot open pipe");
    }

    std::array<char, 256> buffer;

    std::string result;

    while(not std::feof(pPipe)) {
        auto bytes = std::fread(buffer.data(), 1, buffer.size(), pPipe);
        result.append(buffer.data(), bytes);
    }

    auto rc = ::pclose(pPipe);

    if(WIFEXITED(rc)) {
        out_exitStatus = WEXITSTATUS(rc);
    }

    return result;
}

class warning : public std::exception{
  public:
    warning(const std::string& msg) {}
    const char* what() { return msg.c_str(); } //message of warning
  private:
    std::string msg;
};

}




#endif /* UTILS_H */
