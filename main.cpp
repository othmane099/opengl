
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include<GL/freeglut.h>
#include <vector>


#include <iostream>
#include "Shader.h"

#include "DirectionalLight.h";
#include "Material.h"
#include "PointLight.h"


using namespace std;
using namespace glm;

struct STRVertex
{
    vec3 position;
    vec3 couleur;
    vec3 normal;
};

// Global Variables

vector<Shader> shaderList;


// Vertex Shader
static const char* vShader = "./SimpleVertexShader.vertexshader";

// Fragment Shader
static const char* fShader = "./SimpleFragmentShader.fragmentshader";

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];

Material shinyMaterial;
Material dullMaterial;

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}


void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, STRVertex* vertices, unsigned int verticeCount)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i];
        unsigned int in1 = indices[i + 1];
        unsigned int in2 = indices[i + 2];
        glm::vec3 v1 = vertices[in1].position - vertices[in0].position;
        glm::vec3 v2 = vertices[in2].position - vertices[in0].position;
        glm::vec3 normal = cross(v1, v2);
        normal = normalize(normal);

        vertices[in0].normal += normal;
        vertices[in1].normal += normal;
        vertices[in2].normal += normal;
    }

    for (size_t i = 0; i < verticeCount; i++)
    {
        vertices[i].normal = normalize(vertices[i].normal);

    }
}

// Start Mesh


void CreateMesh(GLuint* VAO, GLuint* VBO, GLuint* IBO, STRVertex* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices)
{
    //GLuint VAO = 0, VBO = 0, IBO = 0;

    glGenVertexArrays(1, VAO);
    glBindVertexArray(*VAO);

    glGenBuffers(1, IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numOfIndices, indices, GL_STATIC_DRAW);

    glGenBuffers(1, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(STRVertex), (void*)offsetof(STRVertex, position));
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(STRVertex), (void*)offsetof(STRVertex, couleur));
    glEnableVertexAttribArray(1);


    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(STRVertex), (void*)offsetof(STRVertex, normal));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void RenderMesh(GLuint* VAO, GLuint* IBO, GLuint indexCount)
{
    glBindVertexArray(*VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IBO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// End Mesh

// Start Camera

/* Start Camera */

// keyboard vars
bool keys[1024];

// Mouse vars

double lastX;
double lastY;
// diffrence between current position of mouse and last
double xChange;
double yChange;
// to avoid violently turns of camera
bool mouseFirstMove = true;

// Camera vars

vec3 position = vec3(0.0f, 0.0f, 0.0f);
vec3 front = vec3(0.0f, 0.0f, -1.0f);
vec3 r;
vec3 up;
vec3 worldUp = vec3(0.0f, 1.0f, 0.0f);

// neck turn
double yaw = 0.0f; // turn head left and right
double pitch = 0.0f; // turn head up and down

GLfloat mouvementSpeed = 5.0f; // for key
GLfloat turnSpeed = 0.5f; // for mouse

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

void keyboardControl() {

    GLfloat velocity = mouvementSpeed * deltaTime;

    if (keys[GLFW_KEY_W])
    {
        // position += front * mouvementSpeed using this way, moving speed depend on your UC
        position += front * velocity;
    }

    if (keys[GLFW_KEY_S])
    {
        position -= front * velocity;
    }

    if (keys[GLFW_KEY_A])
    {
        position -= r * velocity;
    }

    if (keys[GLFW_KEY_D])
    {
        position += r * velocity;
    }
}

void mouseControl() {

    xChange *= turnSpeed;
    yChange *= turnSpeed;

    yaw += xChange;
    pitch += yChange;

    if (pitch > 44.0f)
    {
        pitch = 44.0f;
    }

    if (pitch < -44.0f)
    {
        pitch = -44.0f;
    }

    //update();

}

// to change angles when moving
void updateCamera() {
    front.x = cos(radians(yaw)) * cos(radians(pitch));
    front.y = sin(radians(pitch));
    front.z = sin(radians(yaw)) * cos(radians(pitch));
    front = normalize(front);

    r = normalize(cross(front, worldUp));
    up = normalize(cross(r, front));
}

// key callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
            //printf("Pressed %d\n", key);
        }
        else if (action == GLFW_RELEASE)
        {
            keys[key] = false;
            //printf("Released %d\n", key);
        }
    }
}

// mouse callback
static void cursor_position(GLFWwindow* window, double xPos, double yPos)
{
    if (mouseFirstMove) {
        lastX = xPos;
        lastY = yPos;
        mouseFirstMove = false;
    }

    xChange = xPos - lastX;
    yChange = lastY - yPos;

    lastX = xPos;
    lastY = yPos;

    //printf("x: %.6f, y: %.6f\n", xChange, yChange);

}


/* End Camera */

// End Camera


void Resize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}




