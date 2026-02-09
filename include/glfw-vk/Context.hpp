#pragma once

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace glfwvk {

class GlfwContext
{
public:
    static GlfwContext& Get();

    ~GlfwContext();
    GlfwContext(const GlfwContext&) = delete;
    GlfwContext& operator=(const GlfwContext&) = delete;

    bool isVulkanSupported() const;
    std::vector<std::string> getRequiredGlfwVulkanExtensions() const;

private:
    GlfwContext();
};

}  // namespace glfwvk