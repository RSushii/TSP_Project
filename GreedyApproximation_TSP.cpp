/*
    Author: Alex Keller, Ryan Leacock, Jimmy Finnegan, Graham Baker
    Date: 11/24/25
    Purpose: Make a Greedy Approximation algorithm to better find optimal routes through many many cities
*/

#include <iostream>    
#include <fstream> 
#include <vector>     
#include <cmath> 
#include <limits> 
#include <iomanip>
#include <string>      

using namespace std;


//Point struct: Represents a city location in 2D space.
struct Point {
    double x, y;
};


//Computes distance between two cities.
double distEuclid(const Point& a, const Point& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}


//Reads city coordinates from file. Returns true if successful, false if file can't open or empty.
bool loadPoints(const string& filename, vector<Point>& points) {
    ifstream in(filename);
    if (!in.is_open()) return false;

    Point p;
    while (in >> p.x >> p.y) {   //keep reading x y pairs
        points.push_back(p);
    }

    return !points.empty();
}


//Precomputes all pairwise distances. This makes the loop faster.
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

/*
    Outline:
        1) Start at city 0
        2) Repeatedly go to the nearest unvisited city
        3) Return to city 0 to close the tour
*/
vector<int> greedyNearestNeighborTour(const vector<vector<double>>& d) {
    int n = (int)d.size();

    vector<bool> visited(n, false); 
    vector<int> tour; 
    tour.reserve(n + 1);

    int curr = 0;   //start at city 0
    visited[curr] = true;
    tour.push_back(curr);

    //We need to pick the next city (n-1) times
    for (int step = 1; step < n; step++) {
        double bestDist = numeric_limits<double>::infinity();
        int bestCity = -1;

        //Scan all cities to find closest unvisited one
        for (int j = 0; j < n; j++) {
            if (!visited[j] && d[curr][j] < bestDist) {
                bestDist = d[curr][j];
                bestCity = j;
            }
        }

        //Move to that nearest unvisited city
        curr = bestCity;
        visited[curr] = true;
        tour.push_back(curr);
    }

    //Return to the starting city
    tour.push_back(0);
    return tour;
}


//Computes total length of a closed tour.
double tourLength(const vector<int>& tour, const vector<vector<double>>& d) {
    double len = 0.0;
    for (size_t i = 0; i + 1 < tour.size(); i++) {
        len += d[tour[i]][tour[i + 1]];
    }
    return len;
}

void writeSolutionSVG(const vector<Point>& points, const vector<int>& tour, float gridSize, const string& outname)
{
    float scale = 800.0 / gridSize;
    ofstream svg (outname + ".svg");

    if (!svg.is_open())
    {
        cout << "Error: Couldn't write solution SVG" << endl;
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

    for (int i = 0; i < (int)tour.size() - 1; i++)
    {
        int a = tour [i];
        int b = tour [i + 1];

        Point p1 = points[a];
        Point p2 = points[b];

        svg << "<line x1='" << p1.x * scale
            << "' y1='" << p1.y * scale
            << "' x2='" << p2.x * scale
            << "' y2='" << p2.y * scale
            << "' stroke='lime' stroke-width='3' marker-end='url(#arrow)' />\n";
    }

    for (auto& p : points)
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./greedy <points_file.txt>\n";
        return 1;
    }

    string filename = argv[1];
    vector<Point> points;

    //Load city coordinates
    if (!loadPoints(filename, points)) {
        cerr << "Error: could not read points from " << filename << "\n";
        return 1;
    }

    int n = (int)points.size();
    if (n == 1) {
        cout << "Only 1 city. Tour length = 0\n";
        return 0;
    }

    //Build distance matrix once
    vector<vector<double>> d = buildDistanceMatrix(points);

    //Run greedy
    vector<int> tour = greedyNearestNeighborTour(d);

    double len = tourLength(tour, d);

    //Results
    cout << fixed << setprecision(6);
    cout << "Greedy (Nearest-Neighbor) Tour Length: " << len << "\n";
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

    writeSolutionSVG(points, tour, gridSize, base + "_greedySolution");

    return 0;
}
