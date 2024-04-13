#pragma once
#include <SFML\Graphics.hpp>

class TextField {
private:
	//sf::Sprite sprite;
	sf::RectangleShape rect;
	sf::Color fill_c;
	sf::Text text;
	float  scale;
	float character_size;

	void(*func)();
	bool clear_text;

	sf::Vector2f text_pos;

public:
	int max_chars;
	bool isActive;

	TextField(sf::String s, const int& char_s, const sf::Font& f, sf::FloatRect r = { 0,0,200,80 }, const float& sz = 1, sf::Color c = {55,55,55},int max_c=15);
	virtual void Draw(sf::RenderWindow& window);
	void SetFunc(void(*f)());
	bool Check(sf::RenderWindow& window, const bool& left_mouse_pressed,const bool& enter_pressed, const bool& is_left_mouse_pressed);
	void SetPosition(const sf::Vector2f& v);
	sf::Text& GetTextRef();
	void setString(const sf::String& s);
	void PutChar(const wchar_t& c);
	void EraseChar();
	void SetTextLocalPos(const sf::Vector2f& v);
	bool MouseOnIt(sf::RenderWindow& window)const;
};