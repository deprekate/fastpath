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
The only library dependency is uthash (which is included).

Fastpath Example
--------------

Run on included sample data:
```sh
fastpath --source a --target e < input 
```
Output is the path of nodes, and should look like
```sh
a
c
d
e
```

The structure of the graph looks like:

```sh
a─────▶b◀─────f
│      │
│      │
▼      ▼
c─────▶d─────▶e
```

* Strings can be used for the nodes, and the weights can be positive or negative long double 
  numbers. The weights can even be in the form of scientific shorthand (1.6E+9).
