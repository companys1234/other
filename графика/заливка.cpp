lude <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class FloodFill {
private:
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<glm::vec3>> colors;
    int width, height;  // Остаются private
    GLuint textureID;
    std::vector<GLubyte> textureData;

public:
    FloodFill(int w, int h) : width(w), height(h) {
        grid.resize(height, std::vector<bool>(width, false));
        colors.resize(height, std::vector<glm::vec3>(width, glm::vec3(1.0f)));

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        textureData.resize(width * height * 3, 255);
        updateTexture();
    }

    ~FloodFill() {
        glDeleteTextures(1, &textureID);
    }

    // Геттеры для доступа к private полям
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // ... остальные методы без изменений
    void updateTexture() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 3;
                textureData[idx] = static_cast<GLubyte>(colors[y][x].r * 255);
                textureData[idx + 1] = static_cast<GLubyte>(colors[y][x].g * 255);
                textureData[idx + 2] = static_cast<GLubyte>(colors[y][x].b * 255);
            }
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, textureData.data());
    }

    void render() {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
        glEnd();
    }

    void floodFillRecursive(int x, int y, const glm::vec3& targetColor, const glm::vec3& newColor) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        if (colors[y][x] != targetColor) return;
        if (colors[y][x] == newColor) return;

        colors[y][x] = newColor;
        grid[y][x] = true;

        floodFillRecursive(x + 1, y, targetColor, newColor);
        floodFillRecursive(x - 1, y, targetColor, newColor);
        floodFillRecursive(x, y + 1, targetColor, newColor);
        floodFillRecursive(x, y - 1, targetColor, newColor);
    }

    void floodFillStack(int startX, int startY, const glm::vec3& targetColor, const glm::vec3& newColor) {
        if (startX < 0 || startX >= width || startY < 0 || startY >= height) return;
        if (colors[startY][startX] != targetColor) return;

        std::stack<std::pair<int, int>> pixelStack;
        pixelStack.push({ startX, startY });

        while (!pixelStack.empty()) {
            auto [x, y] = pixelStack.top();
            pixelStack.pop();

            if (colors[y][x] == newColor) continue;

            colors[y][x] = newColor;
            grid[y][x] = true;

            if (x + 1 < width && colors[y][x + 1] == targetColor)
                pixelStack.push({ x + 1, y });
            if (x - 1 >= 0 && colors[y][x - 1] == targetColor)
                pixelStack.push({ x - 1, y });
            if (y + 1 < height && colors[y + 1][x] == targetColor)
                pixelStack.push({ x, y + 1 });
            if (y - 1 >= 0 && colors[y - 1][x] == targetColor)
                pixelStack.push({ x, y - 1 });
        }
    }

    void floodFillQueue(int startX, int startY, const glm::vec3& targetColor, const glm::vec3& newColor) {
        if (startX < 0 || startX >= width || startY < 0 || startY >= height) return;
        if (colors[startY][startX] != targetColor) return;

        std::queue<std::pair<int, int>> pixelQueue;
        pixelQueue.push({ startX, startY });

        while (!pixelQueue.empty()) {
            auto [x, y] = pixelQueue.front();
            pixelQueue.pop();

            if (colors[y][x] != targetColor) continue;

            colors[y][x] = newColor;
            grid[y][x] = true;

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;

                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        if (colors[ny][nx] == targetColor) {
                            pixelQueue.push({ nx, ny });
                        }
                    }
                }
            }
        }
    }

    void floodFillScanline(int startX, int startY, const glm::vec3& targetColor, const glm::vec3& newColor) {
        if (startX < 0 || startX >= width || startY < 0 || startY >= height) return;
        if (colors[startY][startX] != targetColor) return;

        std::stack<std::pair<int, int>> stack;
        stack.push({ startX, startY });

        while (!stack.empty()) {
            auto [x, y] = stack.top();
            stack.pop();

            int left = x;
            while (left >= 0 && colors[y][left] == targetColor) {
                colors[y][left] = newColor;
                grid[y][left] = true;
                left--;
            }
            left++;

            int right = x;
            while (right < width && colors[y][right] == targetColor) {
                colors[y][right] = newColor;
                grid[y][right] = true;
                right++;
            }
            right--;

            for (int dy = -1; dy <= 1; dy += 2) {
                int ny = y + dy;
                if (ny >= 0 && ny < height) {
                    bool inSpan = false;
                    for (int nx = left; nx <= right; nx++) {
                        if (colors[ny][nx] == targetColor) {
                            if (!inSpan) {
                                stack.push({ nx, ny });
                                inSpan = true;
                            }
                        }
                        else {
                            inSpan = false;
                        }
                    }
                }
            }
        }
    }

    void createTestImage() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                colors[y][x] = glm::vec3(1.0f, 1.0f, 1.0f);
                grid[y][x] = false;
            }
        }

        for (int y = height / 4; y < 3 * height / 4; y++) {
            for (int x = width / 4; x < 3 * width / 4; x++) {
                colors[y][x] = glm::vec3(0.0f, 0.0f, 1.0f);
                grid[y][x] = true;
            }
        }

        int centerX = width / 2;
        int centerY = height / 2;
        int radius = std::min(width, height) / 4;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int dx = x - centerX;
                int dy = y - centerY;
                if (dx * dx + dy * dy <= radius * radius) {
                    colors[y][x] = glm::vec3(1.0f, 0.0f, 0.0f);
                    grid[y][x] = true;
                }
            }
        }

        updateTexture();
    }

    glm::vec3 getColor(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return colors[y][x];
        }
        return glm::vec3(0.0f);
    }

    void setColor(int x, int y, const glm::vec3& color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            colors[y][x] = color;
            grid[y][x] = (color != glm::vec3(1.0f));
        }
    }
};

