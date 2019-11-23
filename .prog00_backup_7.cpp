#include <vector>
#include "geometry.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <limits>

struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {} 
    Vec3f position;
    float intensity;
};

struct Material {
    Material(const Vec3f &c, const float &spec, const Vec3f &a) : diffuse_colour(c), specular_exponent(spec), albedo(a) {}
    Material() : diffuse_colour(), specular_exponent(), albedo(1, 0, 0) {}
    Vec3f diffuse_colour;
    float specular_exponent;
    Vec3f albedo;
};

struct Sphere {
    Sphere(const Vec3f &c, const float &r, const Material &m) : center(c), radius(r), material(m) {}

    Vec3f center;
    float radius;
    Material material;

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

Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    // reflect I by N
    return I - N * 2.f * (I * N);
}

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

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, int depth) {
    Vec3f hit_point, N;
    Material material;

    if (depth > 6 || !scene_intersect(orig, dir, spheres, hit_point, N, material)) {
        return Vec3f(0.2, 0.7, 0.8);     // background colour            
    }

    Vec3f reflect_dir = reflect(dir, N).normalize();
    Vec3f reflect_orig = reflect_dir * N < 0 ? hit_point - N * 1e-3 : hit_point + N * 1e-3;
    Vec3f reflect_colour = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);

    float diffuse_light_intensity = 0;
    float specular_light_intensity = 0;
    for (size_t i = 0; i < lights.size(); ++i) {
        Vec3f light_dir = (lights[i].position - hit_point).normalize();
        float light_dist = (lights[i].position - hit_point).norm();

        Vec3f shadow_orig = light_dir * N < 0 ? hit_point - N * 1e-3 : hit_point + N * 1e-3;
        Vec3f shadow_hit_point, shadow_N;
        Material tmpmaterial;
        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_hit_point, shadow_N, tmpmaterial) && (shadow_hit_point - shadow_orig).norm() < light_dist) {
            continue;
        }

        diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * N);
        specular_light_intensity += lights[i].intensity * powf(std::max(0.f, reflect(light_dir, N) * dir), material.specular_exponent);
    }

    return material.diffuse_colour * diffuse_light_intensity * material.albedo[0] +
           Vec3f(1., 1., 1.) * specular_light_intensity * material.albedo[1] +
           reflect_colour * material.albedo[2];
}

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
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
            img[i][j] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights, 0);
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
    Material ivory(Vec3f(0.4, 0.4, 0.3), 50., Vec3f(0.6, 0.3, 0.1));
    Material red_rubber(Vec3f(0.3, 0.1, 0.1), 10., Vec3f(0.9, 0.1, 0.0));
    Material mirror(Vec3f(1.0, 1.0, 1.0), 1425., Vec3f(0.0, 10.0, 0.8));
    
    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, 16), 2, ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, 12), 2, mirror));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, 18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f(7, 5, 18), 4, mirror));

    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20, -20), 1.5));
    lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f(30, 20, 30), 1.7));

    render(spheres, lights);
}
