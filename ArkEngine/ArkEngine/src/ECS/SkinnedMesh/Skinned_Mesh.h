#include <unordered_map>
#include <vector>
#include <glad/glad.h>

#include "Utility/Util.h"
#include "Math/3DMath_util.h"
#include "Rendering/Texture/Texture.h"
#include "Math/3DMath_util.h"
#include "Rendering/Material/Material.h"
#include "ECS/Common/Basic_Mesh.h"

class SkinnedMesh : public BasicMesh
{
public:
    SkinnedMesh() {};

    ~SkinnedMesh();

    uint NumBones() const
    {
        return (uint)m_BoneNameToIndexMap.size();
    }

    // This is the main function to drive the animation. It receives the animation time
    // in seconds and a reference to a vector of transformation matrices (one matrix per bone).
    // It calculates the current transformation for each bone according to the current time
    // and updates the corresponding matrix in the vector. This must then be updated in the VS
    // to be accumulated for the final local position (see skinning.vs). The animation index
    // is an optional param which selects one of the animations.
    void GetBoneTransforms(float AnimationTimeSec, vector<Matrix4f>& Transforms, unsigned int AnimationIndex = 0);

    // Same as above but this one blends two animations together based on a blending factor
    void GetBoneTransformsBlended(float AnimationTimeSec,
        vector<Matrix4f>& Transforms,
        unsigned int StartAnimIndex,
        unsigned int EndAnimIndex,
        float BlendFactor);
private:
#define MAX_NUM_BONES_PER_VERTEX 4

    virtual void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices);

    virtual void InitSingleMesh(uint MeshIndex, const aiMesh* paiMesh);

    struct VertexBoneData
    {
        uint BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
        float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };
        int index = 0;  // slot for the next update

        VertexBoneData()
        {
        }

        void AddBoneData(uint BoneID, float Weight)
        {
            for (int i = 0; i < index; i++) {
                if (BoneIDs[i] == BoneID) {
                    //  printf("bone %d already found at index %d old weight %f new weight %f\n", BoneID, i, Weights[i], Weight);
                    return;
                }
            }

            // The iClone 7 Raptoid Mascot (https://sketchfab.com/3d-models/iclone-7-raptoid-mascot-free-download-56a3e10a73924843949ae7a9800c97c7)
            // has a problem of zero weights causing an overflow and the assertion below. This fixes it.
            if (Weight == 0.0f) {
                return;
            }

            // printf("Adding bone %d weight %f at index %i\n", BoneID, Weight, index);

            if (index == MAX_NUM_BONES_PER_VERTEX) {
                // Preserve the most significant influences instead of silently dropping
                // one when a vertex has more than MAX_NUM_BONES_PER_VERTEX weights.
                int minWeightIndex = 0;
                for (int i = 1; i < MAX_NUM_BONES_PER_VERTEX; i++) {
                    if (Weights[i] < Weights[minWeightIndex]) {
                        minWeightIndex = i;
                    }
                }

                if (Weight > Weights[minWeightIndex]) {
                    BoneIDs[minWeightIndex] = BoneID;
                    Weights[minWeightIndex] = Weight;
                }

                assert(0);
                return;
            }

            BoneIDs[index] = BoneID;
            Weights[index] = Weight;

            index++;
        }
    };

    struct SkinnedVertex {
        Vector3f Position;
        Vector2f TexCoords;
        Vector3f Normal;
        VertexBoneData Bones;
    };

    virtual void InitSingleMeshOpt(uint MeshIndex, const aiMesh* paiMesh);
    void OptimizeMesh(int MeshIndex, std::vector<uint>& Indices, std::vector<SkinnedVertex>& Vertices);

    virtual void PopulateBuffers();
    void PopulateBuffersNonDSA();
    void PopulateBuffersDSA();

    void LoadMeshBones(uint MeshIndex, const aiMesh* paiMesh, vector<SkinnedVertex>& SkinnedVertices, int BaseVertex);
    void LoadSingleBone(uint MeshIndex, const aiBone* pBone, vector<SkinnedVertex>& SkinnedVertices, int BaseVertex);
    int GetBoneId(const aiBone* pBone);
    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
    uint FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
    const aiNodeAnim* FindNodeAnim(const aiAnimation& Animation, const string& NodeName);
    void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const Matrix4f& ParentTransform, const aiAnimation& Animation);
    void ReadNodeHierarchyBlended(float StartAnimationTimeTicksm, float EndAnimationTimeTicks, const aiNode* pNode, const Matrix4f& ParentTransform,
        const aiAnimation& StartAnimation, const aiAnimation& EndAnimation, float BlendFactor);
    void MarkRequiredNodesForBone(const aiBone* pBone);
    void InitializeRequiredNodeMap(const aiNode* pNode);
    float CalcAnimationTimeTicks(float TimeInSeconds, unsigned int AnimationIndex);

    struct LocalTransform {
        aiVector3D Scaling;
        aiQuaternion Rotation;
        aiVector3D Translation;
    };

    void CalcLocalTransform(LocalTransform& Transform, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);

    vector<SkinnedVertex> m_SkinnedVertices;

    GLuint m_boneBuffer = 0;

    std::unordered_map<string, uint> m_BoneNameToIndexMap;

    struct BoneInfo
    {
        Matrix4f OffsetMatrix;
        Matrix4f FinalTransformation;

        BoneInfo(const Matrix4f& Offset)
        {
            OffsetMatrix = Offset;
            FinalTransformation.SetZero();
        }
    };

    vector<BoneInfo> m_BoneInfo;

    struct NodeInfo {

        NodeInfo() {}

        NodeInfo(const aiNode* n) { pNode = n; }

        const aiNode* pNode = NULL;
        bool isRequired = false;
    };

    std::unordered_map<string, NodeInfo> m_requiredNodeMap;

    // Per-animation lookup to avoid O(numChannels) string scan for each queried node.
    std::vector<std::unordered_map<std::string, const aiNodeAnim*>> m_animationNodeMap;
    std::unordered_map<const aiAnimation*, uint32_t> m_animationToIndex;
};