#pragma once

#include "../../scene/State.hpp"
#include "../../utilities/Time.hpp"

class StateLoading final : public vlx::State
{
public:
	bool HandleEvent(const sf::Event& event) override;
	bool Update(vlx::Time& time) override;
	void draw() override;



};