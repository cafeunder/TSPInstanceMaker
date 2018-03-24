import sys
import re
import random

def read_tsp_file(filename):
	f = open(filename, "r")
	cities = []
	readCoord = False
	for line in f:
		if "EOF" in line:
			break

		if readCoord:
			record = re.split(" +", line)
			cities.append([float(record[1]), float(record[2])])

		if "NODE_COORD_SECTION" in line:
			readCoord = True

	return cities

if __name__ == "__main__":
	name = sys.argv[1]
	tsp = read_tsp_file(name)

	cityNum = int(sys.argv[2])
	while len(tsp) > cityNum:
		tsp.pop(random.randint(0, len(tsp)-1))

	f = open("out"+str(cityNum)+".tsp", "w")
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

