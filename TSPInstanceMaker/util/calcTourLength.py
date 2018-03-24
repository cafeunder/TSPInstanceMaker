import math
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

def read_tour_file(filename):
	f = open(filename, "r")
	tour = []
	readCity = False
	for line in f:
		line = line[:-1]
		if line == "-1":
			break

		if readCity:
			tour.append(int(line) - 1)

		if line == "TOUR_SECTION":
			readCity = True

	return tour

if __name__ == "__main__":
	tsp = read_tsp_file(sys.argv[1])
	tour = read_tour_file(sys.argv[2])

	tour_coord_x = []
	tour_coord_y = []
	prev = tour[len(tour)-1]

	length = 0
	for city in tour:
		dx = tsp[prev][0] - tsp[city][0]
		dy = tsp[prev][1] - tsp[city][1]
		length += int(math.sqrt(dx * dx + dy * dy) + 0.5)
		prev = city
	print(length)

