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

#ifndef UTILS_H
#define UTILS_H

namespace LCTL {

    template <int u, int v>
    struct gcd {
        enum { value = gcd<v, u % v>::value };
    };
    template <int u>
    struct gcd<u, 0> {
        enum { value = u };
    };
    template <>
    struct gcd<0, 0> {
        enum { value = -1 };
    };

    template <int a, int b>
    struct lcm {
        enum { value = a * b / (gcd<a, b>::value) };
    };
}

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
        for (size_t i = 0; i < size; ++i) {
            std::cout << array[i] << separator;
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
            //if(i % 4 == 0) {str[i] = '_'; ++i;}

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
    
    template<typename base_t> void print_bin(base_t * in, size_t length,size_t printnumbits, char separator){
        for (int i = 0; i < length; ++i){
            for (int j = std::min(sizeof(base_t)*8-1, printnumbits-1); j >= 0; --j)
                std::cout << ((*(in + i) >> j) & 1);
            std::cout << separator;
        }
        std::cout << "\n";
    }


#endif /* UTILS_H */