// Глобальные переменные
FloodFill* floodFill = nullptr;
glm::vec3 currentColor = glm::vec3(0.0f, 1.0f, 0.0f);
int algorithmType = 0;

// ИСПРАВЛЕННЫЙ обработчик мыши
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);

        // ИСПРАВЛЕНИЕ: Используем геттеры вместо прямого доступа
        int texX = static_cast<int>((xpos / winWidth) * floodFill->getWidth());
        int texY = static_cast<int>((1.0 - ypos / winHeight) * floodFill->getHeight());

        glm::vec3 targetColor = floodFill->getColor(texX, texY);

        switch (algorithmType) {
        case 0:
            floodFill->floodFillRecursive(texX, texY, targetColor, currentColor);
            break;
        case 1:
            floodFill->floodFillStack(texX, texY, targetColor, currentColor);
            break;
        case 2:
            floodFill->floodFillQueue(texX, texY, targetColor, currentColor);
            break;
        case 3:
            floodFill->floodFillScanline(texX, texY, targetColor, currentColor);
            break;
        }

        floodFill->updateTexture();
    }
}

// Обработчик клавиатуры (без изменений)
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_R: currentColor = glm::vec3(1.0f, 0.0f, 0.0f); break;
        case GLFW_KEY_G: currentColor = glm::vec3(0.0f, 1.0f, 0.0f); break;
        case GLFW_KEY_B: currentColor = glm::vec3(0.0f, 0.0f, 1.0f); break;
        case GLFW_KEY_Y: currentColor = glm::vec3(1.0f, 1.0f, 0.0f); break;
        case GLFW_KEY_P: currentColor = glm::vec3(1.0f, 0.0f, 1.0f); break;
        case GLFW_KEY_C: currentColor = glm::vec3(0.0f, 1.0f, 1.0f); break;
        case GLFW_KEY_1: algorithmType = 0; std::cout << "Рекурсивный Flood Fill" << std::endl; break;
        case GLFW_KEY_2: algorithmType = 1; std::cout << "Flood Fill со стеком" << std::endl; break;
        case GLFW_KEY_3: algorithmType = 2; std::cout << "Flood Fill с очередью (BFS)" << std::endl; break;
        case GLFW_KEY_4: algorithmType = 3; std::cout << "Flood Fill со сканирующей строкой" << std::endl; break;
        case GLFW_KEY_SPACE: floodFill->createTestImage(); break;
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Flood Fill Algorithm - OpenGL", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    glEnable(GL_TEXTURE_2D);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    floodFill = new FloodFill(256, 256);
    floodFill->createTestImage();

    std::cout << "=== Flood Fill Algorithm Demo ===" << std::endl;
    std::cout << "Управление:" << std::endl;
    std::cout << "  ЛКМ - заливка области" << std::endl;
    std::cout << "  R/G/B/Y/P/C - выбор цвета" << std::endl;
    std::cout << "  1 - рекурсивный алгоритм" << std::endl;
    std::cout << "  2 - алгоритм со стеком" << std::endl;
    std::cout << "  3 - алгоритм с очередью (BFS)" << std::endl;
    std::cout << "  4 - алгоритм со сканирующей строкой" << std::endl;
    std::cout << "  ПРОБЕЛ - сброс изображения" << std::endl;
    std::cout << "  ESC - выход" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        floodFill->render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete floodFill;
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}