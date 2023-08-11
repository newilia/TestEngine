#include "Ball.h"

void Ball::update(const sf::Time& dt) {
	auto pos = mShape.getPosition();
	pos += mSpeed * dt.asSeconds();
	mShape.setPosition(pos);

	const auto bbox = mShape.getGlobalBounds();
	if (float left = bbox.left - mMoveArea.left; left < 0) {
		pos.x -= left * 2;
		mSpeed.x = -mSpeed.x;
	}
	else if (float right = bbox.left + bbox.width - (mMoveArea.left + mMoveArea.width); right > 0) {
		pos.x -= right * 2;
		mSpeed.x = -mSpeed.x;
	}
	if (float top = bbox.top - mMoveArea.top; top < 0) {
		pos.y -= top * 2;
		mSpeed.y = -mSpeed.y;
	}
	else if (float bottom = bbox.top + bbox.height - (mMoveArea.top + mMoveArea.height); bottom > 0) {
		pos.y -= bottom * 2;
		mSpeed.y = -mSpeed.y;
	}
	mShape.setPosition(pos);
}

void Ball::setRadius(float r) {
	mShape.setRadius(r);
	mShape.setOrigin(r, r);
}
