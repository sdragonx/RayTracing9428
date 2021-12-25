#include "scene.h"
#include "time.h"

#include <algorithm>

using namespace std;

Scene::Scene()
{
    nColumns = nRows = 0;
}


Scene::~Scene()
{
}

//load scene file
int Scene::load(const std::string& filename)
{
    std::ifstream in(filename);

    if (in.fail()) {
        return -1;
    }

    cout << "load \"" << filename << "\" ..." << endl;

    this->dispose();

    std::string str;

    while (!in.eof()) {
        // read a line
        std::getline(in, str);

        //skip empty line
        if (str.empty()) {
            continue;
        }

        //allocate stringstream
        std::stringstream stm(str);

        stm >> str;
        if (str == "NEAR") {
            stm >> nearZ;
        }
        else if (str == "LEFT") {
            stm >> left;
        }
        else if (str == "TOP") {
            stm >> top;
        }
        else if (str == "RIGHT") {
            stm >> right;
        }
        else if (str == "BOTTOM") {
            stm >> bottom;
        }
        else if (str == "RES") {
            stm >> nColumns >> nRows;

            //init color buffer
            colorbuf.resize(nColumns * nRows);
        }
        else if (str == "SPHERE") {
            Sphere obj;

            stm >> obj.name;
            stm >> obj.pos.x >> obj.pos.y >> obj.pos.z;
            stm >> obj.scale.x >> obj.scale.y >> obj.scale.z;
            stm >> obj.color.x >> obj.color.y >> obj.color.z;
            stm >> obj.material.ka >> obj.material.kd >> obj.material.ks >> obj.material.kr >> obj.material.n;

            spheres.push_back(obj);
        }
        else if (str == "LIGHT") {
            Light obj;

            stm >> obj.name;
            stm >> obj.pos.x >> obj.pos.y >> obj.pos.z;
            stm >> obj.color.x >> obj.color.y >> obj.color.z;

            lights.push_back(obj);
        }
        else if (str == "BACK") {
            stm >> background.x >> background.y >> background.z;
        }
        else if (str == "AMBIENT") {
            stm >> ambient.x >> ambient.y >> ambient.z;
        }
        else if (str == "OUTPUT") {
            stm >> output;
        }
        else {
            cout << "unknown key word: " << str << endl;
        }
    }

    //print scene info
    print();
}

//clear scene
void Scene::dispose()
{
    spheres.clear();
    lights.clear();
    colorbuf.clear();
}

//print scene info
void Scene::print()
{
    cout << "scene: " << endl;
    cout << "  near        = " << nearZ << endl;
    cout << "  left        = " << left << endl;
    cout << "  top         = " << top << endl;
    cout << "  right       = " << right << endl;
    cout << "  bottom      = " << bottom << endl;

    cout << "  colorbuffer = " << nColumns << " x " << nRows << endl;
    cout << "  background  = " << background << endl;
    cout << "  ambient     = " << ambient << endl;
    cout << "  output      = " << output << endl;
    cout << endl;

    for (auto& obj : lights) {
        cout << "light: " << obj.name << endl;
        cout << "  position    = " << obj.pos << endl;
        cout << "  color       = " << obj.color << endl;
        cout << endl;
    }

    for (auto& obj : spheres) {
        cout << "sphere: " << obj.name << endl;
        cout << "  position    = " << obj.pos << endl;
        cout << "  scale       = " << obj.scale << endl;
        cout << "  color       = " << obj.color << endl;
        cout << "  material    = " << obj.material.ka << ", " << obj.material.kd << ", " << obj.material.ks
            << ", " << obj.material.kr << ", " << obj.material.n << endl;
        /*
        cout << "  material.ka = " << obj.material.ka << endl;
        cout << "  material.kd = " << obj.material.kd << endl;
        cout << "  material.ks = " << obj.material.ks << endl;
        cout << "  material.kr = " << obj.material.kr << endl;
        cout << "  material.n  = " << obj.material.n << endl;
        */
        cout << endl;
    }
}

