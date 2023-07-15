#pragma once
#include <SFML\Graphics.hpp>

class Button {
private:
	sf::Sprite sprite;
	sf::Text text;
	float  scale;
	float character_size;

	void(*func)();

	float choose_multpl;
	float pressed_multpl;

	bool flip;
public:
	Button(sf::String s,const int& char_s, const sf::Font& f,const float& sz);
	Button(const sf::Texture& t,const float& sz);
	virtual void Draw(sf::RenderWindow& window, const float& deltaTime);
	bool Check(sf::RenderWindow& window, const bool& left_mouse_pressed, const bool& left_mouse_released, const bool& is_left_mouse_pressed=0);
	void SetTexture(const sf::Texture& t);
	void SetFunc(void(*f)());
	void SetPosition(const sf::Vector2f& v);
	sf::Text& GetTextRef();
	void setString(const sf::String& s);
	const float& GetScale()const;
	void Flip();
	const sf::Vector2f& GetPosition()const;
};