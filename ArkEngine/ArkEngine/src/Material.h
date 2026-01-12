#pragma once
#include <glm/vec3.hpp>
class Texture;
class Shader;

struct Material {
    Texture* albedo = nullptr;
    Texture* specular = nullptr;
    glm::vec3 color{ 1.0f,1.0f,1.0f };
    float shininess = 32.0f;

    void Bind(Shader& shader) const;
};