#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// Класс для работы с пикселями
class PixelGrid {
private:
    std::vector<float> vertices;
    float pixelSize;
    int gridWidth, gridHeight;

public:
    PixelGrid(int width, int height, float size = 0.02f)
        : gridWidth(width), gridHeight(height), pixelSize(size) {
    }

    // Установить пиксель в позиции (x, y)
    void setPixel(int x, int y, float r = 1.0f, float g = 1.0f, float b = 1.0f) {
        // Нормализуем координаты от -1 до 1
        float normX = (2.0f * x / gridWidth) - 1.0f;
        float normY = 1.0f - (2.0f * y / gridHeight);

        // Создаем квадрат для пикселя
        addSquare(normX, normY, pixelSize, r, g, b);
    }

    // Добавить квадрат (пиксель)
    void addSquare(float centerX, float centerY, float size,
        float r, float g, float b) {
        float half = size / 2.0f;

        // Вершины квадрата (2 треугольника)
        float square[] = {
            // Треугольник 1
            centerX - half, centerY + half, r, g, b,
            centerX - half, centerY - half, r, g, b,
            centerX + half, centerY + half, r, g, b,

            // Треугольник 2
            centerX - half, centerY - half, r, g, b,
            centerX + half, centerY - half, r, g, b,
            centerX + half, centerY + half, r, g, b
        };

        // Добавляем к общему массиву вершин
        for (int i = 0; i < 6 * 5; i++) {
            vertices.push_back(square[i]);
        }
    }

    // Получить массив вершин
    const std::vector<float>& getVertices() const {
        return vertices;
    }

    // Очистить сетку
    void clear() {
        vertices.clear();
    }
};

// Алгоритм Брезенхема для прямой линии
void bresenhamLine(PixelGrid& grid, int x1, int y1, int x2, int y2,
    float r = 1.0f, float g = 1.0f, float b = 1.0f) {
    /*
    Алгоритм Брезенхема:
    1. Вычисляем dx = |x2 - x1| и dy = |y2 - y1|
    2. Определяем знак приращения (sx, sy)
    3. Основная идея: отслеживаем ошибку (разницу между идеальной линией и текущей позицией)
    4. На каждом шаге увеличиваем либо x, либо y в зависимости от ошибки
    */

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    // Определяем направление приращения
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        // Устанавливаем текущий пиксель
        grid.setPixel(x1, y1, r, g, b);

        // Если достигли конечной точки
        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;

        // Корректируем ошибку
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Алгоритм Брезенхема для окружности
void bresenhamCircle(PixelGrid& grid, int xc, int yc, int radius,
    float r = 0.0f, float g = 1.0f, float b = 0.0f) {
    /*
    Алгоритм Брезенхема для окружности:
    1. Начинаем от точки (0, R)
    2. Используем симметрию окружности (8 октантов)
    3. На каждом шаге выбираем пиксель, ближайший к идеальной окружности
    4. Используем только целочисленные операции
    */

    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;  // Начальное значение ошибки

    // Рисуем начальные точки (используя симметрию)
    auto drawCirclePoints = [&](int x, int y) {
        grid.setPixel(xc + x, yc + y, r, g, b);
        grid.setPixel(xc - x, yc + y, r, g, b);
        grid.setPixel(xc + x, yc - y, r, g, b);
        grid.setPixel(xc - x, yc - y, r, g, b);
        grid.setPixel(xc + y, yc + x, r, g, b);
        grid.setPixel(xc - y, yc + x, r, g, b);
        grid.setPixel(xc + y, yc - x, r, g, b);
        grid.setPixel(xc - y, yc - x, r, g, b);
        };

    drawCirclePoints(x, y);

    while (y >= x) {
        x++;

        // Обновляем ошибку
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else {
            d = d + 4 * x + 6;
        }

        drawCirclePoints(x, y);
    }
}

// Алгоритм Брезенхема для эллипса
void bresenhamEllipse(PixelGrid& grid, int xc, int yc, int rx, int ry,
    float r = 1.0f, float g = 0.0f, float b = 1.0f) {
    /*
    Алгоритм для эллипса:
    1. Рисуем первую область (где производная < 1)
    2. Рисуем вторую область (где производная > 1)
    3. Используем симметрию эллипса (4 квадранта)
    */

    float rx2 = rx * rx;
    float ry2 = ry * ry;
    float twoRx2 = 2 * rx2;
    float twoRy2 = 2 * ry2;

    // Регион 1
    int x = 0;
    int y = ry;

    // Начальное значение в регионе 1
    float p1 = ry2 - rx2 * ry + 0.25 * rx2;

    // Рисуем симметричные точки
    auto drawEllipsePoints = [&](int x, int y) {
        grid.setPixel(xc + x, yc + y, r, g, b);
        grid.setPixel(xc - x, yc + y, r, g, b);
        grid.setPixel(xc + x, yc - y, r, g, b);
        grid.setPixel(xc - x, yc - y, r, g, b);
        };

    // Регион 1
    while (twoRy2 * x < twoRx2 * y) {
        drawEllipsePoints(x, y);
        x++;

        if (p1 < 0) {
            p1 += twoRy2 * x + ry2;
        }
        else {
            y--;
            p1 += twoRy2 * x - twoRx2 * y + ry2;
        }
    }

    // Регион 2
    float p2 = ry2 * (x + 0.5) * (x + 0.5) +
        rx2 * (y - 1) * (y - 1) -
        rx2 * ry2;

    while (y >= 0) {
        drawEllipsePoints(x, y);
        y--;

        if (p2 > 0) {
            p2 += -twoRx2 * y + rx2;
        }
        else {
            x++;
            p2 += twoRy2 * x - twoRx2 * y + rx2;
        }
    }
}

// Шейдеры
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec3 aColor;
    
    out vec3 fragColor;
    
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        fragColor = aColor;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    out vec4 FragColor;
    
    void main() {
        FragColor = vec4(fragColor, 1.0);
    }
)";

