#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

out vec3 ourColor;

uniform float rotationAngle;

void main()
{
    // Простое 2D вращение
    float cosA = cos(rotationAngle);
    float sinA = sin(rotationAngle);
    
    // Матрица вращения 2x2
    vec2 rotatedPos;
    rotatedPos.x = aPos.x * cosA - aPos.y * sinA;
    rotatedPos.y = aPos.x * sinA + aPos.y * cosA;
    
    gl_Position = vec4(rotatedPos, 0.0, 1.0);
    ourColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

int main() {
    // Инициализация GLFW
    if (!glfwInit()) return -1;

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 600, "Вращающийся треугольник", NULL, NULL);
    if (!window) {
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
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Создание программы
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Вершины треугольника (2D с цветами)
    float vertices[] = {
        // Позиции   // Цвета (RGB)
        -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  // Красный
         0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  // Зеленый
         0.0f,  0.5f,  0.0f, 0.0f, 1.0f   // Синий
    };

    // Создание буферов
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Настройка атрибутов
    // Позиции
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Цвета
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Получение location для uniform-переменной
    GLint rotationLoc = glGetUniformLocation(shaderProgram, "rotationAngle");

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Передача угла вращения в шейдер
        float angle = (float)glfwGetTime(); // Время в секундах
        glUniform1f(rotationLoc, angle);

        // Отрисовка треугольника
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка
    glDeleteProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glfwTerminate();

    return 0;
}