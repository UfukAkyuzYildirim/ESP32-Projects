#pragma once

#include "ImuSystem.h"
#include "RadioSystem.h"

namespace ImuTest {
bool setup(ImuSystem &imuSystem, RadioSystem &radio);
bool loop(ImuSystem &imuSystem, RadioSystem &radio);
}
