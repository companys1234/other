#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Вершинный шейдер с матрицей вращения
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat3 rotationMatrix;  // Матрица вращения 3x3

void main()
{
    // Применяем матрицу вращения к позиции
    vec2 rotatedPos = vec2(rotationMatrix * vec3(aPos, 1.0));
    
    gl_Position = vec4(rotatedPos, 0.0, 1.0);
    ourColor = aColor;
}
)";

// Фрагментный шейдер
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

// Вершины квадрата (2D)
float vertices[] = {
    // Позиции (x, y)     // Цвета (r, g, b)
    -0.5f, -0.5f,         1.0f, 0.0f, 0.0f,  // Нижний левый - красный
     0.5f, -0.5f,         0.0f, 1.0f, 0.0f,  // Нижний правый - зеленый
     0.5f,  0.5f,         0.0f, 0.0f, 1.0f,  // Верхний правый - синий
    -0.5f,  0.5f,         1.0f, 1.0f, 0.0f   // Верхний левый - желтый
};

// Индексы для двух треугольников, образующих квадрат
unsigned int indices[] = {
    0, 1, 2,  // Первый треугольник
    2, 3, 0   // Второй треугольник
};

// Класс для создания матрицы вращения
class RotationMatrix {
public:
    // Создание матрицы вращения 2D
    static glm::mat3 create2DRotation(float angle) {
        float cosA = cos(angle);
        float sinA = sin(angle);

        // Матрица вращения 2D (3x3 для однородных координат)
        // [ cos  -sin  0 ]
        // [ sin   cos  0 ]
        // [  0     0   1 ]

        glm::mat3 rotation = glm::mat3(1.0f);
        rotation[0][0] = cosA;
        rotation[0][1] = -sinA;
        rotation[1][0] = sinA;
        rotation[1][1] = cosA;

        return rotation;
    }

    // Создание матрицы вращения вокруг произвольной точки
    static glm::mat3 createRotationAroundPoint(float angle, float centerX, float centerY) {
        // Композиция трех матриц:
        // 1. Перенос в начало координат
        // 2. Вращение
        // 3. Обратный перенос

        glm::mat3 translateToOrigin = glm::mat3(1.0f);
        translateToOrigin[2][0] = -centerX;
        translateToOrigin[2][1] = -centerY;

        glm::mat3 rotation = create2DRotation(angle);

        glm::mat3 translateBack = glm::mat3(1.0f);
        translateBack[2][0] = centerX;
        translateBack[2][1] = centerY;

        return translateBack * rotation * translateToOrigin;
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Rotating Square - Rotation Matrix", NULL, NULL);
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

    // Компиляция шейдеров
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Создание шейдерной программы
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Удаление шейдеров
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Создание VAO, VBO и EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Настройка буферов
    glBindVertexArray(VAO);

    // Вершины в VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Индексы в EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Атрибуты вершин
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Получение location uniform-переменной
    int rotationMatrixLoc = glGetUniformLocation(shaderProgram, "rotationMatrix");

    // Настройка режима вращения
    int rotationMode = 0;  // 0 - вокруг центра, 1 - вокруг угла, 2 - вокруг произвольной точки
    float rotationSpeed = 1.0f;  // Скорость вращения
    float rotationCenterX = 0.0f;  // Центр вращения X
    float rotationCenterY = 0.0f;  // Центр вращения Y

    std::cout << "=== Rotating Square Demo ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  SPACE - change rotation mode" << std::endl;
    std::cout << "  UP/DOWN - change rotation speed" << std::endl;
    std::cout << "  LEFT/RIGHT - move rotation center" << std::endl;
    std::cout << "  R - reset rotation center" << std::endl;
    std::cout << "  ESC - exit" << std::endl;

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        // Очистка экрана
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Использование шейдера
        glUseProgram(shaderProgram);

        // Расчет угла вращения
        float time = glfwGetTime();
        float angle = time * rotationSpeed;

        // Создание матрицы вращения в зависимости от режима
        glm::mat3 rotationMatrix;

        switch (rotationMode) {
        case 0:  // Вращение вокруг центра квадрата
            rotationMatrix = RotationMatrix::create2DRotation(angle);
            break;

        case 1:  // Вращение вокруг левого нижнего угла
            rotationMatrix = RotationMatrix::createRotationAroundPoint(angle, -0.5f, -0.5f);
            break;

        case 2:  // Вращение вокруг произвольной точки
            rotationMatrix = RotationMatrix::createRotationAroundPoint(angle, rotationCenterX, rotationCenterY);
            break;
        }

        // Передача матрицы вращения в шейдер
        glUniformMatrix3fv(rotationMatrixLoc, 1, GL_FALSE, glm::value_ptr(rotationMatrix));

        // Отрисовка квадрата
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Обработка ввода
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            rotationMode = (rotationMode + 1) % 3;
            std::cout << "Rotation mode: " << rotationMode << std::endl;
            glfwWaitEventsTimeout(0.3);  // Задержка для предотвращения многократного нажатия
        }

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            rotationSpeed += 0.1f;
            std::cout << "Rotation speed: " << rotationSpeed << std::endl;
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            rotationSpeed = std::max(0.1f, rotationSpeed - 0.1f);
            std::cout << "Rotation speed: " << rotationSpeed << std::endl;
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            rotationCenterX -= 0.05f;
            std::cout << "Rotation center: (" << rotationCenterX << ", " << rotationCenterY << ")" << std::endl;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            rotationCenterX += 0.05f;
            std::cout << "Rotation center: (" << rotationCenterX << ", " << rotationCenterY << ")" << std::endl;
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            rotationCenterX = 0.0f;
            rotationCenterY = 0.0f;
            rotationSpeed = 1.0f;
            std::cout << "Reset to defaults" << std::endl;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // Обновление окна
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}