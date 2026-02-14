#include <cppwindow/cppwindow.hpp>

#include <chrono>
#include <cmath>
#include <glad/glad.h>
#include <iostream>
#include <random>
#include <vector>

using namespace cwin;

/* ===================== Particle Struct ===================== */

struct Particle
{
    float x, y, z;
    float vx, vy, vz;
    float lifetime;
};

/* ===================== Helpers ===================== */

void randomParticle(Particle& p)
{
    p.x = ((rand() % 1000) / 100.f - 5.f);
    p.y = ((rand() % 1000) / 200.f);
    p.z = ((rand() % 1000) / 100.f - 5.f);
    p.vx = ((rand() % 1000) / 1000.f - 1.f);
    p.vy = ((rand() % 1000) / 500.f);
    p.vz = ((rand() % 1000) / 1000.f - 1.f);
    p.lifetime = 1.f + (rand() % 3000) / 1000.f;
}

std::vector<Particle> createParticles(int count)
{
    std::vector<Particle> particles(count);
    for (auto& p : particles) {
        randomParticle(p);
    }
    return particles;
}

/* ===================== Minimal Math ===================== */

struct Vec3
{
    float x, y, z;
};
struct Mat4
{
    float m[16];
};

static Mat4 mat4Identity()
{
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.f;
    return r;
}

static Mat4 mat4Multiply(const Mat4& a, const Mat4& b)
{
    Mat4 r{};
    for (int c = 0; c < 4; c++) {
        for (int r0 = 0; r0 < 4; r0++) {
            for (int k = 0; k < 4; k++) {
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
    r.m[14] = (2.f * zf * zn) / (zn - zf);  // corrected below
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

/* ===================== Shaders ===================== */

const char* vs = R"(
#version 410 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
void main() {
    gl_Position = uMVP * vec4(aPos,1.0);
    gl_PointSize = 5.0;
}
)";

const char* fs = R"(
#version 410 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0,0.7,0.2,1.0);
}
)";

GLuint compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        throw std::runtime_error(log);
    }
    return s;
}

GLuint createProgram()
{
    GLuint vsId = compileShader(GL_VERTEX_SHADER, vs);
    GLuint fsId = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vsId);
    glAttachShader(prog, fsId);
    glLinkProgram(prog);
    glDeleteShader(vsId);
    glDeleteShader(fsId);
    return prog;
}

/* ===================== Main ===================== */

int main()
{
    srand(time(NULL));
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}
                      .title("Particle Example")
                      .size(1280, 720)
                      .openGL({ 4, 1, true })
                      .resizable()
                      .build();

    window.makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.getProcLoader()))) {
        throw std::runtime_error("GLAD failed");
    }

    const int NUM_PARTICLES = 1000;
    auto particles = createParticles(NUM_PARTICLES);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, NUM_PARTICLES * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    GLuint program = createProgram();
    GLint uMVP = glGetUniformLocation(program, "uMVP");

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    // Camera
    Mat4 proj = mat4Perspective(45.f * 3.14159f / 180.f, 1280.f / 720.f, 0.1f, 100.f);
    Mat4 view = mat4LookAt({ 0, 5, 8 }, { 0, 0, 0 }, { 0, 1, 0 });
    Mat4 mvp = mat4Multiply(proj, view);

    auto lastTime = std::chrono::steady_clock::now();

    std::vector<float> buffer(NUM_PARTICLES * 3);
    while (!window.shouldClose()) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> dt = now - lastTime;
        lastTime = now;
        float deltaTime = dt.count();

        ctx.pollEvents();
        for (auto& e : window.events()) {
            if (e.is<Event::Closed>()) {
                window.requestClose();
            }
        }

        // Update particles
        for (auto& p : particles) {
            p.vy -= 9.8f * deltaTime;  // gravity
            p.x += p.vx * deltaTime;
            p.y += p.vy * deltaTime;
            p.z += p.vz * deltaTime;

            p.lifetime -= deltaTime;
            if (p.lifetime <= 0.f) {
                randomParticle(p);
            }
        }

        // Upload positions
        for (int i = 0; i < NUM_PARTICLES; i++) {
            buffer[i * 3 + 0] = particles[i].x;
            buffer[i * 3 + 1] = particles[i].y;
            buffer[i * 3 + 2] = particles[i].z;
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, buffer.size() * sizeof(float), buffer.data());

        // Render
        glClearColor(0.0f, 0.0f, 0.05f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp.m);

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

        window.swapBuffers();
    }
}