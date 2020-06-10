import fastpath as fp
#import faulthandler


f = open("edges.txt", "r")

# write edges to the graph
for line in f:
	ret = fp.add_edge(line)

# find the best path from a source node to a target node
for edge in fp.get_path(source="a", target="e"):
	print(edge)

f.close()

fp.empty()

f = open("edges.txt", "r")

#faulthandler.enable()

# write edges to the graph
for line in f:
	ret = fp.add_edge(line)

for edge in fp.get_path(source="a", target="e"):
	print(edge)
