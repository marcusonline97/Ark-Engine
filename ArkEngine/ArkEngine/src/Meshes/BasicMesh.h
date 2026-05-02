#pragma once 

#include <map>
#include <vector>
#include <glad/glad.h>

#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>

#include "Utility/Util.h"
#include "Math/3DMath_util.h"
#include "Rendering/Texture/Texture.h"
#include "ECS/WorldTransform.h"
#include "Rendering/Material/Material.h"
#include "Mestes/MeshCommon.h"

#define INVALID_MATERIAL 0xffffffff


class New_BasicMesh : public MeshCommon
{
public:
	//-------------------------------------------
	// Properties & Variables
	//-------------------------------------------

	//-------------------------------------------
	// Functions
	//-------------------------------------------
	New_BasicMesh() {};

	~New_BasicMesh();

	bool LoadMesh(const std::string& Filename, int AssimpFlags = ASSIMP_LOAD_FLAGS);

	void Render(IRenderCallbacks* pRenderCallbacks = NULL);

	void Render(uint DrawIndex, uint PrimID);

	void Render(uint NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats);

	const Material& GetMaterial();

	PBRMaterial& GetPBRMaterial() { return m_Materials[0].PBRmaterial; };

	void GetLeadingVertex(uint DrawIndex, uint PrimID, Vector3f& Vertex);

	void SetPBR(bool IsPBR) { m_isPBR = IsPBR; }

protected:
	//-------------------------------------------
	// Properties & Variables
	//-------------------------------------------

	void Clear();
	virtual void ReserveSpace(uint NumVertices, uint NumIndices);
	virtual void InitSingleMesh(uint MeshIndex, const aiMesh* paiMesh);
	virtual void InitSingleMeshOpt(uint MeshIndex, const aiMesh* paiMesh);
	virtual void PopulateBuffers();
	virtual void PopulateBuffersNonDSA();
	virtual void PopulateBuffersDSA();

	struct BasicMeshEntry {
		BasicMeshEntry()
		{
			NumIndices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = INVALID_MATERIAL;
		}

		uint NumIndices;
		uint BaseVertex;
		uint BaseIndex;
		uint MaterialIndex;
	};

	std::vector<BasicMeshEntry> m_Meshes;

	const aiScene* m_pScene;

	Matrix4f m_GlobalInverseTransform;

	vector<uint> m_Indices;

	enum BUFFER_TYPE {
		INDEX_BUFFER = 0,
		VERTEX_BUFFER = 1,
		WVP_MAT_BUFFER = 2,  // required only for instancing
		WORLD_MAT_BUFFER = 3,  // required only for instancing
		NUM_BUFFERS = 4
	};

	GLuint m_VAO = 0;

	GLuint m_Buffers[NUM_BUFFERS] = { 0 };

private: 
	//-------------------------------------------
	// Properties & Variables
	//-------------------------------------------

	//-------------------------------------------
	// Functions
	//-------------------------------------------
	struct Vertex {
		Vector3f Position;
		Vector2f TexCoords;
		Vector3f Normal;
	};

	bool InitFromScene(const aiScene* pScene, const std::string& Filename);

	void CountVerticesAndIndices(const aiScene* pScene, uint& NumVertices, uint& NumIndices);

	void InitAllMeshes(const aiScene* pScene);

	void OptimizeMesh(int MeshIndex, std::vector<uint>& Indices, std::vector<Vertex>& Vertices);

	bool InitMaterials(const aiScene* pScene, const std::string& Filename);

	void LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index);

	void LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int index);
	void LoadDiffuseTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
	void LoadDiffuseTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

	void LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int index);
	void LoadSpecularTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
	void LoadSpecularTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

	void LoadAlbedoTexture(const string& Dir, const aiMaterial* pMaterial, int index);
	void LoadAlbedoTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
	void LoadAlbedoTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

	void LoadMetalnessTexture(const string& Dir, const aiMaterial* pMaterial, int index);
	void LoadMetalnessTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
	void LoadMetalnessTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

	void LoadRoughnessTexture(const string& Dir, const aiMaterial* pMaterial, int index);
	void LoadRoughnessTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
	void LoadRoughnessTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

	void LoadColors(const aiMaterial* pMaterial, int index);

	void SetupRenderMaterialsPhong(unsigned int MeshIndex, unsigned int MaterialIndex, IRenderCallbacks* pRenderCallbacks);
	void SetupRenderMaterialsPBR();

	std::vector<Material> m_Materials;

	// Temporary space for vertex stuff before we load them into the GPU
	vector<Vertex> m_Vertices;

	Assimp::Importer m_Importer;

	bool m_isPBR = false;
};