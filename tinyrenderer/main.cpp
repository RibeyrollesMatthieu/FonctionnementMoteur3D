#include "tgaimage.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <array>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

const int imageSize = 1000;

// draw a @color point on @image at coords @x and @y
void drawPoint(TGAImage &image, int x, int y, TGAColor color) {
    image.set(x, y, color);
}

// draw a @color line on @image from @x0, @y0 to @x1, @y1
void drawLine(TGAImage &image, int x0, int y0, int x1, int y1, TGAColor color) { 
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    int dx = x1-x0; 
    int dy = y1-y0; 
    int derror2 = std::abs(dy)*2; 
    int error2 = 0; 
    int y = y0; 
    for (int x=x0; x<=x1; x++) { 
        if (steep) { 
            image.set(y, x, color); 
        } else { 
            image.set(x, y, color); 
        } 
        error2 += derror2; 
        if (error2 > dx) { 
            y += (y1>y0?1:-1); 
            error2 -= dx*2; 
        } 
    } 
}

//
void drawTriangle(TGAImage &image, std::array<double, 3> coords0, std::array<double, 3> coords1, std::array<double, 3> coords2, TGAColor color) {
    std::cout << coords0[0] << std::endl;
    drawLine(image, coords0[0] * imageSize/2 + imageSize/2, coords0[1] * imageSize/2 + imageSize/2, coords1[0] * imageSize/2 + imageSize/2, coords1[1] * imageSize/2 + imageSize/2, color); // from 0 to 1
    drawLine(image, coords1[0] * imageSize/2 + imageSize/2, coords1[1] * imageSize/2 + imageSize/2, coords2[0] * imageSize/2 + imageSize/2, coords2[1] * imageSize/2 + imageSize/2, color); // from 1 to 2
    drawLine(image, coords2[0] * imageSize/2 + imageSize/2, coords2[1] * imageSize/2 + imageSize/2, coords0[0] * imageSize/2 + imageSize/2, coords0[1] * imageSize/2 + imageSize/2, color); // from 2 to 0
}

std::vector<std::array<double,3>> getVertices(std::ifstream &infile) {
    std::vector<std::array<double,3>> vertices;
    std::string line;

    while(std::getline(infile, line)) {
        if (line.rfind("v ", 0) == 0) { // reading all vertices
            std::array<double, 3> coords;
            std::istringstream streamLine(line.c_str());
            char smth;

            streamLine >> smth >> coords[0] >> coords[1] >> coords[2];
            vertices.push_back(coords);
        }
    }

    return vertices;
}

void readFile(std::string fileName, bool drawCloud = false, bool drawTriangles = true) {
	std::ifstream infile(fileName);

    if (infile) {

        TGAImage image(imageSize, imageSize, TGAImage::RGB);

        std::string line;
        std::vector<std::array<double,3>> vertices = getVertices(infile);
        infile.clear();
        infile.seekg(0);

        while (std::getline(infile, line)) {
            std::istringstream streamLine(line.c_str());
            char smth;            

            if (drawCloud) { // draw a cloud of points (vertices)
                for (auto i = 0; i < vertices.size(); i++) {
                    drawPoint(image, vertices.at(i)[0] * imageSize/2 + imageSize/2, vertices.at(i)[1] * imageSize/2 + imageSize/2, red);
                }
            }

            if (drawTriangles) { // reading for drawing triangles (facets)
                if (line.rfind("f ", 0) == 0) {  
                    int index0, index1, index2, trashInt;
                    char slash;

                    streamLine >> smth >> 
                        index0 >> slash >> trashInt >> slash >> trashInt >> 
                        index1 >> slash >> trashInt >> slash >> trashInt >> 
                        index2 >> slash >> trashInt >> slash >> trashInt;

                    drawTriangle(image, vertices.at(index0-1), vertices.at(index1-1), vertices.at(index2-1), white);
                }
            }
        }

        image.flip_vertically();
        image.write_tga_file("output.tga"); // write output file
        std::cout << "Image stored as ./output.tga" << std::endl;
    } else {
        std::cout << "An error has occured while oppening the file." << std::endl;
    }
}

int main(int argc, char** argv) {
    std::string fileName(argv[1]);

	readFile(fileName);

	return 0;
}

