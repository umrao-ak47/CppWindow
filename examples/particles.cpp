#include <cppwindow/cppwindow.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <glad/glad.h>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace cwin;

namespace {

constexpr float Pi = 3.14159265358979323846f;

struct Vec3
{
    float x, y, z;
};

struct Vec4
{
    float x, y, z, w;
};

struct Mat4
{
    float m[16];
};

struct Particle
{
    Vec3 position;
    Vec3 velocity;
    float age;
    float lifetime;
    float size;
};

struct ParticleVertex
{
    Vec3 position;
    Vec4 color;
    float size;
};

static float randomFloat(std::mt19937& rng, float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

static Vec3 operator+(Vec3 a, Vec3 b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

static Vec3 operator*(Vec3 v, float s)
{
    return { v.x * s, v.y * s, v.z * s };
}

static Vec3 mix(Vec3 a, Vec3 b, float t)
{
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
    };
}

static float smoothstep(float edge0, float edge1, float x)
{
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static void resetParticle(Particle& p, std::mt19937& rng, bool scatter)
{
    const float angle = randomFloat(rng, 0.0f, 2.0f * Pi);
    const float radius = std::sqrt(randomFloat(rng, 0.0f, 1.0f)) * 0.26f;
    const float speed = randomFloat(rng, 2.9f, 5.8f);
    const float drift = randomFloat(rng, 0.10f, 0.72f);

    p.position = {
        std::cos(angle) * radius,
        randomFloat(rng, -0.08f, 0.12f),
        std::sin(angle) * radius,
    };
    p.velocity = {
        std::cos(angle) * drift + randomFloat(rng, -0.16f, 0.16f),
        speed,
        std::sin(angle) * drift + randomFloat(rng, -0.16f, 0.16f),
    };
    p.lifetime = randomFloat(rng, 1.25f, 2.9f);
    p.age = scatter ? randomFloat(rng, 0.0f, p.lifetime) : 0.0f;
    p.size = randomFloat(rng, 18.0f, 42.0f);
}

static std::vector<Particle> createParticles(size_t count, std::mt19937& rng)
{
    std::vector<Particle> particles(count);
    for (auto& p : particles) {
        resetParticle(p, rng, true);
    }
    return particles;
}

static Mat4 mat4Identity()
{
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
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
    float f = 1.0f / std::tan(fov * 0.5f);
    Mat4 r{};
    r.m[0] = f / std::max(aspect, 0.001f);
    r.m[5] = f;
    r.m[10] = (zf + zn) / (zn - zf);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zf * zn) / (zn - zf);
    return r;
}

static Vec3 normalize(Vec3 v)
{
    float l = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (l <= 0.0001f) {
        return { 0.0f, 1.0f, 0.0f };
    }
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
    r.m[14] = f.x * eye.x + f.y * eye.y + f.z * eye.z;
    return r;
}

static ParticleVertex makeVertex(const Particle& p)
{
    const float t = std::clamp(p.age / p.lifetime, 0.0f, 1.0f);
    const Vec3 hot = { 1.0f, 0.86f, 0.34f };
    const Vec3 ember = { 1.0f, 0.25f, 0.08f };
    const Vec3 smokeBlue = { 0.20f, 0.42f, 1.0f };

    Vec3 color =
        t < 0.42f ? mix(hot, ember, t / 0.42f) : mix(ember, smokeBlue, (t - 0.42f) / 0.58f);
    const float fadeIn = smoothstep(0.0f, 0.08f, t);
    const float fadeOut = std::pow(1.0f - t, 1.55f);

    return {
        .position = p.position,
        .color = { color.x, color.y, color.z, fadeIn * fadeOut },
        .size = p.size * (0.55f + 1.85f * t),
    };
}

static const char* VertexShader = R"(
#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aSize;

uniform mat4 uMVP;

out vec4 vColor;

void main()
{
    vec4 clip = uMVP * vec4(aPos, 1.0);
    gl_Position = clip;
    gl_PointSize = aSize * clamp(1.0 / max(clip.w * 0.16, 0.45), 0.35, 2.2);
    vColor = aColor;
}
)";

static const char* FragmentShader = R"(
#version 410 core

in vec4 vColor;
out vec4 FragColor;

void main()
{
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(p, p);
    if (r2 > 1.0) {
        discard;
    }

    float core = smoothstep(0.20, 0.0, r2);
    float halo = smoothstep(1.0, 0.12, r2);
    vec3 color = mix(vColor.rgb * 0.22, vColor.rgb, core);
    FragColor = vec4(color, vColor.a * halo);
}
)";

static GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        throw std::runtime_error(log);
    }

    return shader;
}

static GLuint createProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, VertexShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        throw std::runtime_error(log);
    }

    return program;
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::get();

    auto window = WindowBuilder{}
                      .title("Particle Fountain (OpenGL 4.1)")
                      .size(1280, 720)
                      .openGL({ 4, 1, true })
                      .resizable()
                      .build();

    window.makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.procLoader()))) {
        throw std::runtime_error("Failed to load OpenGL");
    }

    const bool useVSync = true;
    window.setVSync(useVSync);
    FrameLimiter frameLimiter(60.0);
    frameLimiter.setVSyncEnabled(useVSync);

    std::mt19937 rng{ std::random_device{}() };
    constexpr size_t ParticleCount = 4500;
    auto particles = createParticles(ParticleCount, rng);
    std::vector<ParticleVertex> vertices(ParticleCount);

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(ParticleVertex)),
        nullptr,
        GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(ParticleVertex),
        reinterpret_cast<void*>(offsetof(ParticleVertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(ParticleVertex),
        reinterpret_cast<void*>(offsetof(ParticleVertex, color)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(ParticleVertex),
        reinterpret_cast<void*>(offsetof(ParticleVertex, size)));

    GLuint program = createProgram();
    GLint uMVP = glGetUniformLocation(program, "uMVP");

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);

    FrameTimer frameTimer;
    FpsCounter fpsCounter(0.5);
    float elapsed = 0.0f;
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            }
        });

    while (!window.shouldClose()) {
        const FrameTime frame = frameTimer.tick();
        const float deltaTime = static_cast<float>(std::min(frame.deltaSeconds, 1.0 / 30.0));
        elapsed += deltaTime;
        if (fpsCounter.update(frame)) {
            std::ostringstream title;
            title << "Particle Fountain (OpenGL 4.1) - FPS "
                  << static_cast<int>(fpsCounter.framesPerSecond());
            window.setTitle(title.str());
        }

        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        for (size_t i = 0; i < particles.size(); ++i) {
            Particle& p = particles[i];
            const float t = std::clamp(p.age / p.lifetime, 0.0f, 1.0f);
            const Vec3 swirl = {
                -p.position.z * (0.90f + t * 0.35f),
                -1.85f - t * 1.20f,
                p.position.x * (0.90f + t * 0.35f),
            };

            p.velocity = p.velocity + swirl * deltaTime;
            p.position = p.position + p.velocity * deltaTime;
            p.age += deltaTime;

            if (p.age >= p.lifetime || p.position.y < -0.45f) {
                resetParticle(p, rng, false);
            }

            vertices[i] = makeVertex(p);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            static_cast<GLsizeiptr>(vertices.size() * sizeof(ParticleVertex)),
            vertices.data());

        auto [fbWidth, fbHeight] = window.framebufferSize();
        fbWidth = std::max<uint32_t>(fbWidth, 1);
        fbHeight = std::max<uint32_t>(fbHeight, 1);
        glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));

        const float aspect = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
        const float orbit = elapsed * 0.22f;
        Mat4 proj = mat4Perspective(48.0f * Pi / 180.0f, aspect, 0.1f, 100.0f);
        Mat4 view = mat4LookAt(
            { std::cos(orbit) * 4.2f, 2.4f, std::sin(orbit) * 4.2f + 4.7f },
            { 0.0f, 1.75f, 0.0f },
            { 0.0f, 1.0f, 0.0f });
        Mat4 mvp = mat4Multiply(proj, view);

        glClearColor(0.006f, 0.008f, 0.018f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp.m);

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(vertices.size()));

        window.swapBuffers();
        frameLimiter.wait();
    }

    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
