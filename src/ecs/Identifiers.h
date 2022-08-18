#pragma once

#include <cstdint>
#include <vector>

namespace fge
{
	using IDType = std::uint32_t;

	using EntityID = IDType;
	using ComponentTypeID = IDType;
	using ArchetypeID = std::vector<ComponentTypeID>;

	const IDType NULL_ENTITY = NULL;
}