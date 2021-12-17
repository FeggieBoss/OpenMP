#include <algorithm>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream> 
#include <sstream> 
#include <cassert>

void throw_error(std::string &&err) {
    printf("%s\n",err.c_str());
    exit(-1);
}

class pixel_format {
public:
    virtual size_t read_pixel(FILE* &input_filep) = 0;
    virtual void write_pixel(FILE* &output_filep) const = 0;
    virtual void printer() = 0;
};

class pgm_pixel : public pixel_format {
public:
    size_t read_pixel(FILE* &input_filep) override {
        return (fread(&grey, sizeof(unsigned char), 1, input_filep) - 1);
    }
    void write_pixel(FILE* &output_filep) const override {
        fwrite(&grey, sizeof(unsigned char), 1, output_filep);
    }
    void printer() override {
        std::cout<<(int)grey<<'\n';
    }
private:
    unsigned char grey = 0;
};

class ppm_pixel : public pixel_format {
public:
    size_t read_pixel(FILE* &input_filep) override {
        return (fread(&r, sizeof(unsigned char), 1, input_filep) 
            +  fread(&g, sizeof(unsigned char), 1, input_filep) 
            +  fread(&b, sizeof(unsigned char), 1, input_filep) - 3);
    }
    void write_pixel(FILE* &output_filep) const override {
        fwrite(&r, sizeof(unsigned char), 1, output_filep);
        fwrite(&g, sizeof(unsigned char), 1, output_filep);
        fwrite(&b, sizeof(unsigned char), 1, output_filep);
    }
    void printer() override {
        std::cout<<(int)r<<'\n';
        std::cout<<(int)g<<'\n';
        std::cout<<(int)b<<'\n';
    }
private:
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
};

unsigned char my_read(FILE* &file) {
    unsigned char c;
    auto r = fread(&c, sizeof(unsigned char), 1, file);
    if(r){};
    return c;
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

    std::vector<std::vector<pixel_format*>> data(height);
    for(auto &row : data) {
        for(size_t i = 0; i<width; ++i) {
            if(std::string(format) == "P6")
                row.push_back(new ppm_pixel());
            else 
                row.push_back(new pgm_pixel());
        }
    }   

    for(int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            readed_bytes = data[row][col]->read_pixel(input_filep);
            if(readed_bytes<0)
                throw_error("Wrong input file format (not enough bytes in data section)");
        }
    }
    
    for(int row = 0; row < 10; ++row) {
        data[row][0]->printer();
    }

    FILE* output_filep = fopen(argv[3],"wb");
    fprintf(output_filep, "%s\n%d %d\n%d\n", format, width, height, bound);

    for(int row = 0; row < height; ++row)
        for(int col = 0; col < width; ++col)
            data[row][col]->write_pixel(output_filep);

    fclose(input_filep);
    fclose(output_filep);
}