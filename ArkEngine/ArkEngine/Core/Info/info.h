
//This is a Read Only header with just useful info 


/*
#define GLM_FORCE_RADIANS //vulkan specific
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp> //It seems like my vulkan SDK actually come prepacked with glm for some reason
//And my setup seems to wanna link to both my Vulkan SDK and my local GLM inside of the project so be mindful of the pathing
*/

// Tons of shit will break and require a lot of try catching, logging and ensuring things work. Ofcourse it will 98% of the time
// compile but there may be a likely hood that something could occur during runtime. So running a lot of print logs and
// error catching is essential to make vulkan work.