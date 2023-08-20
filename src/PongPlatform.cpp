#include "PongPlatform.h"
#include "Utils.h"

PongPlatform::PongPlatform() {
	mMoveArea = {
	   std::numeric_limits<float>::min(),
	   std::numeric_limits<float>::min(),
	   std::numeric_limits<float>::max(),
	   std::numeric_limits<float>::max()
	};
}

void PongPlatform::update(const sf::Time& dt) {
	sf::Vector2f frictionAccel = -mSpeed * mAirFriction;
	auto totalAccel = mPlayerAccel + frictionAccel;
	mSpeed += totalAccel * dt.asSeconds();
	mSpeed.x = std::clamp(mSpeed.x, -mMaxSpeed, mMaxSpeed);

	if (utils::length(mSpeed) < 10 && mPlayerAccel == sf::Vector2f()) {
		mSpeed = { 0, 0 };
	}
	
	auto pos = mShape.getPosition() + mSpeed * dt.asSeconds();
	const auto& size = mShape.getSize();
	pos.x = std::clamp(pos.x, mMoveArea.left, mMoveArea.left + mMoveArea.width - size.x);
	pos.y = std::clamp(pos.y, mMoveArea.top, mMoveArea.top + mMoveArea.height - size.y);
	mShape.setPosition(pos);
}

void PongPlatform::handleInput(const sf::Event& event) {
	if (event.type == sf::Event::KeyPressed)
	{
		switch (event.key.code)
		{
		case sf::Keyboard::Left:
			mPlayerAccel = { -mPlayerAccelMagnitude, 0 };
			break;
		case sf::Keyboard::Right:
			mPlayerAccel = { mPlayerAccelMagnitude, 0 };
			break;
		default: break;
		}
	}
	else if (event.type == sf::Event::KeyReleased) {
		switch (event.key.code) {
		case sf::Keyboard::Left:
		case sf::Keyboard::Right:
			mPlayerAccel = { 0, 0 };
			break;
		default: break;
		}
	}
}
