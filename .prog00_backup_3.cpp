#include <vector>
#include "geometry.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <limits>

struct Material {
    Material(const Vec3f &c) : diffuse_colour(c) {}
    Material() : diffuse_colour() {}
    Vec3f diffuse_colour;
};

struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f &c, const float &r, const Material &m) : center(c), radius(r), material(m) {}

    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &dist) const {
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

            if (std::max(solution1, solution2) >= 0) {
                if (solution1 >= 0) {
                    //dist = solution1 * sqrt(a * a + b * b + c * c);
                    dist = solution1;
                } else {
                    //dist = solution2 * sqrt(a * a + b * b + c * c);
                    dist = solution2;
                }
                return true;
            } else {
                return false;
            }
        }
    }
};

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit_point, Vec3f &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < spheres.size(); ++i) {
        float dist;
        if (spheres[i].ray_intersect(orig, dir, dist) && dist < spheres_dist) {
            spheres_dist = dist;
            hit_point = orig + dir * dist;
            N = (hit_point - spheres[i].center).normalize();
            material = spheres[i].material;
        }
    }
    return spheres_dist < 1000;
}

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres) {
    Vec3f hit_point, N;
    Material material;

    if (!scene_intersect(orig, dir, spheres, hit_point, N, material)) {
        return Vec3f(0.2, 0.7, 0.8);     // background colour            
    }
    return material.diffuse_colour;    // sphere colour
}

void render(const std::vector<Sphere> &spheres) {
    const int img_width = 1024;     // width
    const int img_height = 768;     // height
    const int fov = M_PI / 2.;       // vision angle
    std::vector< std::vector<Vec3f> > img(img_width, std::vector<Vec3f>(img_height));

    for (size_t j = 0; j < img_height; ++j) {
        for (size_t i = 0; i < img_width; ++i) {
            float x = -img_width / 2. + i;
            float y = -img_height / 2. + j;
            float z = img_height / 2. / tan(fov / 2.);
            Vec3f dir = Vec3f(x, -y, z).normalize();
            img[i][j] = cast_ray(Vec3f(0, 0, 0), dir, spheres);
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
    Material m1(Vec3f(0.4, 0.4, 0.3));
    Material m2(Vec3f(0.3, 0.1, 0.1));
    
    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, 16), 2, m1));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, 12), 2, m2));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, 18), 3, m2));
    spheres.push_back(Sphere(Vec3f(7, 5, 18), 4, m1));

    render(spheres);
}
