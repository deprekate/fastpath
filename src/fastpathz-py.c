/* 

    fastpathz

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
#include "mini-gmp.h"

#if PY_MAJOR >= 3
#define PY3K
#endif

int n = 0;
int e = 0;
//long double INFINITE = LDBL_MAX;
mpz_t INFINITE;

const char * remove_decimals(const char *str){
	char *output;
	unsigned int c, i, len;
	len = 0;
	for (i = 0; i < strlen(str); i++){
		c = (unsigned char) str[i];
		if(c == '.'){
			break;
		}
		len = i+1;
	}
	output = malloc(len+1);
	strncpy(output, str, len);
	output[len] = '\0';
	return output;
}

const char * expand_scinote(const char *str){
	char *output, *ptr;
	size_t int_size, exp_size, size;
	const char *expptr;
	unsigned int i, c;

	int_size = exp_size = 0;
	expptr = NULL;
	for (i = 0; i < strlen(str); i++){
		c = (unsigned char) str[i];
		//printf("c: %c\n", c);
		if(c == 'e' || c == 'E'){
			if(str[i+1] == '-'){
				output = (char *) malloc(2);
				output[0] = '0';
				output[1] = '\0';
				return output;
			}
			expptr = str + i + 1;
			exp_size = i;
			if(!int_size){
				int_size = i;
			}
			break;
		}else if(c == '.'){
			int_size = i;
		}
	}
	size = int_size + strtol(expptr, &ptr, 10);
	output = malloc(size + 1);
	output[size] = '\0';

	for(i = 0; i < size; i++){
		output[i] = '0';
	}
	for(i = 0; i < int_size; i++){
		output[i] = str[i];
	}
	for(i = int_size+1; i < exp_size; i++){
		output[i-1] = str[i];
	}
	return output;
}

struct my_edge {
	int id;
	int src;
	int dst;
	mpz_t weight;
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

void InitializeGraph(int V, mpz_t *dist, int *parent, int src){
	int i;
	for (i = 0; i < V; i++){
		mpz_init_set(dist[i], INFINITE);
		parent[i] = -1;
	}
	mpz_set_str(dist[src], "0", 10);
}
void BellmanFord(int V, mpz_t *dist, int *parent){
	int i, flag;
	struct my_edge *s;
	mpz_t temp;
	mpz_init(temp);

	for (i = 1; i < V; i++){
		flag = 1;
		for(s=edges; s != NULL; s=s->hh.next) {
			int u = s->src;
			int v = s->dst;

			mpz_add(temp, dist[u], s->weight);

			//mpz_out_str(stdout,10,dist[u]);
			if(mpz_cmp(dist[v],temp)>0){
				mpz_set(dist[v], temp);
				parent[v] = u;
				flag = 0;
			}
		}
		if(flag){
			return;
		}
	}
	mpz_clear(temp);
}
int CheckNegativeWeightCycle(mpz_t *dist){
	struct my_edge *s;
	mpz_t temp;
	mpz_init(temp);

	for(s=edges; s != NULL; s=s->hh.next) {
		int u = s->src;
		int v = s->dst;
		mpz_add(temp, dist[u], s->weight);
		if(mpz_cmp(dist[u],INFINITE)!=0 && mpz_cmp(dist[v],temp)>0){
			PyErr_SetString(PyExc_ValueError, "Graph contains negative weight cycle");
			return 0; //	exit(EXIT_FAILURE);
		}
	}
	mpz_clear(temp);
	return 1;
}
int CheckPath(int *parent, int src, int dst){
	int child = dst;
	while(child >= 0){
		if(child == src){
			return 1;
		}
		child = parent[child];
	}
	PyErr_SetString(PyExc_TypeError, "No path to target\n");
    return 0; //exit(EXIT_FAILURE);
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

		// node_name to id
		strncpy(node->key, node_name, 256);
		node->id = n;
		HASH_ADD_STR( nodes, key, node );

		// id to node_name
		name->id = n;
		strncpy(name->value, node_name, 256);
		HASH_ADD_INT( names, id, name );

		src = n; 
		n++;
	}else{
		src = node->id;
	}

	return src;
}

void _add_edge(int edge_id, int src, int dst, mpz_t weight) {
	struct my_edge *s;

	s = (struct my_edge*)malloc(sizeof(struct my_edge));
	s->id = edge_id;
	s->src = src;
	s->dst = dst;
	mpz_init(s->weight);
	mpz_set(s->weight, weight);
	HASH_ADD_INT( edges, id, s );
}


void _empty() {
	struct my_edge *current_edge, *tmp1;
	struct my_node *current_node, *tmp2;
	struct my_name *current_name, *tmp3;

	HASH_ITER(hh, edges, current_edge, tmp1) {
		HASH_DEL(edges, current_edge);  /* delete; users advances to next */
		free(current_edge);             /* optional- if you want to free  */
	}

	HASH_ITER(hh, nodes, current_node, tmp2) {
		HASH_DEL(nodes, current_node);  /* delete; users advances to next */
		free(current_node);             /* optional- if you want to free  */
	}

	HASH_ITER(hh, names, current_name, tmp3) {
		HASH_DEL(names, current_name);  /* delete; users advances to next */
		free(current_name);             /* optional- if you want to free  */
	}
	n = 0;
	e = 0;

}
static PyObject* empty_graph (){
	_empty();
	Py_RETURN_NONE;
}

