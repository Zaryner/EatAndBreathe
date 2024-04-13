#include <SFML/Graphics.hpp>
#include "Entities.h"
#include "EngineFuncs.h"

float nearest(float a, float b, float target) {
	if (target - a >= 0 && target - b >= 0)return std::max(a, b);
	else if (target - a <= 0 && target - b <= 0)return std::min(a, b);
	else if (std::abs(target - a) >= std::abs(target - b))return b;
	else return a;
}

float oppositeZoom(float a) {
	return 2 - a;
}

float Distance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
	return std::sqrtf(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2));
}
void MoveViewTo(sf::View& view, Entity& entity, float speed) {
	auto dist = Distance(view.getCenter(), entity.GetSprite().getPosition());
	if (dist > 0) {
		view.move(speed * (-view.getCenter().x + entity.GetSprite().getPosition().x), speed * (-view.getCenter().y + entity.GetSprite().getPosition().y));
	}
}
void MoveViewTo(sf::View& view, sf::Vector2f pos, float speed) {
	auto dist = Distance(view.getCenter(), pos);
	if (dist > 0) {
		view.move(speed * (-view.getCenter().x + pos.x), speed * (-view.getCenter().y + pos.y));
	}
}
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

std::string to_lower(std::string s) {
	for (auto& c : s) {
		c = std::tolower(c);
	}
	return s;
}

void RotateTo(sf::Sprite& s,float x, float y, float offset) {

	float dx = x - s.getPosition().x;
	float dy = y - s.getPosition().y;

	float rotation = atan2(dy, dx);
	rotation *= 180 / PI;

	s.setRotation(rotation + offset);
}
void RotateTo(sf::Sprite& s,const sf::Vector2f& p, float offset) {

	float dx = p.x - s.getPosition().x;
	float dy = p.y - s.getPosition().y;

	float rotation = atan2(dy, dx);
	rotation *= 180 / PI;

	s.setRotation(rotation + offset);
}