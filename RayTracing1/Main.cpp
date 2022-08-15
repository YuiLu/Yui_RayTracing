#define CLOCKS_PER_SEC ((clock_t)1000) 
#include <iostream>
#include <time.h>
#include <thread>
#include <mutex>
#include "hittable_list.h"
#include "sphere.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "triangle.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"

using namespace std;

// 全局变量
int barWidth = 50;
int progress = 0;
float percent = 0.0;
mutex mtx;
// 图像
//const float aspect_ratio = 1920.0 / 1080.0;
const float aspect_ratio = 1;
const int image_width = 512;       //test：1000; high-quality：1920
const int image_height = static_cast<int>(image_width / aspect_ratio);
int channels = 3;
const int samples_per_pixel = 100;   //test： 50 ; high-quality：256
const int bounce = 50;              //test： 25 ; high-quality：50
const char* image_name = "Bunny.png";
// 相机
point camera_location(13, 2, 3);
point look_at(0, 0, 0);
vec3 up(0, 1, 0);
double fov = 20;
auto aperture = 0.1;
auto focus_dist = 10;
camera cam(camera_location, look_at, up, fov, aspect_ratio, aperture, focus_dist);
// 场景
hittable_list scene;
color background(0, 0, 0);


// 随机场景，One Weekend封面图，棋盘格地面
hittable_list random_scene() {
    hittable_list scene;

    auto checker = make_shared<checker_texture>(color(0.2, 0.2, 0.2), color(0.9, 0.9, 0.9));
    scene.add(make_shared<sphere>(point(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color(random_double() * random_double(), random_double() * random_double(), random_double() * random_double()) ;
                    sphere_material = make_shared<lambertian>(albedo);
                    scene.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color(random_double() * random_double(), random_double() * random_double(), random_double() * random_double());
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    scene.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    scene.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    scene.add(make_shared<sphere>(point(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    scene.add(make_shared<sphere>(point(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    scene.add(make_shared<sphere>(point(4, 1, 0), 1.0, material3));

    double bvh_start = clock();
    scene = hittable_list(make_shared<bvh_node>(scene));
    double bvh_finish = clock();
    cout << "BVH构建完成！构建耗时：" << bvh_finish - bvh_start << "ms" << endl;
    return scene;
}

// The Next Week封面图，无运动模糊
hittable_list the_next_week()
{
    double scene_build_start = clock();
    // 地面是box的组合，box是rect的组合
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));
    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++)
    {
        for (int j = 0; j < boxes_per_side; j++)
        {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 105);
            auto z1 = z0 + w;

            boxes1.add(make_shared<box>(point(x0, y0, z0), point(x1, y1, z1), ground));
        }
    }

    hittable_list scene;
    scene.add(make_shared<bvh_node>(boxes1));
    // Light
    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    scene.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));
    // 左上smooth金属球
    scene.add(make_shared<sphere>(point(450, 400, 200), 60, make_shared<metal>(color(0.8, 0.8, 0.9), 0)));
    // 左上glossy金属球
    //scene.add(make_shared<sphere>(point(450, 400, 200), 60, make_shared<metal>(color(0.8, 0.8, 0.9), 0.9)));
    // 玻璃球
    scene.add(make_shared<sphere>(point(240, 150, -10), 50, make_shared<dielectric>(1.5)));
    // 右下glossy金属球
    scene.add(make_shared<sphere>(point(-20, 180, -10), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)));
    // 玻璃雾球
    auto boundary = make_shared<sphere>(point(390, 150, 130), 70, make_shared<dielectric>(1.5));
    scene.add(boundary);
    scene.add(make_shared<constant_medium>(boundary, 0.01, color(0.2, 0.4, 0.7)));
    // 全局体积雾，加入体积雾会对收敛速度造成极大影响，建议spp至少5k
    boundary = make_shared<sphere>(point(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    scene.add(make_shared<constant_medium>(boundary, 0.00001, color(1, 1, 1)));
    // 地球
    auto earth_mat = make_shared<lambertian>(make_shared<image_texture>("texture/earthmap2.jpg"));
    scene.add(make_shared<sphere>(point(600, 230, 420), 140, earth_mat));
    // 星空球
    auto StarrySky = make_shared<lambertian>(make_shared<image_texture>("texture/StarrySky.jpg"));
    scene.add(make_shared<sphere>(point(30, 270, 100), 80, StarrySky));
    // 柏林噪声
    auto perlin_tex = make_shared<noise_texture>(0.1);
    scene.add(make_shared<sphere>(point(260, 280, 420), 80, make_shared<lambertian>(perlin_tex)));

    // 粒子矩阵
    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    int particle_num = 1000;
    for (int j = 0; j < particle_num; j++) {
        point random_location = point(random_double(0, 150), random_double(0, 150), random_double(0, 150));
        boxes2.add(make_shared<sphere>(random_location, 10, white));
    }
    scene.add(make_shared<translate>
             (make_shared<rotate_y>(
              make_shared<bvh_node>(boxes2),20),
              vec3(0, 300, 200)));

    scene = hittable_list(make_shared<bvh_node>(scene));
    double scene_build_finish = clock();
    cout << "BVH构建完成！场景构建耗时：" << scene_build_finish - scene_build_start << "ms" << endl;
    return scene;
}

// Cornell Box（Standard）
hittable_list cornell_box()
{
    hittable_list scene;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    scene.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));        //左
    scene.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));            //右
    scene.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));    //灯
    scene.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));          //下
    scene.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));        //上
    scene.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));        //后
    // 高
    shared_ptr<hittable> box1 = make_shared<box>(point(0, 0, 0), point(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    scene.add(box1);
    // 矮
    shared_ptr<hittable> box2 = make_shared<box>(point(0, 0, 0), point(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));
    scene.add(box2);

    double bvh_start = clock();
    scene = hittable_list(make_shared<bvh_node>(scene));
    double bvh_finish = clock();
    cout << "BVH构建完成！构建耗时：" << bvh_finish - bvh_start << "ms" << endl;
    return scene;
}

// Cornell Box（Smoke）
hittable_list cornell_box_smoke()
{
    hittable_list scene;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    scene.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));        //左
    scene.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));            //右
    scene.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));    //灯
    scene.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));          //下
    scene.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));        //上
    scene.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));        //后
    // 高
    shared_ptr<hittable> box1 = make_shared<box>(point(0, 0, 0), point(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    scene.add(make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
    // 矮
    shared_ptr<hittable> box2 = make_shared<box>(point(0, 0, 0), point(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));
    scene.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

    double bvh_start = clock();
    scene = hittable_list(make_shared<bvh_node>(scene));
    double bvh_finish = clock();
    cout << "BVH构建完成！构建耗时：" << bvh_finish - bvh_start << "ms" << endl;
    return scene;
}

// Cornell Box（Test）
hittable_list my_cornell_box()
{
    hittable_list scene;
    auto red = make_shared<lambertian>(color(0.65, 0.1, 0.1));
    auto white = make_shared<lambertian>(color(0.8, 0.8, 0.8));
    auto blue = make_shared<lambertian>(color(0.12, 0.34, 0.94));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));
    scene.add(make_shared<yz_rect>(0, 555, 0, 555, 555, red));
    scene.add(make_shared<yz_rect>(0, 555, 0, 555, 0, blue));
    scene.add(make_shared<xz_rect>(180, 376, 180, 340, 554, light));
    scene.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    scene.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    scene.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
    scene.add(make_shared<triangle>(point(500, 50, 300), point(200, 500, 300), point(100, 250, 250), red));

    //double bvh_start = clock();
    //scene = hittable_list(make_shared<bvh_node>(scene));
    //double bvh_finish = clock();
    //cout << "BVH构建完成！构建耗时：" << bvh_finish - bvh_start << "ms" << endl;
    return scene;
}

