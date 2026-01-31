#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// ============================================
// РЕШЕНИЕ 1: Использовать правильные заголовки GLM
// ============================================

// УБЕДИТЕСЬ, ЧТО У ВАС УСТАНОВЛЕН GLM:
// Linux: sudo apt-get install libglm-dev
// Windows: скачайте с https://github.com/g-truc/glm

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // ДЛЯ translate, rotate, perspective
#include <glm/gtc/type_ptr.hpp>          // ДЛЯ value_ptr

// ============================================
// РЕШЕНИЕ 2: Своя реализация, если GLM не установлен
// ============================================

class SimpleMath {
public:
    // Конвертация градусов в радианы
    static float toRadians(float degrees) {
        return degrees * 3.14159265358979323846f / 180.0f;
    }

    // Создание матрицы переноса 4x4
    static void createTranslationMatrix(float tx, float ty, float tz, float* matrix) {
        // Инициализация как единичная матрица
        for (int i = 0; i < 16; i++) matrix[i] = 0.0f;
        matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;

        // Установка компонентов переноса
        matrix[12] = tx;
        matrix[13] = ty;
        matrix[14] = tz;
    }

    // Создание матрицы вращения вокруг оси X
    static void createRotationXMatrix(float angle, float* matrix) {
        float cosA = cos(angle);
        float sinA = sin(angle);

        for (int i = 0; i < 16; i++) matrix[i] = 0.0f;
        matrix[0] = 1.0f;
        matrix[5] = cosA;
        matrix[6] = -sinA;
        matrix[9] = sinA;
        matrix[10] = cosA;
        matrix[15] = 1.0f;
    }

    // Создание матрицы вращения вокруг оси Y
    static void createRotationYMatrix(float angle, float* matrix) {
        float cosA = cos(angle);
        float sinA = sin(angle);

        for (int i = 0; i < 16; i++) matrix[i] = 0.0f;
        matrix[0] = cosA;
        matrix[2] = sinA;
        matrix[5] = 1.0f;
        matrix[8] = -sinA;
        matrix[10] = cosA;
        matrix[15] = 1.0f;
    }

    // Создание матрицы перспективной проекции
    static void createPerspectiveMatrix(float fov, float aspect, float near, float far, float* matrix) {
        float f = 1.0f / tan(fov / 2.0f);

        for (int i = 0; i < 16; i++) matrix[i] = 0.0f;

        matrix[0] = f / aspect;
        matrix[5] = f;
        matrix[10] = (far + near) / (near - far);
        matrix[11] = -1.0f;
        matrix[14] = (2.0f * far * near) / (near - far);
    }

    // Умножение матриц 4x4
    static void multiplyMatrices(const float* a, const float* b, float* result) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result[i * 4 + j] = 0.0f;
                for (int k = 0; k < 4; k++) {
                    result[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
                }
            }
        }
    }
};

// Вершинный шейдер
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = aNormal;  // Упрощенно, без учета вращения модели
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

// Фрагментный шейдер
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // Упрощенное освещение
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    vec3 ambient = vec3(0.1, 0.1, 0.1);
    vec3 result = (ambient + diffuse) * objectColor;
    
    FragColor = vec4(result, 1.0);
}
)";

// Класс для генерации сферы
class Sphere {
private:
    unsigned int VAO, VBO;
    int vertexCount;

public:
    Sphere(float radius = 1.0f, int sectors = 32, int stacks = 16) {
        generateSphere(radius, sectors, stacks);
    }

    ~Sphere() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void render() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }

