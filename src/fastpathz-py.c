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
#include <math.h>
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

const char *remove_decimals(const char *str, long scaling){
	char *found;
	size_t loc, len;
	int i;
	char *decimals;

	found = strchr(str, '.');
	if (found){
		loc = found - str;
		len = strlen(str) - loc - 1;
		decimals = (char*) malloc((len + 1) * sizeof(char));
		memcpy( &decimals[0], &str[loc+1], len * sizeof( char ) );
		decimals[len] = '\0';
	}else{
		loc = strlen(str);
		decimals = (char*) malloc(1 * sizeof(char));
		decimals[0] = '\0';
	}
	char* output = (char*) malloc( (loc+scaling+1) * sizeof(char));
	memcpy( &output[0], &str[0], loc * sizeof( char ) );
	i = 0;
	while ( (decimals[i] != '\0') & (scaling > 0) ){
		output[loc+i] = decimals[i];
		scaling--;
		i++;
	}
	while ( scaling > 0){
		output[loc+i] = '0';
		scaling--;
		i++;
	}
	output[loc+i] = '\0';
	return output;
}

const char * expand_scinote(const char *str, long scaling){
	char *output, *ptr;
	unsigned int i, len;
	char *mantissa, *exponent;
	long exp;

	i = 0;
	while ( (str[i] != 'e') & (str[i] != 'E') ){
		i++;
	}
	mantissa = (char*) malloc( (i+1) * sizeof(char));
	memcpy( &mantissa[0], &str[0], i * sizeof( char ) );
	mantissa[i] = '\0';
	i++;
	len = strlen(str) - i;
	exponent = (char*) malloc( (len+1) * sizeof(char));
	memcpy( &exponent[0], &str[i], (len+1) * sizeof( char ) );
	exponent[len+1] = '\0';

	exp = strtol(exponent, &ptr, 10);
	return remove_decimals(mantissa, exp + scaling);
}

struct my_edge {
	int id;
	int src;
	int dst;
	mpz_t weight;
	UT_hash_handle hh; 
};
struct my_edge *edges = NULL;

/*
struct my_edge *get_edge(int edge_id){
	struct my_edge *s;

	HASH_FIND_INT( edges, &edge_id, s );
	return s;
}
*/

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
	struct my_name *name;
	mpz_t temp;
	mpz_init(temp);

	for(s=edges; s != NULL; s=s->hh.next) {
		int u = s->src;
		int v = s->dst;
		mpz_add(temp, dist[u], s->weight);
		if(mpz_cmp(dist[u],INFINITE)!=0 && mpz_cmp(dist[v],temp)>0){
			printf("%i - %i\n", u, v);
			HASH_FIND_INT(names, &u, name);
			printf("%s - ", name->value);
			HASH_FIND_INT(names, &v, name);
			printf("%s\n", name->value);
			mpz_out_str(stdout,10,dist[u]);
			printf("\n");
			mpz_out_str(stdout,10,dist[v]);
			printf("\n");
			mpz_out_str(stdout,10,temp);
			printf("\n");
			printf("\n");
			fflush(stdout);
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


static PyObject* add_edge (PyObject* self, PyObject* args, PyObject *kwargs){
	char *source, *destination, *weight;
	char *token, *edge_string;
	PyObject *obj, *scl;

	PyObject *s = PyDict_GetItemString(PyModule_GetDict(self), "scaling");
	Py_INCREF(s);
	long scaling = PyLong_AsLong(s);
	
	//static char *kwlist[] = {(char *)"", (char *)"scaling", NULL};
	if(!PyArg_ParseTuple(args, "O", &obj)){
		return NULL;
	}
	if(!PyTuple_Check(obj)){
		if(!PyArg_ParseTuple(args, "s", &edge_string)){
			return NULL;
		}
		// parse edge string
		edge_string[strcspn(edge_string, "\n")] = 0;
		// source
		token = strtok(edge_string, "\t");
		source = malloc(255 * sizeof(char));
		strcpy(source, token);
		// destination
		token = strtok(NULL, "\t");
		destination = malloc(255 * sizeof(char));
		strcpy(destination, token);
		// weight
		token = strtok(NULL, "\t");
		weight = malloc(255 * sizeof(char));
		strcpy(weight, token);
	}else{
  		if(!PyArg_ParseTuple(obj, "sss", &source, &destination, &weight)){
        	return NULL;
		}
    }
	int src, dst;
	mpz_t wgt;

	// source
	src = add_node(source);
	// destination
	dst = add_node(destination);
	// weight
	if( src>=0 && dst>=0 && weight){
		mpz_init(wgt);
		if( (strstr(weight, "E")!=NULL) || (strstr(weight, "e")!=NULL) ) {
			mpz_set_str(wgt,  expand_scinote(weight, scaling), 10);
		}else{
			mpz_set_str(wgt, remove_decimals(weight, scaling), 10);
		}
		_add_edge(e, src, dst, wgt);
		mpz_clear(wgt);
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
	mpz_init_set_str(INFINITE, expand_scinote("1E1000", 0), 10);

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


typedef struct {
    PyObject_HEAD
	struct my_edge *s;
} IterObject;

PyObject* Iterable_iter(PyObject *self)
{
	Py_INCREF(self);
	IterObject *p = (IterObject *)self;
	p->s = edges;
	return self;
}
PyObject* Iterable_iternext(PyObject *self)
{
	struct my_name *name1;
	struct my_name *name2;
	char *w;

	IterObject *p = (IterObject *)self;
	if(p->s != NULL){
		HASH_FIND_INT(names, &p->s->src, name1);
		HASH_FIND_INT(names, &p->s->dst, name2);
		w = mpz_get_str(NULL,10,p->s->weight);
		p->s = p->s->hh.next;
		return Py_BuildValue("(sss)", name1->value, name2->value, w);
	}else{
		PyErr_SetNone(PyExc_StopIteration);
		return NULL;
	}
}
static void Iter_dealloc(IterObject *self){ PyObject_Del(self); }

static PyTypeObject IterableType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "Iter",
    .tp_doc = "Custom objects",
    .tp_basicsize = sizeof(IterObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_dealloc = (destructor) Iter_dealloc,
	.tp_iter      = Iterable_iter,
	.tp_iternext  = Iterable_iternext
};
static PyObject* get_edges(PyObject *self, PyObject *Py_UNUSED(ignored)) {
	IterObject *p;

	p = PyObject_New(IterObject, &IterableType);
	if (!p) return NULL;

	if (!PyObject_Init((PyObject *)p, &IterableType)) {
    	Py_DECREF(p);
    	return NULL;
 	}
	return (PyObject *) p;
}





// Our Module's Function Definition struct
// We require this `NULL` to signal the end of our method
static PyMethodDef fastpathz_methods[] = {
	{ "get_path",    (PyCFunction)    get_path, METH_VARARGS | METH_KEYWORDS, "Finds the path in a graph" },
	{ "add_edge",    (PyCFunction)    add_edge, METH_VARARGS | METH_KEYWORDS, "Adds an edge to the graph" },
	{ "get_edges",   (PyCFunction)   get_edges, METH_VARARGS | METH_KEYWORDS, "Gets the edges in the graph" },
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
	fastpathz_methods,
};
// module initializer for python3
PyMODINIT_FUNC PyInit_fastpathz(void)
{
	PyObject *m = PyModule_Create(&FastPath);
	PyModule_AddIntConstant(m, "scaling", 0);
	//PyModule_AddObject(m, "scaling", (PyObject *) PyUnicode_FromString("asd"));
	return m;
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
