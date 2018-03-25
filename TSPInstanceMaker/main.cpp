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
	double ave_sum = 0;
	char* ave_data = new char[dst_width * dst_height];
	cv::Rect rect(0, 0, grid_size, grid_size);
	for (int y = 0; y < dst_height; y++) {
		for (int x = 0; x < dst_width; x++) {
			rect.x = x * grid_size;
			rect.y = y * grid_size;

			cv::Mat roi = img(rect);
			cv::Scalar m = cv::mean(roi);
			ave_data[x + y*dst_width] = (char)m.val[0];
			ave_sum += m.val[0];
		}
	}
	cv::Mat ave(dst_height, dst_width, img.type(), ave_data);
	double ave_ave = ave_sum / (dst_height * dst_width);

	// calc ganma
	double average_city_per_pixel = ave_ave / 255.0;
	double ganma = (use_improve_higher_contrast) ?
		sqrt((double)require_city_num / (dst_height * dst_width)) / (average_city_per_pixel) :
		(double)require_city_num / (dst_height * dst_width * average_city_per_pixel);
	if (ganma < 1) { ganma = 1; }
	std::cout << "ganma : " << ganma << std::endl;

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

std::vector<std::string> split_string(const std::string &str, char sep) {
	std::vector<std::string> v;
	std::stringstream ss(str);
	std::string buffer;
	while (std::getline(ss, buffer, sep)) {
		v.push_back(buffer);
	}
	return v;
}

// output file
void output(InstanceData& instance, std::string instance_name) {
	std::ostringstream filename;
	filename << "tsp/" << instance_name << instance.cityNum << ".tsp";

	std::ofstream writing_file;

	writing_file.open(filename.str(), std::ios::out);
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
cv::Point2f calc_region_centroid(cv::Mat& img, std::vector<cv::Point2f> regionFacet, bool calc_mask_centroid = true) {
	std::vector<cv::Point> points;
	for (auto& p : regionFacet) { points.push_back(p); }

	// create mask
	cv::Mat mask(img.size().height, img.size().width, 0, cv::Scalar(0));

	cv::Mat region_image;
	cv::fillConvexPoly(mask, &points[0], points.size(), cv::Scalar(255), 8);
	img.copyTo(region_image, mask);

	// calculate centroid
	cv::Moments mu = cv::moments(region_image, false);
	cv::Point2f result = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);

	// If the region is filled with black (white in the source image), The result is nan
	if (calc_mask_centroid) {
		if (isnan(result.x)) {
			// calculate the centroid of the mask
			cv::Moments mu = cv::moments(mask, false);
			result = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
		}
	}

	return result;
}

std::vector<int> idList;
std::vector<std::vector<cv::Point2f>> facetList;
std::vector<cv::Point2f> facetCenters;

// centroid voronoi stipple
void centroid_voronoi_stipple(InstanceData* instance, cv::Subdiv2D* subdiv, cv::Mat& img) {
	// voronoi division
	subdiv->getVoronoiFacetList(idList, facetList, facetCenters);

	// DEBUG
	int count = 0; int prog = instance->cityNum / 10; if (prog == 0) { prog = 1; }

	// calculate centroid voronoi region
	std::vector<cv::Point2f> centroidList;
	for (auto& trig : facetList) {
		centroidList.push_back(calc_region_centroid(img, trig));

		// DEBUG
		if (count != 0 && count%prog == 0) { std::cout << "*"; } count++;
	}

	instance->cityPosition = centroidList;
	std::cout << std::endl;
}

// remove the region filled with white.
void remove_blank_region(InstanceData* instance, cv::Subdiv2D* subdiv, cv::Mat& img) {
	subdiv->getVoronoiFacetList(idList, facetList, facetCenters);

	// DEBUG
	int count = 0; int prog = instance->cityNum / 10; if (prog == 0) { prog = 1; }

	std::vector<cv::Point2f> result;
	for (auto& trig : facetList) {
		cv::Point2f point = calc_region_centroid(img, trig, false);
		if (!isnan(point.x)) {
			result.push_back(point);
		}

		// DEBUG
		if (count != 0 && count%prog == 0) { std::cout << "*"; } count++;
	}

	instance->cityPosition = result;
	instance->cityNum = result.size();
	std::cout << std::endl;
}

