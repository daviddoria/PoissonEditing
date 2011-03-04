#include "Types.h"

namespace Helpers
{

/**
 *  Clamps the input to [0..1].
 *  @return the clamped input
 */
float clamp01(float f);

void DeepCopy(FloatVectorImageType::Pointer input, FloatVectorImageType::Pointer output);

}