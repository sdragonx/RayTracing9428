

#include "scene.h"

using namespace std;

#include <glm/gtx/rotate_vector.inl>

int main(int argc, char* argv[])
{
    Scene scene;
    string filename;

    if (argc <= 1) {
        //ֱ�����У�������
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
        //��ȡ����
        filename = argv[1];
    }

    //load scene
    scene.load(filename);

    //lookat

    //ǰ�ڲ��ԣ������
    //scene.lookat(glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //�����ԣ�����ͷ���λ���ǶԵģ�վ��ԭ����z=-20��λ�ÿ�
    scene.lookat(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    //render
    scene.render();

    //save image
    scene.save_image();

    system(scene.output_image().c_str());
    //system("pause");
    return 0;
}