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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <getopt.h>
#include "uthash.h"


long double INFINITE = LDBL_MAX;

struct my_edge {
	int id;
	int src;
	int dst;
	long double weight;
	UT_hash_handle hh; 
};
struct my_edge *edges = NULL;

void add_edge(int edge_id, int src, int dst, long double weight) {
	struct my_edge *s;

	s = (struct my_edge*)malloc(sizeof(struct my_edge));
	s->id = edge_id;
	s->src = src;
	s->dst = dst;
	s->weight = weight;
	HASH_ADD_INT( edges, id, s );
}
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
			printf("ERROR: Graph contains negative weight cycle\n");
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
	printf("ERROR: No path to target\n");
        exit(EXIT_FAILURE);
}
void GetPath(int *parent, int src, int dst){
	struct my_name *name;
	HASH_FIND_INT(names, &dst, name);
	if(name == NULL){
                printf("ERROR: Node name for %i not found\n", dst);
                exit(EXIT_FAILURE);
	}
	if(dst == src){
		printf("%s\n", name->value);
		return;
	}else{
		GetPath(parent, src, parent[dst]);
		printf("%s\n", name->value);
	}
}
void print_usage() {
    printf("Usage: fastpath --source START_NODE --target END_NODE < NODES_FILE\n\n");
    printf("The NODE_file is a three column list, where the first column is the source node,\n");
    printf("the second column is the destination node, and the third column is the weight.\n");
}


int main(int argc, char *argv[]) {

	char *source = "", *target = "";
	int opt= 0;
	static struct option long_options[] = {
	  {"source",  required_argument, 0, 's'},
	  {"target",  required_argument, 0, 't'},
	  {0, 0, 0, 0}
	};
	int option_index = 0;
	while ((opt = getopt_long(argc, argv, "s:t:", long_options, &option_index )) != -1) {
		switch (opt) {
			case 's' : source = optarg; 
				break;
			case 't' : target = optarg;
				break;
			default: print_usage(); 
				exit(EXIT_FAILURE);
		}
	}
	if(source[0] == '\0' || target[0] == '\0'){
		print_usage();
		exit(EXIT_FAILURE);
	}
	/* remaining command line arguments (not options). */
	if (optind < argc){
		while (optind < argc)
			break;
			//printf ("%s ", argv[optind++]);
 	}


/*-----------------------------------------------------------------------------------------------*/
/* Read in node data from stdin                                                                  */
/*-----------------------------------------------------------------------------------------------*/

	struct my_node *node;
	struct my_name *name;
	char buf[256];
	char *token, *err;
	int src, dst;
	long double weight;
	int e = 0;
	int n = 0;
	while (fgets (buf, sizeof(buf), stdin)) {
		buf[strcspn(buf, "\n")] = 0;
		token = strtok(buf, "\t");	
		
		HASH_FIND_STR( nodes, token, node);
		if(node == NULL){
			node = (struct my_node*)malloc(sizeof(struct my_node));
			name = (struct my_name*)malloc(sizeof(struct my_name));
			strncpy(node->key, token, 256);
			strncpy(name->value, token, 256);
			node->id = n;
			name->id = n;
			HASH_ADD_STR( nodes, key, node );
			HASH_ADD_INT( names, id, name );
			src = n; 
			n++;
		}else{
			src = node->id;
		}
		token = strtok(NULL, "\t");
		HASH_FIND_STR( nodes, token, node);
		if(node == NULL){
			node = (struct my_node*)malloc(sizeof(struct my_node));
			name = (struct my_name*)malloc(sizeof(struct my_name));
			strncpy(node->key, token, 256);
			strncpy(name->value, token, 256);
			node->id = n;
			name->id = n;
			HASH_ADD_STR( nodes, key, node );
			HASH_ADD_INT( names, id, name );
			dst = n; 
			n++;
		}else{
			dst = node->id;
		}
		token = strtok(NULL, "\t");
		weight = strtold(token, &err);
		add_edge(e, src, dst, weight);
		e++;

	}
	
/*-----------------------------------------------------------------------------------------------*/
/* Run path finding algorithm                                                                    */
/*-----------------------------------------------------------------------------------------------*/


	HASH_FIND_STR( nodes, source, node);
	if(node == NULL){
		printf("ERROR: Source node %s not found\n", source);
		exit(EXIT_FAILURE);
	}
	src = node->id;
	HASH_FIND_STR( nodes, target, node);
	if(node == NULL){
		printf("ERROR: Target node %s not found\n", target);
		exit(EXIT_FAILURE);
	}
	dst = node->id;

	int V = HASH_COUNT(nodes);
	long double dist[V];
	int parent[V];

	InitializeGraph(V, dist, parent, src);
	BellmanFord(V, dist, parent);
	CheckNegativeWeightCycle(dist); 
	CheckPath(parent, src, dst);
	GetPath(parent, src, dst);
   
	return 0;
}
