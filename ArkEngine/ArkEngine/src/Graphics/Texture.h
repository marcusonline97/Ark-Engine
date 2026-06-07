#pragma once

#include <GLAD/glad.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine
{
	enum class MipmapFilter;

	class Texture
	{
	public:
		Texture(int width, int height, int numChannels, unsigned char* data);

		~Texture();

		GLuint GetID() const;


		void Init(int width, int height, int numChannels, unsigned char* data);

		int GetWidth() const;
		int GetHeight() const;

		static std::shared_ptr<Texture> Load(const std::string& path);

		void SetFilter(GLint minFilter, GLint magFilter);

	private:
		int m_width = 0;
		int m_height = 0;
		int m_numChannels = 0;
		GLuint m_textureID = 0;
	};

	class TextureManager
	{
	public:
		std::shared_ptr<Texture> GetOrLoadTexture(const std::string& path);
		void SetCurrentFilter(MipmapFilter filter);
		void SetFilterOnAllTextures(MipmapFilter filter);
		void SetFilterOnAllTextures(GLint minFilter, GLint magFilter);

	private:
		std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
		MipmapFilter m_currentFilter = static_cast<MipmapFilter>(0);
	};
}