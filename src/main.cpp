#include <algorithm>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream> 
#include <sstream> 

void throw_error(std::string &&err) {
    printf("%s\n",err.c_str());
    exit(-1);
}

int main(int argc, char* argv[]) {
    if(!(4 <= argc && argc <= 5))
        throw_error("Wrong format of arguments");
    
    if(!argv[1])
        throw_error("Wrong format of first argument (number of threads)");
    size_t threads_c = atoi(argv[1]);
    if(threads_c){};

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
    

    std::cout<<format<<'\n'<<width<<'\n'<<height<<'\n'<<bound<<'\n';

    unsigned char mas[height][width];
    for(int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            readed_bytes = fread(&mas[row][col], sizeof(unsigned char), 1, input_filep);
            if(!readed_bytes)
                throw_error("Wrong input file format (not enough bytes in data section)");
        }
    }
    

    for(int row=0;row<10;++row) 
        std::cout<<(int)mas[row][0]<<"\n";
    
    FILE* output_filep = fopen(argv[3],"wb");
    fprintf(output_filep, "%s\n%d %d\n%d\n", format,
                                              (int)width,
                                              (int)height,
                                              (int)bound);

    for(int row = 0; row < height; ++row)
        for(int col = 0; col < width; ++col)
            fwrite(&mas[row][col], sizeof(unsigned char), 1, output_filep);

    fclose(input_filep);
    fclose(output_filep);
}