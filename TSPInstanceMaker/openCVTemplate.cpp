#include "stdafx.h"
#include "cvHeader.h"
#include <Windows.h>
#include <vector>
#include <random>
#include <iostream>
#include <string>
#include <fstream> 

int main() {
	std::random_device rnd;
	std::mt19937 mt(rnd());

	// read image file
	cv::Mat src = cv::imread("img/miku3.png", 0);

	// grid size
	int grid_size = 8;
	int dst_width = src.size().width / grid_size;
	int dst_height = src.size().height / grid_size;

	// calc average image
	char* data = new char[dst_width * dst_height];
	char min = 255;
	cv::Rect rect(0, 0, grid_size, grid_size);
	for (int y = 0; y < dst_height; y++) {
		for (int x = 0; x < dst_width; x++) {
			rect.x = x * grid_size;
			rect.y = y * grid_size;

			cv::Mat roi = src(rect);
			cv::Scalar m = cv::mean(roi);
			data[x + y*dst_width] = (char)m.val[0];
		}
	}
	cv::Mat ave(dst_height, dst_width, src.type(), data);

	// calc number of city
	int city_num = 0;
	int ganma = 15;
	int* city_per_pixel = new int[dst_width * dst_height];
	char* sample_data = new char[dst_width * dst_height];
	for (int y = 0; y < dst_height; y++) {
		for (int x = 0; x < dst_width; x++) {
			int g = ganma - (int)((ganma + 1) * (ave.data[x + y*dst_width] / 255.0));
			city_per_pixel[x + y*dst_width] = (int)(1 / 3.0 * g * g);
			city_num += city_per_pixel[x + y*dst_width];
			sample_data[x + y*dst_width] = 255 - (char)(city_per_pixel[x + y*dst_width] / 10.0 * 255);
		}
	}
	cv::Mat sample_img(dst_height, dst_width, src.type(), sample_data);

	// deploy city
	int count = 0;
	double rate = 100.0;
	double** city_array = new double*[city_num];
	std::uniform_real_distribution<double> position(0.0, 1.0);
	for (int y = 0; y < dst_height; y++) {
		for (int x = 0; x < dst_width; x++) {
			for (int i = 0; i < city_per_pixel[x + y*dst_width]; i++) {
				city_array[count] = new double[2];
				city_array[count][0] = (x + position(mt)) * rate;
				city_array[count][1] = (y + position(mt)) * rate;
				count++;
			}
		}
	}
	std::cout << count << std::endl;
	delete city_per_pixel;

	// output tsp file
	std::ofstream writing_file;
	writing_file.open("output.tsp", std::ios::out);
	writing_file << "NAME : output" << std::endl;
	writing_file << "TYPE : TSP" << std::endl;
	writing_file << "DIMENSION : " << city_num << std::endl;
	writing_file << "EDGE_WEIGHT_TYPE : EUC_2D" << std::endl;
	writing_file << "NODE_COORD_SECTION" << std::endl;
	for (int i = 0; i < city_num; i++) {
		writing_file << (i+1) << " " << city_array[i][0] << " " << city_array[i][1] << std::endl;
	}
	writing_file << "EOF" << std::endl;

	// show sample image
	cv::namedWindow("sample");
	cv::imshow("sample", sample_img);
	cv::waitKey();
	delete sample_data;

	// finalize
	delete[] city_array;
	delete data;
}