#include "tgaimage.h"
#include <fstream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void readFile() {
	std::ifstream infile("../obj/african_head/african_head.obj");
}

int main(int argc, char** argv) {

	readFile();
	// TGAImage image(100, 100, TGAImage::RGB);
	// image.set(52, 41, red);
	// image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	// image.write_tga_file("output.tga");
	return 0;
}

