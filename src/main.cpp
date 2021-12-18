#include <algorithm>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream> 
#include <sstream> 
#include <cassert>
#include <climits>
#include <omp.h>

void throw_error(std::string &&err) {
    printf("%s\n",err.c_str());
    exit(-1);
}

inline uint8_t calc_norm(uint8_t x, uint8_t l, uint8_t r) {
    return (r!=l ? (255/2) + (2*x-r-l)*(255/2)/(r-l) : x);
}

int main(int argc, char* argv[]) {
    if(!(4 <= argc && argc <= 5))
        throw_error("Wrong format of arguments");
    
    if(!argv[1])
        throw_error("Wrong format of first argument (number of threads)");
    size_t threads_c = atoi(argv[1]);
    if(threads_c <= 0)
        throw_error("Wrong format of first argument (number of threads should be positive numer)");

    FILE* input_filep = fopen(argv[2], "rb" );
    if(!input_filep)
        throw_error("Wrong input file name");

    char format[3];
    size_t readed_bytes = fscanf(input_filep, "%s\n", format);
    if(std::string(format) != "P5" && std::string(format) != "P6") 
        throw_error("Wrong input file format (PGM/PPM required)");
    
    int width, height;
    readed_bytes = fscanf(input_filep, "%d %d\n", &(width), &(height));
    if(width < 0)
        throw_error("Wrong input file format (incorrect width)");

    if(height < 0)
        throw_error("Wrong input file format (incorrect height)");
    

    int bound;
    readed_bytes = fscanf(input_filep, "%d\n", &(bound));
    if(bound != 255)
        throw_error("Wrong input file format (bound must be 255)");


    uint8_t max_r, min_r, max_g, min_g, max_b, min_b, max_grey, min_grey;
    max_r = max_g = max_b = max_grey = 0;
    min_r = min_g = min_b = min_grey = UINT8_MAX;

    uint8_t *data;
    int pixel_sizeof = std::string(format) == "P6" ? 3 : 1;
    data = new uint8_t[height*width*pixel_sizeof];
    readed_bytes = fread(data, sizeof(uint8_t), height*width*pixel_sizeof, input_filep);
    if((int)readed_bytes != height*width*pixel_sizeof)
        throw_error("Wrong input file format (not enough bytes in data section)");
    
    double start = (double)clock();
    #pragma omp parallel for reduction(min: min_grey, min_r, min_g, min_b) reduction(max: max_grey, max_r, max_g, max_b) shared(data, pixel_sizeof) schedule(static) num_threads(threads_c)
    for(int i=0;i<height*width;++i) {
        if(pixel_sizeof == 1) {
            max_grey = std::max(max_grey, data[i]);
            min_grey = std::min(min_grey, data[i]);
        }
        else {
            max_r = std::max(max_r, data[i*pixel_sizeof]);
            min_r = std::min(min_r, data[i*pixel_sizeof]);
            max_g = std::max(max_g, data[i*pixel_sizeof+1]);
            min_g = std::min(min_g, data[i*pixel_sizeof+1]);
            max_b = std::max(max_b, data[i*pixel_sizeof+2]);
            min_b = std::min(min_b, data[i*pixel_sizeof+2]);
        }
    }   
    
    #pragma omp parallel for shared(data, pixel_sizeof, max_grey, max_r, max_g, max_b, min_r, min_g, min_b) schedule(static) num_threads(threads_c)
    for(int i=0;i<height*width;++i) {
        if(pixel_sizeof == 1) {
            data[i] = calc_norm(data[i], min_grey, max_grey);
        }
        else {
            data[i*pixel_sizeof] = calc_norm(data[i*pixel_sizeof], min_r, max_r);
            data[i*pixel_sizeof+1] = calc_norm(data[i*pixel_sizeof+1], min_g, max_g);
            data[i*pixel_sizeof+2] = calc_norm(data[i*pixel_sizeof+2], min_b, max_b);
        }
    }

    printf("Time (%i thread(s)): %g ms\n", (int)threads_c, (((double)clock()-start)/CLOCKS_PER_SEC)*1000);

    FILE* output_filep = fopen(argv[3],"wb");
    fprintf(output_filep, "%s\n%d %d\n%d\n", format, width, height, bound);
    fwrite(data, sizeof(uint8_t), height*width*pixel_sizeof, output_filep);

    delete data;
    fclose(input_filep);
    fclose(output_filep);
}