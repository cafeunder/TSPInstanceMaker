#include "stdafx.h"
#include "cvHeader.h"
#include <Windows.h>
#include <vector>
#include <random>
#include <iostream>
#include <string>
#include <fstream> 
#include <sstream>

class InstanceData {
public:
	InstanceData() {
		cityNum = 0;
	}

	int cityNum;
	std::vector<cv::Point2f> cityPosition;
};

// grid based stipple
void grid_based_stipple(
	InstanceData& OUT_instance,
	cv::Mat img,
	std::mt19937 mt,
	int require_city_num,
	int grid_size,
	bool use_improve_higher_contrast
) {
	// grid size
	int dst_width = img.size().width / grid_size;
	int dst_height = img.size().height / grid_size;

	// calc average image
	char* ave_data = new char[dst_width * dst_height];
	cv::Rect rect(0, 0, grid_size, grid_size);
	for (int y = 0; y < dst_height; y++) {
		for (int x = 0; x < dst_width; x++) {
			rect.x = x * grid_size;
			rect.y = y * grid_size;

			cv::Mat roi = img(rect);
			cv::Scalar m = cv::mean(roi);
			ave_data[x + y*dst_width] = (char)m.val[0];
		}
	}
	cv::Mat ave(dst_height, dst_width, img.type(), ave_data);

	// calc ganma
	double average_city_per_pixel = cv::mean(img).val[0] / 255.0;
	double ganma = (use_improve_higher_contrast) ?
		sqrt((double)require_city_num / (dst_height * dst_width)) / (average_city_per_pixel) :
		(double)require_city_num / (dst_height * dst_width) / (average_city_per_pixel);
	if (ganma < 1) { ganma = 1; }

	// calc number of city per pixel
	int city_num = 0;
	int* city_per_pixel = new int[dst_width * dst_height];
	for (int y = 0; y < dst_height; y++) {
		for (int x = 0; x < dst_width; x++) {
			int g = (int)(ganma * (ave.data[x + y*dst_width] / 255.0));
			city_per_pixel[x + y*dst_width] = (use_improve_higher_contrast) ? (int)(1 / 3.0 * g * g) : g;
			city_num += city_per_pixel[x + y*dst_width];
		}
	}
	OUT_instance.cityNum = city_num;

	// deploy city
	std::uniform_real_distribution<float> position(0.0, 1.0);
	for (int y = 0; y < dst_height; y++) {
		for (int x = 0; x < dst_width; x++) {
			for (int i = 0; i < city_per_pixel[x + y*dst_width]; i++) {
				OUT_instance.cityPosition.push_back(
					cv::Point2f(
						(x + position(mt)) * grid_size,	// px
						(y + position(mt)) * grid_size	// py
					)
				);
			}
		}
	}

	delete city_per_pixel;
	delete ave_data;
}

// output file
void output(InstanceData& instance, std::string filename) {
	std::ofstream writing_file;

	writing_file.open(filename, std::ios::out);
	writing_file << "NAME : output" << std::endl;
	writing_file << "TYPE : TSP" << std::endl;
	writing_file << "DIMENSION : " << instance.cityNum << std::endl;
	writing_file << "EDGE_WEIGHT_TYPE : EUC_2D" << std::endl;
	writing_file << "NODE_COORD_SECTION" << std::endl;
	for (int i = 0; i < instance.cityNum; i++) {
		cv::Point2f& point = instance.cityPosition[i];
		writing_file << (i + 1) << " " << point.x << " " << point.y << std::endl;
	}
	writing_file << "EOF" << std::endl;
	writing_file.close();
}

std::vector<std::string> split_string(const std::string &str, char sep) {
	std::vector<std::string> v;
	std::stringstream ss(str);
	std::string buffer;
	while (std::getline(ss, buffer, sep)) {
		v.push_back(buffer);
	}
	return v;
}

