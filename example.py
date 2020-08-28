import fastpath as fp
import fastpathz as fz


f = open("edges.txt", "r")


#--------------------------------------------------------------------#
# write edges to the graph
#--------------------------------------------------------------------#
for line in f:
	edge = tuple(line.rstrip().split("\t"))
	ret = fz.add_edge(edge)
	ret = fp.add_edge(line)

#--------------------------------------------------------------------#
# find the best path from a source node to a target node
#--------------------------------------------------------------------#
print("\nfastpath", "fastpathz", sep="\t")
for node in zip(fp.get_path(source="A", target="Z"),fz.get_path(source="A", target="Z")):
	print(node[0], node[1], sep="\t\t")


# reset everything
fz.empty_graph()
f.seek(0)

#--------------------------------------------------------------------#
# fastpathz only works on integers, so multiply to keep decimal accuracy
#--------------------------------------------------------------------#
scale = lambda a : (a[0], a[1], str(float(a[2])*1000))

for line in f:
	edge = scale(tuple(line.rstrip().split("\t")))
	ret = fz.add_edge(edge)

print("\nfastpathz (scaled)")
for node in fz.get_path(source="A", target="Z"):
	print(node, sep="\t\t")
print()


# reset everything
fz.empty_graph()
f.seek(0)

#--------------------------------------------------------------------#
# Writing edges are tab delimited strings is still supported
#--------------------------------------------------------------------#
print("\nfastpath tab delimited")
for line in f:
    ret = fz.add_edge(line)

for node in fz.get_path(source="A", target="Z"):
    print(node)

f.close()

exit()
# read edges in graph
print("The edges written to graph")
for edge in fz.get_edges():
	print(edge)
