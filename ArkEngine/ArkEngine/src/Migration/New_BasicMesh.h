#include <Assimp/scene.h>
#include <Assimp/postprocess.h>

#include "Additional_Util.h"
#include "Math/3DMath_util.h"
#include "New_Texture.h"
#include "New_WorldTransform.h"

class New_BasicMesh
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

	bool LoadMesh(const std::string& Filename);

	void Render();

	void Render(unsigned int NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats);


	WorldTrans& GetWorldTransform() { return m_worldTransform; }

protected:
	//-------------------------------------------
	// Properties & Variables
	//-------------------------------------------

	//-------------------------------------------
	// Functions
	//-------------------------------------------
private: 
	//-------------------------------------------
	// Properties & Variables
	//-------------------------------------------

	//-------------------------------------------
	// Functions
	//-------------------------------------------
};