import os
import sys
import re
from tsp_file_util import *


if __name__ == "__main__":
	name = sys.argv[1]
	tsp = read_tsp_file(name)
	print('To end this program, please enter "end"')

	while True:
		s = input("point(x y): ")
		if s == "end":
			break

		p = s.split(" ")
		min_dist = float("inf")
		min_index = -1
		index = 0
		for city in tsp:
			dx = float(p[0]) - city[0]
			dy = float(p[1]) - city[1]
			dist = dx * dx + dy * dy
			if dist < min_dist:
				min_dist = dist
				min_index = index
			index += 1
		print(str(min_index) + str(tsp[min_index]))
		tsp.pop(min_index)

	base_name = os.path.basename(name)
	f = open("_" + base_name, "w")
	index = 0
	f.write("NAME : output\n")
	f.write("TYPE : TSP\n")
	f.write("DIMENSION : " + str(len(tsp)) + "\n")
	f.write("EDGE_WEIGHT_TYPE : EUC_2D\n")
	f.write("NODE_COORD_SECTION\n")
	for city in tsp:
		f.write(str(index+1) + " " + str(city[0]) + " " + str(city[1]) + "\n")
		index += 1
	f.write("EOF\n")
	f.close()