static PyObject* add_edge (PyObject* self, PyObject* args){
	//void add_edge(int edge_id, int src, int dst, long double weight) {
	char *token;
	int src, dst;
	mpz_t weight;

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
	if( src>=0 && dst>=0 && token){
		mpz_init(weight);
		if(strstr(token, "E") != NULL) {
			mpz_set_str(weight, expand_scinote(token), 10);
		}else{
			mpz_set_str(weight, remove_decimals(token), 10);
		}
		_add_edge(e, src, dst, weight);
		mpz_clear(weight);
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
	mpz_init_set_str(INFINITE, expand_scinote("1E1000"), 10);

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
	int parent[V];
	mpz_t dist[V];

	InitializeGraph(V, dist, parent, src);
	BellmanFord(V, dist, parent);
	if(!CheckNegativeWeightCycle(dist) || !CheckPath(parent, src, dst)){
		return NULL;
	}

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
static PyMethodDef fastpathz_methods[] = {
	{ "get_path",    (PyCFunction)    get_path, METH_VARARGS | METH_KEYWORDS, "Finds the path in a graph" },
	{ "add_edge",    (PyCFunction)    add_edge, METH_VARARGS | METH_KEYWORDS, "Adds an edge to the graph" },
	{ "empty_graph", (PyCFunction) empty_graph, METH_VARARGS | METH_KEYWORDS, "Empties out the graph" },
	{ NULL, NULL, 0, NULL }
};
//#ifdef PY3K
// module definition structure for python3
static struct PyModuleDef FastPath = {
	 PyModuleDef_HEAD_INIT,
	"FastPath",
	"mod doc",
	-1,
	fastpathz_methods
};
// module initializer for python3
PyMODINIT_FUNC PyInit_fastpathz(void)
{
	return PyModule_Create(&FastPath);
}
//#else
// module initializer for python2
//PyMODINIT_FUNC initfastpathz() {
//	Py_InitModule3("FastPath", fastpathz_methods, "mod doc");
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
	PyImport_AppendInittab("fastpathz", PyInit_fastpathz);

	/* Pass argv[0] to the Python interpreter */
	Py_SetProgramName(program);

	/* Initialize the Python interpreter.  Required. */
	Py_Initialize();

	/* Optionally import the module; alternatively,
           import can be deferred until the embedded script
           imports it. */
	PyImport_ImportModule("fastpathz");

	PyMem_RawFree(program);

	return 0;
}	
