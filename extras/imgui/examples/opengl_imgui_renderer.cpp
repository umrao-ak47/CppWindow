#include "opengl_imgui_renderer.hpp"

#include <glad/glad.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace example {
namespace {

constexpr GLuint PositionAttribute = 0;
constexpr GLuint TexCoordAttribute = 1;
constexpr GLuint ColorAttribute = 2;

[[nodiscard]] std::string shaderSource(const char* glslVersion, std::string_view body)
{
    std::string source = (glslVersion != nullptr && glslVersion[0] != '\0')
        ? glslVersion
        : "#version 410";
    source += '\n';
    source += body;
    return source;
}

[[nodiscard]] std::string shaderLog(GLuint shader)
{
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return {};
    }

    std::string log(static_cast<std::size_t>(length), '\0');
    glGetShaderInfoLog(shader, length, nullptr, log.data());
    return log;
}

[[nodiscard]] std::string programLog(GLuint program)
{
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return {};
    }

    std::string log(static_cast<std::size_t>(length), '\0');
    glGetProgramInfoLog(program, length, nullptr, log.data());
    return log;
}

[[nodiscard]] GLuint compileShader(GLenum type, const std::string& source)
{
    const GLuint shader = glCreateShader(type);
    const char* sourceData = source.c_str();
    glShaderSource(shader, 1, &sourceData, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        const std::string log = shaderLog(shader);
        glDeleteShader(shader);
        throw std::runtime_error("Failed to compile ImGui OpenGL shader: " + log);
    }

    return shader;
}

[[nodiscard]] GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {
        const std::string log = programLog(program);
        glDeleteProgram(program);
        throw std::runtime_error("Failed to link ImGui OpenGL shader program: " + log);
    }

    return program;
}

[[nodiscard]] ImTextureID toTextureId(GLuint texture)
{
    return static_cast<ImTextureID>(static_cast<std::uintptr_t>(texture));
}

[[nodiscard]] GLuint fromTextureId(ImTextureID texture)
{
    return static_cast<GLuint>(static_cast<std::uintptr_t>(texture));
}

void restoreCapability(GLenum capability, GLboolean wasEnabled)
{
    if (wasEnabled == GL_TRUE) {
        glEnable(capability);
    } else {
        glDisable(capability);
    }
}

struct TextureUploadState
{
    GLint textureBinding = 0;
    GLint unpackAlignment = 0;
    GLint unpackRowLength = 0;

    TextureUploadState()
    {
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);
#ifdef GL_UNPACK_ROW_LENGTH
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &unpackRowLength);
#endif
    }

    ~TextureUploadState()
    {
#ifdef GL_UNPACK_ROW_LENGTH
        glPixelStorei(GL_UNPACK_ROW_LENGTH, unpackRowLength);
#endif
        glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(textureBinding));
    }
};

struct RenderState
{
    GLint activeTexture = 0;
    GLint currentProgram = 0;
    GLint textureBinding = 0;
    GLint samplerBinding = 0;
    GLint arrayBuffer = 0;
    GLint elementArrayBuffer = 0;
    GLint vertexArray = 0;
    std::array<GLint, 4> viewport{};
    std::array<GLint, 4> scissorBox{};
    GLint blendSrcRgb = 0;
    GLint blendDstRgb = 0;
    GLint blendSrcAlpha = 0;
    GLint blendDstAlpha = 0;
    GLint blendEquationRgb = 0;
    GLint blendEquationAlpha = 0;
    GLboolean blend = GL_FALSE;
    GLboolean cullFace = GL_FALSE;
    GLboolean depthTest = GL_FALSE;
    GLboolean stencilTest = GL_FALSE;
    GLboolean scissorTest = GL_FALSE;

