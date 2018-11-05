#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <thread>
#include <chrono>
#include <atomic>
#define MAX_ID 10000
#define MAX_EDGES 300000
#define MAX_DIST 3000000
#define MAX_THREAD_NUM 3

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

// multi-thread support
int num_thread = 0;
atomic_flag lock_num_thread = ATOMIC_FLAG_INIT;
void edit_num_thread(int change){
	while(lock_num_thread.test_and_set()){
		std::this_thread::yield();
	}
	num_thread += change;
	lock_num_thread.clear();
}

int num_finished_task = 0;
atomic_flag lock_num_finished_task = ATOMIC_FLAG_INIT;
void finish_task(){
	while(lock_num_finished_task.test_and_set()){
		std::this_thread::yield();
	}
	num_finished_task ++;
	lock_num_finished_task.clear();
}

struct Label{
	int dest_ind;
	int dist;
};
struct LabelList{
	vector<Label> data;
	atomic_flag lock = ATOMIC_FLAG_INIT;
	void append(Label item){
		while(lock.test_and_set()){
			//std::this_thread::yield();
		}
		data.push_back(item);
		lock.clear();
	}
	int size(){
		return data.size();
	}
	vector<Label> copy(){
		while(lock.test_and_set()){
			std::this_thread::yield();
		}
		vector<Label> t = data;
		lock.clear();
		return t;
	}
};
struct Labels{
	LabelList list[MAX_ID];
	int size(){
		int sum = 0;
		for(int i=0; i<MAX_ID; i++){
			sum += list[i].size();
		}
		return sum;
	}
	vector<Label>* copy(){
		vector<Label>* t = new vector<Label>[MAX_ID];
		for(int i=0; i<MAX_ID; i++){
			t[i] = list[i].copy();
		}
		return t;
	}
}labels;

int query(int from, int to, vector<Label>* l){
    int distance = MAX_DIST;
    int from_len = l[from].size();
    int to_len = l[to].size();
    if(from_len == 0 || to_len == 0) return distance;
    int from_point = 0;
    int to_point = 0;
    while(from_point < from_len && to_point < to_len){
    	if(l[from][from_point].dest_ind == l[to][to_point].dest_ind){
    		distance = min(distance, l[from][from_point].dist + l[to][to_point].dist);
    		from_point ++;
    		to_point ++;
    	}
    	else if(l[from][from_point].dest_ind < l[to][to_point].dest_ind){
    		from_point ++;
    	}
    	else{
    		to_point ++;
    	}
    }
    return distance;
}

void pruned_bfs(int index, vector<Label>* l){
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
    	labels.list[u].append(tmp);
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
    srand(index);
    int s = rand()%10 + 1;
    printf("Index %d delay %d s.\n", index, s);
   	std::this_thread::sleep_for(std::chrono::seconds(s));
    printf("Indexes: %d, Node: %d, Labels: %d\n", index, dicts[index].id, labels.size());
    edit_num_thread(-1);
    finish_task();
}

void index_stage(){
	for(int i=0; i<num_node; i++){
		while(num_thread >= MAX_THREAD_NUM){
			std::this_thread::yield();
		}
		//printf("Dealing node %d\n", dicts[i]);
		std::thread t(pruned_bfs, i, labels.copy());
		t.detach();
	}
	while(num_finished_task < num_node){
		std::this_thread::yield();
	}
}


int main(){
	freopen("small.txt", "r", stdin);
	freopen("small_log.txt", "w", stdout);
	read_graph();
	index_stage();
	printf("Test result:\n");
	vector<Label>* ptr = labels.copy();
	printf("%d\n", query(1, 5, ptr));
	printf("%d\n", query(2, 5, ptr));
	printf("%d\n", query(5, 5, ptr));
	printf("%d\n", query(4, 2, ptr));
	printf("%d\n", query(3, 1, ptr));
	printf("%d\n", query(1, 3, ptr));
	printf("%d\n", query(1, 1, ptr));
	return 0;
}