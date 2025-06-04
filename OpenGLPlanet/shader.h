#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    void CheckCompileErrors(GLuint shader, std::string type);
    void enable();
    void disable();

    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;

    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    void setBool(const std::string& name, bool value) const;

    std::string PreprocessShader(const std::string& source, const std::string& includePath = "");

};
