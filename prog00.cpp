#include <vector>
#include "geometry.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <limits>
#include <stdlib.h>
#include <string>

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
        Vec3f l = orig - center;
        float a = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
        float b = 2 * (dir.x * l.x + dir.y * l.y + dir.z * l.z);
        float c = l.x * l.x + l.y * l.y + l.z * l.z - radius * radius;
        if (b * b < 4 * a * c) {
            return false;
        } else {
            float d = b * b - 4 * a * c;
            float solution1 = (-b - sqrt(d)) / (2 * a);
            float solution2 = (-b + sqrt(d)) / (2 * a);

            if (std::max(solution1, solution2) >= 0) {
                if (solution1 >= 0) {
                    dist = solution1;
                } else {
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

    if (depth > 4 || !scene_intersect(orig, dir, spheres, hit_point, N, material)) {
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

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights, int img_width, int img_height, float fov) {
    std::vector< std::vector<Vec3f> > img(img_width, std::vector<Vec3f>(img_height));

    for (size_t j = 0; j < img_height; ++j) {
        for (size_t i = 0; i < img_width; ++i) {
            float x = -img_width / 2. + i;
            float y = img_height / 2. - j;
            float z = img_height / 2. / tan(fov / 2.);
            Vec3f dir = Vec3f(x, y, z).normalize();
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
    int img_width, img_height;
    float fov;
    std::cout << "Для ввода целого числа     (int)              напишите его и нажмите Enter.\n";
    std::cout << "          дробного числа   (float)            напишите его, используя десятичную точку, и нажмите Enter.\n";
    std::cout << "          нескольких чисел (multiple numbers) напишите их через пробел без каких-либо других разделителей и нажмите Enter.\n";
    std::cout << "          списка           (list)             напишите каждый его элемент на новой строке, после ввода всех элементов введите -666.\n\n";

    std::cout << "Цвета       вводятся, как 3 числа типа float (от 0 до 1 каждое), обозначающие красную, синюю и зеленую компоненты в цветовой модели rgb.\n";
    std::cout << "Коэффициеты вводятся, как числа типа float.\n\n";

    std::cout << "Введите ширину картинки (int):          "; std::cin >> img_width;
    std::cout << "Введите высоту картинки (int):          "; std::cin >> img_height;
    std::cout << "Введите угол обзора в градусах (float): "; std::cin >> fov;

    std::cout << "\nВведите список (list) материалов в формате (цвет; показатель бликов (float); коэффициенты влияния: собственного цвета, бликов, отражения):\n";
    std::vector<Material> materials;
    float r, g, b, spec_exp, a0, a1, a2;
    for (size_t i = 1; 1; ++i) {
        std::cout << i << ". ";
        std::cin >> r; if (r == -666) break;
        std::cin >> g >> b >> spec_exp >> a0 >> a1 >> a2;
        materials.push_back(Material(Vec3f(r, g, b), spec_exp, Vec3f(a0, a1, a2)));
    }
    std::cout << "\n\n";


    std::cout << "Камера находится по координатам (0, 0, 0) и направлена вдоль третьей оси.\n\n";
    std::cout << "Введите список (list) сфер в формате (координаты (3 числа float); радиус(float); номер материала):\n";
    std::vector<Sphere> spheres;
    float x, y, z, rad;
    int imat;
    for (size_t i = 1; 1; ++i) {
        std::cout << i << ". ";
        std::cin >> x; if (x == -666) break;
        std::cin >> y >> z >> rad >> imat;
        spheres.push_back(Sphere(Vec3f(x, y, z), rad, materials[imat - 1]));
    }
    std::cout << "\n\n";


    std::cout << "Введите список (list) источников света в формате (координаты (3 числа float); интенсивность (float)):\n";
    std::vector<Light> lights;
    float intens;
    for (size_t i = 1; 1; ++i) {
        std::cout << i << ". ";
        std::cin >> x; if (x == -666) break;
        std::cin >> y >> z >> intens;
        lights.push_back(Light(Vec3f(x, y, z), intens));
    }

    std::cout << "\nПрограмма начала работу... Позднее вы получите сообщение о завершении работы.\n";
    render(spheres, lights, img_width, img_height, fov * M_PI / 180);
    std::cout << "Работа завершена! Картинка сохранена под именем \"out.ppm.\"\n";
}