// input file
void input(InstanceData* OUT_instance, std::string filename) {
	std::ifstream reading_file;

	reading_file.open(filename);
	std::string str;
	bool coord = false;
	while (std::getline(reading_file, str)) {
		if (str == "EOF") { break; }

		if (coord) {
			std::vector<std::string> record = split_string(str, ' ');
			OUT_instance->cityPosition.push_back(cv::Point2f(stof(record[1]), stof(record[2])));
		}

		if (str == "NODE_COORD_SECTION") {
			coord = true;
		}
	}
	reading_file.close();

	OUT_instance->cityNum = OUT_instance->cityPosition.size();
}

// draw point
void draw_point(cv::Mat& img, cv::Point2f p, cv::Scalar color) {
	circle(img, p, 1, color, -1);
}

// draw voronoi border
void draw_voronoi(cv::Mat& img, std::vector<std::vector<cv::Point2f>> facetList) {
	for (auto& trig : facetList) {
		std::vector<cv::Point2f>::iterator it = trig.begin();
		cv::Point2f p1 = *it;
		it++;
		while (it != trig.end()) {
			cv::Point2f p2 = *it;
			line(img, p1, p2, cv::Scalar(0, 255, 0));
			p1 = p2;
			it++;
		}
	}
}

// calculate centroid
cv::Point2f calc_region_centroid(cv::Mat& img, std::vector<cv::Point2f> regionFacet) {
	std::vector<cv::Point> points;
	for (auto& p : regionFacet) { points.push_back(p); }

	// create mask
	cv::Mat mask(img.size().height, img.size().width, 0);
	cv::Mat region_image;
	cv::fillConvexPoly(mask, &points[0], points.size(), cv::Scalar(255), 8);
	img.copyTo(region_image, mask);

	// calculate centroid
	cv::Moments mu = cv::moments(region_image, false);
	cv::Point2f result = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);

	// If the region is filled with black (white in the source image), The result is nan
	if (isnan(result.x)) {
		// calculate the centroid of the mask
		cv::Moments mu = cv::moments(mask, false);
		result = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
	}

	/*
	cv::namedWindow("mask");
	cv::imshow("mask", mask);

	cv::namedWindow("dst");
	cv::imshow("dst", region_image);

	cv::Mat out = cv::imread("img/miku32.png", 1);
	draw_point(out, result, cv::Scalar(0, 0, 255));
	cv::namedWindow("voronoi");
	cv::imshow("voronoi", out);

	std::cout << result.x << "," << result.y << std::endl;
	*/
	return result;
}

int main() {
	std::random_device rnd;
	std::mt19937 mt(rnd());

	// read source image (nega)
	cv::string img_name = "img/miku32.png";
	cv::Mat src = cv::imread(img_name, 0);
	src = ~src;

	InstanceData instance;
	// grid based stipple
	grid_based_stipple(instance, src, mt, 30000, 8, true);
	std::cout << instance.cityNum << std::endl;

	// create subdiv
	cv::Size size = src.size();
	cv::Rect rect(0, 0, size.width, size.height);
	cv::Subdiv2D subdiv(rect);
	subdiv.insert(instance.cityPosition);

	// voronoi division
	std::vector<int> idList;
	std::vector<std::vector<cv::Point2f>> facetList;
	std::vector<cv::Point2f> facetCenters;
	subdiv.getVoronoiFacetList(idList, facetList, facetCenters);

	// calculate centroid voronoi region
	std::vector<cv::Point2f> centroidList;
	for (auto& trig : facetList) {
		centroidList.push_back(calc_region_centroid(src, trig));
	}

	// draw voronoi region
	cv::Mat out = cv::imread(img_name, 1);
	draw_voronoi(out, facetList);
	for (auto& point : instance.cityPosition) {
		draw_point(out, point, cv::Scalar(0, 0, 255));
	}
	for (auto& point : centroidList) {
		draw_point(out, point, cv::Scalar(255, 0, 0));
	}

	cv::namedWindow("voronoi");
	cv::imshow("voronoi", out);
	cv::waitKey();

	// output tsp file
	std::ostringstream filename;
	filename << "tsp/out" << instance.cityNum << ".tsp";
	output(instance, filename.str());

	return 0;
}
