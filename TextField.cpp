#include "TextField.h"

#include <SFML\Graphics.hpp>
#include <iostream>

TextField::TextField(sf::String s, const int& char_s, const sf::Font& f, sf::FloatRect r, const float& sz, sf::Color c, int max_c) {
	isActive = 0;
	scale = sz;
	text.setString(s);
	character_size = char_s;
	text.setCharacterSize(character_size * sz);
	text.setFont(f);
	text.setFillColor(sf::Color::Black);
	func = nullptr;
	rect.setPosition({ r.left,r.top });
	text_pos = { 10,7 };
	text.setPosition({ r.left + text_pos.x,r.top + text_pos.y });
	rect.setSize({ r.width,r.height });
	rect.setFillColor(c);
	fill_c = c;
	rect.setOutlineColor(sf::Color::Black);
	clear_text = 1;
	max_chars = max_c;
}

void TextField::Draw(sf::RenderWindow& window) {
	window.draw(rect);
	window.draw(text);
}

void TextField::SetFunc(void(*f)()) {
	func = f;
}

bool TextField::Check(sf::RenderWindow& window, const bool& left_mouse_pressed, const bool& enter_pressed, const bool& is_left_mouse_pressed) {
	bool r = 0;
	if (!isActive) {
		if (rect.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))
			|| text.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
			sf::Color color_change = fill_c;
			color_change.r -= 2;
			color_change.b -= 2;
			color_change.g -= 2;
			rect.setFillColor(color_change);

			if (is_left_mouse_pressed) {
				sf::Color color_change = fill_c;
				color_change.r -= 6;
				color_change.b -= 6;
				color_change.g -= 6;
				rect.setFillColor(color_change);
			}

			if (left_mouse_pressed) {
				if (clear_text) {
					text.setString("");
				}
				isActive = !isActive;
			}
		}
		else if (!MouseOnIt(window)) {
			rect.setFillColor(fill_c);
		}
	}
	else {
		if (!(rect.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))
			|| text.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))))) {
			if (left_mouse_pressed) {
				isActive = !isActive;
			}
		}
		if (enter_pressed) {
			if (func != nullptr) {
				func();
			}
			r = 1;
		}
	}
	return r;
}

void TextField::SetPosition(const sf::Vector2f& v) {
	rect.setPosition(v);
	text.setPosition(v + text_pos);
}

sf::Text& TextField::GetTextRef() {
	return text;
}

void TextField::setString(const sf::String& s) {
	text.setString(s);
}

void TextField::PutChar(const wchar_t& c) {
	if (text.getString().getSize() < max_chars)
		text.setString(text.getString() + c);
}

void TextField::EraseChar() {
	if (text.getString().getSize() > 0)
		text.setString(text.getString().substring(0, text.getString().getSize() - 1));
}

void TextField::SetTextLocalPos(const sf::Vector2f& v) {
	text_pos = v;
	text.setPosition(text.getPosition() + text_pos);
}

bool TextField::MouseOnIt(sf::RenderWindow& window)const {
	return rect.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))
		|| text.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
}