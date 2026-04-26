#include "FpsCounterBehaviour.h"

#include "Engine/App/EngineInterface.h"
#include "Engine/Core/PropertyMeta.h"
#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/TextVisual.h"

#include <SFML/Graphics/Text.hpp>

#include <fmt/format.h>

#include <cassert>
#include <iterator>

std::shared_ptr<SceneNode> CreateFpsCounterNode() {
	auto node = std::make_shared<SceneNode>();
	node->SetName("Fps");

	EngineContext& engine = EngineContext::Instance();
	auto* font = engine.GetFontManager()->GetDefaultFont();
	assert(font);

	sf::Vector2f pos = {engine.GetMainWindow()->getSize().x - 50.f, 0.f};
	auto text = std::make_shared<sf::Text>(*font, "", 15);
	text->setFillColor(sf::Color::White);
	text->setPosition(pos);

	node->AddBehaviour(std::make_shared<FpsCounterBehaviour>(std::move(text)));
	return node;
}

FpsCounterBehaviour::FpsCounterBehaviour(std::shared_ptr<sf::Text> text) : _text(std::move(text)) {}

void FpsCounterBehaviour::OnAttached() {
	if (!_text) {
		return;
	}
	_text->setFillColor(_textColor);
	auto visual = std::make_shared<TextVisual>(_text);
	if (auto node = GetNode()) {
		node->SetVisual(std::move(visual));
	}
}

void FpsCounterBehaviour::OnUpdate(const sf::Time& dt) {
	(void)dt;
	auto frameTime = EngineContext::Instance().GetFrameDt(true);
	float newFps = 1.f / frameTime.asSeconds();
	if (_fps == 0.f) {
		_fps = newFps;
	}
	else {
		_fps = _fps * _smoothFactor + newFps * (1.f - _smoothFactor);
	}
	if (_text) {
		_text->setString(fmt::format("{:.0f}", _fps));
	}
}

void FpsCounterBehaviour::BuildPropertyTree(Engine::PropertyBuilder& b) {
	b.pushObject("fps_counter", "FpsCounterBehaviour");

	{
		Engine::PropertyMeta ro;
		ro.readOnly = true;
		ro.tooltip = "Smoothed frames per second (read-only).";
		b.addFloat("fps", "FPS", [this] { return _fps; }, [](float) {}, ro);
	}

	{
		Engine::PropertyMeta m;
		m.numericMin = 0.0;
		m.numericMax = 1.0;
		m.numericStep = 0.001;
		m.dragSpeed = 0.001f;
		m.tooltip = "Exponential smoothing factor for FPS display.";
		b.addFloat(
		    "smooth", "Smooth factor", [this] { return _smoothFactor; }, [this](float v) { _smoothFactor = v; }, m);
	}

	b.addVec2f(
	    "demo_offset", "Demo offset (px)", [this] { return _demoOffset; }, [this](sf::Vector2f v) { _demoOffset = v; });

	b.addVec3f("demo_vec3", "Demo Vec3", [this] { return _demoVec3; }, [this](sf::Vector3f v) { _demoVec3 = v; });

	b.addColor(
	    "text_color", "Text color", [this] { return _textColor; },
	    [this](sf::Color c) {
		    _textColor = c;
		    if (_text) {
			    _text->setFillColor(c);
		    }
	    });

	{
		Engine::PropertyMeta seqMeta;
		seqMeta.minElementCount = 1;
		seqMeta.maxElementCount = 16;
		seqMeta.tooltip = "Sample curve weights (editable list).";
		b.beginSequence("curve", "Curve samples",
		                Engine::PropAccessSequence{[this] { return _curveSamples.size(); },
		                                           [this](std::size_t n) {
			                                           if (n < 1) {
				                                           n = 1;
			                                           }
			                                           if (n > 16) {
				                                           n = 16;
			                                           }
			                                           _curveSamples.resize(n, 0.5f);
		                                           }},
		                seqMeta);
		for (std::size_t i = 0; i < _curveSamples.size(); ++i) {
			b.pushObject(fmt::format("s{}", i), fmt::format("[{}]", i));
			const std::size_t index = i;
			b.addFloat(
			    "w", "Weight", [this, index] { return _curveSamples[index]; },
			    [this, index](float v) { _curveSamples[index] = v; });
			b.pop();
		}
		b.endSequence();
	}

	{
		Engine::PropertyMeta mapMeta;
		mapMeta.tooltip = "Example key/value table (string → float).";
		b.beginAssociative("stats", "Demo stats (map)",
		                   Engine::PropAccessAssociative{[this] {
			                                                 std::size_t n = _demoStats.size();
			                                                 std::string k = fmt::format("stat{}", n);
			                                                 while (_demoStats.contains(k)) {
				                                                 k.push_back('_');
			                                                 }
			                                                 _demoStats[k] = 0.f;
		                                                 },
		                                                 [this](std::size_t pairIndex) {
			                                                 if (pairIndex >= _demoStats.size()) {
				                                                 return;
			                                                 }
			                                                 auto it = _demoStats.begin();
			                                                 std::advance(it, static_cast<std::ptrdiff_t>(pairIndex));
			                                                 _demoStats.erase(it);
		                                                 }},
		                   mapMeta);
		std::size_t idx = 0;
		for (auto it = _demoStats.begin(); it != _demoStats.end(); ++it, ++idx) {
			const std::string keyCopy = it->first;
			b.pushObject(fmt::format("pair{}", idx), keyCopy);
			{
				Engine::PropertyMeta keyMeta;
				keyMeta.readOnly = true;
				b.addString("key", "Key", [keyCopy] { return keyCopy; }, [](std::string) {}, keyMeta);
			}
			b.addFloat(
			    "value", "Value", [this, keyCopy] { return _demoStats.at(keyCopy); },
			    [this, keyCopy](float v) { _demoStats[keyCopy] = v; });
			b.pop();
		}
		b.endAssociative();
	}

	b.pop();
}