    RenderState()
    {
        glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
        glActiveTexture(GL_TEXTURE0);
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);
        glGetIntegerv(GL_SAMPLER_BINDING, &samplerBinding);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementArrayBuffer);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArray);
        glGetIntegerv(GL_VIEWPORT, viewport.data());
        glGetIntegerv(GL_SCISSOR_BOX, scissorBox.data());
        glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRgb);
        glGetIntegerv(GL_BLEND_DST_RGB, &blendDstRgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDstAlpha);
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEquationRgb);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEquationAlpha);
        blend = glIsEnabled(GL_BLEND);
        cullFace = glIsEnabled(GL_CULL_FACE);
        depthTest = glIsEnabled(GL_DEPTH_TEST);
        stencilTest = glIsEnabled(GL_STENCIL_TEST);
        scissorTest = glIsEnabled(GL_SCISSOR_TEST);
    }

    ~RenderState()
    {
        if (currentProgram == 0 || glIsProgram(static_cast<GLuint>(currentProgram)) == GL_TRUE) {
            glUseProgram(static_cast<GLuint>(currentProgram));
        }
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(textureBinding));
        glBindSampler(0, static_cast<GLuint>(samplerBinding));
        glActiveTexture(static_cast<GLenum>(activeTexture));
        glBindVertexArray(static_cast<GLuint>(vertexArray));
        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(arrayBuffer));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(elementArrayBuffer));
        glBlendEquationSeparate(
            static_cast<GLenum>(blendEquationRgb),
            static_cast<GLenum>(blendEquationAlpha));
        glBlendFuncSeparate(
            static_cast<GLenum>(blendSrcRgb),
            static_cast<GLenum>(blendDstRgb),
            static_cast<GLenum>(blendSrcAlpha),
            static_cast<GLenum>(blendDstAlpha));
        restoreCapability(GL_BLEND, blend);
        restoreCapability(GL_CULL_FACE, cullFace);
        restoreCapability(GL_DEPTH_TEST, depthTest);
        restoreCapability(GL_STENCIL_TEST, stencilTest);
        restoreCapability(GL_SCISSOR_TEST, scissorTest);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(scissorBox[0], scissorBox[1], scissorBox[2], scissorBox[3]);
    }
};

}  // namespace

struct OpenGLImGuiRenderer::State
{
    GLuint program = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLuint vao = 0;
    GLint textureLocation = -1;
    GLint projectionLocation = -1;

    explicit State(const char* glslVersion)
    {
        static constexpr std::string_view VertexShader = R"(
layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec4 Color;

uniform mat4 Projection;

out vec2 FragTexCoord;
out vec4 FragColor;

void main()
{
    FragTexCoord = TexCoord;
    FragColor = Color;
    gl_Position = Projection * vec4(Position.xy, 0.0, 1.0);
}
)";

