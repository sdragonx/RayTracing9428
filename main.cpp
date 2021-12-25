

#include "scene.h"

using namespace std;

#include <glm/gtx/rotate_vector.inl>

int main(int argc, char* argv[])
{
    Scene scene;
    string filename;

    if (argc <= 1) {
        //直接运行，测试用
        filename = "testAmbient.txt";
        filename = "testDiffuse.txt";
        filename = "testSpecular.txt";
        //filename = "testBackground.txt";
        //filename = "testIntersection.txt";
        filename = "testShadow.txt";
        filename = "testReflection.txt";
        filename = "testSample.txt";
    }
    else {
        //读取参数
        filename = argv[1];
    }

    //load scene
    scene.load(filename);

    //lookat

    //前期测试，这个对
    //scene.lookat(glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //最后测试，摄像头这个位置是对的，站在原点往z=-20的位置看
    scene.lookat(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    //render
    scene.render();

    //save image
    scene.save_image();

    system(scene.output_image().c_str());
    //system("pause");
    return 0;
}