#include "PointLightBehaviour.h"

#include "Engine/Render/SceneLighting.h"
#include "PointLightBehaviour.generated.hpp"

void PointLightBehaviour::OnInit() {
	Behaviour::OnInit();
	Engine::SceneLighting::GetInstance().RegisterPointLight(shared_from_this());
}

void PointLightBehaviour::OnDeinit() {
	Behaviour::OnDeinit();
	Engine::SceneLighting::GetInstance().UnregisterPointLight(this);
}

void PointLightBehaviour::OnEnabled(bool isEnabled) {
	if (isEnabled) {
		Engine::SceneLighting::GetInstance().RegisterPointLight(shared_from_this());
	}
	else {
		Engine::SceneLighting::GetInstance().UnregisterPointLight(this);
	}
}

sf::Color PointLightBehaviour::GetLightColor() const {
	return _lightColor;
}

void PointLightBehaviour::SetLightColor(const sf::Color& color) {
	_lightColor = color;
}

float PointLightBehaviour::GetIntensity() const {
	return _intensity;
}

void PointLightBehaviour::SetIntensity(float value) {
	_intensity = value;
}

float PointLightBehaviour::GetRadius() const {
	return _radius;
}

void PointLightBehaviour::SetRadius(float value) {
	_radius = value;
}

bool PointLightBehaviour::IsLightEnabled() const {
	return _lightEnabled;
}

void PointLightBehaviour::SetLightEnabled(bool value) {
	_lightEnabled = value;
}
