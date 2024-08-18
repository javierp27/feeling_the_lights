#include <SDL2/SDL.h>

#include <vector>
#include <cmath>

const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 400;
const int XC = 100;
const int YC = 100;
const int CIRCLE_DIAMETER = 150;
const int DELAY_MS = 20;


#include <vector>
#include <utility>
#include <cmath>

// Define a type alias for better readability
using Point = std::pair<int, int>;

std::vector< std::vector<Point> > generate_triangle_side_points(int x, int y, int side_length, int points_per_side, double angle_shift) {
    std::vector< std::vector<Point> > triangle_points(3);
    const double PI = 3.141592653;
    double angle_rad = angle_shift * PI / 180.0;

    // Calculate the angles for the three vertices
    double angle1 = angle_rad;
    double angle2 = PI / 3 + angle_rad;


    // Calculate the positions of the three vertices
    int x1 = x + side_length * cos(angle1);
    int y1 = y + side_length * sin(angle1);
    int x2 = x + side_length * cos(angle2);
    int y2 = y + side_length * sin(angle2);

    // Generate points for the first side (x, y) -> (x1, y1)
    for (int i = 0; i <= points_per_side; i++) {
        double t = static_cast<double>(i) / points_per_side;
        int px = static_cast<int>(x + t * (x1 - x));
        int py = static_cast<int>(y + t * (y1 - y));
        triangle_points[0].emplace_back(px, py);
    }

    // Generate points for the second side (x1, y1) -> (x2, y2)
    for (int i = 0; i <= points_per_side; i++) {
        double t = static_cast<double>(i) / points_per_side;
        int px = static_cast<int>(x1 + t * (x2 - x1));
        int py = static_cast<int>(y1 + t * (y2 - y1));
        triangle_points[1].emplace_back(px, py);
    }

    // Generate points for the third side (x, y) -> (x2, y2)
    for (int i = 0; i <= points_per_side; i++) {
        double t = static_cast<double>(i) / points_per_side;
        int px = static_cast<int>(x + t * (x2 - x));
        int py = static_cast<int>(y + t * (y2 - y));
        triangle_points[2].emplace_back(px, py);
    }

    return triangle_points;
}



double degToRad(double degrees) {
    const double PI = 3.141592653;
    return degrees * PI / 180.0;
}



void draw_equilateral_triangle(SDL_Renderer* renderer, int x, int y, int side_length, double angle_shift) {

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    const double PI = 3.141592653;
    double angle_rad = angle_shift  * PI / 180;

    double angle1 =  angle_rad;
    double angle2 = PI / 3 + angle_rad;


    int x1 = x + side_length * cos(angle1);
    int y1 = y + side_length * sin(angle1);
    int x2 = x + side_length * cos(angle2);
    int y2 = y + side_length * sin(angle2);

    //printf("angle1: %f, angle2: %f\n", sin(angle1), sin(angle2));

    SDL_RenderDrawLine(renderer, x, y, x1, y1);
    SDL_RenderDrawLine(renderer, x, y, x2, y2);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

}


int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Display", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    SDL_Event event;
    int current_iteration = 0;


    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        double x1, y1, x2, y2;

        x1 = XC; // + current_iteration % 3;
        x2 = XC + 20;   //- (current_iteration*2)%43;
        y1 = YC;
        y2 = YC + 60;

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

        //draw_equilateral_triangle(renderer, XC, YC, CIRCLE_DIAMETER, 0);

        const int n_points = 10;
        std::vector< std::vector<Point> > points = generate_triangle_side_points(XC, YC, CIRCLE_DIAMETER, n_points, 0);

        // Access points from each side

        int j1 = current_iteration % n_points;
        int j2 = (current_iteration + 1)  % n_points;
        int j3 = (current_iteration + 2) % n_points;
        int j4 = (current_iteration + 3) % n_points;
        int j5 = (current_iteration + 4) % n_points;
        int j6 = (current_iteration + 5) % n_points;
        int j7 = (current_iteration + 6) % n_points;

        int indices[] = {j1, j2, j3, j4, j5, j6, j7};

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

        for (int i = 0; i < 5; ++i) {
            SDL_RenderDrawLine(renderer, points[0][indices[i]].first, points[0][indices[i]].second,
                                            points[1][indices[i]].first, points[1][indices[i]].second);
        }

        // Change color and draw lines between points[1] and points[2]
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);

        for (int i = 0; i < 5; ++i) {
            SDL_RenderDrawLine(renderer, points[1][indices[i]].first, points[1][indices[i]].second,
                                            points[2][n_points - indices[i]].first, points[2][n_points - indices[i]].second);
        }

        // Change color and draw lines between points[0] and points[2]
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

        for (int i = 0; i < 5; ++i) {
            SDL_RenderDrawLine(renderer, points[0][indices[i]].first, points[0][indices[i]].second,
                                            points[2][n_points - indices[i]].first, points[2][n_points - indices[i]].second);
        }


        // Or draw a line between points on different sides
        //SDL_RenderDrawLine(renderer, points[0][0].first, points[0][0].second, points[1][5].first, points[1][5].second);

        SDL_RenderPresent(renderer);

        // Increment the iteration counter
        current_iteration++;

        // Add a delay for animation speed control
        SDL_Delay(DELAY_MS); // 100 milliseconds delay
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
