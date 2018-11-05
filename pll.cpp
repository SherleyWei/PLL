#include <cstdio>
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#define MAX_ID 10000
#define MAX_EDGES 300000
#define MAX_DIST 3000000

using namespace std;

struct dict{
	int id;
	int degree = 0;
	bool operator< (dict o){
		return degree > o.degree;
	}
	void print(){
		printf("{%d, %d}\n", id, degree);
	}
}dicts[MAX_ID];

struct node{
	int first = -1;
	int degree = 0;
	void print(){
		printf("{%d, %d}\n", first, degree);
	}
}nodes[MAX_ID];

struct edge{
	int from;
	int to;
	int next;
	void print(){
		printf("{%d, %d, %d}\n", from, to, next);
	}
}edges[MAX_EDGES];

int ptr_edge = 0;
int num_edge, num_node;

void insert_edge(int from, int to){
	edges[ptr_edge] = {from, to, nodes[from].first};
	nodes[from].first = ptr_edge;
	ptr_edge ++;
}

void print_arr(int type){
	if(type == 0){
		for(int i=0; i<num_node; i++){
			dicts[i].print();
		}
	}
	else if(type == 1){
		for(int i=0; i<num_edge; i++){
			edges[i].print();
		}
	}
}

void read_graph(){
	scanf("%d%d", &num_node, &num_edge);
	printf("Total nodes: %d, total edges: %d\n", num_node, num_edge);
	int i;
	for(i=0; i<MAX_ID; i++){
		dicts[i].id = i;
	}
	int from, to;
	while(scanf("%d%d", &from, &to) != EOF){
		dicts[from].degree ++;
		dicts[to].degree ++;
		nodes[from].degree ++;
		nodes[to].degree ++;
		insert_edge(from, to);
		insert_edge(to, from);
	}
	sort(dicts, dicts + MAX_ID);
}

struct Label{
	int dest_ind;
	int dist;
};
struct Labels{
	vector<Label> data[MAX_ID];
	int size(){
		int sum = 0;
		for(int i=0; i<MAX_ID; i++){
			sum += data[i].size();
		}
		return sum;
	}
}labels;

int query(int from, int to, Labels& l){
    int distance = MAX_DIST;
    int from_len = l.data[from].size();
    int to_len = l.data[to].size();
    if(from_len == 0 || to_len == 0) return distance;
    int from_point = 0;
    int to_point = 0;
    while(from_point < from_len && to_point < to_len){
    	if(l.data[from][from_point].dest_ind == l.data[to][to_point].dest_ind){
    		distance = min(distance, l.data[from][from_point].dist + l.data[to][to_point].dist);
    		from_point ++;
    		to_point ++;
    	}
    	else if(l.data[from][from_point].dest_ind < l.data[to][to_point].dest_ind){
    		from_point ++;
    	}
    	else{
    		to_point ++;
    	}
    }
    return distance;
}

void pruned_bfs(int index, Labels l){
	int vertex = dicts[index].id;
    int distance[MAX_ID];
    for(int i=0; i<MAX_ID; i++){
    	if(i == vertex) distance[i] = 0;
    	else distance[i] = MAX_DIST;
    }
    queue<int> q;
    q.push(vertex);
    while(!q.empty()){
    	int u = q.front();
    	q.pop();
    	if(query(vertex, u, l) <= distance[u]) continue;
    	Label tmp;
    	tmp.dest_ind = index;
    	tmp.dist = distance[u];
    	labels.data[u].push_back(tmp);
    	int neighbor = nodes[u].first;
    	while(neighbor != -1){
    		edge e = edges[neighbor];
    		if(distance[e.to] == MAX_DIST){
    			distance[e.to] = distance[u] + 1;
    			q.push(e.to);
    		}
    		neighbor = e.next;
    	}
    }
}

void index_stage(){
	for(int i=0; i<num_node; i++){
        int num_label1 = labels.size();
        pruned_bfs(i, labels);
        int num_label2 = labels.size();
        printf("Indexes: %d, Node: %d, add label: %d\n", i, dicts[i].id, num_label2 - num_label1);
	}
}


int main(){
	freopen("complete.txt", "r", stdin);
	freopen("log.txt", "w", stdout);
	read_graph();
	index_stage();
	// printf("Test result:\n");
	// printf("%d\n", query(1, 5, labels));
	// printf("%d\n", query(2, 5, labels));
	// printf("%d\n", query(5, 5, labels));
	// printf("%d\n", query(4, 2, labels));
	// printf("%d\n", query(3, 1, labels));
	// printf("%d\n", query(1, 3, labels));
	// printf("%d\n", query(1, 1, labels));
	return 0;
}