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

class pixel_format {
public:
    virtual int read_pixel(FILE* &input_filep) = 0;
    virtual void write_pixel(FILE* &output_filep) const = 0;
    virtual void printer() = 0;
    virtual void normilize() = 0;
protected:
    unsigned char calc_norm(unsigned char x, unsigned l, unsigned r) {
        return (r!=l ? (255/2) + (2*x-r-l)*(255/2)/(r-l) : x);
    }
};

class pgm_pixel : public pixel_format {
public:
    int read_pixel(FILE* &input_filep) override {
        int rct = (fread(&grey, sizeof(unsigned char), 1, input_filep) - 1);
        max_grey = std::max(max_grey, grey);
        min_grey = std::min(min_grey, grey);
        return rct;
    }
    void write_pixel(FILE* &output_filep) const override {
        fwrite(&grey, sizeof(unsigned char), 1, output_filep);
    }
    void printer() override {
        std::cout<<(int)grey<<'\n';
    }
    void normilize() override {
        grey = calc_norm(grey, min_grey, max_grey);
    }
    static unsigned char max_grey;
    static unsigned char min_grey;
private:
    unsigned char grey = 0;
};
unsigned char pgm_pixel::max_grey = 0;
unsigned char pgm_pixel::min_grey = UCHAR_MAX;


class ppm_pixel : public pixel_format {
public:
    int read_pixel(FILE* &input_filep) override {
        int rct = (fread(&r, sizeof(unsigned char), 1, input_filep) 
              + fread(&g, sizeof(unsigned char), 1, input_filep) 
              + fread(&b, sizeof(unsigned char), 1, input_filep) - 3);
        max_r = std::max(max_r, r);
        min_r = std::min(min_r, r);
        max_g = std::max(max_g, g);
        min_g = std::min(min_g, g);
        max_b = std::max(max_b, b);
        min_b = std::min(min_b, b);
        return rct;
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
    void normilize() override {
        r = calc_norm(r, min_r, max_r);
        g = calc_norm(g, min_g, max_g);
        b = calc_norm(b, min_b, max_b);
    }
    static unsigned char max_r;
    static unsigned char min_r;
    static unsigned char max_g;
    static unsigned char min_g;
    static unsigned char max_b;
    static unsigned char min_b;
private:
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
};
unsigned char ppm_pixel::max_r = 0, ppm_pixel::max_g = 0, ppm_pixel::max_b = 0;
unsigned char ppm_pixel::min_r = UCHAR_MAX, ppm_pixel::min_g = UCHAR_MAX, ppm_pixel::min_b = UCHAR_MAX;


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
        for(int i = 0; i < width; ++i) {
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
    
    for(int row = 0; row < height; ++row)
        for (int col = 0; col < width; ++col)
            data[row][col] -> normilize();

    FILE* output_filep = fopen(argv[3],"wb");
    fprintf(output_filep, "%s\n%d %d\n%d\n", format, width, height, bound);

    for(int row = 0; row < height; ++row)
        for(int col = 0; col < width; ++col)
            data[row][col]->write_pixel(output_filep);

    fclose(input_filep);
    fclose(output_filep);

    printf("Time (%i thread(s)): %g ms\n", (int)threads_c, (double)clock()/CLOCKS_PER_SEC);
}