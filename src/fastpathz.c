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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <getopt.h>
#include "uthash.h"
#include "mini-gmp.h"


//long double INFINITE = LDBL_MAX;
mpz_t INFINITE;

const char * remove_decimals(const char *str){
        char *output;
        int c, i, len;
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
        const char *intptr, *expptr;
        int i, c, offset;

        int_size = 0;
        offset = 0;
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

void add_edge(int edge_id, int src, int dst, mpz_t weight) {
	struct my_edge *s;

	s = (struct my_edge*)malloc(sizeof(struct my_edge));
	s->id = edge_id;
	s->src = src;
	s->dst = dst;
	mpz_init(s->weight);
	mpz_set(s->weight, weight);
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

void empty() {
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

}

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
	for (i = 1; i < V-1; i++){
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
void CheckNegativeWeightCycle(mpz_t *dist, struct my_name *names){
	struct my_name *name;
	struct my_edge *s;
	mpz_t temp;
	mpz_init(temp);
	for(s=edges; s != NULL; s=s->hh.next) {
		int u = s->src;
		int v = s->dst;
		mpz_add(temp, dist[u], s->weight);
		if(mpz_cmp(dist[u],INFINITE)!=0 && mpz_cmp(dist[v],temp)>0){
			printf("ERROR: Graph contains negative weight cycle\n");
			printf("%i -> %i\n", u, v);
			HASH_FIND_INT(names, &u, name);
			printf("%s\n", name->value);
			HASH_FIND_INT(names, &v, name);
			printf("%s\n", name->value);
        		exit(EXIT_FAILURE);
		}
			
	}
	mpz_clear(temp);
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
void PrintEdges(){
	struct my_name *name;
	struct my_edge *s;
	for(s=edges; s != NULL; s=s->hh.next) {
		int u = s->src;
		int v = s->dst;
		printf("%i -> %i\t\t", u,v);
		HASH_FIND_INT(names, &u, name);
		printf("%s -> ", name->value);
		HASH_FIND_INT(names, &v, name);
		printf("%s\t", name->value);
		
		mpz_out_str(stdout,10,s->weight);
		printf("\n");
	}
	return;
}

void print_usage() {
    printf("Usage: fastpathz --source START_NODE --target END_NODE < NODES_FILE\n\n");
    printf("The NODE_file is a three column list, where the first column is the source node,\n");
    printf("the second column is the destination node, and the third column is the weight.\n");
}


int main(int argc, char *argv[]) {
	mpz_init_set_str(INFINITE, expand_scinote("1E1000"), 10);
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
	char *token;
	int src, dst;
	mpz_t weight;
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
		mpz_init(weight);
		if(strstr(token, "E") != NULL) {
			mpz_set_str(weight, expand_scinote(token), 10);
		}else{
			mpz_set_str(weight, remove_decimals(token), 10);
		}
		add_edge(e, src, dst, weight);
		mpz_clear(weight);
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
	int parent[V];
	mpz_t dist[V];

	InitializeGraph(V, dist, parent, src);
	BellmanFord(V, dist, parent);
	CheckNegativeWeightCycle(dist, names); 
	CheckPath(parent, src, dst);
	GetPath(parent, src, dst);
   
	return 0;
}