//save ppm image
void Scene::save_image()
{
    //transform float buffer to byte buffer
    std::vector<uint8_t> bytebuf;

    bytebuf.resize(nColumns * nRows * 3);

    for (size_t i = 0; i < nColumns * nRows; ++i) {
        bytebuf[i * 3 + 0] = (int) (colorbuf[i].r * 255.0f);
        bytebuf[i * 3 + 1] = (int) (colorbuf[i].g * 255.0f);
        bytebuf[i * 3 + 2] = (int) (colorbuf[i].b * 255.0f);
    }

    //save_imageP3(nColumns, nRows, output.c_str(), bytebuf.data());
    save_imageP6(nColumns, nRows, output.c_str(), bytebuf.data());

    //cout << "output: \"" << output << "\" ok." << endl;
}


//碰撞的最近距离和最远距离
//const float t_min = 0.000001f;    //这个有斑点
const float t_min = 0.0001f;
const float t_max = 999999.0f;

//render
void Scene::render()
{
    if (nColumns == 0 || nRows == 0 || colorbuf.empty()) {
        cout << "scene is not init." << endl;
        return;
    }

    Ray r;
    glm::vec3 color;

    time_t now = clock();
    //traverse all pixels and start rendering the image
    for (int y = 0; y < nRows; ++y) {
        for (int x = 0; x < nColumns; ++x) {
            r = get_ray(x, y);

            color = this->raytracing(r, 0);
            color = glm::clamp(color, 0.0f, 1.0f);  //clamp to 0~1
            colorbuf[y * nColumns + x] = color;
        }
    }

    clock_t t = clock() - now;
    cout << "render time: " << float(t) / 1000.0f << "s" << endl;
}

//calc ray
Ray Scene::get_ray(int x, int y)
{
    //float u = 1.0 - (float) x / float(nColumns - 1);
    float u = (float) x / float(nColumns - 1);
    float v = 1.0 - (float) y / float(nRows - 1);

    glm::vec3 dir = viewOffset + u * viewScaleX + v * viewScaleY - camera.pos;
    dir = normalize(dir);

    //cout << camera.pos << " " << dir << endl;

    return Ray(camera.pos, dir);
}

// intersect
bool ray_sphere_intersection(const Ray& ray, const glm::vec3& pos, float radius, float& dist)
{
    glm::vec3 oc = ray.pos - pos;
    float a = glm::dot(ray.dir, ray.dir);
    float b = 2 * glm::dot(oc, ray.dir);
    float c = glm::dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant > 0)
    {
        float temp = (-b - glm::sqrt(discriminant)) / (2 * a);
        if (temp < t_max && temp > t_min) {
            dist = temp;
            return true;
        }

        temp = (-b + glm::sqrt(discriminant)) / (2 * a);
        if (temp < t_max && temp > t_min) {
            dist = temp;
            return true;
        }
    }

    return false;
}

