import fastpath as fp
import fastpathz as fz


f = open("edges.txt", "r")


# write edges to the graph
for edge in f:
	ret = fp.add_edge(edge)

	# the fastpath C extension alters the line string, so revert it
	edge = edge.replace("\x00", "\t", 2).replace("\x00", "\n")
	ret = fz.add_edge(edge)

# find the best path from a source node to a target node
print("\nfastpath", "fastpathz", sep="\t")
for node in zip(fp.get_path(source="z", target="e"),fz.get_path(source="z", target="e")):
	print(node[0], node[1], sep="\t\t")


# reset everything and run multiplied fastpathz
fz.empty_graph()
f.seek(0)

# fastpathz only works on integers, so multiply to keep decimal accuracy
multiply = lambda a: "\t".join(a.split("\t")[0:2] + [str(float(a.split("\t")[2])*100)])

for edge in f:
	ret = fz.add_edge(multiply(edge))

print("\nfastpathz (multipied)")
for node in fz.get_path(source="z", target="e"):
	print(node, sep="\t\t")
print()

f.close()
