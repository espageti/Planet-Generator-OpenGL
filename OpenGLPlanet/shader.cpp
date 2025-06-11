#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath ) {
    // 1. Read shader source files
    std::ifstream vFile(vertexPath), fFile(fragmentPath);
    std::stringstream vStream, fStream;
    
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();
    
    std::string vCode = PreprocessShader(vStream.str(), "shaders/");
    std::string fCode = PreprocessShader(fStream.str(), "shaders/");
    
    const char* vShaderCode = vCode.c_str();
    const char* fShaderCode = fCode.c_str();

    // 2. Compile vertex shader
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    CheckCompileErrors(vertex, "VERTEX");

    // 3. Compile fragment shader
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    CheckCompileErrors(fragment, "FRAGMENT");

    // 4. Optional geometry shader
    unsigned int geometry = 0;
    if (geometryPath != nullptr) {
        std::ifstream gFile(geometryPath);
        std::stringstream gStream;
        gStream << gFile.rdbuf();
        std::string gCode = PreprocessShader(gStream.str(), "shaders/");
        const char* gShaderCode = gCode.c_str();

        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        CheckCompileErrors(geometry, "GEOMETRY");
    }

    // 5. Create shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    if (geometryPath != nullptr) {
        glAttachShader(ID, geometry);
    }
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    CheckCompileErrors(ID, "PROGRAM");

    // 6. Cleanup shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometryPath != nullptr) {
        glDeleteShader(geometry);
    }
}

void Shader::CheckCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];

    if (type != "PROGRAM") {
        // Check shader compilation errors
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << "\n"
                << "-------------------------------------------------------" << std::endl;
        }
    }
    else {
        // Check program linking errors
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                << infoLog << "\n"
                << "-------------------------------------------------------" << std::endl;
        }

        // Optional: Validate program (useful during development)
        glValidateProgram(shader);
        glGetProgramiv(shader, GL_VALIDATE_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_VALIDATION_ERROR of type: " << type << "\n"
                << infoLog << "\n"
                << "-------------------------------------------------------" << std::endl;
        }
    }
}


void Shader::enable() {
    glUseProgram(ID);
}
void Shader::disable() {
    glUseProgram(0);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

std::string Shader::PreprocessShader(const std::string& source, const std::string& includePath) {
    std::regex includeRegex(R"(#include\s+\"([^\"]+)\")");
    std::smatch matches;
    std::string result = source;

    while (std::regex_search(result, matches, includeRegex)) {
        std::string includeFile = includePath + matches[1].str();
        std::ifstream file(includeFile);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open include file: " + includeFile);
        }
        std::string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        result.replace(matches.position(), matches.length(), content);
    }
    return result;
}