// 双球场景，此场景较简单，因此不构建BVH
hittable_list two_sphere()
{
    hittable_list scene;
    auto checker = make_shared<checker_texture>(color(0.0, 0.0, 0.0), color(1.0, 1.0, 1.0));
    scene.add(make_shared<sphere>(point(0, -10, 0), 10, make_shared<lambertian>(checker)));
    scene.add(make_shared<sphere>(point(0, 10, 0), 10, make_shared<lambertian>(checker)));
    return scene;
}

// 柏林噪声，此场景较简单，因此不构建BVH
hittable_list perlin_test()
{
    hittable_list scene;
    auto perlin_texture = make_shared<noise_texture>(4);
    scene.add(make_shared<sphere>(point(0, -1000, 0), 1000, make_shared<lambertian>(perlin_texture)));
    scene.add(make_shared<sphere>(point(0, 2, 0), 2, make_shared<lambertian>(perlin_texture)));

    auto diff_light = make_shared<diffuse_light>(color(4, 4, 4));
    scene.add(make_shared<xy_rect>(3, 5, 1, 3, -2, diff_light));
    return scene;
}


// 场景与相机设置
void set_scene_and_camera()
{
    int choice = 1;
    cout << "请选择场景：\n"
         << "1、One Weekend封面图，棋盘格地面\n"
         << "2、The Next Week封面图（无运动模糊）\n"
         << "3、Cornell Box (Standard)\n"
         << "4、Cornell Box (Smoke)\n"
         << "5、Cornell Box (Test)\n"
         << "6、棋盘格经纬球*2\n"
         << "7、Perlin Test，可用于测试ToneMapping\n"
         << endl;
    cin >> choice;
    switch (choice)
    {
    case 1:
        scene = random_scene();
        background = color(0.70, 0.80, 1.00);
        camera_location = point(13, 2, 3);
        cam = camera(camera_location, look_at, up, fov, aspect_ratio, aperture, focus_dist);
        break;
    case 2:
        scene = the_next_week();
        background = color(0, 0, 0);
        camera_location = point(450, 280, -525);
        look_at = point(300, 300, 0);
        fov = 45.0;
        cam = camera(camera_location, look_at, up, fov, aspect_ratio, aperture, focus_dist);
        break;
    case 3:
        scene = cornell_box();
        background = color(0, 0, 0);
        camera_location = point(278, 278, -800);
        look_at = point(278, 278, 0);
        fov = 40.0;
        cam = camera(camera_location, look_at, up, fov, aspect_ratio, aperture, focus_dist);
        break;
    case 4:
        scene = cornell_box_smoke();
        background = color(0, 0, 0);
        camera_location = point(278, 278, -800);
        look_at = point(278, 278, 0);
        fov = 40.0;
        cam = camera(camera_location, look_at, up, fov, aspect_ratio, aperture, focus_dist);
        break;
    case 5:
        scene = my_cornell_box();
        background = color(0, 0, 0);
        camera_location = point(278, 278, -800);
        look_at = point(278, 278, 0);
        fov = 40.0;
        cam = camera(camera_location, look_at, up, fov, aspect_ratio, aperture, focus_dist);
        break;
    case 6:
        scene = two_sphere();
        background = color(0.70, 0.80, 1.00);
        break;
    case 7:
        scene = perlin_test();
        background = color(0, 0, 0);
        camera_location = point(26, 3, 6);
        look_at = point(0, 2, 0);
        cam = camera(camera_location, look_at, up, fov, aspect_ratio, aperture, focus_dist);
        break;

    default:
        background = color(0, 0, 0);
        break;
    }
}