private:
    void generateSphere(float radius, int sectors, int stacks) {
        std::vector<float> vertices;

        float sectorStep = 2 * 3.14159265358979323846f / sectors;
        float stackStep = 3.14159265358979323846f / stacks;

        // Генерация вершин
        for (int i = 0; i <= stacks; ++i) {
            float stackAngle = 3.14159265358979323846f / 2 - i * stackStep;
            float xy = radius * cosf(stackAngle);
            float z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectors; ++j) {
                float sectorAngle = j * sectorStep;

                float x = xy * cosf(sectorAngle);
                float y = xy * sinf(sectorAngle);

                // Для каждой вершины создаем два треугольника
                if (i < stacks && j < sectors) {
                    // Первый треугольник текущего квада
                    addVertex(x, y, z, vertices);
                    addVertex(xy * cosf(sectorAngle + sectorStep),
                        xy * sinf(sectorAngle + sectorStep), z, vertices);

                    float nextStackAngle = 3.14159265358979323846f / 2 - (i + 1) * stackStep;
                    float nextXY = radius * cosf(nextStackAngle);
                    float nextZ = radius * sinf(nextStackAngle);

                    addVertex(nextXY * cosf(sectorAngle),
                        nextXY * sinf(sectorAngle), nextZ, vertices);

                    // Второй треугольник
                    addVertex(nextXY * cosf(sectorAngle),
                        nextXY * sinf(sectorAngle), nextZ, vertices);

                    addVertex(xy * cosf(sectorAngle + sectorStep),
                        xy * sinf(sectorAngle + sectorStep), z, vertices);

                    addVertex(nextXY * cosf(sectorAngle + sectorStep),
                        nextXY * sinf(sectorAngle + sectorStep), nextZ, vertices);
                }
            }
        }

        vertexCount = vertices.size() / 6;  // 6 значений на вершину (позиция + нормаль)

        // Создание OpenGL объектов
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
            vertices.data(), GL_STATIC_DRAW);

        // Позиции (атрибут 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Нормали (атрибут 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
            (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void addVertex(float x, float y, float z, std::vector<float>& vertices) {
        // Позиция
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);

        // Нормаль (нормализованная позиция)
        float length = sqrt(x * x + y * y + z * z);
        vertices.push_back(x / length);
        vertices.push_back(y / length);
        vertices.push_back(z / length);
    }
};

// Класс для работы с шейдерами
class Shader {
private:
    unsigned int ID;

public:
    Shader(const char* vertexSource, const char* fragmentSource) {
        compileShader(vertexSource, fragmentSource);
    }

    void use() {
        glUseProgram(ID);
    }

    void setMat4(const char* name, const float* mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, mat);
    }

    void setVec3(const char* name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name), x, y, z);
    }

private:
    void compileShader(const char* vertexSource, const char* fragmentSource) {
        // Компиляция вершинного шейдера
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);

        // Компиляция фрагментного шейдера
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);

        // Создание шейдерной программы
        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
};

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Настройка GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Sphere without GLM", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);

    // Создание шейдера
    Shader shader(vertexShaderSource, fragmentShaderSource);

    // Создание сферы
    Sphere sphere(1.0f, 32, 16);

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        // Очистка буферов
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Активация шейдера
        shader.use();

        // ============================================
        // ВАРИАНТ 1: С использованием GLM (если установлен)
        // ============================================
#ifdef USE_GLM
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        // Вращение
        float time = glfwGetTime();
        model = glm::rotate(model, time * 0.5f, glm::vec3(0.5f, 1.0f, 0.0f));

        // Камера
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        // Проекция
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float aspect = (float)width / (float)height;
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        // Передача матриц
        shader.setMat4("model", glm::value_ptr(model));
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));

        // ============================================
        // ВАРИАНТ 2: Без GLM (используем SimpleMath)
        // ============================================
#else
        float time = glfwGetTime();

        // Матрица модели (вращение вокруг оси Y)
        float modelMatrix[16];
        SimpleMath::createRotationYMatrix(time * 0.5f, modelMatrix);

        // Матрица вида (камера отодвинута назад)
        float viewMatrix[16];
        SimpleMath::createTranslationMatrix(0.0f, 0.0f, -3.0f, viewMatrix);

        // Матрица проекции
        float projectionMatrix[16];
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float aspect = (float)width / (float)height;
        SimpleMath::createPerspectiveMatrix(SimpleMath::toRadians(45.0f),
            aspect, 0.1f, 100.0f,
            projectionMatrix);

        // Передача матриц в шейдер
        shader.setMat4("model", modelMatrix);
        shader.setMat4("view", viewMatrix);
        shader.setMat4("projection", projectionMatrix);
#endif

        // Параметры освещения
        shader.setVec3("lightPos", 2.0f, 2.0f, 2.0f);
        shader.setVec3("viewPos", 0.0f, 0.0f, 3.0f);
        shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        shader.setVec3("objectColor", 0.5f, 0.3f, 0.8f);

        // Отрисовка сферы
        sphere.render();

        // Обновление окна
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}