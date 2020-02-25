/* 

    fastpath

    Copyright (C) 2016 Katelyn McNair and Robert Edwards

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


*/

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <getopt.h>
#include "uthash.h"

#if PY_MAJOR >= 3
#define PY3K
#endif

long double INFINITE = LDBL_MAX;

int n = 0;
int e = 0;

struct my_edge {
	int id;
	int src;
	int dst;
	long double weight;
	UT_hash_handle hh; 
};
struct my_edge *edges = NULL;

struct my_edge *get_edge(int edge_id){
	struct my_edge *s;

	HASH_FIND_INT( edges, &edge_id, s );
	return s;
}

struct my_node {
	char key[256];
	int id;
	UT_hash_handle hh;
};
struct my_node *nodes = NULL;

struct my_name {
	int id;
	char value[256];
	UT_hash_handle hh;
};
struct my_name *names = NULL;

void InitializeGraph(int V, long double *dist, int *parent, int src){
	int i;
	for (i = 0; i < V; i++){
		dist[i] = INFINITE;
		parent[i] = -1;
	}
	dist[src] = 0;
}
void BellmanFord(int V, long double *dist, int *parent){
	int i, flag;
	struct my_edge *s;

	for (i = 1; i < V; i++){
		flag = 1;
		for(s=edges; s != NULL; s=s->hh.next) {
			int u = s->src;
			int v = s->dst;
			long double weight = s->weight;
			if (dist[u] != INFINITE && dist[u] + weight < dist[v]){
				dist[v] = dist[u] + weight;
				parent[v] = u;
				flag = 0;
			}
		}
		if(flag){
                        return;
                }
	}
}
void CheckNegativeWeightCycle(long double *dist){
	struct my_edge *s;
	for(s=edges; s != NULL; s=s->hh.next) {
		int u = s->src;
		int v = s->dst;
		long double weight = s->weight;
		if (dist[u] != INFINITE && dist[u] + weight < dist[v]){
			PyErr_SetString(PyExc_ValueError, "Graph contains negative weight cycle");
        		exit(EXIT_FAILURE);
		}
			
	}
}
void CheckPath(int *parent, int src, int dst){
	int child = dst;
	while(child >= 0){
		if(child == src){
			return;
		}
		child = parent[child];
	}
	PyErr_SetString(PyExc_TypeError, "No path to target\n");
        exit(EXIT_FAILURE);
}
void GetPath(int *parent, int src, int dst, PyObject* pl){
	struct my_name *name;
	HASH_FIND_INT(names, &dst, name);
	if(name == NULL){
		PyErr_SetString(PyExc_ValueError, "Node name not found");
                exit(EXIT_FAILURE);
	}
	if(dst == src){
		PyList_Append(pl, Py_BuildValue("s", name->value));
		return;
	}else{
		GetPath(parent, src, parent[dst], pl);
		PyList_Append(pl, Py_BuildValue("s", name->value));
	}
}

int add_node(char *node_name) {
	struct my_node *node;
	struct my_name *name;
	int src;

	if(!node_name){
		return -1;
	}
	HASH_FIND_STR( nodes, node_name, node);
	if(node == NULL){
		node = (struct my_node*)malloc(sizeof(struct my_node));
		name = (struct my_name*)malloc(sizeof(struct my_name));
		strncpy(node->key, node_name, 256);
		strncpy(name->value, node_name, 256);
		node->id = n;
		name->id = n;
		HASH_ADD_STR( nodes, key, node );
		HASH_ADD_INT( names, id, name );
		src = n; 
		n++;
	}else{
		src = node->id;
	}

	return src;
}

void _add_edge(int edge_id, int src, int dst, long double weight) {
	struct my_edge *s;

	s = (struct my_edge*)malloc(sizeof(struct my_edge));
	s->id = edge_id;
	s->src = src;
	s->dst = dst;
	s->weight = weight;
	HASH_ADD_INT( edges, id, s );
}

