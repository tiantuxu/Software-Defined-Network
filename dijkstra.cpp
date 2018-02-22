
// Created by teddyxu on 1/23/18.
//

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include "message.h"
#include "graph.h"

 //Program to find Dijkstra's shortest path using STL set
#include<bits/stdc++.h>
using namespace std;
//int SWITCH_NUM;

// This class represents a directed graph using
// adjacency list representation

// Allocates memory for adjacency list

void add_edge(graph& g, int start, int finish, int length) {
	if ((int)g.size() <= (std::max)(start, finish))
		g.resize((std::max)(start, finish) + 1);
	g[start].push_back({ finish, length });
	g[finish].push_back({ start, length });
}


result dijkstra(const graph &graph, int source, int target) {
	//cout << "here" << endl;
	//std::vector<int> widest_len( graph.size(), INT_MAX );

	std::vector<int> widest_len(graph.size(), INT_MIN); //*****Change to Int_MIN;
	widest_len[source] = INT_MAX; //**********************************Change to Int_MIN;
								  //widest_len[source] = INT_MAX;
	set< pair<int, int>> active_vertices;//, classCompPair<int>
	//priority_queue<pair<int,int>>active_vertices;
	//active_vertices.insert({INT_MAX,source});
	//active_vertices.insert({0,source});
	active_vertices.insert({ INT_MAX ,source });//**************************Change to {INT_MAX,source};
	vector<path> path1(SWITCH_NUM + 1);
	path1[source].push_back(source);

	while (!active_vertices.empty()) {
		int where = active_vertices.rbegin()->second;
		if (where == target)
		{
			result result;
			result.distance = widest_len[where];
			result.p = path1[where];
			return result;
		}

		active_vertices.erase({widest_len[where], where });
		for (auto ed : graph[where])
			if (widest_len[ed.to] < min(widest_len[where], ed.length))
			{
				//if (widest_len[ed.to] > widest_len[where]<ed.length?widest_len[where]:ed.length) {
				active_vertices.erase({ widest_len[ed.to],ed.to});
				widest_len[ed.to] = min(widest_len[where], ed.length);//change
				path1[ed.to] = path1[where];
				path1[ed.to].push_back(ed.to);
				active_vertices.insert({ widest_len[ed.to],ed.to });
			}
	}
	return { INT_MAX };
}