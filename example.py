import fastpath as fp
import fastpathz as fz


f = open("edges.txt", "r")

# write edges to the graph
for line in f:
	ret = fp.add_edge(line)
	line = line.replace("\x00", "\t", 2).replace("\x00", "\n")
	ret = fz.add_edge(line)

f.close()

# find the best path from a source node to a target node
for edge in fp.get_path(source="z", target="e"):
	print(edge)
fp.empty_graph()

print()

# find the best path from a source node to a target node
for edge in fz.get_path(source="z", target="e"):
	print(edge)
fz.empty_graph()
