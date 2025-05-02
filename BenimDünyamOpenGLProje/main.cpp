#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>

// Pencere Boyutlarý
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Kamera Deðiþkenleri
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Zaman Deðiþkenleri
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Fare Kontrol Deðiþkenleri
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Çayýr Kontrol Deðiþkeni
bool hasPassedDoor = false;

// Kapý Tanýmlayýcýlarý
const float DOOR_MIN_X = -1.3f;
const float DOOR_MAX_X = 1.3f;
const float DOOR_Z_THRESHOLD = -10.0f;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

// Fonksiyon Bildirimleri
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
std::string readShaderFile(const char* filePath);
GLuint compileShader(GLenum shaderType, const std::string& source);
GLuint createShaderProgram();
std::vector<Vertex> GenerateNormalsFromIndexedMesh(const float* vertices, const unsigned int* indices, size_t vertexCount, size_t indexCount);
std::vector<glm::vec3> CalculateNormalsForIcosahedron(
    const std::vector<glm::vec3>& vertices,
    const std::vector<unsigned int>& indices);

int main() {
    // GLFW Baþlatma ve Yapýlandýrma
    if (!glfwInit()) {
        std::cerr << "GLFW baþlatýlamadý!" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Pencere Oluþturma
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Benim Dunyam", nullptr, nullptr);
    if (!window) {
        std::cerr << "Pencere oluþturulamadý!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW Baþlatma
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW baþlatýlamadý!" << std::endl;
        return -1;
    }

    // OpenGL Durum Ayarlarý
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Shader Programý Oluþturma
    GLuint shaderProgram = createShaderProgram();

    // Kapý Vertex Verileri
    float doorVertices[] = {
        -1.3f, 0.0f, 0.0f,
         1.3f, 0.0f, 0.0f,
         1.3f, 4.0f, 0.0f,
        -1.3f, 4.0f, 0.0f,
    };
    unsigned int doorIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    std::vector<Vertex> doorData = GenerateNormalsFromIndexedMesh(
        doorVertices,
        doorIndices,
        sizeof(doorVertices) / sizeof(float) / 3,
        sizeof(doorIndices) / sizeof(unsigned int)
    );

    unsigned int doorVAO, doorVBO;
    glGenVertexArrays(1, &doorVAO);
    glGenBuffers(1, &doorVBO);

    glBindVertexArray(doorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, doorVBO);
    glBufferData(GL_ARRAY_BUFFER, doorData.size() * sizeof(Vertex), doorData.data(), GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Çayýr Vertex Verileri
    float grassVertices[] = {
        //  x      y      z      nx    ny    nz
        -50.0f, 0.0f, -50.0f,   0.0f, 1.0f, 0.0f, // Sol alt
         50.0f, 0.0f, -50.0f,   0.0f, 1.0f, 0.0f, // Sað alt
         50.0f, 0.0f,  50.0f,   0.0f, 1.0f, 0.0f, // Sað üst
        -50.0f, 0.0f,  50.0f,   0.0f, 1.0f, 0.0f  // Sol üst
    };

    unsigned int grassIndices[] = {
        0, 1, 2, // Ýlk üçgen
        2, 3, 0  // Ýkinci üçgen
    };
    unsigned int grassVAO, grassVBO, grassEBO;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glGenBuffers(1, &grassEBO);
    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), grassVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grassEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(grassIndices), grassIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);              // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // normal
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Aðaç Boyu
    //Aðaç Gövdesi Vertex Verileri
// Aðaç Yüksekliði
    float treeHeight = 6.0f;
    float trunkHeight = treeHeight / 3.0f * 1.0f;
    float leavesStartHeight = trunkHeight;
    float leavesHeight = treeHeight / 3.0f * 2.0f;

    // Daha Kalýn ve Belirli Oranlý Aðaç Gövdesi Vertex Verileri
    float trunkVertices[] = {
        // Taban Daire Merkezi
         0.0f, 0.0f,  0.0f,
         // Taban Daire Kenar Noktalarý
          0.5f, 0.0f,  0.0f,
          0.433f, 0.0f,  0.25f,
          0.25f, 0.0f,  0.433f,
          0.0f, 0.0f,  0.5f,
         -0.25f, 0.0f,  0.433f,
         -0.433f, 0.0f,  0.25f,
         -0.5f, 0.0f,  0.0f,
         -0.433f, 0.0f, -0.25f,
         -0.25f, 0.0f, -0.433f,
          0.0f, 0.0f, -0.5f,
          0.25f, 0.0f, -0.433f,
          0.433f, 0.0f, -0.25f,
          // Üst Daire Merkezi (gövde yüksekliðine göre)
           0.0f, trunkHeight,  0.0f,
           // Üst Daire Kenar Noktalarý (gövde yüksekliðine göre)
            0.5f, trunkHeight,  0.0f,
            0.433f, trunkHeight,  0.25f,
            0.25f, trunkHeight,  0.433f,
            0.0f, trunkHeight,  0.5f,
           -0.25f, trunkHeight,  0.433f,
           -0.433f, trunkHeight,  0.25f,
           -0.5f, trunkHeight,  0.0f,
           -0.433f, trunkHeight, -0.25f,
           -0.25f, trunkHeight, -0.433f,
            0.0f, trunkHeight, -0.5f,
            0.25f, trunkHeight, -0.433f,
            0.433f, trunkHeight, -0.25f
    };

    unsigned int trunkIndices[] = {
        // Taban yüzeyi
         0,  1,  2,   0,  2,  3,   0,  3,  4,   0,  4,  5,
         0,  5,  6,   0,  6,  7,   0,  7,  8,   0,  8,  9,
         0,  9, 10,   0, 10, 11,   0, 11, 12,   0, 12,  1,
         // Üst yüzeyi
         13, 14, 15,  13, 15, 16,  13, 16, 17,  13, 17, 18,
         13, 18, 19,  13, 19, 20,  13, 20, 21,  13, 21, 22,
         13, 22, 23,  13, 23, 24,  13, 24, 25,  13, 25, 14,
         // Yan yüzeyler
          1, 14, 15,   1, 15,  2,
          2, 15, 16,   2, 16,  3,
          3, 16, 17,   3, 17,  4,
          4, 17, 18,   4, 18,  5,
          5, 18, 19,   5, 19,  6,
          6, 19, 20,   6, 20,  7,
          7, 20, 21,   7, 21,  8,
          8, 21, 22,   8, 22,  9,
          9, 22, 23,   9, 23, 10,
         10, 23, 24,  10, 24, 11,
         11, 24, 25,  11, 25, 12,
         12, 25, 14,  12, 14,  1
    };

    // Normalleri otomatik hesapla
    std::vector<Vertex> trunkVerticesWithNormals = GenerateNormalsFromIndexedMesh(
        trunkVertices,
        trunkIndices,
        sizeof(trunkVertices) / (3 * sizeof(float)),
        sizeof(trunkIndices) / sizeof(unsigned int)
    );

    // VAO, VBO oluþtur (EBO'ya gerek yok)
    unsigned int trunkVAO, trunkVBO;
    glGenVertexArrays(1, &trunkVAO);
    glGenBuffers(1, &trunkVBO);
    glBindVertexArray(trunkVAO);

    // Vertex verilerini yükle (artýk indisler VBO'da sýralý)
    glBindBuffer(GL_ARRAY_BUFFER, trunkVBO);
    glBufferData(GL_ARRAY_BUFFER, trunkVerticesWithNormals.size() * sizeof(Vertex), trunkVerticesWithNormals.data(), GL_STATIC_DRAW);

    // Vertex attribute ayarlarý
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Daha Yoðun ve Büyük Aðaç Yapraklarý Vertex Verileri (daha geniþ taban ve yukarý kaydýrýlmýþ tepe noktasý)
// Önce vertex ve index verilerini vektörlere aktar
    std::vector<float> leavesVerticesVector = {
        0.0f, leavesStartHeight + leavesHeight, 0.0f,  // Tepe noktasý
        0.0f, leavesStartHeight, 0.0f,               // Taban merkezi
        1.0f, leavesStartHeight, 0.0f,
        0.866f, leavesStartHeight, 0.5f,
        0.5f, leavesStartHeight, 0.866f,
        0.0f, leavesStartHeight, 1.0f,
        -0.5f, leavesStartHeight, 0.866f,
        -0.866f, leavesStartHeight, 0.5f,
        -1.0f, leavesStartHeight, 0.0f,
        -0.866f, leavesStartHeight, -0.5f,
        -0.5f, leavesStartHeight, -0.866f,
        0.0f, leavesStartHeight, -1.0f,
        0.5f, leavesStartHeight, -0.866f,
        0.866f, leavesStartHeight, -0.5f
    };

    std::vector<unsigned int> leavesIndicesVector = {
        // Taban yüzeyi
        1,2,3, 1,3,4, 1,4,5, 1,5,6,
        1,6,7, 1,7,8, 1,8,9, 1,9,10,
        1,10,11, 1,11,12, 1,12,13, 1,13,2,
        // Yan yüzeyler
        0,2,3, 0,3,4, 0,4,5, 0,5,6,
        0,6,7, 0,7,8, 0,8,9, 0,9,10,
        0,10,11, 0,11,12, 0,12,13, 0,13,2
    };

    // Normalleri hesapla
    std::vector<Vertex> leavesVerticesWithNormals = GenerateNormalsFromIndexedMesh(
        leavesVerticesVector.data(),
        leavesIndicesVector.data(),
        leavesVerticesVector.size() / 3,
        leavesIndicesVector.size()
    );

    for (auto& vertex : leavesVerticesWithNormals) {
        vertex.normal = -vertex.normal;
    }

    unsigned int leavesVAO, leavesVBO, leavesEBO;
    glGenVertexArrays(1, &leavesVAO);
    glGenBuffers(1, &leavesVBO);
    glGenBuffers(1, &leavesEBO);

    glBindVertexArray(leavesVAO);

    // Vertex verilerini yükle (pozisyon + normal)
    glBindBuffer(GL_ARRAY_BUFFER, leavesVBO);
    glBufferData(GL_ARRAY_BUFFER,
        leavesVerticesWithNormals.size() * sizeof(Vertex),
        leavesVerticesWithNormals.data(),
        GL_STATIC_DRAW);

    // Yeni indisleri oluþtur (0'dan baþlayarak sýralý)
    std::vector<unsigned int> newLeavesIndices(leavesVerticesWithNormals.size());
    for (unsigned int i = 0; i < newLeavesIndices.size(); i++) {
        newLeavesIndices[i] = i;
    }

    // Index buffer'ý yükle
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leavesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        newLeavesIndices.size() * sizeof(unsigned int),
        newLeavesIndices.data(),
        GL_STATIC_DRAW);

    // Vertex attribute pointer'larý ayarla
    // Pozisyon
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Yeni Aðaç Çeþidi Yapraklarý Vertex Verileri (Daha Küresel Görünüm)
    std::vector<glm::vec3> icosahedronVertices;
    std::vector<unsigned int> icosahedronIndices;

    float t = (1.0f + sqrt(5.0f)) / 2.0f;

    // Köþe Noktalarý
    icosahedronVertices.push_back(glm::normalize(glm::vec3(-1.0f, t, 0.0f)));
    icosahedronVertices.push_back(glm::normalize(glm::vec3(1.0f, t, 0.0f)));
    icosahedronVertices.push_back(glm::normalize(glm::vec3(-1.0f, -t, 0.0f)));
    icosahedronVertices.push_back(glm::normalize(glm::vec3(1.0f, -t, 0.0f)));

    icosahedronVertices.push_back(glm::normalize(glm::vec3(0.0f, -1.0f, t)));
    icosahedronVertices.push_back(glm::normalize(glm::vec3(0.0f, 1.0f, t)));
    icosahedronVertices.push_back(glm::normalize(glm::vec3(0.0f, -1.0f, -t)));
    icosahedronVertices.push_back(glm::normalize(glm::vec3(0.0f, 1.0f, -t)));

    icosahedronVertices.push_back(glm::normalize(glm::vec3(t, 0.0f, -1.0f)));
    icosahedronVertices.push_back(glm::normalize(glm::vec3(t, 0.0f, 1.0f)));
    icosahedronVertices.push_back(glm::normalize(glm::vec3(-t, 0.0f, -1.0f)));
    icosahedronVertices.push_back(glm::normalize(glm::vec3(-t, 0.0f, 1.0f)));

    // Yüzeyler (Üçgenler)
    icosahedronIndices.insert(icosahedronIndices.end(), { 0, 11, 5 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 0, 5, 1 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 0, 1, 7 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 0, 7, 10 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 0, 10, 11 });

    icosahedronIndices.insert(icosahedronIndices.end(), { 1, 5, 9 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 5, 11, 4 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 11, 10, 2 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 10, 7, 6 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 7, 1, 8 });

    icosahedronIndices.insert(icosahedronIndices.end(), { 3, 9, 4 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 3, 4, 2 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 3, 2, 6 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 3, 6, 8 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 3, 8, 9 });

    icosahedronIndices.insert(icosahedronIndices.end(), { 4, 9, 5 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 2, 4, 11 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 6, 2, 10 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 8, 6, 7 });
    icosahedronIndices.insert(icosahedronIndices.end(), { 9, 8, 1 });

    // Normalleri hesapla
    std::vector<glm::vec3> normals = CalculateNormalsForIcosahedron(icosahedronVertices, icosahedronIndices);

    // Vertex verilerini (pozisyon + normal) birleþtir
    std::vector<float> verticesWithNormals;
    for (size_t i = 0; i < icosahedronVertices.size(); ++i) {
        verticesWithNormals.push_back(icosahedronVertices[i].x);
        verticesWithNormals.push_back(icosahedronVertices[i].y);
        verticesWithNormals.push_back(icosahedronVertices[i].z);
        verticesWithNormals.push_back(normals[i].x);
        verticesWithNormals.push_back(normals[i].y);
        verticesWithNormals.push_back(normals[i].z);
    }

    unsigned int leaves2VAO, leaves2VBO, leaves2EBO;
    glGenVertexArrays(1, &leaves2VAO);
    glGenBuffers(1, &leaves2VBO);
    glGenBuffers(1, &leaves2EBO);

    glBindVertexArray(leaves2VAO);

    // Vertex buffer (pozisyon + normal)
    glBindBuffer(GL_ARRAY_BUFFER, leaves2VBO);
    glBufferData(GL_ARRAY_BUFFER,
        verticesWithNormals.size() * sizeof(float),
        verticesWithNormals.data(),
        GL_STATIC_DRAW);

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leaves2EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        icosahedronIndices.size() * sizeof(unsigned int),
        icosahedronIndices.data(),
        GL_STATIC_DRAW);

    // Vertex attribute pointer'larý
    // Pozisyon
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

	// Kulübe Vertex Verileri

    // Küp boyutlarý (kenar uzunluðu)
