#pragma once

#include <vector>
#include "Rendering/Texture/bitmap.h"

int ConvertEquirectangularImageToCubemap(const Bitmap& b, std::vector<Bitmap>& Cubemap);