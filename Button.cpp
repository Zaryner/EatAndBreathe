#include "Button.h"

#include <SFML\Graphics.hpp>
#include <iostream>

Button::Button(sf::String s, const int& char_s, const sf::Font& f, const float& sz) {
	text.setString(s);
	text.setCharacterSize(char_s);
	text.setFont(f);

	scale = sz;
	character_size = char_s;

	func = nullptr;

	choose_multpl = 1.1f;
	pressed_multpl = 1.3f;
	flip = 0;
}

Button::Button(const sf::Texture& t, const float& sz) {
	sprite.setTexture(t);
	sprite.setOrigin(t.getSize().x / 2, t.getSize().y / 2);
	sprite.setScale({ sz,sz });
	scale = sz;

	func = nullptr;

	choose_multpl = 1.1f;
	pressed_multpl = 1.2f;
	flip = 0;
}

void Button::Draw(sf::RenderWindow& window) {
	window.draw(sprite);
	window.draw(text);
}

bool Button::Check(sf::RenderWindow& window, const bool& left_mouse_pressed, const bool& left_mouse_released, const bool& is_left_mouse_pressed) {
	bool r = 0;
	if (sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))
		|| text.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
		sprite.setScale({ scale * choose_multpl, scale * choose_multpl });
		if (flip)
			sprite.setScale({ -scale * choose_multpl, scale * choose_multpl });
		text.setCharacterSize(scale * character_size * scale * choose_multpl);
		if (left_mouse_pressed) {
			if (func != nullptr) {
				func();
			}
			r = 1;
		}
		if (is_left_mouse_pressed) {
			sprite.setScale({ scale * pressed_multpl , scale * pressed_multpl });
			if (flip)
				sprite.setScale({ -scale * pressed_multpl , scale * pressed_multpl });
			text.setCharacterSize(scale * character_size * scale * pressed_multpl);
		}
	}
	else {
		sprite.setScale({ scale , scale });
		if (flip)
			sprite.setScale({ -scale, scale });
		text.setCharacterSize(scale * character_size);
	}
	return r;
}

bool Button::MouseOnButton(sf::RenderWindow& window) {
	return sprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))
		|| text.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
}

void  Button::SetTexture(const sf::Texture& t) {
	sprite.setTexture(t);
}
void Button::SetFunc(void(*f)()) {
	func = f;
}
void Button::SetPosition(const sf::Vector2f& v) {
	text.setPosition(v);
	sprite.setPosition(v);
}
sf::Text& Button::GetTextRef() {
	return text;
}
void Button::setString(const sf::String& s) {
	text.setString(s);
}
const float& Button::GetScale()const {
	return scale;
}
void Button::Flip() {
	flip = 1;
}
const sf::Vector2f& Button::GetPosition()const {
	return sprite.getPosition();
}
void Button::SetRotation(float r) {
	sprite.setRotation(r);
	text.setRotation(r);
}

sf::Sprite& Button::GetSpriteRef() {
	return sprite;
}