// Küp boyutlarý (kenar uzunluðu)
    float cubeSide = 4.0f;

    // Üçgen prizma boyutlarý
    float prismBaseWidth = cubeSide;
    float prismHeight = 2.0f;
    float prismLength = cubeSide;

    // Kulübe vertex verileri (pozisyon + normal)
    std::vector<Vertex> cabinVertices;

    // Küp Vertexleri ve Normalleri
    // Ön yüz
    cabinVertices.push_back({ {0.0f, 0.0f, cubeSide}, {0.0f, 0.0f, 1.0f} });
    cabinVertices.push_back({ {cubeSide, 0.0f, cubeSide}, {0.0f, 0.0f, 1.0f} });
    cabinVertices.push_back({ {cubeSide, cubeSide, cubeSide}, {0.0f, 0.0f, 1.0f} });
    cabinVertices.push_back({ {0.0f, cubeSide, cubeSide}, {0.0f, 0.0f, 1.0f} });

    // Arka yüz
    cabinVertices.push_back({ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} });
    cabinVertices.push_back({ {cubeSide, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} });
    cabinVertices.push_back({ {cubeSide, cubeSide, 0.0f}, {0.0f, 0.0f, -1.0f} });
    cabinVertices.push_back({ {0.0f, cubeSide, 0.0f}, {0.0f, 0.0f, -1.0f} });

    // Sað yüz
    cabinVertices.push_back({ {cubeSide, 0.0f, cubeSide}, {1.0f, 0.0f, 0.0f} });
    cabinVertices.push_back({ {cubeSide, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
    cabinVertices.push_back({ {cubeSide, cubeSide, 0.0f}, {1.0f, 0.0f, 0.0f} });
    cabinVertices.push_back({ {cubeSide, cubeSide, cubeSide}, {1.0f, 0.0f, 0.0f} });

    // Sol yüz
    cabinVertices.push_back({ {0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} });
    cabinVertices.push_back({ {0.0f, 0.0f, cubeSide}, {-1.0f, 0.0f, 0.0f} });
    cabinVertices.push_back({ {0.0f, cubeSide, cubeSide}, {-1.0f, 0.0f, 0.0f} });
    cabinVertices.push_back({ {0.0f, cubeSide, 0.0f}, {-1.0f, 0.0f, 0.0f} });

    // Üst yüz
    cabinVertices.push_back({ {0.0f, cubeSide, cubeSide}, {0.0f, 1.0f, 0.0f} });
    cabinVertices.push_back({ {cubeSide, cubeSide, cubeSide}, {0.0f, 1.0f, 0.0f} });
    cabinVertices.push_back({ {cubeSide, cubeSide, 0.0f}, {0.0f, 1.0f, 0.0f} });
    cabinVertices.push_back({ {0.0f, cubeSide, 0.0f}, {0.0f, 1.0f, 0.0f} });

    // Alt yüz (Artýk Y=0'da)
    cabinVertices.push_back({ {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} });
    cabinVertices.push_back({ {cubeSide, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} });
    cabinVertices.push_back({ {cubeSide, 0.0f, cubeSide}, {0.0f, -1.0f, 0.0f} });
    cabinVertices.push_back({ {0.0f, 0.0f, cubeSide}, {0.0f, -1.0f, 0.0f} });

    // Üçgen Prizma Vertexleri ve Normalleri (Çatý) - Yükseklik ayarlandý
    cabinVertices.push_back({ {0.0f, cubeSide, cubeSide}, {0.0f, 1.0f, 1.0f} }); // Köþe 1
    cabinVertices.push_back({ {cubeSide, cubeSide, cubeSide}, {0.0f, 1.0f, 1.0f} }); // Köþe 2
    cabinVertices.push_back({ {cubeSide / 2.0f, cubeSide + prismHeight, cubeSide}, {0.0f, 1.0f, 1.0f} }); // Tepe

    cabinVertices.push_back({ {0.0f, cubeSide, 0.0f}, {0.0f, 1.0f, -1.0f} }); // Köþe 3
    cabinVertices.push_back({ {cubeSide, cubeSide, 0.0f}, {0.0f, 1.0f, -1.0f} }); // Köþe 4
    cabinVertices.push_back({ {cubeSide / 2.0f, cubeSide + prismHeight, 0.0f}, {0.0f, 1.0f, -1.0f} }); // Tepe

    // Sol eðimli yüz
    glm::vec3 normalSol = glm::normalize(glm::cross(glm::vec3(cubeSide / 2.0f - 0.0f, prismHeight, -cubeSide), glm::vec3(0.0f, 0.0f, -cubeSide)));
    cabinVertices.push_back({ {0.0f, cubeSide, cubeSide}, normalSol });
    cabinVertices.push_back({ {cubeSide / 2.0f, cubeSide + prismHeight, cubeSide}, normalSol });
    cabinVertices.push_back({ {cubeSide / 2.0f, cubeSide + prismHeight, 0.0f}, normalSol });
    cabinVertices.push_back({ {0.0f, cubeSide, 0.0f}, normalSol });

    // Sað eðimli yüz
    glm::vec3 normalSag = glm::normalize(glm::cross(glm::vec3(cubeSide / 2.0f - cubeSide, prismHeight, cubeSide), glm::vec3(0.0f, 0.0f, -cubeSide)));
    cabinVertices.push_back({ {cubeSide, cubeSide, cubeSide}, normalSag });
    cabinVertices.push_back({ {cubeSide / 2.0f, cubeSide + prismHeight, cubeSide}, normalSag });
    cabinVertices.push_back({ {cubeSide / 2.0f, cubeSide + prismHeight, 0.0f}, normalSag });
    cabinVertices.push_back({ {cubeSide, cubeSide, 0.0f}, normalSag });

    // Taban dikdörtgen (isteðe baðlý, alttaki küp yüzeyiyle çakýþýk)
    // Ýhtiyaç duyulursa buraya eklenebilir.

    std::vector<unsigned int> cabinIndices = {
        // Küp indisleri
        0, 1, 2, 2, 3, 0,       // Ön yüz
        4, 5, 6, 6, 7, 4,       // Arka yüz
        8, 9, 10, 10, 11, 8,    // Sað yüz
        12, 13, 14, 14, 15, 12, // Sol yüz
        16, 17, 18, 18, 19, 16, // Üst yüz
        20, 21, 22, 22, 23, 20, // Alt yüz

        // Üçgen prizma indisleri
        24, 25, 26,             // Ön üçgen
        27, 28, 29,             // Arka üçgen
        30, 31, 32, 32, 33, 30, // Sol eðimli yüz
        34, 35, 36, 36, 37, 34  // Sað eðimli yüz
    };

    unsigned int cabinVAO, cabinVBO, cabinEBO;
    glGenVertexArrays(1, &cabinVAO);
    glGenBuffers(1, &cabinVBO);
    glGenBuffers(1, &cabinEBO);

    glBindVertexArray(cabinVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cabinVBO);
    glBufferData(GL_ARRAY_BUFFER, cabinVertices.size() * sizeof(Vertex), cabinVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cabinEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cabinIndices.size() * sizeof(unsigned int), cabinIndices.data(), GL_STATIC_DRAW);

    // Pozisyon attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Aðaç Konumlarý
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distribX(-45.0f, 45.0f); // Çayýrýn X sýnýrlarýna yakýn aralýk
    std::uniform_real_distribution<> distribZ(-45.0f, 45.0f); // Çayýrýn Z sýnýrlarýna yakýn aralýk

    // Rastgele Aðaç Konumlarý
    std::vector<glm::vec3> treePositions;
    int numberOfTrees = 20; // Ýstediðiniz aðaç sayýsý
    float minDistance = 5.0f;
    for (int i = 0; i < numberOfTrees; ++i) {
        glm::vec3 newPos;
        bool positionValid;
        int attempts = 0;
        do {
            positionValid = true;
            newPos = { distribX(gen), 0.0f, distribZ(gen) };
            for (const auto& pos : treePositions) {
                if (glm::distance(newPos, pos) < minDistance) {
                    positionValid = false;
                    break;
                }
            }
            attempts++;
        } while (!positionValid && attempts < 100); // 100 denemeden sonra vazgeç

        if (positionValid) {
            treePositions.push_back(newPos);
        }
    }

    std::vector<glm::vec3> treePositions2;
    int numberOfTrees2 = 30; // Ýkinci çeþit aðaç sayýsý (farklý olabilir)
    for (int i = 0; i < numberOfTrees2; ++i) {
        glm::vec3 newPos;
        bool positionValid;
        int attempts = 0;
        do {
            positionValid = true;
            newPos = { distribX(gen), 0.0f, distribZ(gen) };
            for (const auto& pos : treePositions2) {
                if (glm::distance(newPos, pos) < minDistance) {
                    positionValid = false;
                    break;
                }
            }
            attempts++;
        } while (!positionValid && attempts < 100); // 100 denemeden sonra vazgeç

        if (positionValid) {
            treePositions2.push_back(newPos);
        }
    }

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLuint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    GLuint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    // Ana Döngü
    while (!glfwWindowShouldClose(window)) {
        // Zaman Hesaplamasý
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Giriþ Ýþleme
        processInput(window);

        // Kamera pozisyonunu sabitle (örneðin karakterin boyunu simüle etmek için)
        cameraPos.y = 1.6f;

        // ESC Tuþu Kontrolü (processInput içine de alýnabilir)
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // Arka Planý Temizle
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Shader Programýný Kullan
        glUseProgram(shaderProgram);

        // Iþýk özellikleri
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        glm::vec3 lightPos(5.0f, 10.0f, 5.0f);
        glm::vec3 globalLightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.5f));

        // View ve Projection Matrislerini Oluþtur
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);

        // Ortak shader uniform'larýný ayarla
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(shaderProgram, "lightDir"),
            globalLightDir.x, globalLightDir.y, globalLightDir.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"),
            lightColor.r, lightColor.g, lightColor.b);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"),
            lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"),
            cameraPos.x, cameraPos.y, cameraPos.z);

        // Kapýyý Çiz
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.5f, 0.25f, 0.0f);
        glm::mat4 doorModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, DOOR_Z_THRESHOLD));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(doorModel));
        glBindVertexArray(doorVAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(doorData.size()));

        // Kapýdan geçiþ kontrolü
        if (!hasPassedDoor &&
            cameraPos.z < DOOR_Z_THRESHOLD &&
            cameraPos.x > DOOR_MIN_X &&
            cameraPos.x < DOOR_MAX_X) {
            hasPassedDoor = true;
            glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Gökyüzü rengi
        }

        // Çayýr sahnesi (kapýdan geçildiyse)
        if (hasPassedDoor) {
            
            // Çim zemini çiz
            glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.2f, 0.6f, 0.2f);
            glm::mat4 grassModel = glm::mat4(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(grassModel));
            glBindVertexArray(grassVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // 1. tip aðaçlarý çiz (çam aðaçlarý)
            for (const glm::vec3& pos : treePositions) {
                // Gövde
                glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.6f, 0.3f, 0.0f);
                glBindVertexArray(trunkVAO);
                glDrawArrays(GL_TRIANGLES, 0, trunkVerticesWithNormals.size());

                // Yapraklar
                glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.1f, 0.4f, 0.2f);
                glBindVertexArray(leavesVAO);
                glDrawElements(GL_TRIANGLES, newLeavesIndices.size(), GL_UNSIGNED_INT, 0);
            }

            // 2. tip aðaçlarý çiz
            for (const glm::vec3& pos : treePositions2) {
                // Gövde
                glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.6f, 0.3f, 0.0f);
                glBindVertexArray(trunkVAO);
                glDrawArrays(GL_TRIANGLES, 0, trunkVerticesWithNormals.size());

                // Yapraklar (ikosahedron)
                glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.9f, 0.5f, 0.9f);
                float icosahedronRadius = leavesHeight / 2.0f;
                glm::mat4 leavesModel = glm::translate(glm::mat4(1.0f),
                    glm::vec3(pos.x, leavesStartHeight + icosahedronRadius - 0.3f, pos.z));
                leavesModel = glm::scale(leavesModel, glm::vec3(icosahedronRadius));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(leavesModel));
                glBindVertexArray(leaves2VAO);
                glDrawElements(GL_TRIANGLES, icosahedronIndices.size(), GL_UNSIGNED_INT, 0);
            }

            // Kulübeyi çiz
            glm::mat4 cabinModel = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 0.0f, -25.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cabinModel));
            glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.8f, 0.5f, 0.2f);
            glBindVertexArray(cabinVAO);
            glDrawElements(GL_TRIANGLES, cabinIndices.size(), GL_UNSIGNED_INT, 0);
        }

        // Tamponlarý Takas Et ve Olaylarý Yokla
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Kaynaklarý Temizle
    glDeleteVertexArrays(1, &leaves2VAO);
    glDeleteBuffers(1, &leaves2VBO);
    glDeleteBuffers(1, &leaves2EBO);
    glDeleteVertexArrays(1, &trunkVAO);
    glDeleteBuffers(1, &trunkVBO);
    glDeleteVertexArrays(1, &leavesVAO);
    glDeleteBuffers(1, &leavesVBO);
    glDeleteBuffers(1, &leavesEBO);
    glDeleteVertexArrays(1, &doorVAO);
    glDeleteBuffers(1, &doorVBO);
    glDeleteVertexArrays(1, &grassVAO);
    glDeleteBuffers(1, &grassVBO);
    glDeleteBuffers(1, &grassEBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// Çerçeve Boyutu Geri Çaðýrým Fonksiyonu
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Giriþ Ýþleme Fonksiyonu
void processInput(GLFWwindow* window) {
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// Fare Geri Çaðýrým Fonksiyonu
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// Shader Dosyasýný Okuma Fonksiyonu
std::string readShaderFile(const char* filePath) {
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        shaderFile.open(filePath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        return shaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "HATA: " << filePath << " dosyasi acilamadi!\n";
        std::cerr << "Sebep: " << e.what() << std::endl;
        return "";
    }
}

// Shader Derleme Fonksiyonu
GLuint compileShader(GLenum shaderType, const std::string& source) {
    GLuint shader = glCreateShader(shaderType);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader derleme hatasi:\n" << infoLog << std::endl;
    }
    return shader;
}

// Shader Programý Oluþturma Fonksiyonu
GLuint createShaderProgram() {
    std::string vertexShaderSource = readShaderFile("shader.vert");
    std::string fragmentShaderSource = readShaderFile("shader.frag");

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        std::cerr << "Shader dosyalari okunamadi!" << std::endl;
        return 0; // Hata durumunda 0 dön
    }

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Program link hatasi:\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}