// Компиляция шейдера
unsigned int compileShader(GLenum type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
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
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Размеры сетки пикселей
    const int GRID_WIDTH = 100;
    const int GRID_HEIGHT = 100;

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 800, "Алгоритм Брезенхема", NULL, NULL);
    if (!window) {
        std::cerr << "Ошибка создания окна" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Ошибка инициализации GLEW" << std::endl;
        return -1;
    }

    // Создание шейдерной программы
    unsigned int shaderProgram = createShaderProgram();

    // Создание сетки пикселей
    PixelGrid grid(GRID_WIDTH, GRID_HEIGHT, 0.018f);

    // Демонстрация разных алгоритмов
    int demoStep = 2;

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        // Обработка нажатий клавиш для смены демо
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            demoStep = (demoStep + 1) % 4;
            grid.clear();
        }

        // Очистка экрана
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Рисование разных демонстраций
        switch (demoStep) {
        case 0: // Линия под углом 45°
            bresenhamLine(grid, 30, 45, 90, 90, 1.0f, 0.0f, 0.0f);
            break;

        case 1: // Горизонтальная и вертикальная линии
            bresenhamLine(grid, 20, 50, 80, 50, 0.0f, 1.0f, 0.0f); // Горизонтальная
            bresenhamLine(grid, 50, 20, 50, 80, 0.0f, 1.0f, 0.0f); // Вертикальная
            break;

        case 2: // Круг
            bresenhamCircle(grid, 50, 50, 30, 0.0f, 0.0f, 1.0f);
            break;

        case 3: // Эллипс
            bresenhamEllipse(grid, 50, 50, 40, 20, 1.0f, 0.0f, 1.0f);
            break;
        }

        // Создание буферов OpenGL
        unsigned int VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, grid.getVertices().size() * sizeof(float),
            grid.getVertices().data(), GL_STATIC_DRAW);

        // Атрибуты вершин
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Рендеринг
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, grid.getVertices().size() / 5);

        // Очистка буферов
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        // Обмен буферов и обработка событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return 0;
}