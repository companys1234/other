#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <glm/common.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
class Shape2D {
protected:
    unsigned int VAO, VBO;
    std::vector<float> vertices;
    unsigned int drawMode;
    float posX, posY;  // Позиция объекта

public:
    Shape2D() : posX(0.0f), posY(0.0f) {}
    virtual ~Shape2D() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    virtual void create() = 0;

    void render() {
        glBindVertexArray(VAO);
        glDrawArrays(drawMode, 0, vertices.size() / 5);
        glBindVertexArray(0);
    }

    void setPosition(float x, float y) {
        posX = x;
        posY = y;
    }

    float getX() const { return posX; }
    float getY() const { return posY; }
};

class Circle : public Shape2D {
private:
    float radius;

public:
    Circle(float r = 0.5f, int segments = 50,
        float red = 0.0f, float green = 0.0f, float blue = 1.0f)
        : radius(r) {

        const float M_PI = 3.14159265358979323846f;

        // Центр круга (вершина 0)
        vertices.push_back(0.0f);  // x
        vertices.push_back(0.0f);  // y
        vertices.push_back(red);   // r
        vertices.push_back(green); // g
        vertices.push_back(blue);  // b

        // Остальные вершины по окружности
        for (int i = 0; i <= segments; i++) {
            float theta = 2.0f * M_PI * float(i) / float(segments);
            float x = radius * cosf(theta);
            float y = radius * sinf(theta);

            vertices.push_back(x);  // x
            vertices.push_back(y);  // y
            vertices.push_back(red);    // r
            vertices.push_back(green);  // g
            vertices.push_back(blue);   // b
        }
        drawMode = GL_TRIANGLE_FAN;
    }

    void create() override {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
            vertices.data(), GL_STATIC_DRAW);

        // Атрибут позиции (0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
            5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Атрибут цвета (1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    float getRadius() const { return radius; }
};

// Функция для расчета высоты прыжков по арифметической прогрессии
std::vector<float> calculateBounceHeights(float initialHeight, float bounceFactor, int bounces) {
    std::vector<float> heights;
    heights.push_back(initialHeight);

    float currentHeight = initialHeight;
    for (int i = 1; i < bounces; i++) {
        currentHeight *= bounceFactor;  // Уменьшаем высоту на коэффициент
        heights.push_back(currentHeight);
    }

    return heights;
}

// Вершинный шейдер
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;
uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(aPos, 0.0, 1.0);
    ourColor = aColor;
}
)";

// Фрагментный шейдер
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

// Компиляция шейдера
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Ошибка компиляции шейдера: " << infoLog << std::endl;
    }

    return shader;
}

// Создание шейдерной программы
unsigned int createShaderProgram() {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Ошибка линковки шейдерной программы: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Ошибка инициализации GLFW" << std::endl;
        return -1;
    }

    // Настройка GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 600, "Мячик с арифметической прогрессией", NULL, NULL);
    if (!window) {
        std::cerr << "Ошибка создания окна GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Ошибка инициализации GLEW" << std::endl;
        return -1;
    }

    // Ввод параметров арифметической прогрессии
    float initialHeight, bounceFactor;
    int numBounces;

    std::cout << "=== visualisation ===" << std::endl;
    std::cout << "input start height (0.1 - 1.0): ";
    std::cin >> initialHeight;

    std::cout << "coeficent ymenchenia (0.1 - 0.9): ";
    std::cin >> bounceFactor;

    std::cout << "amount jumps: ";
    std::cin >> numBounces;

    // Ограничиваем значения
    initialHeight = std::max(0.1f, std::min(1.0f, initialHeight));
    bounceFactor = std::max(0.1f, std::min(0.9f, bounceFactor));
    numBounces = std::max(1, std::min(20, numBounces));

    // Расчет высот прыжков
    std::vector<float> bounceHeights = calculateBounceHeights(initialHeight, bounceFactor, numBounces);

    std::cout << "\nВысоты прыжков:" << std::endl;
    for (size_t i = 0; i < bounceHeights.size(); i++) {
        std::cout << "Прыжок " << i + 1 << ": " << bounceHeights[i] << std::endl;
    }

    // Создание шейдерной программы
    unsigned int shaderProgram = createShaderProgram();

    // Создание мячика
    Circle ball(0.1f, 50, 1.0f, 0.0f, 0.0f);  // Красный мячик
    ball.create();

    // Настройка OpenGL
    glUseProgram(shaderProgram);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // Получение location uniform-переменной transform
    int transformLoc = glGetUniformLocation(shaderProgram, "transform");

    // Переменные для анимации
    float currentTime = 0.0f;
    int currentBounce = 0;
    bool goingUp = true;
    float ballX = 0.0f;
    float ballY = 0.0f;
    float velocity = 0.0f;
    const float gravity = -2.0f;

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Расчет времени
        float time = glfwGetTime();
        float deltaTime = 0.016f;  // Примерно 60 FPS

        // Физика мячика (упрощенная)
        if (goingUp) {
            velocity = sqrtf(2.0f * bounceHeights[currentBounce] * -gravity);
            ballY += velocity * deltaTime;

            if (ballY >= bounceHeights[currentBounce]) {
                ballY = bounceHeights[currentBounce];
                goingUp = false;
            }
        }
        else {
            velocity += gravity * deltaTime;
            ballY += velocity * deltaTime;

            if (ballY <= 0.0f) {
                ballY = 0.0f;
                currentBounce = (currentBounce + 1) % bounceHeights.size();
                goingUp = true;

                // Изменение цвета мячика в зависимости от высоты
                float colorIntensity = bounceHeights[currentBounce] / initialHeight;
                glUseProgram(0);  // Временно отключаем шейдер
                // Здесь можно изменить цвет мячика
                glUseProgram(shaderProgram);
            }
        }

        // Создание матрицы трансформации для мячика
        float scale = 0.5f;  // Масштабирование для OpenGL координат
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(ballX, ballY * scale, 0.0f));

        // Передача матрицы трансформации в шейдер
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

        // Отрисовка мячика
        ball.render();

        // Отрисовка пола
        glUseProgram(0);  // Используем фиксированный конвейер для простоты
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, -0.9f);
        glVertex2f(1.0f, -0.9f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(-1.0f, -1.0f);
        glEnd();
        glUseProgram(shaderProgram);

        // Отрисовка графиков высот (опционально)
        glUseProgram(0);
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINE_STRIP);
        for (size_t i = 0; i < bounceHeights.size(); i++) {
            float x = -0.8f + (i * 1.6f / (bounceHeights.size() - 1));
            float y = -0.8f + (bounceHeights[i] * 0.8f);
            glVertex2f(x, y);
        }
        glEnd();
        glUseProgram(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // Небольшая задержка для контроля скорости анимации
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Очистка
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}