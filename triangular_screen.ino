#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <vector>
#include <cmath>

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240

// Define los pines correspondientes a tu pantalla
#define TFT_BLK  15   // Backlight
#define TFT_DC   13   // Data/Command
#define TFT_RES  14   // Reset
#define TFT_CS   17   // Chip Select
#define TFT_SPI_SPEED  100000000  // 100 MHz

Adafruit_GC9A01A tft = Adafruit_GC9A01A(TFT_CS, TFT_DC, TFT_RES);

using Point = std::pair<int, int>;

std::vector<std::vector<Point>> generate_triangle_side_points(int x, int y, int side_length, int points_per_side, double angle_shift) {
    std::vector<std::vector<Point>> triangle_points(3);
    const double PI = 3.141592653;
    double angle_rad = angle_shift * PI / 180.0;

    double angle1 = angle_rad;
    double angle2 = PI / 3 + angle_rad;

    int x1 = x + side_length * cos(angle1);
    int y1 = y + side_length * sin(angle1);
    int x2 = x + side_length * cos(angle2);
    int y2 = y + side_length * sin(angle2);

    for (int i = 0; i <= points_per_side; i++) {
        double t = static_cast<double>(i) / points_per_side;
        int px = static_cast<int>(x + t * (x1 - x));
        int py = static_cast<int>(y + t * (y1 - y));
        triangle_points[0].emplace_back(px, py);
    }

    for (int i = 0; i <= points_per_side; i++) {
        double t = static_cast<double>(i) / points_per_side;
        int px = static_cast<int>(x1 + t * (x2 - x1));
        int py = static_cast<int>(y1 + t * (y2 - y1));
        triangle_points[1].emplace_back(px, py);
    }

    for (int i = 0; i <= points_per_side; i++) {
        double t = static_cast<double>(i) / points_per_side;
        int px = static_cast<int>(x + t * (x2 - x));
        int py = static_cast<int>(y + t * (y2 - y));
        triangle_points[2].emplace_back(px, py);
    }

    return triangle_points;
}

void draw_equilateral_triangle(int x, int y, int side_length, double angle_shift) {
    const double PI = 3.141592653;
    double angle_rad = angle_shift * PI / 180;

    double angle1 = angle_rad;
    double angle2 = PI / 3 + angle_rad;

    int x1 = x + side_length * cos(angle1);
    int y1 = y + side_length * sin(angle1);
    int x2 = x + side_length * cos(angle2);
    int y2 = y + side_length * sin(angle2);

    tft.drawLine(x, y, x1, y1, tft.color565(255, 255, 255));
    tft.drawLine(x, y, x2, y2, tft.color565(255, 255, 255));
    tft.drawLine(x1, y1, x2, y2, tft.color565(255, 255, 255));
}

void setup() {
    // Configuración inicial
    analogWriteFreq(5000);
    analogWriteRange(256);
    analogWriteResolution(8);
    analogWrite(TFT_BLK, 128);  // Configura la intensidad del backlight

    tft.begin(TFT_SPI_SPEED);
    tft.fillScreen(GC9A01A_BLACK);  // Limpia la pantalla al iniciar
}

void loop() {
    static int current_iteration = 0;

    tft.fillScreen(GC9A01A_BLACK);

    int XC = DISPLAY_WIDTH / 2;
    int YC = DISPLAY_HEIGHT / 2;
    int CIRCLE_DIAMETER = 100;

    const int n_points = 10;
    std::vector<std::vector<Point>> points = generate_triangle_side_points(XC, YC, CIRCLE_DIAMETER, n_points, 0);

    int j1 = current_iteration % n_points;
    int j2 = (current_iteration + 1) % n_points;
    int j3 = (current_iteration + 2) % n_points;
    int j4 = (current_iteration + 3) % n_points;
    int j5 = (current_iteration + 4) % n_points;
    int j6 = (current_iteration + 5) % n_points;
    int j7 = (current_iteration + 6) % n_points;

    int indices[] = {j1, j2, j3, j4, j5, j6, j7};

    // Dibuja líneas entre los puntos en diferentes lados con diferentes colores
    for (int i = 0; i < 5; ++i) {
        tft.drawLine(points[0][indices[i]].first, points[0][indices[i]].second,
                     points[1][indices[i]].first, points[1][indices[i]].second,
                     tft.color565(255, 255, 0));

        tft.drawLine(points[1][indices[i]].first, points[1][indices[i]].second,
                     points[2][n_points - indices[i]].first, points[2][n_points - indices[i]].second,
                     tft.color565(255, 0, 255));

        tft.drawLine(points[0][indices[i]].first, points[0][indices[i]].second,
                     points[2][n_points - indices[i]].first, points[2][n_points - indices[i]].second,
                     tft.color565(0, 255, 255));
    }

    current_iteration++;
    delay(20);
}