// DEBUG YOU NO SYUTURYOKU MESODDO
void DEBUG_output(cv::Mat& img, InstanceData& instance, int id = 0) {
	// draw centroid (DEBUG)
	cv::Mat dst(img.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	draw_voronoi(dst, facetList);
	for (auto& point : facetCenters) {
		draw_point(dst, point, cv::Scalar(0, 0, 255));
	}
	for (auto& point : instance.cityPosition) {
		draw_point(dst, point, cv::Scalar(255, 0, 0));
	}

	std::ostringstream filename;
	filename << "tmp/" << id << ".png";
	cv::imwrite(filename.str(), dst);

	filename.str("");
	filename.clear(std::stringstream::goodbit);
	filename << "out" << id << "_";
	// output(instance, filename.str());
}

int main(int argc, char *argv[]) {
	// === option specify === //
	// image file path
	const cv::string IMG_PATH = "img/miku3.png";
	// initial tsp file path (If do not read the tsp file, specify empty string "")
	const cv::string TSP_PATH = "";
	// output tsp name
	const cv::string OUT_NAME = "mk";
	// number of cities requested (In most cases, the number of cities output will be less than the specified number)
	const int REQUEST_CITY = 47000;
	// grid size used in grid-based algorithm
	const int GRID_SIZE = 8;
	// number of iterations of voronoi stippling algorithm
	const int ITERATION_NUM = 10;
	// debug output?
	const bool DEBUG_OUTPUT = true;



	// random
	std::random_device rnd;
	std::mt19937 mt(rnd());

	// read source image (nega)
	cv::Mat src = cv::imread(IMG_PATH, 0);
	src = ~src;

	InstanceData instance;
	if (TSP_PATH == "") {
		// grid based stipple
		grid_based_stipple(instance, src, mt, REQUEST_CITY, GRID_SIZE, false);
		std::cout << "cityNum : " << instance.cityNum << std::endl;
	} else {
		// read tspfile
		input(&instance, TSP_PATH);
	}

	// debug output
	if (DEBUG_OUTPUT) {
		cv::Mat dst(src.size(), CV_8UC3, cv::Scalar(255, 255, 255));
		// cv::cvtColor(~src, dst, CV_GRAY2BGR);
		for (auto& point : instance.cityPosition) {
			draw_point(dst, point, cv::Scalar(255, 0, 0));
		}
		cv::imwrite("tmp/0.png", dst);
	}

	// create subdiv
	cv::Size size = src.size();
	cv::Rect rect(0, 0, size.width, size.height);

	cv::Subdiv2D subdiv;
	int prog = (instance.cityNum == 0) ? 1 : instance.cityNum / 10;
	// centroid voronoi stipple
	for (int i = 0; i < ITERATION_NUM; i++) {
		// draw voronoi region (DEBUG)
		std::cout << i << std::endl;

		// create subdiv
		subdiv.initDelaunay(rect);
		subdiv.insert(instance.cityPosition);
		
		// centroid voronoi stipple
		centroid_voronoi_stipple(&instance, &subdiv, src);

		// draw centroid (DEBUG)
		if (DEBUG_OUTPUT) {
			DEBUG_output(src, instance, (i + 1));
		}
	}

	std::cout << "blank remove" << std::endl;
	subdiv.initDelaunay(rect);
	subdiv.insert(instance.cityPosition);
	remove_blank_region(&instance, &subdiv, src);
	output(instance, OUT_NAME);

	// show output image
	if (DEBUG_OUTPUT) {
		cv::Mat dst(src.size(), CV_8UC3, cv::Scalar(255, 255, 255));
		// cv::cvtColor(~src, dst, CV_GRAY2BGR);
		for (auto& point : instance.cityPosition) {
			draw_point(dst, point, cv::Scalar(255, 0, 0));
		}
		cv::imwrite("tmp/final.png", dst);
	}
	return 0;
}
