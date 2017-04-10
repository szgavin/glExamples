#include "../common/util.h"
#include "../common/shader.h"
#include "material.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* VERTEX_SRC = "#version 330 core\n"
                         "layout(location=0) in vec3 position;"
                         "layout(location=1) in vec3 normal;"
                         "layout(location=2) in vec2 texcoord;"
                         "uniform mat4 model;"
                         "uniform mat4 view;"
                         "uniform mat4 projection;"
                         "uniform vec3 ambientLight;"
                         "uniform vec3 lightPos;"
                         "uniform vec3 lightColor;"
                         "out vec3 fColor;"
                         "out vec2 fTexcoord;"
                         "void main()"
                         "{"
                         "    fTexcoord = texcoord;"
                         "    vec3 normal_v = normalize(mat3(transpose(inverse(model))) * normal);"
                         "    vec3 ms_position = vec3(model * vec4(position, 1.0));"
                         "    vec3 lightDir = normalize(lightPos - ms_position);"
                         "    float diff = max(dot(normal_v, -lightDir), 0.0);"
                         "    fColor = ambientLight + diff * lightColor;"
                         "    gl_Position = projection * view * model * vec4(position, 1.0);"
                         "}"
                         ;

const char* FRAGMENT_SRC = "#version 330 core\n"
                           "in vec3 fColor;"
                           "in vec2 fTexcoord;"
                           "uniform sampler2D diffuse;"
                           "out vec4 outputColor;"
                           "void main()"
                           "{"
                           "    outputColor = texture(diffuse, fTexcoord) * vec4(fColor, 1.0);"
                           "}"
                           ;

int main(void)
{
    GLFWwindow* window;

    window = init("Vertex Shading", 640, 480);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)640/(float)480, 0.1f, 1000.0f);
    glm::mat4 view;

    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

    Material mat;
    if(!mat.load(VERTEX_SRC, FRAGMENT_SRC))
    {
        std::cerr << "Could not load shaders" << std::endl;
        return -1;
    }
    mat.use();
    int w, h;
    GLuint texture = loadImage("asteroid.png", &w, &h, 0, false);
    if(!texture)
    {
        std::cerr << "Could not load texture" << std::endl;
        return -1;
    }
    mat.setDiffuseTexture(texture);

    Mesh mesh;
    if(!mesh.load("asteroid.obj"))
    {
        std::cerr << "Could not load mesh" << std::endl;
    }
    
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

    float angle = 0.0f;

    glm::vec3 ambientLight(0.3, 0.3, 0.3);
    glm::vec3 lightPos(0.0f, -3.0f, -4.0f);
    glm::vec3 lightColor(0.7, 0.5, 0.2);

    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rotate the mesh over time so we see the 3D effect better
        angle += glfwGetTime();
        mesh.setAngle(angle, angle / 2.0f, 0.0f);
        glfwSetTime(0.0);

        const glm::mat4& model = mesh.getModelMatrix();

        mat.bind();
        mat.setUniform("model", model);
        mat.setUniform("view", view);
        mat.setUniform("projection", proj);
        mat.setUniform("ambientLight", ambientLight);
        mat.setUniform("lightPos", lightPos);
        mat.setUniform("lightColor", lightColor);

        mesh.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}