        static constexpr std::string_view FragmentShader = R"(
in vec2 FragTexCoord;
in vec4 FragColor;

uniform sampler2D Texture;

out vec4 OutColor;

void main()
{
    OutColor = FragColor * texture(Texture, FragTexCoord.st);
}
)";

        GLuint vertexShader = 0;
        GLuint fragmentShader = 0;
        try {
            vertexShader = compileShader(GL_VERTEX_SHADER, shaderSource(glslVersion, VertexShader));
            fragmentShader = compileShader(GL_FRAGMENT_SHADER, shaderSource(glslVersion, FragmentShader));
            program = linkProgram(vertexShader, fragmentShader);
        } catch (...) {
            if (vertexShader != 0) {
                glDeleteShader(vertexShader);
            }
            if (fragmentShader != 0) {
                glDeleteShader(fragmentShader);
            }
            throw;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        textureLocation = glGetUniformLocation(program, "Texture");
        projectionLocation = glGetUniformLocation(program, "Projection");
        if (textureLocation < 0 || projectionLocation < 0) {
            glDeleteProgram(program);
            program = 0;
            throw std::runtime_error("Failed to locate ImGui OpenGL shader uniforms");
        }

        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glGenVertexArrays(1, &vao);
    }

    ~State()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteProgram(program);
    }

    void setupRenderState(const ImDrawData& drawData, int framebufferWidth, int framebufferHeight) const
    {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_SCISSOR_TEST);

        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glUseProgram(program);
        glUniform1i(textureLocation, 0);
        glBindSampler(0, 0);

        const float left = drawData.DisplayPos.x;
        const float right = drawData.DisplayPos.x + drawData.DisplaySize.x;
        const float top = drawData.DisplayPos.y;
        const float bottom = drawData.DisplayPos.y + drawData.DisplaySize.y;
        const float projection[4][4] = {
            { 2.0f / (right - left), 0.0f, 0.0f, 0.0f },
            { 0.0f, 2.0f / (top - bottom), 0.0f, 0.0f },
            { 0.0f, 0.0f, -1.0f, 0.0f },
            { (right + left) / (left - right), (top + bottom) / (bottom - top), 0.0f, 1.0f },
        };
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glEnableVertexAttribArray(PositionAttribute);
        glEnableVertexAttribArray(TexCoordAttribute);
        glEnableVertexAttribArray(ColorAttribute);
        glVertexAttribPointer(
            PositionAttribute,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(ImDrawVert),
            reinterpret_cast<const GLvoid*>(offsetof(ImDrawVert, pos)));
        glVertexAttribPointer(
            TexCoordAttribute,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(ImDrawVert),
            reinterpret_cast<const GLvoid*>(offsetof(ImDrawVert, uv)));
        glVertexAttribPointer(
            ColorAttribute,
            4,
            GL_UNSIGNED_BYTE,
            GL_TRUE,
            sizeof(ImDrawVert),
            reinterpret_cast<const GLvoid*>(offsetof(ImDrawVert, col)));
    }

    void updateTexture(ImTextureData& texture) const
    {
        if (texture.Status == ImTextureStatus_WantCreate) {
            if (texture.Format != ImTextureFormat_RGBA32) {
                throw std::runtime_error("CppWindow ImGui OpenGL example renderer supports RGBA32 textures only");
            }

            TextureUploadState uploadState;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            GLuint textureHandle = 0;
            glGenTextures(1, &textureHandle);
            glBindTexture(GL_TEXTURE_2D, textureHandle);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                texture.Width,
                texture.Height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                texture.GetPixels());

            texture.BackendUserData =
                reinterpret_cast<void*>(static_cast<std::uintptr_t>(textureHandle));
            texture.SetTexID(toTextureId(textureHandle));
            texture.SetStatus(ImTextureStatus_OK);
            return;
        }

        if (texture.Status == ImTextureStatus_WantUpdates) {
            TextureUploadState uploadState;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glBindTexture(GL_TEXTURE_2D, fromTextureId(texture.TexID));
#ifdef GL_UNPACK_ROW_LENGTH
            glPixelStorei(GL_UNPACK_ROW_LENGTH, texture.Width);
#endif
            for (int updateIndex = 0; updateIndex < texture.Updates.Size; ++updateIndex) {
                const ImTextureRect& update = texture.Updates[updateIndex];
                glTexSubImage2D(
                    GL_TEXTURE_2D,
                    0,
                    update.x,
                    update.y,
                    update.w,
                    update.h,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    texture.GetPixelsAt(update.x, update.y));
            }
            texture.SetStatus(ImTextureStatus_OK);
            return;
        }

        if (texture.Status == ImTextureStatus_WantDestroy && texture.UnusedFrames > 0) {
            destroyTexture(texture);
        }
    }

    void destroyTexture(ImTextureData& texture) const
    {
        if (texture.BackendUserData == nullptr) {
            return;
        }

        GLuint textureHandle = fromTextureId(texture.TexID);
        glDeleteTextures(1, &textureHandle);
        texture.BackendUserData = nullptr;
        texture.SetTexID(ImTextureID_Invalid);
        texture.SetStatus(ImTextureStatus_Destroyed);
    }
};

void loadOpenGL(cwin::WindowContext& context)
{
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(context.getProcLoader()))) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
}

OpenGLImGuiRenderer::OpenGLImGuiRenderer(const char* glslVersion)
    : state_(std::make_unique<State>(glslVersion))
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.BackendRendererUserData != nullptr) {
        throw std::runtime_error("Dear ImGui already has a renderer backend");
    }

    io.BackendRendererName = "cppwindow_example_opengl";
    io.BackendRendererUserData = state_.get();
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
}

