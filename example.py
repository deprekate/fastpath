import fastpath as fp

f = open("input", "r")
for line in f:
	ret = fp.add_edge(line)

print(fp.get_path(source="a", target="e"))
