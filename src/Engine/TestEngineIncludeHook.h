#pragma once

// MSVC: applied first in every TU (/FI). Maps legacy includes to the fixed header; maps old API name for BodyPullHandler.
#define getAllBodies GetAllBodies

#pragma include_alias("AbstractShapeBody.h", "AbstractShapeBodyFixed.h")
#pragma include_alias("Engine/Physics/AbstractShapeBody.h", "Engine/Physics/AbstractShapeBodyFixed.h")