//执行光追过程
//这个是光追程序的主要函数，光线Ray从摄像头位置射出，和场景里面的所有物品碰撞
//如果发生碰撞，返回碰撞物品的材质、颜色属性
glm::vec3 Scene::raytracing(const Ray& r, int depth)
{
    glm::vec3 pos;
    float dist;             //碰撞距离

    Sphere* obj = nullptr;  //最终碰撞的球
    float objDist = t_max;  //碰撞测试过程中，记录最小距离
    glm::vec3 fragPos;      //焦点
    glm::vec3 fragNormal;   //法线

    //碰撞检测部分

    #if 1
    for (auto& sphere : spheres) {
        //使用缩放重新计算球体位置
        pos = sphere.pos / sphere.scale;

        //射线也除以球体的缩放，比如球体上下拉长，射线就上下缩小点，这样求出来的值是对的
        glm::vec3 p = r.pos / sphere.scale;
        glm::vec3 d = r.dir / sphere.scale;

        //判断是否发生碰撞，并记录距离最短的哪个球
        if (ray_sphere_intersection(Ray(p, d), pos, 1.0f, dist)) {
            if (dist < objDist) {
                obj = &sphere;
                objDist = dist;
                //这两个方法，道理大同小异
                #if 1
                //方法1：ok
                fragPos = r.pos + r.dir * dist;
                fragNormal = fragPos - sphere.pos;
                //fragPos *= sphere.scale;
                fragNormal /= sphere.scale * sphere.scale;
                #else 
                //方法2：ok
                fragPos = p + d * dist;
                fragPos *= obj->scale;
                fragNormal = fragPos - sphere.pos;
                //fragPos *= sphere.scale;
                fragNormal /= sphere.scale * sphere.scale;
                #endif

                fragNormal = glm::normalize(fragNormal);
            }
        }
    }

    #else
    //pdf里面的矩阵方法，因为这个只有缩放，实际用不用矩阵无所谓，这个效果和上面一样
    glm::mat3 m;
    glm::mat3 inv_m;

    for (auto& sphere : spheres) {
        //计算矩阵
        m = glm::scale(glm::identity<glm::mat4>(), sphere.scale);
        inv_m = glm::inverse(m);

        pos = inv_m * sphere.pos;

        glm::vec3 p = inv_m * r.pos;
        glm::vec3 d = inv_m * r.dir;

        if (ray_sphere_intersection(Ray(p, d), pos, 1.0f, dist)) {
            if (dist < objDist) {
                obj = &sphere;
                objDist = dist;
                fragPos = r.pos + r.dir * dist;
                fragNormal = fragPos - sphere.pos;
                //fragPos = inv_m * fragPos;
                fragNormal = inv_m * inv_m * fragNormal;
                fragNormal = glm::normalize(fragNormal);
            }
        }
    }
    #endif

    //如果没有发生任何碰撞，返回背景色
    if(obj == nullptr) {
        //这里有个分支，这个题目反射颜色没有计算背景色，不然结果不对
        if (depth != 0)return glm::vec3();
        return background;
    }

    //光照部分，这里用的是文档里面的方法，实际gl计算光照有差异，不然结果不正确

    //环境光 Ka*Ia[c]*O[c]
    glm::vec3 ambientColor = ambient * obj->material.ka * obj->color;

    //最终光照颜色
    glm::vec3 lightColor = glm::vec3(0.0f);

    //foreach point light (p) { Kd * Ip[c] * (N dot L) * O[c] + Ks * Ip[c] * (R dot V)^n }
    for (size_t i = 0; i < lights.size(); ++i) {
        // test inside shadow
        if (inside_shadow(lights[i], fragPos)) {
            //阴影里面什么也不干
        }
        else {
            // lighting
            lightColor += compute_light(lights[i], obj, r.pos, fragPos, fragNormal);
        }
    }

    //反射颜色，最大深度3
    glm::vec3 reflectColor = glm::vec3(0.0f);
    if (depth < 3) {
        glm::vec3 dir = glm::reflect(r.dir, fragNormal);

        //new ray
        Ray ray(fragPos, dir);

        //再追踪一次
        reflectColor = raytracing(ray, depth + 1) * obj->material.kr;//反射率
    }

    //返回最终结果
    glm::vec3 color = ambientColor + lightColor + reflectColor;
    return color;
}

// test fragment is inside shadow
bool Scene::inside_shadow(const Light& light, const glm::vec3& fragPos)
{
    float dist;

    //重新计算光线的方向
    glm::vec3 lightDir = light.pos - fragPos;
    lightDir = glm::normalize(lightDir);

    // 线段检测，这里目前有问题
    for (auto& sphere : spheres) {
        glm::vec3 pos = sphere.pos / sphere.scale;

        //创建一个新的射线，从点的位置，向光源方向发射
        Ray ray(fragPos / sphere.scale, lightDir / sphere.scale);

        if (ray_sphere_intersection(ray, pos, 1.0f, dist)) {
            //只要发生碰撞，就表示这个点在阴影里面
            return true;
        }
    }

    return false;
}

//compute light color
glm::vec3 Scene::compute_light(const Light& light,
    Sphere* obj,
    const glm::vec3& viewPos,
    const glm::vec3& fragPos,
    const glm::vec3& fragNormal)
{
    glm::vec3 normal = glm::normalize(fragNormal);

    //point light direction
    glm::vec3 lightDir = glm::normalize(light.pos - fragPos);

    //ambient
    //glm::vec3 ambientColor = light.color * obj->material.ka;

    //diffuse
    float diffuse = std::max(glm::dot(normal, lightDir), 0.0f);
    glm::vec3 diffuseColor = light.color * (obj->material.kd * diffuse) * obj->color;

    //specular
    glm::vec3 viewDir = glm::normalize(viewPos - fragPos);
    glm::vec3 reflectDir = glm::normalize(glm::reflect(-lightDir, normal));
    float specular = std::max(glm::dot(reflectDir, viewDir), 0.0f);
    specular = glm::pow(specular, obj->material.n);
    glm::vec3 specularColor = light.color * (obj->material.ks * specular);

    //return ambientColor + diffuseColor + specularColor;
    return diffuseColor + specularColor;
}