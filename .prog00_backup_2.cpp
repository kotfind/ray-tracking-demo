#include <vector>
#include "geometry.h"
#include <fstream>
#include <iostream>
#include <cmath>

struct Sphere {
    Vec3f center;
    float radius;

    Sphere(const Vec3f &c, const float &r) : center(c), radius(r) {}

    bool ray_intersect(const Vec3f &orig, const Vec3f &dir) const {
        float a = dir.x - orig.x;
        float b = dir.y - orig.y;
        float c = dir.z - orig.z;
        float d = orig.x - center.x;
        float e = orig.y - center.y;
        float f = orig.z - center.z;
        float A = a * a + b * b + c * c;
        float B = 2 * (a * d + b * e + c * f);
        float C = d * d + e * e + f * f - radius * radius;
        if (B * B < 4 * A * C) {
            return false;
        } else {
            float D = B * B - 4 * A * C;
            float solution1 = (-B - sqrt(D)) / (2 * A);
            float solution2 = (-B + sqrt(D)) / (2 * A);

            return true;
            if (std::max(solution1, solution2) >= 0) {
                return true;
            } else {
                return false;
            }
        }
    }
};

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const Sphere &sphere) {
    if (!sphere.ray_intersect(orig, dir)) {
        return Vec3f(0.2, 0.7, 0.8);     // background colour            
    }
    return Vec3f(0.4, 0.4, 0.3);    // sphere colour
}

void render(const Sphere &sphere) {
    const int img_width = 1024;     // width
    const int img_height = 768;     // height
    const int fov = M_PI / 2.;       // vision angle
    std::vector< std::vector<Vec3f> > img(img_width, std::vector<Vec3f>(img_height));

    for (size_t j = 0; j < img_height; ++j) {
        for (size_t i = 0; i < img_width; ++i) {
            // float x =  (2*(i + 0.5)/(float)img_width  - 1)*tan(fov/2.)*img_width/(float)img_height;
            // float y = -(2*(j + 0.5)/(float)img_height - 1)*tan(fov/2.);
            // Vec3f dir = Vec3f(x, y, -1).normalize();
            // img[i][j] = cast_ray(Vec3f(0, 0, 0), dir, sphere);
            
            float x = -img_width / 2. + i;
            float y = -img_height / 2. + j;
            float z = img_height / 2. / tan(fov / 2.);
            Vec3f dir = Vec3f(x, y, -z).normalize();
            img[i][j] = cast_ray(Vec3f(0, 0, 0), dir, sphere);
        }
    }

    // save the img to file
    std::ofstream fout;
    fout.open("./out.ppm");
    fout << "P6\n";
    fout << img_width << " " << img_height << "\n";
    fout << 255 << "\n";

    for (size_t j = 0; j < img_height; ++j) {
        for (size_t i = 0; i < img_width; ++i) {
            for (int c = 0; c < 3; ++c) {
                fout << (char)(255 * std::max(0.f, std::min(1.f, img[i][j][c])));
            }
        }
    }
    fout.close();
}

int main() {
    Sphere sphere(Vec3f(-3, 0, -16), 2);
}
