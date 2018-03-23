import sys
import re

def read_tsp_file(filename):
	f = open(filename, "r")
	cities = []
	readCoord = False
	for line in f:
		line = line[:-1]
		if line == "EOF":
			break

		if readCoord:
			record = re.split(" +", line)
			cities.append([float(record[1]), float(record[2])])

		if line == "NODE_COORD_SECTION":
			readCoord = True

	return cities

if __name__ == "__main__":
	print(sys.argv[1])
	name = sys.argv[1]
	tsp = read_tsp_file(name + ".tsp")
	print(len(tsp))

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

	f = open("out.tsp", "w")
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