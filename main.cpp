#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

//READ FILE TO STRING (for shader files)
std::string get_file_contents(const char* filename)
{
    std::ifstream in(filename, std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize((size_t)in.tellg()); 
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

int main()
{
    //initialize
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(600, 600, "Bravo_quiz1 - Danielle R. Bravo", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();

    //LOAD SHADERS FROM FILE
    std::string vertexCode = get_file_contents("Shaders/sample.vert");
    std::string fragmentCode = get_file_contents("Shaders/sample.frag");

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    //compile Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    //compile Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    //Link
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //LOAD MESH 
    std::string path = "3D/bunny.obj";
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    tinyobj::attrib_t attributes;

    bool isLoaded = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, path.c_str());

    //fail checker
    if (!isLoaded) {
        std::cout << "Failed to load mesh: " << err << std::endl;
        return -1;
    }

    std::vector<GLuint> mesh_indices;
    std::vector<GLfloat> mesh_vertices;

    for (size_t i = 0; i < shapes[0].mesh.indices.size(); i++) {
        tinyobj::index_t vData = shapes[0].mesh.indices[i];

        mesh_indices.push_back((GLuint)i);

        mesh_vertices.push_back((GLfloat)attributes.vertices[3 * vData.vertex_index + 0]); // X
        mesh_vertices.push_back((GLfloat)attributes.vertices[3 * vData.vertex_index + 1]); // Y
        mesh_vertices.push_back((GLfloat)attributes.vertices[3 * vData.vertex_index + 2]); // Z
    }

    //BUFFERS
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mesh_vertices.size(), mesh_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh_indices.size(), mesh_indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //RENDER LOOP
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        float currTime = (float)glfwGetTime();

        //CAMERA SETUP
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), 600.0f / 600.0f, 0.1f, 1000.0f); 
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 90.0f), // camera moved back because bunnies are big
            glm::vec3(0.0f, 0.0f, 0.0f),  // Looking at center
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up is Y
        );

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);

        float offset = (2.0f * 3.14159f) / 3.0f;

        //BUNNY 1
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::rotate(model1, currTime, glm::vec3(0.0f, 0.0f, 1.0f));
        model1 = glm::translate(model1, glm::vec3(25.0f, 0.0f, 0.0f)); 
        model1 = glm::scale(model1, glm::vec3(90.0f, 90.0f, 90.0f));   
        model1 = glm::rotate(model1, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //Face Camera

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glDrawElements(GL_TRIANGLES, (GLsizei)mesh_indices.size(), GL_UNSIGNED_INT, 0);

        //BUNNY 2
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::rotate(model2, currTime + offset, glm::vec3(0.0f, 0.0f, 1.0f));
        model2 = glm::translate(model2, glm::vec3(25.0f, 0.0f, 0.0f));
        model2 = glm::scale(model2, glm::vec3(90.0f, 90.0f, 90.0f));
        model2 = glm::rotate(model2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
        glDrawElements(GL_TRIANGLES, (GLsizei)mesh_indices.size(), GL_UNSIGNED_INT, 0);

        //BUNNY 3
        glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::rotate(model3, currTime + (offset * 2), glm::vec3(0.0f, 0.0f, 1.0f));
        model3 = glm::translate(model3, glm::vec3(25.0f, 0.0f, 0.0f));
        model3 = glm::scale(model3, glm::vec3(90.0f, 90.0f, 90.0f));
        model3 = glm::rotate(model3, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
        glDrawElements(GL_TRIANGLES, (GLsizei)mesh_indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}