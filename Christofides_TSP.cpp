/*
    Author: Alex Keller, Ryan Leacock, Jimmy Finnegan, Graham Baker
    Date: 11/25/25
    Purpose: Use Cristofides Algorithm to solve TSP hopefully at O(n^3)
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <string>

using namespace std;

//Represents a city in 2D space
struct Point {
    double x, y;
};

//Distance between two points
double distEuclid(const Point& a, const Point& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}


//Reads (x y) pairs line by line from a text file. Returns true if at least one point was read.
bool loadPoints(const string& filename, vector<Point>& points) {
    ifstream in(filename);
    if (!in.is_open()) return false;

    Point p;
    while (in >> p.x >> p.y) {
        points.push_back(p);
    }
    return !points.empty();
}


//Precompute d[i][j] = distance between city i and city j. Important for making later steps simpler
vector<vector<double>> buildDistanceMatrix(const vector<Point>& pts) {
    int n = (int)pts.size();
    vector<vector<double>> d(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            d[i][j] = distEuclid(pts[i], pts[j]);
        }
    }
    return d;
}


//Sum of distances along a closed tour.
double tourLength(const vector<int>& tour,
                  const vector<vector<double>>& d) {
    double len = 0.0;
    for (size_t i = 0; i + 1 < tour.size(); i++) {
        len += d[tour[i]][tour[i+1]];
    }
    return len;
}

//Prim's algorithm for Minimum Spanning Tree on a complete graph.
vector<int> primMST(const vector<vector<double>>& d) {
    int n = (int)d.size();

    vector<double> key(n, numeric_limits<double>::infinity());
    vector<int> parent(n, -1);
    vector<bool> inMST(n, false);

    //Start MST from city 0
    key[0] = 0.0;

    for (int iter = 0; iter < n; iter++) {
        double best = numeric_limits<double>::infinity();
        int u = -1;
        for (int v = 0; v < n; v++) {
            if (!inMST[v] && key[v] < best) {
                best = key[v];
                u = v;
            }
        }

        inMST[u] = true;

        //Update keys for neighbors
        for (int v = 0; v < n; v++) {
            if (!inMST[v] && d[u][v] < key[v]) {
                key[v] = d[u][v];
                parent[v] = u;
            }
        }
    }

    return parent;
}


//Convert parent[] representation of MST into adjacency list. For each v>0: edge (v, parent[v]).
vector<vector<int>> buildAdjFromParent(const vector<int>& parent) {
    int n = (int)parent.size();
    vector<vector<int>> adj(n);
    for (int v = 1; v < n; v++) {
        int p = parent[v];
        adj[v].push_back(p);
        adj[p].push_back(v);
    }
    return adj;
}


//Returns all vertices that have an odd degree in the given adjacency list.
vector<int> findOddDegreeVertices(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<int> odd;
    for (int i = 0; i < n; i++) {
        if (adj[i].size() % 2 == 1) {
            odd.push_back(i);
        }
    }
    return odd;
}

//Combine adjacent unmatched vertices
void addGreedyPerfectMatching(const vector<int>& odd,
                              const vector<vector<double>>& d,
                              vector<vector<int>>& adj) {
    int k = (int)odd.size();
    if (k == 0) return;

    vector<bool> used(k, false);

    for (int i = 0; i < k; i++) {
        if (used[i]) continue;

        double best = numeric_limits<double>::infinity();
        int bestj = -1;

        //find closest unmatched partner for odd[i]
        for (int j = i + 1; j < k; j++) {
            if (!used[j]) {
                double dist = d[odd[i]][odd[j]];
                if (dist < best) {
                    best = dist;
                    bestj = j;
                }
            }
        }

        //mark both as matched
        used[i] = true;
        used[bestj] = true;

        int u = odd[i];
        int v = odd[bestj];

        //add edge (u,v) to the multigraph adjacency list
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
}

//Find a good cycle using Hierholzer's algorithm
vector<int> eulerianTourHierholzer(int start,
                                   vector<vector<int>> adj) {
    vector<int> circuit;
    vector<int> stack; 

    stack.push_back(start);

    while (!stack.empty()) {
        int u = stack.back();

        if (!adj[u].empty()) {
            //Take one edge u -> v
            int v = adj[u].back();
            adj[u].pop_back();

            //Remove the opposite edge v -> u
            auto& nbrs = adj[v];
            auto it = find(nbrs.begin(), nbrs.end(), u);
            if (it != nbrs.end()) {
                nbrs.erase(it);
            }

            //Follow that edge
            stack.push_back(v);
        } else {
            //No more edges out of u: add u to circuit and backtrack
            circuit.push_back(u);
            stack.pop_back();
        }
    }

    //circuit currently has vertices in reverse traversal order
    reverse(circuit.begin(), circuit.end());
    return circuit;
}

//The actual Christofides part using greedy matching
vector<int> christofidesTour(const vector<vector<double>>& d) {
    int n = (int)d.size();

    //Build MST
    vector<int> parent = primMST(d);
    vector<vector<int>> adj = buildAdjFromParent(parent);

    //Find odd-degree vertices in MST
    vector<int> odd = findOddDegreeVertices(adj);

    //Greedy min-weight perfect matching on odd vertices
    addGreedyPerfectMatching(odd, d, adj);

    //Eulerian cycle in the multigraph (make all the degrees even)
    vector<int> euler = eulerianTourHierholzer(0, adj);

    //Shortcut repeated vertices to get tour
    vector<bool> visited(n, false);
    vector<int> tour;

    tour.reserve(n + 1);

    for (int v : euler) {
        if (!visited[v]) {
            tour.push_back(v);
            visited[v] = true;
        }
    }

    // Rotate so that tour starts at 0 (helps with consistancy)
    auto it0 = find(tour.begin(), tour.end(), 0);
    if (it0 != tour.begin() && it0 != tour.end()) {
        rotate(tour.begin(), it0, tour.end());
    }

    //Close the tour
    tour.push_back(0);

    return tour;
}

void writeSolutionSVG(const vector<Point>& points, const vector<int>& bestTour, float gridSize, const string& outname)
{
    float scale = 800.0 / gridSize;
    ofstream svg(outname + ".svg");

    if (!svg.is_open()) {
        cout << "Error: Could not write solution SVG" << endl;
        return;
    }

    // Start with black background
    svg << "<svg width='800' height='800' xmlns='http://www.w3.org/2000/svg' style='background:black'>\n";

    // This defines a reusable marker that gets placed at the end of each path/destination line
    svg << "<defs>\n";
    svg << "  <marker id='arrow' markerWidth='10' markerHeight='10' refX='5' refY='3' orient='auto'>\n";
    svg << "      <path d='M0,0 L0,6 L6,3 z' fill='lime'/>\n";
    svg << "  </marker>\n";
    svg << "</defs>\n";

    // Draws directed TSP path edges (Lime green arrows)
    // bestTour contains a full cycle [0, 1, ..., n - 1, n]
    for (int i = 0; i < (int)bestTour.size() - 1; i++)
    {
        int a = bestTour[i];     // Current city index
        int b = bestTour[i + 1]; // Next city index

        Point p1 = points[a];    // Coordinates of city a
        Point p2 = points[b];    // Coordinates of city b

        // Draw lime green directional arrows
        svg << "<line x1='" << p1.x * scale
            << "' y1='" << p1.y * scale
            << "' x2='" << p2.x * scale
            << "' y2='" << p2.y * scale
            << "' stroke='lime' stroke-width='3' marker-end='url(#arrow)' />\n";
    }

    // Draw red dots (cities)
    for (auto& p: points)
    {
        svg << "<circle cx='" << p.x * scale
            << "' cy='" << p.y * scale
            << "' r='5' fill='red' />\n";
    }

    svg << "</svg>";
    svg.close();

    // Output that it successfully wrote solution to svg
    cout << "Solution SVG written to: " << outname << ".svg" << endl;
}

//Run all this nonsense
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./christofides <points_file.txt>\n";
        return 1;
    }

    string filename = argv[1];
    vector<Point> points;

    if (!loadPoints(filename, points)) {
        cerr << "Error: could not read points from " << filename << "\n";
        return 1;
    }

    int n = (int)points.size();
    if (n == 1) {
        cout << "Only 1 city. Tour length = 0\n";
        return 0;
    }

    //Precompute distances
    vector<vector<double>> d = buildDistanceMatrix(points);

    //Run the Christofides-style algorithm
    vector<int> tour = christofidesTour(d);
    double len = tourLength(tour, d);

    //Output
    cout << fixed << setprecision(6);
    cout << "Christofides-style Tour Length: " << len << "\n";
    cout << "Tour order: ";
    for (size_t i = 0; i < tour.size(); i++) {
        cout << tour[i];
        if (i + 1 < tour.size()) cout << " -> ";
    }
    cout << "\n";

    // Drops the ".txt" if it has it and replaces it with ".svg"
    string base = filename;
    if (base.size() >= 4 && base.substr(base.size() - 4) == ".txt")
    {
        base = base.substr(0, base.size() - 4);
    }

    // Get original grid size (used to make svg the same size and points accurate)
    float gridSize;
    cout << "Enter original gridSize used to generate " << filename << ": ";
    cin >> gridSize;

    writeSolutionSVG(points, tour, gridSize, base + "_christofidesSolution");

    return 0;
}
