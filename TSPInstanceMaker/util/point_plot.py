import os
import sys
import re
import matplotlib.pyplot as plt
from tsp_file_util import *


if __name__ == "__main__":
	name = sys.argv[1]
	tsp = read_tsp_file(name)

	tour_coord_x = []
	tour_coord_y = []
	for city in tsp:
		tour_coord_x.append(city[0])
		tour_coord_y.append(city[1])

	fig = plt.figure()
	ax = fig.add_subplot(111, aspect='equal')

	plt.plot(tour_coord_x, tour_coord_y, marker="o", markersize=.1, linewidth=0, color="#000000")
	plt.xlim(min(tour_coord_x), max(tour_coord_x))
	plt.ylim(max(tour_coord_y), min(tour_coord_y))

	plt.tick_params(top='off', bottom='off', labelbottom="off")
	plt.tick_params(left='off', right='off', labelleft="off")

	ax.spines["right"].set_color("None")
	ax.spines["top"].set_color("None")
	ax.spines["left"].set_color("None")
	ax.spines["bottom"].set_color("None")

	base_name, _ = os.path.splitext(os.path.basename(name))
	plt.savefig(base_name + ".png", bbox_inches="tight", pad_inches=0.0)
	plt.show()

	plt.close()
