import fastpath as fp
import fastpathz as fz


f = open("edges.txt", "r")


# write edges to the graph
for line in f:
	edge = tuple(line.rstrip().split("\t"))
	ret = fz.add_edge(edge)
	ret = fp.add_edge(line)

# find the best path from a source node to a target node
print("\nfastpath", "fastpathz", sep="\t")
for node in zip(fp.get_path(source="a", target="z"),fz.get_path(source="a", target="z")):
	print(node[0], node[1], sep="\t\t")


# reset everything to run scaled fastpathz
fz.empty_graph()
f.seek(0)

# fastpathz only works on integers, so multiply to keep decimal accuracy
scale = lambda a : (a[0], a[1], str(float(a[2])*1000))

for line in f:
	edge = scale(tuple(line.rstrip().split("\t")))
	ret = fz.add_edge(edge)

print("\nfastpathz (multipied)")
for node in fz.get_path(source="a", target="z"):
	print(node, sep="\t\t")
print()

f.close()

exit()
# read edges in graph
print("The edges written to graph")
for edge in fz.get_edges():
	print(edge)
