#pragma once

#include "Graphics/ResourceHolder.hpp"
#include "Graphics/ResourceLoader.hpp"
#include "Graphics/SFMLLoaders.hpp"
#include "Graphics/Resources.h"

#include "Graphics/SpriteAtlas.h"
#include "Graphics/SpriteBatch.h"

#include "Graphics/Components/IBatchable.h"
#include "Graphics/Components/GlobalTransformDirty.h"
#include "Graphics/Components/GlobalTransformMatrix.h"
#include "Graphics/Components/GlobalTransformMatrixInverse.h"
#include "Graphics/Components/GlobalTransformTranslation.h"
#include "Graphics/Components/GlobalTransformRotation.h"
#include "Graphics/Components/GlobalTransformScale.h"
#include "Graphics/Components/Transform.h"
#include "Graphics/Components/TransformMatrix.h"
#include "Graphics/Components/TransformMatrixInverse.h"
#include "Graphics/Components/Relation.h"
#include "Graphics/Components/Sprite.h"
#include "Graphics/Components/Animation.h"

#include "Graphics/Systems/CullingSystem.h"
#include "Graphics/Systems/RelationSystem.h"
#include "Graphics/Systems/RenderSystem.h"
#include "Graphics/Systems/TransformSystem.h"
#include "Graphics/Systems/AnimationSystem.h"