OpenGLImGuiRenderer::~OpenGLImGuiRenderer()
{
    if (ImGui::GetCurrentContext() == nullptr || state_ == nullptr) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.BackendRendererUserData != state_.get()) {
        return;
    }

    for (int textureIndex = 0; textureIndex < io.Fonts->TexList.Size; ++textureIndex) {
        state_->destroyTexture(*io.Fonts->TexList[textureIndex]);
    }

    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    for (int textureIndex = 0; textureIndex < platformIO.Textures.Size; ++textureIndex) {
        state_->destroyTexture(*platformIO.Textures[textureIndex]);
    }

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
}

void OpenGLImGuiRenderer::newFrame()
{
    if (state_ == nullptr) {
        throw std::runtime_error("ImGui OpenGL renderer is not initialized");
    }
}

void OpenGLImGuiRenderer::render(ImDrawData* drawData)
{
    if (state_ == nullptr || drawData == nullptr) {
        return;
    }

    const int framebufferWidth =
        static_cast<int>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    const int framebufferHeight =
        static_cast<int>(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (framebufferWidth <= 0 || framebufferHeight <= 0) {
        return;
    }

    if (drawData->Textures != nullptr) {
        for (int textureIndex = 0; textureIndex < drawData->Textures->Size; ++textureIndex) {
            ImTextureData* texture = (*drawData->Textures)[textureIndex];
            if (texture != nullptr && texture->Status != ImTextureStatus_OK) {
                state_->updateTexture(*texture);
            }
        }
    }

    if (drawData->TotalVtxCount <= 0) {
        return;
    }

    const RenderState previousState;
    state_->setupRenderState(*drawData, framebufferWidth, framebufferHeight);

    const ImVec2 clipOffset = drawData->DisplayPos;
    const ImVec2 clipScale = drawData->FramebufferScale;
    const GLenum indexType =
        sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

    for (const ImDrawList* drawList : drawData->CmdLists) {
        const GLsizeiptr vertexBufferSize =
            static_cast<GLsizeiptr>(drawList->VtxBuffer.Size * sizeof(ImDrawVert));
        const GLsizeiptr indexBufferSize =
            static_cast<GLsizeiptr>(drawList->IdxBuffer.Size * sizeof(ImDrawIdx));

        glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, drawList->VtxBuffer.Data, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, drawList->IdxBuffer.Data, GL_STREAM_DRAW);

        for (const ImDrawCmd& command : drawList->CmdBuffer) {
            if (command.UserCallback != nullptr) {
                if (command.UserCallback == ImDrawCallback_ResetRenderState) {
                    state_->setupRenderState(*drawData, framebufferWidth, framebufferHeight);
                } else {
                    command.UserCallback(drawList, &command);
                }
                continue;
            }

            const ImVec2 clipMin{
                (command.ClipRect.x - clipOffset.x) * clipScale.x,
                (command.ClipRect.y - clipOffset.y) * clipScale.y,
            };
            const ImVec2 clipMax{
                (command.ClipRect.z - clipOffset.x) * clipScale.x,
                (command.ClipRect.w - clipOffset.y) * clipScale.y,
            };
            if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) {
                continue;
            }

            glScissor(
                static_cast<GLint>(clipMin.x),
                static_cast<GLint>(static_cast<float>(framebufferHeight) - clipMax.y),
                static_cast<GLint>(clipMax.x - clipMin.x),
                static_cast<GLint>(clipMax.y - clipMin.y));
            glBindTexture(GL_TEXTURE_2D, fromTextureId(command.GetTexID()));
            glDrawElementsBaseVertex(
                GL_TRIANGLES,
                static_cast<GLsizei>(command.ElemCount),
                indexType,
                reinterpret_cast<const GLvoid*>(
                    static_cast<std::uintptr_t>(command.IdxOffset * sizeof(ImDrawIdx))),
                static_cast<GLint>(command.VtxOffset));
        }
    }
}

void clearFramebuffer(cwin::Window& window, const ImVec4& color)
{
    auto [fbWidth, fbHeight] = window.getFramebufferSize();
    glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void drawRect(int x, int y, int width, int height, const ImVec4& color)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

}  // namespace example
