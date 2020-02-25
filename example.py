import fastpath as fp


f = open("nodes.txt", "r")
for line in f:
	ret = fp.add_edge(line)

for edge in fp.get_path(source="Node('source','source',0,0)", target="Node('target','target',0,48837)"):
	print(edge)