int main(int argc, char* argv[]) {
    if (!glfwInit()) {
        printf("Could not initialize glfw.\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window;
    window = glfwCreateWindow(500, 500, "OpenGL TP 3", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);




    if (glewInit() != GLEW_OK)
    {
        printf("GLEW initialisation failed!");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwSetFramebufferSizeCallback(window, Resize);

    // init keys array
    for (size_t i = 0; i < 1024; i++)
    {
        keys[i] = 0;
    }

    // handle key callback
    glfwSetKeyCallback(window, key_callback);

    // handle mouse callback
    glfwSetCursorPosCallback(window, cursor_position);

    // disable cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);




    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
        3, 4, 2,
        0, 4, 1,
        2, 4, 1,
        0, 4, 3
    };


    struct STRVertex vertices[] = {
        {vec3(1.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(-1.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(-1.0f, 0.0f, -1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(1.0f, 0.0f, -1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(0.0f, 1.0f, 0.0f), vec3(0.1f, 0.1f, 0.1f), vec3(0.0f, 0.0f, 0.0f)},
    };

    unsigned int indices2[] = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };


    struct STRVertex vertices2[] = {
        {vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(0.0f, -1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(1.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(0.0f, 1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f)}
    };

    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    struct STRVertex floorVertices[] = {
        {vec3(-10.0f, 0.0f, -10.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)},
        {vec3(10.0f, 0.0f, -10.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)},
        {vec3(-10.0f, 0.0f, 10.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)},
        {vec3(10.0f, 0.0f, 10.0f), vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)}
    };


    float curAngle1 = 0.0f;
    float curAngle2 = 0.0f;


    GLuint  VAO = 0, VBO = 0, IBO = 0, numOfVertices = 5, numOfIndices = 18,
        VAO2 = 0, VBO2 = 0, IBO2 = 0, numOfVertices2 = 4, numOfIndices2 = 12,
        VAO3 = 0, VBO3 = 0, IBO3 = 0, numOfVertices3 = 4, numOfIndices3 = 6;

    // Start Sun Shape
    unsigned int sunIndices[] = {
        0, 1, 2,
        2, 3, 0,
        3, 4, 2,
        0, 4, 1,
        2, 4, 1,
        0, 4, 3
    };
    struct STRVertex sunVertices[] = {
        {vec3(1.0f, 0.0f, 1.0f), vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(-1.0f, 0.0f, 1.0f), vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(-1.0f, 0.0f, -1.0f), vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(1.0f, 0.0f, -1.0f), vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)},
        {vec3(0.0f, 1.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)},
    };
    GLuint sunVAO = 0, sunVBO = 0, sunIBO = 0, numOfSunVertices = 5, numOfSunIndices = 18;
    calcAverageNormals(sunIndices, 18, sunVertices, 5);
    CreateMesh(&sunVAO, &VBO, &sunIBO, sunVertices, sunIndices, numOfSunVertices, numOfSunIndices);
    // End Sun Shape

    // First Shape
    calcAverageNormals(indices, 18, vertices, 5);
    CreateMesh(&VAO, &VBO, &IBO, vertices, indices, numOfVertices, numOfIndices);

    // Second Shape
    calcAverageNormals(indices2, 12, vertices2, 4);
    CreateMesh(&VAO2, &VBO2, &IBO2, vertices2, indices2, numOfVertices2, numOfIndices2);

    // Floor Shape
    CreateMesh(&VAO3, &VBO3, &IBO3, floorVertices, floorIndices, numOfVertices3, numOfIndices3);

    glClearColor(0.0f, 0.0f, 0.7f, 1.0f);


    CreateShaders();

    mat4 Projection = perspective(radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;

    shinyMaterial = Material(1.0f, 32);
    dullMaterial = Material(0.3f, 4);

    mainLight = DirectionalLight(0.0f, 0.0f, 0.0f
                                ,0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f);

    unsigned int pointLightCount = 0;
    pointLights[0] = PointLight(1.0f, 1.0f, 1.0f,
        10.5f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.3f, 0.2f, 0.1f);
    pointLightCount++;
    /*pointLights[1] = PointLight(0.0f, 1.0f, 0.0f,
        0.0f, 0.0f,
        -4.0f, 2.0f, 0.0f,
        0.3f, 0.1f, 0.1f);
    pointLightCount++;*/


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        curAngle1 += 0.0001f;
        if (curAngle1 >= 360)
            curAngle1 -= 360;

        curAngle2 += 0.001f;
        if (curAngle2 >= 360)
            curAngle2 -= 360;

        mouseControl();
        updateCamera();
        keyboardControl();



        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();

        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);

        mat4 View = lookAt(position, position + front, up);


        // Render Sun Shape
        mat4 Model = mat4();
        Model = rotate(0 * radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, &Model[0][0]);
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, &Projection[0][0]);
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, &View[0][0]);
        RenderMesh(&sunVAO, &sunIBO, numOfSunIndices);


        // Render First Shape
        mat4 ModelTmp = mat4(1.0f);
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, &Projection[0][0]);
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, &View[0][0]);
        glUniform3f(uniformEyePosition, position.x, position.y, position.z);


        ModelTmp = rotate(ModelTmp, curAngle1 * radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
        ModelTmp = translate(ModelTmp, vec3(10.0f, 0.0f, 0.0f));
        Model = ModelTmp * rotate(curAngle2 * radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, &Model[0][0]);
        dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);

        RenderMesh(&VAO, &IBO, numOfIndices);

        // Render Second Shape
        //Model = mat4(1.0f);
        //glUniformMatrix4fv(uniformModel, 1, GL_FALSE, &Model[0][0]);
        //glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, &Projection[0][0]);
        //glUniformMatrix4fv(uniformView, 1, GL_FALSE, &View[0][0]);

        //RenderMesh(&VAO2, &IBO2, numOfIndices2);

        //// Render Third Shape
        //Model = mat4();
        //ModelTmp = rotate(ModelTmp, curAngle1 * radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
        //Model = translate(vec3(0.0f, -2.0f, 0.0f));
        //glUniformMatrix4fv(uniformModel, 1, GL_FALSE, &Model[0][0]);
        //glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, &Projection[0][0]);
        //glUniformMatrix4fv(uniformView, 1, GL_FALSE, &View[0][0]);
        //shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);

        //RenderMesh(&VAO3, &IBO3, numOfIndices3);


        glUseProgram(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

