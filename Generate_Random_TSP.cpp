/*
    Authors : Alex Keller, Ryan Leacock, Jimmy Finnegan, Graham Baker
    Date    : 11/21/2025
    Purpose : Generate random 2D points for TSP and draw SVG
*/

#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <vector>

using namespace std;

struct Point
{
    float x, y;
};

/*
    Generates 'n' random 2D points inside of a square region/grid,
    where (0, 0) is the minimum coordinate and (gridSize, gridSize)
    is the maximum coordinate

    n: number of cities/points to generate
    seed: deterministic RNG seed (same seed >> same results)
    gridSize: size of coordinate space (Ex: 1.0, 1.0 or 50.0, 50.0)
    filename: output text file where coordinates are saved/stored

    Output format for each line:
    x y
    EXAMPLE: 
    0.591238 0.897115
*/

void generateRandomTSP(int n, int seed, float gridSize, string& filename)
{
    // Mersene Twister RNG for reproducible pseudorandom numbers
    mt19937 rng(seed);
    uniform_real_distribution<float> dist(0.0, gridSize); // Uniform distribution from 0.0 to gridSize (square)

    // Store all points for SVG drawing
    vector<Point> points;


    // -- BEGIN TEXT FILE SECTION --
    // Open output file and check
    string textFilename = filename + ".txt"; // Create text filename
    ofstream outputFile(textFilename);
    if (!outputFile.is_open())
    {
        cout << "Unable to open output file" << endl;
        return;
    }

    // generate x y coordinate for n cities
    for (int i = 0; i < n; i++)
    {
        float x = dist(rng);
        float y = dist(rng);
        points.push_back({x, y}); // Store points for later SVG section
        outputFile << x << " " << y << endl;
    }

    outputFile.close(); // Close file
    cout << "Generated" << n << " random points into " << filename << endl;
    // -- END TEXT FILE SECTION --

    // -- BEGIN SVG OUTPUT SECTION --
    string svgFilename = filename + ".svg"; //Create svg filename
    ofstream svgFile(svgFilename);
    if (!svgFile.is_open())
    {
        cout << "Unable to open SVG file" << endl;
        return;
    }

    // Scale points to fixed 800x800 SVG canvas
    float scale = 800.0 / gridSize;
    svgFile << "<svg width='800' height='800' xmlns='http://www.w3.org/2000/svg' style='background:black'>\n";

    // Draw lines connecting every point to every other point (complete graph)
    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            Point p1 = points[i];
            Point p2 = points[j];
            svgFile << "<line x1='" << p1.x * scale << "' y1='" << p1.y * scale
                << "' x2='" << p2.x * scale << "' y2='" << p2.y * scale
                << "' stroke='white' stroke-width='1'/>\n"; // Thin white lines
        }
    }

    // Draw red circles for each city/point
    for (auto& p : points)
    {
        svgFile << "<circle cx='" << p.x * scale << "' cy='" << p.y * scale << "' r='5' fill='red'/>\n"; // Red cirlce radius of 5
    }

    svgFile << "</svg>";
    svgFile.close();
    cout << "SVG Visual saved to " << svgFilename << endl;
    // -- END SVG OUTPUT SECTION --
}

int main()
{
    int n;           // Number of cities
    int seed;        // RNG seed for reproducibility
    float gridSize;  // Coordinate limits
    string filename; // Output file name

    // Prompts user for input and gets input
    cout << "Number of cities (n): ";
    cin >> n;

    cout << "Grid size (float): ";
    cin >> gridSize;

    cout << "Seed (integer): ";
    cin >> seed;

    cout << "Output filename: ";
    cin >> filename;

    // Generate TSP coordinate file
    generateRandomTSP(n, seed, gridSize, filename);
    return 0;
}