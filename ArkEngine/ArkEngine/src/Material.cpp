#include "Material.h"
#include "Texture.h"
#include "Shader.h"

void Material::Bind(Shader& shader) const {
    shader.Bind();
    if (albedo) { albedo->BindUnit(0); shader.SetInt("uAlbedo", 0); }
    if (specular) { specular->BindUnit(1); shader.SetInt("uSpecular", 1); }
    shader.SetVec3("uColor", color);
    shader.SetFloat("uShininess", shininess);
}