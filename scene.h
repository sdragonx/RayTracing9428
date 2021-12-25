#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>


//保存ppm图片
void save_imageP6(int Width, int Height, const char* fname, const unsigned char* pixels);
void save_imageP3(int Width, int Height, const char* fname, const unsigned char* pixels);

//Material
struct Material
{
    float ka;
    float kd;
    float ks;
    float kr;
    float n;
};

//SPHERE <name> <pos x> <pos y> <pos z> <scl x> <scl y> <scl z> <r> <g> <b> <Ka> <Kd> <Ks> <Kr> <n> 
struct Sphere
{
    std::string name;
    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec3 color;
    Material material;
};

//LIGHT <name> <pos x> <pos y> <pos z> <Ir> <Ig> <Ib> 
struct Light
{
    std::string name;
    glm::vec3 pos;
    glm::vec3 color;        //Ir Ig Ib
};

//Ray
struct Ray
{
    glm::vec3 pos;
    glm::vec3 dir;

    Ray() : pos(), dir()
    {

    }

    Ray(glm::vec3 p, glm::vec3 d) : pos(p), dir(d)
    {

    }
};

//Camera
class Camera
{
public:
    glm::vec3 pos;
    glm::vec3 center;
    glm::vec3 dir;
    glm::vec3 up;
    glm::vec3 u;         // left
    glm::vec3 v;         // top
};

//
// Scene
//

class Scene
{
protected:
    float nearZ;
    float left;
    float top;
    float right;
    float bottom;

    std::vector<glm::vec3> colorbuf;//renderer color buffer
    int nColumns, nRows;            //color buffer size
    glm::vec3 background;           //background color
    glm::vec3 ambient;              //globel AMBIENT
    std::string output;             //output image filename

    std::vector<Sphere> spheres;    //spheres
    std::vector<Light> lights;      //lights


    //camera
    Camera camera;
    glm::vec3 viewOffset;
    glm::vec3 viewScaleX;
    glm::vec3 viewScaleY;

public:
    Scene();
    ~Scene();

    //load scene file
    int load(const std::string& filename);

    //clear scene
    void dispose();

    //print scene info
    void print();

    //save ppm image
    void save_image();

    //render
    void render();

    std::string output_image()const
    {
        return output;
    }

    //摄像头观察
    void lookat(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& viewUp)
    {
        camera.pos = eye;
        camera.center = center;
        camera.dir = normalize(center - eye);
        camera.up = viewUp;

        camera.u = glm::normalize(glm::cross(camera.dir, camera.up));
        camera.v = glm::cross(camera.u, camera.dir);

        //fov 90度是对的
        float fov = 90.0f;
        float aspect = float(nColumns) / float(nRows);
        this->perspective(fov, aspect, nearZ, 1000.0f);
    }

private:
    // 透视投影计算，主要计算像素左上角的坐标，和缩放值
    void perspective(float fov, float aspect, float nearZ, float farZ)
    {
        float theta = glm::radians(fov);
        float half_height = glm::tan(theta / 2);
        float half_width = aspect * half_height;

        viewOffset = glm::vec3(-half_width, -half_height, -1.0);
        viewOffset = camera.pos - half_width * camera.u - half_height * camera.v + camera.dir;
        viewScaleX = 2.0f * half_width * camera.u;
        viewScaleY = 2.0f * half_height * camera.v;
    }

    //calc ray
    Ray get_ray(int x, int y);

    //do raytracing
    glm::vec3 raytracing(const Ray& r, int depth);

    // test fragment is inside shadow
    bool inside_shadow(const Light& light, const glm::vec3& fragPos);

    //compute light color
    glm::vec3 compute_light(const Light& light,
        Sphere* obj,
        const glm::vec3& viewPos,
        const glm::vec3& fragPos,
        const glm::vec3& fragNormal);
};

