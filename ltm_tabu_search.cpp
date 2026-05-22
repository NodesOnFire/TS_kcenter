#include "tsplib.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <limits>
#include <queue>
#include <string>

TSPLIBInstance graph;
std::mt19937 gen(1);

std::vector<int> random_solution(int n, int k) {
    std::vector<int> src(n);
    std::iota(src.begin(), src.end(), 0);
    std::vector<int> out;
    out.reserve(k);
    std::sample(src.begin(), src.end(), std::back_inserter(out), k, gen);
    return out;
}

std::pair<int, std::vector<std::vector<int>>> fitness(std::vector<int> x,
                           int n, std::vector<std::vector<int>> x_distances, 
                           int prev_vertex=-1, int new_vertex=-1){
    int max_dist = 0;
    if(prev_vertex == -1){
        for(int i=0;i<n;i++){
            int min_dist = std::numeric_limits<int>::max();
            for(int j=0;j<size(x);j++){
                int d = graph.distance(i, x[j]);
                if(d < min_dist){
                    min_dist = d;
                    x_distances[i][0] = x[j];
                }
            }
            x_distances[i][1] = min_dist;
            if(min_dist > max_dist){
                max_dist = min_dist;
            }
        }
    }else{
        for(int i=0;i<n;i++){
            if(x_distances[i][0] == prev_vertex){
                int min_dist = std::numeric_limits<int>::max();
                for(int j=0;j<size(x);j++){
                    int d = graph.distance(i, x[j]);
                    if(d < min_dist){
                        min_dist = d;
                        x_distances[i][0] = x[j];
                    }
                }
                x_distances[i][1] = min_dist;
                if(min_dist > max_dist){
                    max_dist = min_dist;
                }
            }else{
                int d = graph.distance(i,new_vertex);
                if(d < x_distances[i][1]){
                    x_distances[i][0] = new_vertex;
                    x_distances[i][1] = d;
                    if(d > max_dist){
                        max_dist = d;
                    }
                }else{
                    d = x_distances[i][1];
                    if(d > max_dist){
                        max_dist = d;
                    }
                }
            }
        }
    }
    return {max_dist,x_distances};
}

int main(int argc, char **argv){
    // load input graph
    std::string path = argv[1];
    graph = parse_tsplib(path);
    std::cout << "Name: " << graph.name << "\n";
    std::cout << "Order: " << graph.dimension << "\n";
    std::cout << "Type: " << graph.edge_weight_type << "\n";
    int n = graph.dimension;
    double penalty_factor = std::stod(argv[4]);
    // stop condition
    double max_time = std::stod(argv[2]);
    auto start = std::chrono::high_resolution_clock::now();
    double elapsed_time = 0;
    // initial random solution
    int k = std::stoi(argv[3]);
    std::vector<int> x = random_solution(n, k);
    std::cout << "initial solution: ";
    for(int i : x) std::cout << i << " "; std::cout << "\n";
    // initial solution size
    int x_size;
    std::vector<std::vector<int>> x_distances(n, std::vector<int>(2,0));
    std::pair<int,std::vector<std::vector<int>>> fitness_info = fitness(x,n,x_distances);
    x_size = fitness_info.first;
    x_distances = fitness_info.second;
    std::vector<int> x_best = x;
    int x_best_size = x_size;
    std::cout << "initial solution size: " << x_size << "\n";
    // Long-term memory
    std::vector<int> frequency(n,0);
    int iteration = 0;
    while(elapsed_time < max_time){
        // neighborhood initialization
        iteration += 1;
        std::vector<int> x_star;
        int x_star_size = std::numeric_limits<int>::max();
        std::vector<std::vector<int>> x_star_distances;
        bool break_cycle = false;
        int added_vertex = -1;
        std::vector<int> shuffled_integers(k);
        for(int i=0;i<k;i++) shuffled_integers[i] = i;
        std::shuffle(shuffled_integers.begin(), shuffled_integers.end(), gen);
        for(int j : shuffled_integers){
            for(int i=0;i<n;i++){
                if(graph.distance(x[j],i) < x_best_size){
                    if(i != x[j]){
                        std::vector<int> x_prime = x;
                        x_prime[j] = i;
                        int x_prime_size;
                        std::vector<std::vector<int>> x_prime_distances;
                        std::pair<int,std::vector<std::vector<int>>> result = fitness(x_prime,n,x_distances,x[j],i);
                        x_prime_size = result.first;
                        x_prime_distances = result.second;
                        if(x_prime_size < x_size){
                            x_star = x_prime;
                            x_star_size = x_prime_size;
                            x_star_distances = x_prime_distances;
                            added_vertex = i;
                            break_cycle = true;
                            break;
                        }else{
                            double modified_x_prime_size = double(x_prime_size)*(1+penalty_factor*(double(frequency[i])/double(iteration)));
                            if(modified_x_prime_size < x_star_size){
                                x_star = x_prime;
                                x_star_size = x_prime_size;
                                x_star_distances = x_prime_distances;
                                added_vertex = i;
                            }
                        }
                    }
                }
            }
            if(break_cycle){
                break;
            }
        }
        if(x_star_size < x_best_size){
            x_best = x_star;
            x_best_size = x_star_size;
        }
        x = x_star;
        x_size = x_star_size;
        x_distances = x_star_distances;
        // Update memory
        frequency[added_vertex] += 1;
        std::cout << "--------- " << "\n";
        for(int i=0;i<k;i++) std::cout << x[i] << " "; std::cout << "\n";
        std::cout << "x_size = " << x_size << "\n";
        std::cout << "x_best_size = " << x_best_size << "\n";
        // elapsed time
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
        elapsed_time = duration.count();
    }
    std::cout << "elapsed time: " << elapsed_time << " milliseconds" << "\n";
}