// 求交并计算颜色
color ray_color(const ray& r, const color& background, const hittable& scene, int bounce)
{
    hit_record rec;
    // 超过光线弹射限制
    if (bounce <= 0)
        return vec3(0, 0, 0);
    // hit nothing
    if (!scene.hit(r, 0.001, infinity, rec))
        return background;
    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;
    // 递归
    return emitted + attenuation * ray_color(scattered, background, scene, bounce - 1);

    //// 背景
    //vec3 normalized_direction = normalize(r.Get_Direction());
    //auto t = 0.5 * (normalized_direction.y() + 1.0);
    //return lerp(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t);
}

// 渲染

void RayTracingInOneThread(unsigned char*& data, int thread_id, int thread_num)
{
    for (int j = image_height - thread_id - 1; j >= 0; j-= thread_num)
    {
        //double debug_time_start = clock();
        for (int i = 0; i < image_width; i++)
        {
            color col(0, 0, 0);
            // SSAA抗锯齿
            for (int s = 0; s < samples_per_pixel; s++)
            {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                col += (ray_color(r, background, scene, bounce) / samples_per_pixel);
            }
            // 截断
            col[0] = col[0] > 1 ? 1 : col[0];
            col[1] = col[1] > 1 ? 1 : col[1];
            col[2] = col[2] > 1 ? 1 : col[2];
            // Gamma校正+颜色映射
            int final_r = int(255.99 * pow(col[0], 0.45));
            int final_g = int(255.99 * pow(col[1], 0.45));
            int final_b = int(255.99 * pow(col[2], 0.45));
            data[(image_height - j - 1) * image_width * channels + i * channels] = final_r;
            data[(image_height - j - 1) * image_width * channels + i * channels + 1] = final_g;
            data[(image_height - j - 1) * image_width * channels + i * channels + 2] = final_b;
        }

        // 进度条
        mtx.lock();  //互斥锁
        progress++;
        percent = ((float)progress / image_height) * 100;
        float pos = percent / 100 * barWidth;
        cout << "[";
        for (int i = 0; i < barWidth; ++i)
        {
            if (i < pos)
                cout << "=";
            else if (i == pos)
                cout << ">";
            else
                cout << " ";
        }
        cout << "]" << (int)percent << "%\r";
        cout.flush();
        mtx.unlock();
        //// debug
        //finish = clock();
        //cout << "第" << image_height - j << "行执行消耗" << (finish - start) / CLOCKS_PER_SEC << "s" << endl;
    }
}



int main()
{
    // 输出图片信息
    printf("图片大小：%d×%d，通道数：%d\n", image_width, image_height, channels);
    printf("采样数：%d，bounce：%d\n", samples_per_pixel, bounce);

    unsigned char* data = new unsigned char[image_width * image_height * channels];
    set_scene_and_camera();

    // 多线程
    int thread_num = 8;
    cout << "线程数：" << thread_num << endl;
    vector<thread> threads;
    for (int k = 0; k < thread_num; k++)
    {
        threads.push_back(thread(RayTracingInOneThread, ref(data), k, thread_num));
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
    cout << endl;
    // 执行耗时
    int time = (clock() / CLOCKS_PER_SEC) / 60;
    printf("执行耗时：%d h %d min", time / 60, time % 60);
    stbi_write_png(image_name, image_width, image_height, channels, data, 0);
    return 0;
}
