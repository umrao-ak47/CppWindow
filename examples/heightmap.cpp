#include <cppwindow/cppwindow.hpp>

#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace cwin;

/* ===================== Minimal Math ===================== */

struct Vec3
{
    float x, y, z;
};
struct Mat4
{
    float m[16];
};  // column-major

static Mat4 mat4Identity()
{
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.f;
    return r;
}

static Mat4 mat4Multiply(const Mat4& a, const Mat4& b)
{
    Mat4 r{};
    for (int c = 0; c < 4; ++c) {
        for (int r0 = 0; r0 < 4; ++r0) {
            for (int k = 0; k < 4; ++k) {
                r.m[c * 4 + r0] += a.m[k * 4 + r0] * b.m[c * 4 + k];
            }
        }
    }
    return r;
}

static Mat4 mat4Perspective(float fov, float aspect, float zn, float zf)
{
    float f = 1.f / std::tan(fov * 0.5f);
    Mat4 r{};
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zf + zn) / (zn - zf);
    r.m[11] = -1.f;
    r.m[14] = (2.f * zf * zn) / (zn - zf);
    return r;
}

static Vec3 normalize(Vec3 v)
{
    float l = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return { v.x / l, v.y / l, v.z / l };
}

static Vec3 cross(Vec3 a, Vec3 b)
{
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

static Mat4 mat4LookAt(Vec3 eye, Vec3 center, Vec3 up)
{
    Vec3 f = normalize({ center.x - eye.x, center.y - eye.y, center.z - eye.z });

    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);

    Mat4 r = mat4Identity();

    r.m[0] = s.x;
    r.m[4] = s.y;
    r.m[8] = s.z;
    r.m[1] = u.x;
    r.m[5] = u.y;
    r.m[9] = u.z;
    r.m[2] = -f.x;
    r.m[6] = -f.y;
    r.m[10] = -f.z;

    r.m[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    r.m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    r.m[14] = (f.x * eye.x + f.y * eye.y + f.z * eye.z);

    return r;
}

/* ===================== Grid ===================== */

struct Vertex
{
    float x, y, z;
};

static std::vector<Vertex> createGrid(int size, float scale)
{
    std::vector<Vertex> v(size * size);
    int i = 0;

    for (int z = 0; z < size; ++z)
        for (int x = 0; x < size; ++x) {
            float fx = float(x) / (size - 1);
            float fz = float(z) / (size - 1);

            v[i++] = { (fx - 0.5f) * scale, 0.f, (fz - 0.5f) * scale };
        }
    return v;
}

static std::vector<uint32_t> generateIndices(int size)
{
    std::vector<uint32_t> i;
    for (int z = 0; z < size - 1; ++z)
        for (int x = 0; x < size - 1; ++x) {
            uint32_t k = z * size + x;
            i.insert(i.end(), { k, k + size, k + 1, k + 1, k + size, k + size + 1 });
        }
    return i;
}

/* ===================== Shaders ===================== */

static const char* vsSource = R"(
#version 410 core
layout (location = 0) in vec3 aPos;

uniform mat4 uMVP;
uniform float uTime;

void main()
{
    float h =
        sin(aPos.x * 3.0 + uTime) *
        cos(aPos.z * 3.0 + uTime) * 0.5;

    vec3 pos = aPos;
    pos.y = h;

    gl_Position = uMVP * vec4(pos, 1.0);
}
)";

static const char* fsSource = R"(
#version 410 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(0.25, 0.8, 0.4, 1.0);
}
)";

static GLuint compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        throw std::runtime_error(log);
    }
    return s;
}

static GLuint createProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

/* ===================== Main ===================== */

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}
                      .title("Orbit Camera + Vertex Heightmap (GL 4.1)")
                      .size(1280, 720)
                      .openGL({ 4, 1, true })
                      .build();

    window.makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.getProcLoader())))
        throw std::runtime_error("Failed to load OpenGL");

    constexpr int GRID = 128;

    auto vertices = createGrid(GRID, 4.f);
    auto indices = generateIndices(GRID);

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(uint32_t),
        indices.data(),
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    GLuint program = createProgram();
    GLint uMVP = glGetUniformLocation(program, "uMVP");
    GLint uTime = glGetUniformLocation(program, "uTime");

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    Mat4 proj = mat4Perspective(45.f * 3.1415926f / 180.f, 1280.f / 720.f, 0.1f, 100.f);

    using clock = std::chrono::steady_clock;

    auto fpsTime = clock::now();
    auto prevFrame = clock::now();

    int frames = 0;
    float time = 0.f;
    float cameraAngle = 0.f;

    while (!window.shouldClose()) {
        auto now = clock::now();

        // delta time
        std::chrono::duration<float> dt = now - prevFrame;
        prevFrame = now;
        float deltaTime = dt.count();
        time += deltaTime;
        cameraAngle += 0.25f * deltaTime;

        // FPS counting
        frames++;
        std::chrono::duration<double> fpsDelta = now - fpsTime;

        if (fpsDelta.count() >= 1.0) {
            double fps = frames / fpsDelta.count();
            frames = 0;
            fpsTime = now;

            std::cout << "OpenGL Heightmap | FPS: " << static_cast<int>(fps) << "\n";
        }

        ctx.pollEvents();
        for (auto& e : window.events()) {
            if (e.is<Event::Closed>()) {
                window.requestClose();
            }
        }

        Vec3 eye{ std::cos(cameraAngle) * 3.0f, 2.0f, std::sin(cameraAngle) * 3.0f };

        Mat4 view = mat4LookAt(eye, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f });

        Mat4 mvp = mat4Multiply(proj, view);

        glClearColor(0.05f, 0.05f, 0.08f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp.m);
        glUniform1f(uTime, time);

        glBindVertexArray(vao);
        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(indices.size()),
            GL_UNSIGNED_INT,
            nullptr);

        window.swapBuffers();
    }
}