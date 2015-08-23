#include "../common/util.h"
#include "../common/shader.h"
#include "../common/camera.h"
#include "material.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WIDTH 640
#define HEIGHT 480
#define NUM_ASTEROIDS 2000
#define SEED 1993

/*
 * Multiple render targets!
 * We create a frame buffer with several GL_COLOR_ATTACHMENTs,
 * which allows us to render to multiple textures at once
 */
struct GBuffer
{
    GBuffer()
    {
        // Generate a framebuffer
        glGenFramebuffers(1, &buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);

        // Create a texture for the position buffer
        glGenTextures(1, &position);
        glBindTexture(GL_TEXTURE_2D, position);
        gTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Bind the position buffer to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position, 0);

        // Create a texture for the normal buffer
        glGenTextures(1, &normal);
        glBindTexture(GL_TEXTURE_2D, normal);
        gTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Bind the normal buffer to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal, 0);

        // Create a texture for the color buffer
        glGenTextures(1, &color);
        glBindTexture(GL_TEXTURE_2D, color);
        gTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Bind the color buffer to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, color, 0);

        GLuint colorAttachments[] =
        {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2
        };

        glDrawBuffers(3, colorAttachments);

        // Create a depth buffer
        glGenRenderbuffers(1, &depth);
        glBindRenderBuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
        // Bind the depth buffer to the framebuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Could not create G-Buffer!" << std::endl
                << "Error number: " << glGetError() << std::endl;
            exit(1);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };

    ~GBuffer()
    {
        // or glDeleteTextures(3, &position);
        glDeleteTextures(1, &position);
        glDeleteTextures(1, &normal);
        glDeleteTextures(1, &color);
        glDeleteRenderbuffers(1, &depth);
        glDeleteFramebuffers(1, &buffer);
    };

    GLuint buffer, position, normal, color, depth;
};

int main(void)
{
    GLFWwindow* window;
    window = init("Deferred Shading", WIDTH, HEIGHT);
    if(!window)
    {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Camera camera(CAMERA_PERSPECTIVE, 45.0f, 0.1f, 1000.0f, (float)WIDTH, (float)HEIGHT);
    setCamera(&camera);

    /*GLuint program;
    {
        GLuint vertex = createShader(VERTEX_SRC, GL_VERTEX_SHADER);
        GLuint fragment = createShader(FRAGMENT_SRC, GL_FRAGMENT_SHADER);
        program = createShaderProgram(vertex, fragment);
        linkShader(program);
        validateShader(program);
        glDetachShader(program, vertex);
        glDeleteShader(vertex);
        glDetachShader(program, fragment);
        glDeleteShader(fragment);
    }*/ // TODO

    Mesh mesh;
    if(!mesh.load("asteroid.obj"))
    {
        std::cerr << "Could not load mesh" << std::endl;
        return -1;
    }

    // Set random positions for the asteroids
    std::vector<glm::mat4> models;
    models.resize(NUM_ASTEROIDS); // reserve and push_back might be more efficient
    srand(SEED);
    for(int i = 0; i < NUM_ASTEROIDS; ++i)
    {
        glm::mat4 model;

        // Translate
        model = glm::translate(model, glm::vec3(rand() % 100 - 50.0f, rand() % 100 - 50.0f, rand() % 100));

        // Scale
        float scale = (rand() % 200) / 100.0f + 0.1f;
        model = glm::scale(model, glm::vec3(scale, scale, scale));

        // Rotate
        model = glm::rotate(model, glm::radians((float)(rand() % 100)), glm::vec3(1.0f, 1.0f, 1.0f));

        models[i] = model;
    }

    mesh.setInstances(NUM_ASTEROIDS, models);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        }

        updateCamera(WIDTH, HEIGHT, window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: GEOM PASS + LIGHT PASS

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteTextures(1, &texture);

    glfwTerminate();
    return 0;
}