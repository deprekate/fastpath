Introduction
------------

Fastpath is a fast and lightweight tool for finding the shortest path in a weighted
graph.  As input it only needs the starting node, the ending node, and the weights
of each node to node edge.  For versatility it uses the Bellman-Ford algorithm, which
allows for negative weights.  Future version will incorporate the Dijkstra algorithm
to speed up runtimes on graphs that do not contain negative edges.
To install `fastpath`,
```sh
git clone git@github.com:deprekate/fastpath.git
cd fastpath; make
```
The only library dependency for fastpath is uthash (which is included).
The fastpathz has the extra dependency of mini-gmp (which is included).

There are two flavors of `fastpath`.  The first is the default `fastpath`, which will work
for 99% of needed cases.  It's limitation is that it uses the C-type long double for edge
weights, which can cause rounding errors if you have extremely large/small numbers for edge
weights (ie -1E50 or 1E50).
This is because during the path relaxation step of the Bellmanford code, C cannot distinguish
a difference between 1E50 and 1E50 + 1
If your numbers are extremely large/small, then you can use the `fastpathz` version, which
uses infinite-precision integers as edge weights. The downside of using `fastpathz` is that
decimal places get dropped, so the C code does not distinguish between 1 and 1.1.  This 
limitation can partially be overcome by just multiplying all your weights by an arbitrary
number.



Fastpath Example
--------------

Run either flavor on the included sample data:
```sh
fastpath --source z --target e < edges.txt 
```
```sh
fastpathz --source z --target e < edges.txt 
```
The output of either command is the path of nodes, and should look like
```sh
z
a
b
d
e
```

The structure of the graph looks like:

```sh
z ─────▶ a ─────▶ b ◀───── f
         │        │
         │        │
         ▼        ▼
         c ─────▶ d ─────▶ e
```

* Strings can be used for the nodes, and the weights can be positive or negative long double 
  numbers. The weights can even be in the form of scientific shorthand (1.6E+9).


Python Example
--------------

FastPath is now also available as a PIP package available at [pypi.org](https://pypi.org/project/fastpath/)

It is installable by simply using pip
```sh
pip install fastpath 
```

To use in your python code, first import the module, write edges to the graph, and then provide a beginning node (source) and an end node (target)
```sh
import fastpath as fp

f = open("edges.txt", "r")
for line in f:
        ret = fp.add_edge(line)

for edge in fp.get_path(source="a", target="e"):
        print(edge)
```

Output is the path of nodes, and should look like
```sh
$ python example.py 
a
c
d
e
```