static PyObject* add_edge (PyObject* self, PyObject* args){
	//void add_edge(int edge_id, int src, int dst, long double weight) {
	char *token, *err;
	int src, dst;
	long double weight;

	char *edge_string;
	if(!PyArg_ParseTuple(args, "s", &edge_string)) {
		return NULL;
	}

	// parse edge string
	edge_string[strcspn(edge_string, "\n")] = 0;
	token = strtok(edge_string, "\t");	
	// source
	src = add_node(token);
	// destination
	token = strtok(NULL, "\t");
	dst = add_node(token);
	// weight
	token = strtok(NULL, "\t");
	if( src>=0 && dst>=0 && token && (weight = strtold(token, &err)) ){
		_add_edge(e, src, dst, weight);
		e++;
	}else{
		PyErr_SetString(PyExc_ValueError, "Invalid edge");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject* get_path (PyObject* self, PyObject* args, PyObject *kwargs){
	//(int argc, char *argv[]) {
	char *source, *target, *infile;
	struct my_node *node;
	int src, dst;

	static char *kwlist[] = {(char *)"source", (char *)"target", (char *)"var", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ss|s", kwlist, &source, &target, &infile)) 
	{
		return NULL;
	}

/*-----------------------------------------------------------------------------------------------*/
/* Run path finding algorithm                                                                    */
/*-----------------------------------------------------------------------------------------------*/
	HASH_FIND_STR( nodes, source, node);
	if(node == NULL){
		PyErr_SetString(PyExc_ValueError, "Source node not found");
		return NULL;
	}
	src = node->id;
	HASH_FIND_STR( nodes, target, node);
	if(node == NULL){
		PyErr_SetString(PyExc_ValueError, "Target node not found");
		return NULL;
	}
	dst = node->id;

	int V = HASH_COUNT(nodes);
	long double dist[V];
	int parent[V];

	InitializeGraph(V, dist, parent, src);
	BellmanFord(V, dist, parent);
	CheckNegativeWeightCycle(dist); 
	CheckPath(parent, src, dst);

	PyObject *path_list = PyList_New(0);
	GetPath(parent, src, dst, path_list);
   
	return path_list;
}



/*-----------------------------------------------------------------------------------------------*/
/* Read in node data from stdin                                                                  */
/*-----------------------------------------------------------------------------------------------*/
/*
void read_file(){
	struct my_node *node;
	struct my_name *name;
	char buf[256];
	char *token, *err;
	int src, dst;
	long double weight;
	int e = 0;
	int n = 0;
	while (fgets (buf, sizeof(buf), stdin)) {
	}
}
*/

// Our Module's Function Definition struct
// We require this `NULL` to signal the end of our method
static PyMethodDef fastpath_methods[] = {
	{ "get_path", (PyCFunction) get_path, METH_VARARGS | METH_KEYWORDS, "Finds the path in a graph" },
	{ "add_edge", (PyCFunction) add_edge, METH_VARARGS | METH_KEYWORDS, "Adds an edge to the graph" },
	{ NULL, NULL, 0, NULL }
};
//#ifdef PY3K
// module definition structure for python3
static struct PyModuleDef FastPath = {
	 PyModuleDef_HEAD_INIT,
	"FastPath",
	"mod doc",
	-1,
	fastpath_methods
};
// module initializer for python3
PyMODINIT_FUNC PyInit_fastpath(void)
{
	return PyModule_Create(&FastPath);
}
//#else
// module initializer for python2
//PyMODINIT_FUNC initfastpath() {
//	Py_InitModule3("FastPath", fastpath_methods, "mod doc");
//}
//#endif

int
main(int argc, char *argv[])
{
	wchar_t *program = Py_DecodeLocale(argv[0], NULL);
	if (program == NULL) {
		fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
		exit(1);
	}

	/* Add a built-in module, before Py_Initialize */
	PyImport_AppendInittab("fastpath", PyInit_fastpath);

	/* Pass argv[0] to the Python interpreter */
	Py_SetProgramName(program);

	/* Initialize the Python interpreter.  Required. */
	Py_Initialize();

	/* Optionally import the module; alternatively,
           import can be deferred until the embedded script
           imports it. */
	PyImport_ImportModule("fastpath");

	PyMem_RawFree(program);

	return 0;
}	
