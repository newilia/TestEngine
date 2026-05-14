#include "AttractiveBehaviour.h"

#include "AttractiveBehaviour.generated.hpp"
#include "Engine/Core/MainContext.h"
#include "Engine/Simulation/AttractionField.h"
#include "Engine/Simulation/PhysicsProcessor.h"

void AttractiveBehaviour::OnInit() {
	Behaviour::OnInit();

	const auto node = GetNode();
	if (!node) {
		return;
	}

	if (const auto self = node->FindBehaviour<AttractiveBehaviour>()) {
		if (const auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
			if (auto field = ph->GetAttractionField()) {
				field->Register(shared_from_this());
			}
		}
	}
}

void AttractiveBehaviour::OnDeinit() {
	Behaviour::OnDeinit();

	if (const auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		if (auto field = ph->GetAttractionField()) {
			field->Unregister(shared_from_this());
		}
	}
}

bool AttractiveBehaviour::IsEnabled() const {
	return _isEnabled;
}

void AttractiveBehaviour::SetEnabled(bool isEnabled) {
	_isEnabled = isEnabled;
}

float AttractiveBehaviour::GetAttraction() const {
	return _attraction;
}

void AttractiveBehaviour::SetAttraction(float value) {
	_attraction = value;
}

float AttractiveBehaviour::GetFalloffExponent() const {
	return _falloffExponent;
}

void AttractiveBehaviour::SetFalloffExponent(float value) {
	_falloffExponent = value;
}
