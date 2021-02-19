#include "tgaimage.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

const int imageSize = 1000;

class Vec3f {
    private:
        
    public:
        double x, y, z;
        Vec3f(double x, double y, double z);
        Vec3f worldToScreen();
};

Vec3f::Vec3f(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Vec3f Vec3f::worldToScreen() {
    return Vec3f(
        this->x * imageSize/2 + imageSize/2,
        this->y * imageSize/2 + imageSize/2,
        this->z* imageSize/2 + imageSize/2
    );
}

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
void drawTriangle(TGAImage &image, Vec3f *vec3fs, TGAColor color) {
    drawLine(image, vec3fs[0].x * imageSize/2 + imageSize/2, vec3fs[0].y * imageSize/2 + imageSize/2, vec3fs[1].x * imageSize/2 + imageSize/2, vec3fs[1].y * imageSize/2 + imageSize/2, color); // from 0 to 1
    drawLine(image, vec3fs[1].x * imageSize/2 + imageSize/2, vec3fs[1].y * imageSize/2 + imageSize/2, vec3fs[2].x * imageSize/2 + imageSize/2, vec3fs[2].y * imageSize/2 + imageSize/2, color); // from 1 to 2
    drawLine(image, vec3fs[2].x * imageSize/2 + imageSize/2, vec3fs[2].y * imageSize/2 + imageSize/2, vec3fs[0].x * imageSize/2 + imageSize/2, vec3fs[0].y * imageSize/2 + imageSize/2, color); // from 2 to 0
}

std::vector<Vec3f> getVertices(std::ifstream &infile) {
    std::vector<Vec3f> vertices;
    std::string line;

    while(std::getline(infile, line)) {
        if (line.rfind("v ", 0) == 0) { // reading all vertices
            double x, y, z;
            
            std::istringstream streamLine(line.c_str());
            char smth;

            streamLine >> smth >> x >> y >> z;
            Vec3f coords(x, y, z);
            vertices.push_back(coords);
        }
    }

    return vertices;
}

Vec3f crossProduct(Vec3f v1, Vec3f v2) {
    return Vec3f(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

Vec3f getBarycentric(std::vector<Vec3f> &triangle, double *point) {
    Vec3f ab(triangle[1].x - triangle[0].x, triangle[1].y - triangle[0].y, triangle[1].z - triangle[0].z);
    Vec3f ac(triangle[2].x - triangle[0].x, triangle[2].y - triangle[0].y, triangle[2].z - triangle[0].z);
    Vec3f ap(point[0] - triangle[0].x, point[1] - triangle[0].y, point[2] - triangle[0].z);

    double z = crossProduct(ab, ap).z / crossProduct(ab, ac).z;
    double y = crossProduct(ap, ac).z  / crossProduct(ab, ac).z;
    double x = 1.f - y - z;

    //if (std::abs(temp.z) < 1) return Vec3f(-1, 1, 1);
    return Vec3f(x, y, z);
}

void readFile(std::string fileName, bool drawCloud = false, bool drawTriangles = true, bool fillTriangles = true) {
	std::ifstream infile(fileName);

    if (infile) {

        TGAImage image(imageSize, imageSize, TGAImage::RGB);

        std::string line;
        std::vector<Vec3f> vertices = getVertices(infile);
        infile.clear();
        infile.seekg(0);

        while (std::getline(infile, line)) {
            std::istringstream streamLine(line.c_str());
            char smth;            

            if (drawCloud) { // draw a cloud of points (vertices)
                for (auto i = 0; i < vertices.size(); i++) {
                    drawPoint(image, vertices.at(i).x * imageSize/2 + imageSize/2, vertices.at(i).y * imageSize/2 + imageSize/2, red);
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

                    
                    Vec3f triangle[] = {vertices.at(index0-1), vertices.at(index1-1), vertices.at(index2-1)};
                    drawTriangle(image, triangle, white);
                }
            }

            if (fillTriangles) {
                if (line.rfind("f ", 0) == 0) {  
                    int index0, index1, index2, trashInt;
                    char slash;

                    streamLine >> smth >> 
                        index0 >> slash >> trashInt >> slash >> trashInt >> 
                        index1 >> slash >> trashInt >> slash >> trashInt >> 
                        index2 >> slash >> trashInt >> slash >> trashInt;


                        Vec3f a = vertices.at(index0-1);
                        Vec3f b = vertices.at(index1-1);
                        Vec3f c = vertices.at(index2-1);

                        double x_min = std::min(a.x, std::min(b.x, c.x)) * imageSize/2 + imageSize/2;
                        double y_min = std::min(a.y, std::min(b.y, c.y)) * imageSize/2 + imageSize/2;

                        double x_max = std::max(a.x, std::max(b.x, c.x)) * imageSize/2 + imageSize/2;
                        double y_max = std::max(a.y, std::max(b.y, c.y)) * imageSize/2 + imageSize/2;

                        std::vector<Vec3f> triangles;
                        TGAColor randomColor(std::rand()%255,std::rand()%255,std::rand()%255,255);
                        for (auto x = x_min; x <= x_max; x++) {
                            for (auto y = y_min ; y <= y_max; y++) {
                                triangles.push_back(a.worldToScreen());
                                triangles.push_back(b.worldToScreen());
                                triangles.push_back(c.worldToScreen());

                                double point[] = {x, y};

                                Vec3f bary = getBarycentric(triangles, point);
                                //std::cout << bary.x << " " << bary.y << " " << bary.z << std::endl;
                                if (bary.x < 0 || bary.y < 0 || bary.z < 0) continue;
                                drawPoint(image, x, y, randomColor);

                                triangles.clear();
                            } 
                        }
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

	readFile(fileName, false, false, true);

	return 0;
}

