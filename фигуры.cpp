#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// Шейдерные программы
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec3 aColor;
    
    out vec3 fragColor;
    
    void main()
    {
        gl_Position = vec4(aPos, 0.0, 1.0);
        fragColor = aColor;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    out vec4 FragColor;
    
    void main()
    {
        FragColor = vec4(fragColor, 1.0);
    }
)";

class Shape2D {
protected:
    unsigned int VAO, VBO;
    std::vector<float> vertices;
    int drawMode;

public:
    Shape2D() : VAO(0), VBO(0), drawMode(GL_TRIANGLES) {}

    virtual void create() = 0;

    void render() {
        glBindVertexArray(VAO);
        glDrawArrays(drawMode, 0, vertices.size() / 5);
        glBindVertexArray(0);
    }

    virtual ~Shape2D() {
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
    }
};

class Triangle : public Shape2D {
public:
    Triangle(float r = 1.0f, float g = 0.0f, float b = 0.0f) {
        vertices = {
            // Позиции      // Цвета
            0.0f,  0.5f,    r, g, b,  // Верхняя вершина
           -0.5f, -0.5f,    r, g, b,  // Левая нижняя
            0.5f, -0.5f,    r, g, b   // Правая нижняя
        };
        drawMode = GL_TRIANGLES;
    }

    void create() override {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
            vertices.data(), GL_STATIC_DRAW);

        // Позиции
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Цвета
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

class Rectangle : public Shape2D {
public:
    Rectangle(float r = 0.0f, float g = 1.0f, float b = 0.0f) {
        vertices = {
            // Позиции      // Цвета
           -0.5f,  0.5f,    r, g, b,  // Левый верхний
           -0.5f, -0.5f,    r, g, b,  // Левый нижний
            0.5f, -0.5f,    r, g, b,  // Правый нижний
            0.5f, -0.5f,    r, g, b,  // Правый нижний
            0.5f,  0.5f,    r, g, b,  // Правый верхний
           -0.5f,  0.5f,    r, g, b   // Левый верхний
        };
        drawMode = GL_TRIANGLES;
    }

    void create() override {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
            vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

class Circle : public Shape2D {
public:
    Circle(int segments = 50, float r = 0.0f, float g = 0.0f, float b = 1.0f) {
        float radius = 0.5f;
        float M_PI = 3.14f;
        for (int i = 0; i <= segments; i++) {
            float theta = 2.0f * M_PI * float(i) / float(segments);
            float x = radius * cosf(theta);
            float y = radius * sinf(theta);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(r);
            vertices.push_back(g);
            vertices.push_back(b);
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

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

class Line : public Shape2D {
public:
    Line(float r = 1.0f, float g = 1.0f, float b = 0.0f, float thickness = 2.0f) {
        vertices = {
            // Позиции      // Цвета
           -0.5f,  0.0f,    r, g, b,
            0.5f,  0.0f,    r, g, b
        };
        drawMode = GL_LINES;
    }

    void create() override {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
            vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

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
        std::cerr << "Не удалось инициализировать GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 600, "2D Фигуры OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Не удалось создать окно GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Не удалось инициализировать GLEW" << std::endl;
        return -1;
    }

    // Компиляция шейдеров
    unsigned int shaderProgram = createShaderProgram();

    // Создание фигур
    Triangle triangle(1.0f, 0.0f, 0.0f);
    Rectangle rectangle(0.0f, 1.0f, 0.0f);
    Circle circle(50, 0.0f, 0.0f, 1.0f);
    Line line(1.0f, 1.0f, 0.0f);

    triangle.create();
    rectangle.create();
    circle.create();
    line.create();

    // Основной цикл рендеринга
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Рисование фигур в разных позициях
        // Можно добавить матрицы трансформации для перемещения

        // Треугольник
        triangle.render();

        // Прямоугольник
        rectangle.render();

        // Круг
        circle.render();

        // Линия
        line.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}