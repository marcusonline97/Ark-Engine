#include "ObjGpuMesh.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace Ark::Rendering
{
	ObjGpuMesh::~ObjGpuMesh()
	{
		if (m_vao) glDeleteVertexArrays(1, &m_vao);
		if (m_vbo) glDeleteBuffers(1, &m_vbo);
		m_vao = 0;
		m_vbo = 0;
		m_vertexCount = 0;
	}

	ObjGpuMesh::ObjGpuMesh(ObjGpuMesh&& other) noexcept
	{
		*this = std::move(other);
	}

	ObjGpuMesh& ObjGpuMesh::operator=(ObjGpuMesh&& other) noexcept
	{
		if (this == &other)
			return *this;

		if (m_vao) glDeleteVertexArrays(1, &m_vao);
		if (m_vbo) glDeleteBuffers(1, &m_vbo);

		m_vao = other.m_vao;
		m_vbo = other.m_vbo;
		m_vertexCount = other.m_vertexCount;

		other.m_vao = 0;
		other.m_vbo = 0;
		other.m_vertexCount = 0;

		return *this;
	}

	ObjGpuMesh::ObjIndex ObjGpuMesh::ParseFaceToken(const std::string& token, int vCount, int vtCount, int vnCount)
	{
		ObjIndex out{};

		// formats:
		//  v
		//  v/vt
		//  v//vn
		//  v/vt/vn
		const size_t p0 = token.find('/');
		if (p0 == std::string::npos)
		{
			out.v = FixIndex(std::stoi(token), vCount);
			return out;
		}

		const std::string sV = token.substr(0, p0);
		out.v = sV.empty() ? -1 : FixIndex(std::stoi(sV), vCount);

		const size_t p1 = token.find('/', p0 + 1);
		if (p1 == std::string::npos)
		{
			const std::string sVT = token.substr(p0 + 1);
			out.vt = sVT.empty() ? -1 : FixIndex(std::stoi(sVT), vtCount);
			return out;
		}

		const std::string sVT = token.substr(p0 + 1, p1 - (p0 + 1));
		out.vt = sVT.empty() ? -1 : FixIndex(std::stoi(sVT), vtCount);

		const std::string sVN = token.substr(p1 + 1);
		out.vn = sVN.empty() ? -1 : FixIndex(std::stoi(sVN), vnCount);

		return out;
	}

	int ObjGpuMesh::FixIndex(int idx, int count)
	{
		// OBJ:  1..N, negative indexes allowed (relative), 0 is invalid.
		if (idx > 0) return idx - 1;
		if (idx < 0) return count + idx;
		return -1;
	}

	bool ObjGpuMesh::LoadFromObj(const std::filesystem::path& path, bool normalizeScale = false)
	{
		std::ifstream f(path);
		if (!f.is_open())
			return false;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;

		positions.reserve(4096);
		normals.reserve(4096);
		uvs.reserve(4096);

		struct Vertex
		{
			glm::vec3 pos{};
			glm::vec3 nrm{ 0.0f, 1.0f, 0.0f };
			glm::vec2 uv{ 0.0f, 0.0f };
		};

		std::vector<Vertex> triVerts;
		triVerts.reserve(16384);

		std::string line;
		while (std::getline(f, line))
		{
			if (line.empty())
				continue;

			size_t start = 0;
			while (start < line.size() && std::isspace(static_cast<unsigned char>(line[start])))
				++start;
			if (start >= line.size())
				continue;

			if (line[start] == '#')
				continue;

			std::istringstream iss(line.substr(start));
			std::string tag;
			iss >> tag;

			if (tag == "v")
			{
				float x = 0, y = 0, z = 0;
				iss >> x >> y >> z;
				positions.emplace_back(x, y, z);
			}
			else if (tag == "vn")
			{
				float x = 0, y = 0, z = 0;
				iss >> x >> y >> z;
				normals.emplace_back(glm::normalize(glm::vec3(x, y, z)));
			}
			else if (tag == "vt")
			{
				float u = 0, v = 0;
				iss >> u >> v;
				uvs.emplace_back(u, v);
			}
			else if (tag == "f")
			{
				std::vector<ObjIndex> face;
				face.reserve(8);

				std::string tok;
				while (iss >> tok)
				{
					face.push_back(ParseFaceToken(tok,
						static_cast<int>(positions.size()),
						static_cast<int>(uvs.size()),
						static_cast<int>(normals.size())));
				}

				if (face.size() < 3)
					continue;

				for (size_t i = 1; i + 1 < face.size(); ++i)
				{
					const ObjIndex idxs[3] = { face[0], face[i], face[i + 1] };

					Vertex vs[3]{};
					bool hasAnyNormal = true;
					for (int k = 0; k < 3; ++k)
					{
						const int vi = idxs[k].v;
						if (vi < 0 || vi >= static_cast<int>(positions.size()))
						{
							hasAnyNormal = false;
							break;
						}

						vs[k].pos = positions[static_cast<size_t>(vi)];

						if (idxs[k].vn >= 0 && idxs[k].vn < static_cast<int>(normals.size()))
							vs[k].nrm = normals[static_cast<size_t>(idxs[k].vn)];
						else
							hasAnyNormal = false;

						if (idxs[k].vt >= 0 && idxs[k].vt < static_cast<int>(uvs.size()))
							vs[k].uv = uvs[static_cast<size_t>(idxs[k].vt)];
					}

					if (!hasAnyNormal)
					{
						const glm::vec3 e0 = vs[1].pos - vs[0].pos;
						const glm::vec3 e1 = vs[2].pos - vs[0].pos;
						glm::vec3 n = glm::cross(e0, e1);
						const float ln = glm::length(n);
						n = (ln > 0.000001f) ? (n / ln) : glm::vec3(0.0f, 1.0f, 0.0f);

						vs[0].nrm = n;
						vs[1].nrm = n;
						vs[2].nrm = n;
					}

					triVerts.push_back(vs[0]);
					triVerts.push_back(vs[1]);
					triVerts.push_back(vs[2]);
				}
			}
		}

		if (triVerts.empty())
			return false;

		glm::vec3 mn = triVerts[0].pos;
		glm::vec3 mx = triVerts[0].pos;
		for (const auto& v : triVerts)
		{
			mn = glm::min(mn, v.pos);
			mx = glm::max(mx, v.pos);
		}

		const glm::vec3 center = 0.5f * (mn + mx);
		const glm::vec3 ext = (mx - mn);
		const float maxExtent = std::max(ext.x, std::max(ext.y, ext.z));
		const float inv = (maxExtent > 0.000001f) ? (1.0f / maxExtent) : 1.0f;

		for (auto& v : triVerts)
			v.pos = (v.pos - center) * inv;

		m_vertexCount = static_cast<GLsizei>(triVerts.size());

		if (!m_vao) glGenVertexArrays(1, &m_vao);
		if (!m_vbo) glGenBuffers(1, &m_vbo);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			static_cast<GLsizeiptr>(triVerts.size() * sizeof(Vertex)),
			triVerts.data(),
			GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nrm));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		return true;
	}

	void ObjGpuMesh::Draw() const
	{
		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
	}
}