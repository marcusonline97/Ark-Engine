#include "Utility/Technique.h"
#include "Math/3DMath_util.h"

class GeomPassTech : public Technique {
public:

    GeomPassTech();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);

private:

    GLuint m_WVPLocation;
};