/*
    Author: Alex Keller, Ryan Leacock, Jimmy Finnegan, Graham Baker
    Date: 11/24/25
    Purpose: Brute Force algorithm to find the best TSP route
*/

#include <iostream>   
#include <fstream>    
#include <vector>     
#include <cmath>      
#include <algorithm>  
#include <limits>     
#include <string>   
#include <iomanip>

using namespace std;


//Point struct: Stores a city location (x,y).
struct Point {
    double x, y;
};

// SVG solution section
void writeSolutionSVG(const vector<Point>& points, const vector<int>& bestTour, float gridSize, const string& outname)
{
    float scale = 800.0 / gridSize;
    ofstream svg(outname + ".svg");

    if (!svg.is_open())
    {
        cout << "Error: Couldn't write solution to SVG" << endl;
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

    // Close svg
    svg << "</svg>";
    svg.close();

    // Output that it successfully wrote solution to svg
    cout << "Solution SVG written to: " << outname << ".svg" << endl;
}


//distEuclid: finds distance
double distEuclid(const Point& a, const Point& b) {
    double dx = a.x - b.x;     //difference in x
    double dy = a.y - b.y;     //difference in y
    return sqrt(dx*dx + dy*dy);
}


//loadPoints: Reads points from a text file into the 'points' vector. Returns true if successful, false otherwise.
bool loadPoints(const string& filename, vector<Point>& points) {
    ifstream in(filename);          
    if (!in.is_open()) return false;

    Point p;
    while (in >> p.x >> p.y) {      //read until EOF
        points.push_back(p);        //store each city
    }

    //return true only if we got at least one point
    return !points.empty();
}

int main(int argc, char* argv[]) {
    //argc counts how many command line pieces exist.
    if (argc < 2) {
        cout << "Usage: ./brute <points_file.txt>\n";
        return 1;
    }

    string filename = argv[1];
    vector<Point> points;

    //Load cities from Alex's random generator
    if (!loadPoints(filename, points)) {
        cout << "Error: couldn't read points from " << filename << "\n";
        return 1;
    }

    int n = (int)points.size();   //number of cities

    //Edge case: if there's only 1 city, tour is length 0
    if (n == 1) {
        cout << "Only 1 city. Tour length = 0\n";
        return 0;
    }

    /*
        d[i][j] = distance from city i to city j.
        Using this to avoid doing sqrt over and over.
        Time to build: O(n^2).
    */
    vector<vector<double>> d(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            d[i][j] = distEuclid(points[i], points[j]);
        }
    }

    /*
        The actual Brute Force part.
        There are (n-1)! permutations to test.
    */
    vector<int> perm;
    perm.reserve(n - 1);

    //Fill perm with cities 1..n-1
    for (int i = 1; i < n; i++) {
        perm.push_back(i);
    }

    //bestLen starts as infinity so any real tour improves it
    double bestLen = numeric_limits<double>::infinity();
    vector<int> bestTour;        //store best path found

    /*
        - sort() ensures we start with smallest lexicographic order
        - next_permutation() visits every permutation exactly once
    */
    sort(perm.begin(), perm.end());

    do {
        double len = 0.0;
        int prev = 0; 

    
        //Add distance from prev -> current city. Prev updates each step.
        for (int curr : perm) {
            len += d[prev][curr]; 
            prev = curr;     
            if (len >= bestLen) break;
        }

        //Close cycle, last city back to city 0
        len += d[prev][0];

        //If this run was the best run, store
        if (len < bestLen) {
            bestLen = len;

            //build full tour: 0 + perm + 0
            bestTour.clear();
            bestTour.push_back(0);
            bestTour.insert(bestTour.end(),
                            perm.begin(), perm.end());
            bestTour.push_back(0);
        }

    } while (next_permutation(perm.begin(), perm.end()));

    
    //Final output
    cout << fixed << setprecision(6);  //formatting
    cout << "Brute-force optimal tour length: " << bestLen << "\n";
    cout << "Tour order: ";

    for (size_t i = 0; i < bestTour.size(); i++) {
        cout << bestTour[i];
        if (i + 1 < bestTour.size()) cout << " -> ";
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

    // Call function to make solution SVG
    writeSolutionSVG(points, bestTour, gridSize, base + "_solution");

    return 0;
}