std::vector<Vertex> GenerateNormalsFromIndexedMesh(const float* vertices, const unsigned int* indices, size_t vertexCount, size_t indexCount) {
    std::vector<Vertex> result;

    for (size_t i = 0; i < indexCount; i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        glm::vec3 p0(vertices[i0 * 3], vertices[i0 * 3 + 1], vertices[i0 * 3 + 2]);
        glm::vec3 p1(vertices[i1 * 3], vertices[i1 * 3 + 1], vertices[i1 * 3 + 2]);
        glm::vec3 p2(vertices[i2 * 3], vertices[i2 * 3 + 1], vertices[i2 * 3 + 2]);

        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        result.push_back({ p0, normal });
        result.push_back({ p1, normal });
        result.push_back({ p2, normal });
    }

    return result;
}


std::vector<glm::vec3> CalculateNormalsForIcosahedron(
    const std::vector<glm::vec3>& vertices,
    const std::vector<unsigned int>& indices)
{
    std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        glm::vec3 v0 = vertices[i0];
        glm::vec3 v1 = vertices[i1];
        glm::vec3 v2 = vertices[i2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        normals[i0] += normal;
        normals[i1] += normal;
        normals[i2] += normal;
    }

    // Normalleri normalize et
    for (auto& n : normals) {
        n = glm::normalize(n);
    }

    return normals;
}