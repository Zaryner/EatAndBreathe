#include <iostream>
#include <vector>
#include <list>
#include <ctime>
#include <thread>
#include <cmath>
#include <fstream>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include "EngineFuncs.h"
#include "Entities.h"
#include "Resources.h"
#include "Button.h"

#define PI 3.1415926535

int main();
int main_menu();
int main_game();

sf::RenderWindow window;

struct settings {
	bool show_fps;
	bool show_ui;
	bool full_screen;
	sf::Vector2i screen_resolution;
	int frame_limit;
	unsigned short language;
	int draw_cell_distance;
	float draw_obj_distance;
	float draw_obj_time;
	int player_num = 0;

	settings() {
		show_fps = 0;
		full_screen = 0;
		show_ui = 1;
		screen_resolution = { 1280, 720 };
		frame_limit = 9999;
		language = 0;
		draw_cell_distance = 7;
		draw_obj_distance = 1000;
		draw_obj_time = 0.1f;
	}
};

namespace states {
	static bool resources_loaded = 0;
	static bool free_vision = 0;
	static bool locked_vision = 0;
	static float locked_vision_zoom = 1;

	bool left_mouse_pressed = 0;
	bool left_mouse_released = 0;
	bool right_mouse_pressed = 0;
	bool right_mouse_released = 0;

	int choosen_item_pos = -1;

	bool item_throw_confirmation = 0;
	Item* throwing_item;

	bool player_on_water = 0;
	bool mouse_on_active_ui = 0;
}

namespace enable_to_change {
	static bool show_fps = 1;
	static bool show_ui = 1;
	static bool full_screen = 1;
	static bool move_view = 1;
	static bool show_colliders = 1;
};

float ScreenScaler = 1;
float ScreenScalerX = 1;
float ScreenScalerY = 1;

int cell_size = 128;

void DrawActiveBuilding(sf::RenderWindow& window, const settings& my_settings, Player& player, std::vector<MapActiveObject*>& near_mao, const float& delta_time) {
	if (player.ActiveItem() == nullptr ||
		(typeid(*player.ActiveItem()->item) != typeid(BuildItem)))return;

	int entity_id = static_cast<BuildItem const*>(player.ActiveItem()->item)->GetEntityId();
	MapActiveObject* b;
	if (typeid(*base_entity[entity_id]) == typeid(SpikyBuilding)) {
		b = new SpikyBuilding(*static_cast<SpikyBuilding*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(Bonfire)) {
		b = new Bonfire(*static_cast<Bonfire*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(Building)) {
		b = new Building(*static_cast<Building*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(MaterialSource)) {
		b = new MaterialSource(*static_cast<MaterialSource*>(base_entity[entity_id]));
	}
	else {
		b = new Building(*static_cast<Building*>(base_entity[entity_id]));
		std::cout << "Invalid build item class(Pre drawing)\n";
	}
	float px = player.GetPosition().x, py = player.GetPosition().y;
	px = px + 100 * std::cos(player.GetRotation() * PI / 180);
	py = py + 100 * std::sin(player.GetRotation() * PI / 180);
	b->Transform({ px,py }, base_entity[entity_id]->GetSize());
	b->GetSpriteRef().setColor(sf::Color(255, 255, 255, 150));
	for (auto& obj : near_mao) {
		if (obj->DetectCollision(*b)) b->GetSpriteRef().setColor(sf::Color(255, 55, 55, 150));
	}
	b->Draw(window, delta_time);

}

void DrawMap(sf::RenderWindow& window, const std::vector<std::vector<std::vector<unsigned short>>>& current_map, Player& player, std::vector<MapActiveObject*>& activeObjects, std::vector<Entity*>& drawObjects, const float& delta_time, const settings& my_settings) {
	sf::Vector2i map_cell_player_pos = { (int)((player.GetPosition().x - map_start_pos.x) / (cell_size - 2)),(int)((player.GetPosition().y - map_start_pos.y) / (cell_size - 2)) };
	map_cell_player_pos.x -= my_settings.draw_cell_distance;
	map_cell_player_pos.y -= my_settings.draw_cell_distance;

	if (map_cell_player_pos.x < 0)map_cell_player_pos.x = 0;
	if (map_cell_player_pos.y < 0)map_cell_player_pos.y = 0;

	for (int l = 0; l < current_map.size(); l++) {
		for (int i = map_cell_player_pos.y; i < current_map[l].size() && i < map_cell_player_pos.y + my_settings.draw_cell_distance * 2; i++) {
			for (int j = map_cell_player_pos.x; j < current_map[l][i].size() && j < map_cell_player_pos.x + my_settings.draw_cell_distance * 2; j++) {
				sf::Sprite cell_sprite(cell_textures[current_map[l][i][j]]);
				cell_sprite.setScale(1, 1);
				cell_sprite.setPosition(map_start_pos.x + j * (cell_size - 2), map_start_pos.y + i * (cell_size - 2));
				window.draw(cell_sprite);
			}
		}
	}
	player.Draw(window, delta_time);
	for (auto& object : drawObjects) {
		object->Draw(window, delta_time);
	}
	for (auto& object : activeObjects) {
		object->Draw(window, delta_time);
	}
	DrawActiveBuilding(window, my_settings, player, activeObjects, delta_time);
}
sf::Color active_inv_color(166, 120, 78, 180);
int active_item_number = -1;

void ItemThrowConfirmation(sf::RenderWindow& window, const settings& my_settings, const int& dt) {

	sf::Text confirm;
	confirm.setLetterSpacing(2);
	confirm.setCharacterSize(40 * ScreenScaler);
	confirm.setFont(you2013);
	confirm.setString("Are you sure you want to discard this item?");
	if (my_settings.language == 0)
		confirm.setString(L"Вы уверены что хотите выбросить\nэтот предмет?");
	else if (my_settings.language == 2)
		confirm.setString(L"Вы упэунены, што хочаце выкинуць\nгэты прадмет?");
	confirm.setOutlineColor(sf::Color::Black);
	confirm.setOutlineThickness(5 * ScreenScaler);
	confirm.setFillColor(sf::Color(230, 230, 230, 245));
	confirm.setPosition({ window.getView().getCenter().x - 380 * ScreenScalerX ,window.getView().getCenter().y - 100 * ScreenScalerY });
	window.draw(confirm);

	Button ok1("Throw one", 36 * ScreenScaler, you2013, 1);
	if (my_settings.language == 0)
		ok1.setString(L"Выбросить один");
	else if (my_settings.language == 2)
		ok1.setString(L"Выкинуць адзин");
	ok1.GetTextRef().setLetterSpacing(2);
	ok1.GetTextRef().setFillColor(sf::Color(220, 220, 140, 245));
	ok1.GetTextRef().setOutlineColor(sf::Color::Black);
	ok1.GetTextRef().setOutlineThickness(5 * ScreenScaler);
	ok1.SetPosition({ window.getView().getCenter().x + 50 * ScreenScalerX ,window.getView().getCenter().y + 70 * ScreenScalerY });
	ok1.SetFunc([]() {states::throwing_item->count--; if (!states::throwing_item->count)states::throwing_item->Reset(); states::item_throw_confirmation = 0; });
	ok1.Check(window, states::left_mouse_pressed, states::left_mouse_released);
	ok1.Draw(window, dt);

	Button ok2("Throw all", 36 * ScreenScaler, you2013, 1);
	if (my_settings.language == 0)
		ok2.setString(L"Выбросить все");
	else if (my_settings.language == 2)
		ok2.setString(L"Выкинуць усе");
	ok2.GetTextRef().setLetterSpacing(2);
	ok2.GetTextRef().setFillColor(sf::Color(250, 210, 210, 245));
	ok2.GetTextRef().setOutlineColor(sf::Color::Black);
	ok2.GetTextRef().setOutlineThickness(5 * ScreenScaler);
	ok2.SetPosition({ window.getView().getCenter().x + 50 * ScreenScalerX ,window.getView().getCenter().y + 120 * ScreenScalerY });
	ok2.SetFunc([]() {states::throwing_item->Reset(); states::item_throw_confirmation = 0; });
	ok2.Check(window, states::left_mouse_pressed, states::left_mouse_released);
	ok2.Draw(window, dt);

	Button cancel("Cancel", 36 * ScreenScaler, you2013, 1);
	if (my_settings.language == 0)
		cancel.setString(L"Отмена");
	else if (my_settings.language == 2)
		cancel.setString(L"Адмена");
	cancel.GetTextRef().setLetterSpacing(2);
	cancel.GetTextRef().setFillColor(sf::Color(220, 220, 210, 245));
	cancel.GetTextRef().setOutlineColor(sf::Color::Black);
	cancel.GetTextRef().setOutlineThickness(5 * ScreenScaler);
	cancel.SetPosition({ window.getView().getCenter().x - 290 * ScreenScalerX ,window.getView().getCenter().y + 90 * ScreenScalerY });
	cancel.SetFunc([]() {states::item_throw_confirmation = 0; });
	cancel.Check(window, states::left_mouse_pressed, states::left_mouse_released);
	cancel.Draw(window, dt);
}

void DrawRecipes(sf::RenderWindow& window, const settings& my_settings, const int& fps, Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {

	int available_recipes = 0;
	for (int i = 0; i < recipes.size(); i++) {
		if (recipes[i] != nullptr && recipes[i]->Available(player)) { available_recipes++; }
	}

	std::vector<sf::Sprite> cell(available_recipes, sf::Sprite(inventory_cell_texture[3]));
	std::vector<sf::Sprite>recipe_sprite;
	sf::Sprite descrSp(inventory_cell_texture[2]);
	descrSp.setScale(0.35 * ScreenScaler, 0.185 * ScreenScaler);
	descrSp.setColor(sf::Color(5, 5, 5, 200));
	sf::Text item_description, item_name;

	item_description.setFillColor(sf::Color(240, 210, 210, 253));
	item_description.setCharacterSize(14 * ScreenScaler);
	item_description.setFont(fregat);
	item_description.setStyle(sf::Text::Bold);
	item_description.setLetterSpacing(1);

	item_name.setFillColor(sf::Color(235, 160, 87, 253));
	item_name.setCharacterSize(16 * ScreenScaler);
	item_name.setFont(comfortaa);
	item_name.setStyle(sf::Text::Bold);

	sf::Vector2f cell_pos = { window.getView().getCenter().x - 600 * ScreenScalerX, window.getView().getCenter().y - 350 * ScreenScalerY };
	int xpos = 0, ypos = 0;

	for (int i = 0; i < available_recipes; i++) {
		cell[i].setPosition(cell_pos.x + 100 * xpos * ScreenScaler, cell_pos.y + 100 * ypos * ScreenScaler);
		cell[i].setColor(sf::Color(0, 0, 0, 90));
		cell[i].setScale(0.11 * ScreenScaler, 0.11 * ScreenScaler);

		xpos++;
		if (xpos >= 5) {
			xpos = 0;
			ypos++;
		}
	}
	int mouse_on_cell = -1, recipe = 0;
	for (int i = 0, j = 0; i < recipes.size(); i++) {
		if (recipes[i] != nullptr && recipes[i]->Available(player)) {

			if (cell[j].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
				mouse_on_cell = j;
				recipe = i;
				states::mouse_on_active_ui = 1;
			}

			sf::Sprite s(recipes[i]->GetItem()->GetTexture());
			s.setOrigin(s.getTexture()->getSize().x / 2, s.getTexture()->getSize().y / 2);
			s.setPosition(cell[j].getPosition().x + cell[j].getTexture()->getSize().x / 2 * cell[j].getScale().x,
				cell[j].getPosition().y + cell[j].getTexture()->getSize().y / 2 * cell[j].getScale().y);
			s.setScale(cell[j].getScale().x * 3.5 / recipes[i]->GetItem()->GetTexture().getSize().x * 150,
				cell[j].getScale().y * 3.5 / recipes[i]->GetItem()->GetTexture().getSize().y * 150);
			recipe_sprite.push_back(s);
			j++;
		}
	}

	if (mouse_on_cell != -1) {

		cell[mouse_on_cell].setScale(0.12 * ScreenScaler, 0.12 * ScreenScaler);
		cell[mouse_on_cell].setColor(sf::Color(0, 0, 0, 120));
		cell[mouse_on_cell].move(-3, -3);

		if (my_settings.language == 0) {
			item_description.setString(sf::String::fromUtf8(recipes[recipe]->GetItem()->GetRuDescription().begin(), recipes[recipe]->GetItem()->GetRuDescription().end()));
			item_name.setString(sf::String::fromUtf8(recipes[recipe]->GetItem()->GetRuName().begin(), recipes[recipe]->GetItem()->GetRuName().end()));
		}
		else if (my_settings.language == 1) {
			item_description.setString(recipes[recipe]->GetItem()->GetEngDescription());
			item_name.setString(recipes[recipe]->GetItem()->GetEngName());
		}
		else if (my_settings.language == 2) {
			item_description.setString(sf::String::fromUtf8(recipes[recipe]->GetItem()->GetBelDescription().begin(), recipes[recipe]->GetItem()->GetBelDescription().end()));
			item_name.setString(sf::String::fromUtf8(recipes[recipe]->GetItem()->GetBelName().begin(), recipes[recipe]->GetItem()->GetBelName().end()));
		}
		descrSp.setPosition(cell[mouse_on_cell].getPosition().x + 55 * ScreenScaler, cell[mouse_on_cell].getPosition().y + 120 * ScreenScaler);
		window.draw(descrSp);
		item_description.setPosition(descrSp.getPosition().x + 5 * ScreenScaler, descrSp.getPosition().y + 40 * ScreenScaler);
		window.draw(item_description);
		item_name.setPosition(descrSp.getPosition().x + 5 * ScreenScaler, descrSp.getPosition().y + 5 * ScreenScaler);
		window.draw(item_name);

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			cell[mouse_on_cell].setScale(0.125 * ScreenScaler, 0.125 * ScreenScaler);
			cell[mouse_on_cell].setColor(sf::Color(0, 0, 0, 125));
			cell[mouse_on_cell].move(-1, -1);

			//recipe_sprite[mouse_on_cell].setScale(cell[mouse_on_cell].getScale().x * 3.5 / recipes[recipe]->GetItem()->GetTexture().getSize().x * 151,
			//	cell[mouse_on_cell].getScale().y * 3.5 / recipes[recipe]->GetItem()->GetTexture().getSize().y * 151);

		}
		if (states::left_mouse_released)
		{
			player.Craft(recipes[recipe]);
		}
	}

	for (int i = 0; i < available_recipes; i++) {
		window.draw(cell[i]);
		window.draw(recipe_sprite[i]);
	}

}

void DrawInventory(sf::RenderWindow& window, const settings& my_settings, const int& fps, Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {

	std::vector<sf::Sprite> inventory_cell(player.InventoryCapacity(), sf::Sprite(inventory_cell_texture[3]));
	sf::Sprite descrSp(inventory_cell_texture[2]);
	descrSp.setScale(0.35 * ScreenScaler, 0.185 * ScreenScaler);
	descrSp.setColor(sf::Color(5, 5, 5, 200));
	sf::Text item_count, item_description, item_name;

	item_count.setFillColor(sf::Color::White);
	item_count.setCharacterSize(20 * ScreenScaler);
	item_count.setFont(arkhip);
	item_count.setPosition(720, 5);

	item_description.setFillColor(sf::Color(240, 210, 210, 253));
	item_description.setCharacterSize(14 * ScreenScaler);
	item_description.setFont(fregat);
	item_description.setStyle(sf::Text::Bold);
	item_description.setLetterSpacing(1);

	item_name.setFillColor(sf::Color(235, 160, 87, 253));
	item_name.setCharacterSize(16 * ScreenScaler);
	item_name.setFont(comfortaa);
	item_name.setStyle(sf::Text::Bold);

	sf::Vector2f inv_cell_pos = { window.getView().getCenter().x - 350 * ScreenScalerX, window.getView().getCenter().y + 270 * ScreenScalerY };
	for (int i = 0; i < player.InventoryCapacity(); i++) {
		inventory_cell[i].setPosition(inv_cell_pos.x + 100 * i * ScreenScaler, inv_cell_pos.y);
	}
	for (int i = 0; i < player.InventoryCapacity(); i++) {
		inventory_cell[i].setColor(sf::Color(0, 0, 0, 90));
		if (!player.GetItem(i).item->GetId())
			inventory_cell[i].setScale(0.115 * ScreenScaler, 0.115 * ScreenScaler);
		else {
			inventory_cell[i].setScale(0.11 * ScreenScaler, 0.11 * ScreenScaler);
			item_count.setString(std::to_string(player.GetItemCount(i)));

			if (my_settings.language == 0) {
				item_description.setString(sf::String::fromUtf8(player.GetItem(i).item->GetRuDescription().begin(), player.GetItem(i).item->GetRuDescription().end()));
				item_name.setString(sf::String::fromUtf8(player.GetItem(i).item->GetRuName().begin(), player.GetItem(i).item->GetRuName().end()));
			}
			else if (my_settings.language == 1) {
				item_description.setString(player.GetItem(i).item->GetEngDescription());
				item_name.setString(player.GetItem(i).item->GetEngName());
			}
			else if (my_settings.language == 2) {
				item_description.setString(sf::String::fromUtf8(player.GetItem(i).item->GetBelDescription().begin(), player.GetItem(i).item->GetBelDescription().end()));
				item_name.setString(sf::String::fromUtf8(player.GetItem(i).item->GetBelName().begin(), player.GetItem(i).item->GetBelName().end()));
			}
		}

		if (inventory_cell[i].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
			inventory_cell[i].setScale(0.12 * ScreenScaler, 0.12 * ScreenScaler);
			if (player.GetItem(i).item->GetId())
				inventory_cell[i].setColor(sf::Color(0, 0, 0, 60));
			else
				inventory_cell[i].setColor(sf::Color(0, 0, 0, 120));
			inventory_cell[i].move(-3, -3);

			if (states::left_mouse_pressed) {
				if (active_item_number == i) {
					active_item_number = -1;
					player.ChangeActiveItem();
				}
				else if (player.GetItem(i).item->isActivated()) {
					active_item_number = i;
					player.ChangeActiveItem(player.GetItem(i));
				}
				else {
					player.UseItem(i, near_mao, mao);
				}
			}
			if (states::right_mouse_pressed && player.GetItem(i).item->GetId()) {
				if (player.GetItem(i).item->GetId()) {
					states::choosen_item_pos = i;
				}
			}

		}

		if (active_item_number == i && player.ActiveItem() != nullptr)
			inventory_cell[i].setColor(active_inv_color);

		sf::Sprite s(player.GetItem(i).item->GetTexture());
		s.setOrigin(s.getTexture()->getSize().x / 2, s.getTexture()->getSize().y / 2);
		s.setPosition(inventory_cell[i].getPosition().x + inventory_cell[i].getTexture()->getSize().x / 2 * inventory_cell[i].getScale().x,
			inventory_cell[i].getPosition().y + inventory_cell[i].getTexture()->getSize().y / 2 * inventory_cell[i].getScale().y);
		if (states::choosen_item_pos == i) {
			s.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
		}
		s.setScale(inventory_cell[i].getScale().x * 3.5 / player.GetItem(i).item->GetTexture().getSize().x * 150,
			inventory_cell[i].getScale().y * 3.5 / player.GetItem(i).item->GetTexture().getSize().y * 150);

		item_count.setPosition(inventory_cell[i].getPosition().x + 4 * ScreenScaler, inventory_cell[i].getPosition().y + 40 * ScreenScaler);

		window.draw(inventory_cell[i]);
		window.draw(s);
		if (player.GetItem(i).item->GetId()) {
			if (item_count.getString() != "1")
				window.draw(item_count);
			if (inventory_cell[i].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
				descrSp.setPosition(inventory_cell[i].getPosition().x - 55 * ScreenScaler, inventory_cell[i].getPosition().y - 120 * ScreenScaler);
				window.draw(descrSp);
				item_description.setPosition(descrSp.getPosition().x + 5 * ScreenScaler, descrSp.getPosition().y + 40 * ScreenScaler);
				window.draw(item_description);
				item_name.setPosition(descrSp.getPosition().x + 5 * ScreenScaler, descrSp.getPosition().y + 5 * ScreenScaler);
				window.draw(item_name);
			}
		}
	}
	int mouse_on_cell = -1;
	for (int i = 0; i < player.InventoryCapacity(); i++) {
		if (inventory_cell[i].getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
			mouse_on_cell = i;
			states::mouse_on_active_ui = 1;
		}
	}
	if (states::right_mouse_released) {
		if (states::choosen_item_pos != -1 && mouse_on_cell != -1&& states::choosen_item_pos != mouse_on_cell) {
			if (player.GetItem(states::choosen_item_pos).item == player.GetItem(mouse_on_cell).item) {
				int count = player.GetItem(mouse_on_cell).item->GetMaxCount()-player.GetItem(mouse_on_cell).count;
				player.GetItem(mouse_on_cell).count += std::min(count, player.GetItem(states::choosen_item_pos).count);
				player.GetItem(states::choosen_item_pos).count -= count;
				if (player.GetItem(states::choosen_item_pos).count <= 0)player.GetItem(states::choosen_item_pos).Reset();
			}
			else player.SwapItems(mouse_on_cell, states::choosen_item_pos);
		}
		else if (states::choosen_item_pos != -1 && states::choosen_item_pos != mouse_on_cell) {
			states::item_throw_confirmation = 1;
			states::throwing_item = &player.GetItem(states::choosen_item_pos);
		}
		states::choosen_item_pos = -1;
	}

}

void DrawPlayerStates(sf::RenderWindow& window, const settings& my_settings, const int& fps, Player& player) {
	//                          *******Characteristics bars********

	sf::Sprite bar(bar_texture), bar_filling(bar_filling_texture[0]);
	bar.setScale(0.3 * ScreenScaler, 0.3 * ScreenScaler);
	bar.setPosition(window.getView().getCenter().x - 570 * ScreenScalerX, window.getView().getCenter().y + 200 * ScreenScalerY);
	bar_filling.setPosition(window.getView().getCenter().x - 570 * ScreenScalerX, window.getView().getCenter().y + 200 * ScreenScalerY);
	for (int i = 0; i < 4; i++) {
		bar_filling.setScale(0.3 * ScreenScaler, 0.3 * ScreenScaler);
		bar_filling.setColor(filling_colors_bg[i]);
		window.draw(bar_filling);

		bar_filling.setColor(filling_colors[i]);
		bar_filling.setScale((0.3 * player.GetState(i) / 100.0) * ScreenScaler, 0.3 * ScreenScaler);
		window.draw(bar_filling);

		window.draw(bar);
		bar.move(300 * ScreenScaler, 0);
		bar_filling.move(300 * ScreenScaler, 0);
	}

	//                          *******Characteristics icons placements********

	sf::Sprite icon_placement(icon_placement_texture[0]);
	icon_placement.setScale(0.12 * ScreenScaler, 0.12 * ScreenScaler);
	icon_placement.setPosition(window.getView().getCenter().x - 607 * ScreenScalerX, window.getView().getCenter().y + 173 * ScreenScalerY);
	icon_placement.setColor(bar_color[0]);
	window.draw(icon_placement);

	icon_placement.move(307 * ScreenScaler, 0);
	icon_placement.setColor(bar_color[1]);
	window.draw(icon_placement);

	icon_placement.move(280 * ScreenScaler, 0);
	icon_placement.setColor(bar_color[2]);
	window.draw(icon_placement);

	icon_placement.move(310 * ScreenScaler, 0);
	icon_placement.setColor(bar_color[3]);
	window.draw(icon_placement);

	//                          *******Characteristics icons********

	sf::Sprite heart(heart_texture);
	heart.setPosition(window.getView().getCenter().x - 600 * ScreenScalerX, window.getView().getCenter().y + 190 * ScreenScalerY);
	heart.setScale(0.09 * ScreenScaler, 0.09 * ScreenScaler);
	heart.setColor(sf::Color(255, 255, 255, 240));
	window.draw(heart);

	sf::Sprite meat(meat_texture);
	meat.setPosition(heart.getPosition().x + 290 * ScreenScaler, heart.getPosition().y - 5 * ScreenScaler);
	meat.setScale(0.12 * ScreenScaler, 0.12 * ScreenScaler);
	meat.setColor(sf::Color(255, 255, 255, 240));
	window.draw(meat);

	sf::Sprite tempreture(tempreture_texture);
	tempreture.setPosition(meat.getPosition().x + 310 * ScreenScaler, meat.getPosition().y - 15 * ScreenScaler);
	tempreture.setScale(0.12 * ScreenScaler, 0.12 * ScreenScaler);
	tempreture.setColor(sf::Color(255, 255, 255, 230));
	window.draw(tempreture);

	sf::Sprite water(water_texture);
	water.setPosition(tempreture.getPosition().x + 310 * ScreenScaler, tempreture.getPosition().y + 0 * ScreenScaler);
	water.setScale(0.1 * ScreenScaler, 0.1 * ScreenScaler);
	water.setColor(sf::Color(255, 255, 255, 240));
	window.draw(water);
}

void DrawFps(sf::RenderWindow& window, const settings& my_settings, const int& fps) {
	sf::Text fps_text;
	fps_text.setFillColor(sf::Color::Green);
	fps_text.setCharacterSize(24 * ScreenScaler);
	fps_text.setFont(arkhip);
	fps_text.setPosition(sf::Vector2f(720, 5));
	fps_text.setString(std::to_string(fps));

	if (my_settings.show_fps) {
		fps_text.setPosition(window.getView().getCenter().x + 520 * ScreenScalerX, window.getView().getCenter().y - 340 * ScreenScalerY);
		window.draw(fps_text);
	}
}
void DrawUI(sf::RenderWindow& window, const settings& my_settings, const int& fps, Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {
	DrawFps(window, my_settings, fps);
	DrawPlayerStates(window, my_settings, fps, player);
	DrawInventory(window, my_settings, fps, player, near_mao, mao);
	DrawRecipes(window, my_settings, fps, player, near_mao, mao);
}

settings my_settings;

int main_game()
{
	srand(time(0));
	setlocale(LC_ALL, "rus");

	sf::View main_view(sf::FloatRect(0.f, 0.f, my_settings.screen_resolution.x, my_settings.screen_resolution.y));
	sf::View ui_view(sf::FloatRect(0.f, 0.f, my_settings.screen_resolution.x, my_settings.screen_resolution.y));
	ScreenScalerX = my_settings.screen_resolution.x / 1280.0f;
	ScreenScalerY = my_settings.screen_resolution.y / 720.0f;
	ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1.f);
	main_view.zoom(1.f / ScreenScaler);
	window.setView(main_view);


	sf::Clock clock;
	sf::Clock delta_clock;
	float delta_time = 0;


	int fps_counter = 0;
	int fps = 0;

	sf::Time lastFps = sf::seconds(0);

	main_view.setCenter({ 5000,4000 });

	Player player;
	float normal_player_speed = 300, water_player_speed = 160;
	player.SetSpeed(normal_player_speed);
	player.SetTexture(player_textures[my_settings.player_num], { 20,90 });
	player.SetArmsTexture(player_arm_textures[my_settings.player_num]);
	player.SetSize(1, 1);
	player.SetPosition(2000 + std::rand() % 2000, 2000 + std::rand() % 2000);
	player.AddItem(items[item_id["Wood"]], 3);
	player.SetDamage(1);
	player.SetDef(0);
	player.SetMaxHp(100);
	player.SetHp(100);

	main_map.push_back(std::vector<std::vector<unsigned short>>());
	LoadMap(main_map[0]);
	std::vector<std::vector<std::vector<unsigned short>>> current_map;
	current_map = main_map;
	sf::Vector2f map_start_pos = { 0,0 };

	std::vector<MapActiveObject*> map_active_objects;
	map_active_objects.reserve(2000);

	std::vector<Entity*> map_draw_objects;
	map_draw_objects.reserve(2000);

	for (int i = 0; i < 60; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[3])));
		float rsz = (std::rand() % 2) * 0.01f;
		map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 }, { 0.2f + rsz,0.2f + rsz }, -20 + std::rand() % 20);
	}
	for (int i = 0; i < 80; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[4])));
		float rsz = (std::rand() % 6) * 0.01f;
		map_active_objects[map_active_objects.size() - 1]->SetPosition({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 });
		map_active_objects[map_active_objects.size() - 1]->SetRotation(-20 + std::rand() % 25);
	}
	for (int i = 0; i < 20; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[5])));
		float rsz = (std::rand() % 6) * 0.01f;
		map_active_objects[map_active_objects.size() - 1]->SetPosition({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 });
		map_active_objects[map_active_objects.size() - 1]->SetRotation(-20 + std::rand() % 25);
	}
	for (int i = 0; i < 5; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[6])));
		float rsz = (std::rand() % 6) * 0.01f;
		map_active_objects[map_active_objects.size() - 1]->SetPosition({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 });
		map_active_objects[map_active_objects.size() - 1]->SetRotation(-20 + std::rand() % 25);
	}
	for (int i = 0; i < 3; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[7])));
		float rsz = (std::rand() % 6) * 0.01f;
		map_active_objects[map_active_objects.size() - 1]->SetPosition({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 });
		map_active_objects[map_active_objects.size() - 1]->SetRotation(-20 + std::rand() % 25);
	}
	for (int i = 0; i < 250; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[std::rand() % 2])));
		float rsz = (std::rand() % 6) * 0.01f;
		map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 }, { 0.6f + +rsz,0.6f + +rsz }, -20 + std::rand() % 20);
	}
	for (int i = 0; i < 20; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[2])));
		float rsz = (std::rand() % 6) * 0.01f;
		map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 }, { 0.6f + +rsz,0.6f + +rsz }, -20 + std::rand() % 20);
	}

	//////////////////////////////////
		// left wood wall
	for (int i = 0; i < 60; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[1])));
		map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f,800 + 130 * (float)i }, { 0.6,0.6 }, -20 + std::rand() % 20);
	}
	for (int i = 0; i < 68; i++)
		for (int j = 0; j < 6; j++) {
			map_draw_objects.push_back(new Entity(*base_entity[std::rand() % 2]));
			map_draw_objects[map_draw_objects.size() - 1]->Transform({ 800.f - j * 120.f,480 + 130 * (float)i }, { 0.6,0.6 }, -20 + std::rand() % 20);
		}
	// top wood wall
	for (int i = 0; i < 60; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[1])));
		map_active_objects[map_active_objects.size() - 1]->Transform({ 800 + 130 * (float)i,800 }, { 0.6,0.6 }, -20 + std::rand() % 20);
	}
	for (int i = 0; i < 68; i++)
		for (int j = 0; j < 6; j++) {
			map_draw_objects.push_back(new Entity(*base_entity[std::rand() % 2]));
			map_draw_objects[map_draw_objects.size() - 1]->Transform({ 800 + 130 * (float)i,800 - j * 120.f }, { 0.6,0.6 }, -20 + std::rand() % 20);
		}
	// right wood wall
	for (int i = 0; i < 60; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[1])));
		map_active_objects[map_active_objects.size() - 1]->Transform({ 800 + 130 * 60,800 + 130 * (float)i }, { 0.6,0.6 }, -20 + std::rand() % 20);
	}
	for (int i = 0; i < 68; i++)
		for (int j = 0; j < 6; j++) {
			map_draw_objects.push_back(new Entity(*base_entity[std::rand() % 2]));
			map_draw_objects[map_draw_objects.size() - 1]->Transform({ 800.f + 130 * 60 + j * 120.f,480 + 130 * (float)i }, { 0.6,0.6 }, -20 + std::rand() % 20);
		}
	// down wood wall
	for (int i = 0; i < 60; i++) {
		map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[1])));
		map_active_objects[map_active_objects.size() - 1]->Transform({ 800 + 130 * (float)i,800 + 130 * 60 }, { 0.6,0.6 }, -20 + std::rand() % 20);
	}
	for (int i = 0; i < 68; i++)
		for (int j = 0; j < 6; j++) {
			map_draw_objects.push_back(new Entity(*base_entity[std::rand() % 2]));
			map_draw_objects[map_draw_objects.size() - 1]->Transform({ 800 + 130 * (float)i,800 + j * 120.f + 130 * 60 }, { 0.6,0.6 }, -20 + std::rand() % 20);
		}

	std::vector<MapActiveObject*>near_active_objects;
	sf::Time last_near_ao_update = sf::seconds(-2);

	std::vector<Entity*>near_draw_objects;

	window.setFramerateLimit(my_settings.frame_limit);
	while (window.isOpen())
	{
		states::left_mouse_pressed = 0;
		states::left_mouse_released = 0;
		states::right_mouse_pressed = 0;
		states::right_mouse_released = 0;
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::Resized) {
				sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
				main_view = sf::View(visibleArea);
				ui_view = sf::View(visibleArea);
				ScreenScalerX = event.size.width / 1280.0f;
				ScreenScalerY = event.size.height / 720.0f;
				ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1.f);
				main_view.zoom(1.f / ScreenScaler);
			}
			else if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Button::Left) {
					states::left_mouse_pressed = 1;
					if (!states::mouse_on_active_ui)
						player.UseActiveItem(near_active_objects, map_active_objects);
				}
				else if (event.mouseButton.button == sf::Mouse::Button::Right) {
					states::right_mouse_pressed = 1;
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased) {
				if (event.mouseButton.button == sf::Mouse::Button::Left) {
					states::left_mouse_released = 1;
				}
				else if (event.mouseButton.button == sf::Mouse::Button::Right) {
					states::right_mouse_released = 1;
				}
			}
			else if (event.type == sf::Event::MouseWheelScrolled) {
				/*	if (states::locked_vision) {
						main_view.zoom(1 - event.mouseWheelScroll.delta * 25 * delta_time);
						states::locked_vision_zoom /= 1 - event.mouseWheelScroll.delta * 25 * delta_time;
					}*/
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::F2) {
					if (enable_to_change::show_colliders) {
						draw_colliders = !draw_colliders;
						enable_to_change::show_colliders = 0;
					}
				}
				if (event.key.code == sf::Keyboard::F3) {
					if (enable_to_change::show_fps) {
						my_settings.show_fps = !my_settings.show_fps;
						enable_to_change::show_fps = 0;
					}
				}
				if (event.key.code == sf::Keyboard::F4) {
					if (enable_to_change::show_ui) {
						my_settings.show_ui = !my_settings.show_ui;
						enable_to_change::show_ui = 0;
					}
				}
				if (event.key.code == sf::Keyboard::F9) {
					if (enable_to_change::full_screen) {
						if (!my_settings.full_screen) {
							window.create(sf::VideoMode(sf::VideoMode::getDesktopMode()), "Eat and breathe", sf::Style::Fullscreen);
							if (my_settings.language == 0 || my_settings.language == 2)
								window.setTitle(L"Ешь и дыши");
							main_view.setSize(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
							ui_view.setSize(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);

							ScreenScalerX = sf::VideoMode::getDesktopMode().width / 1280.0;
							ScreenScalerY = sf::VideoMode::getDesktopMode().height / 720.0;
							ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1);
							main_view.zoom(1 / ScreenScaler);

						}
						else {
							window.create(sf::VideoMode(my_settings.screen_resolution.x, my_settings.screen_resolution.y), "Eat and breathe", sf::Style::Default);
							if (my_settings.language == 0 || my_settings.language == 2)
								window.setTitle(L"Ешь и дыши");
							main_view.setSize(my_settings.screen_resolution.x, my_settings.screen_resolution.y);
							ui_view.setSize(my_settings.screen_resolution.x, my_settings.screen_resolution.y);

							ScreenScalerX = my_settings.screen_resolution.x / 1280.0;
							ScreenScalerY = my_settings.screen_resolution.y / 720.0;
							ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1);
							main_view.zoom(1 / ScreenScaler);

						}
						window.setFramerateLimit(my_settings.frame_limit);
						my_settings.full_screen = !my_settings.full_screen;
						enable_to_change::full_screen = 0;
					}
				}
				if (event.key.code == sf::Keyboard::V) {
					//states::free_vision = 1;
				}
				if (event.key.code == sf::Keyboard::C) {

					if (enable_to_change::move_view) {
						main_view.setCenter(window.mapPixelToCoords(sf::Mouse::getPosition(window), main_view));
						main_view.zoom(1 - 250 * 0.002);
						states::locked_vision_zoom /= 1 - 250 * 0.002;

						enable_to_change::move_view = 0;
						states::locked_vision = 1;
					}
				}
				if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9) {
					player.UseItem(event.key.code - sf::Keyboard::Num1, near_active_objects, map_active_objects);
				}
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::F2) {
					enable_to_change::show_colliders = 1;
				}
				if (event.key.code == sf::Keyboard::F3) {
					enable_to_change::show_fps = 1;
				}
				if (event.key.code == sf::Keyboard::F4) {
					enable_to_change::show_ui = 1;
				}
				if (event.key.code == sf::Keyboard::F9) {
					enable_to_change::full_screen = 1;
				}
				if (event.key.code == sf::Keyboard::V) {
					states::free_vision = 0;
				}
				if (event.key.code == sf::Keyboard::C) {
					enable_to_change::move_view = 1;

					states::locked_vision = 0;
					main_view.zoom(states::locked_vision_zoom);
					states::locked_vision_zoom = 1;
				}
			}
		}
		//
		delta_time = delta_clock.restart().asSeconds();
		if (my_settings.show_fps) {
			fps_counter++;
			if (clock.getElapsedTime().asSeconds() - lastFps.asSeconds() >= 1) {
				lastFps = clock.getElapsedTime();
				fps = fps_counter;
				fps_counter = 0;
			}
		}
		if (clock.getElapsedTime().asSeconds() - last_near_ao_update.asSeconds() >= my_settings.draw_obj_time) {
			near_active_objects.resize(0);
			for (auto& activeObject : map_active_objects) {

				if (player.Distance(*activeObject) < my_settings.draw_obj_distance) {
					near_active_objects.push_back(activeObject);
				}
			}
			near_draw_objects.resize(0);
			for (auto& obj : map_draw_objects) {
				if (player.Distance(*obj) < my_settings.draw_obj_distance) {
					near_draw_objects.push_back(obj);
				}
			}
			last_near_ao_update = clock.getElapsedTime();
		}
		//

		for (int i = 0; i < near_active_objects.size(); i++) {
			if (!near_active_objects[i]->ActiveBehaviour(player, delta_time, clock.getElapsedTime())) {
				auto obj = std::find(map_active_objects.begin(), map_active_objects.end(), near_active_objects[i]);

				if (map_active_objects.size() >= 90000) {
					std::swap(near_active_objects[i], near_active_objects[near_active_objects.size() - 1]);
					delete near_active_objects[near_active_objects.size() - 1];
					near_active_objects.pop_back();

					std::swap(*obj, map_active_objects[map_active_objects.size() - 1]);
					map_active_objects.pop_back();
				}
				else {
					near_active_objects.erase(std::find(near_active_objects.begin(), near_active_objects.end(), near_active_objects[i]));

					delete map_active_objects[(obj - map_active_objects.begin())];
					map_active_objects.erase(obj);
					i--;
				}
			}
		}
		sf::Vector2i map_cell_player_pos = { (int)((player.GetPosition().x - map_start_pos.x) / (cell_size - 2)),(int)((player.GetPosition().y - map_start_pos.y) / (cell_size - 2)) };
		if (map_cell_player_pos.y < current_map[0].size() && map_cell_player_pos.x < current_map[0][map_cell_player_pos.y].size()) {
			if (current_map[0][map_cell_player_pos.y][map_cell_player_pos.x] == 2) {
				player.SetSpeed(water_player_speed);
				states::player_on_water = 1;
			}
			else {
				player.SetSpeed(normal_player_speed);
				states::player_on_water = 0;
			}
		}

		if (player.GetHp() <= 0) {
			main_menu();
			break;
		}

		player.CheckStats(delta_time, states::player_on_water);
		if (window.hasFocus()) {
			player.CheckMovement(delta_time, std::vector<SolidObject*>(near_active_objects.begin(), near_active_objects.end()));
			player.CheckControl(delta_time);
		}

		player.RotateTo(window.mapPixelToCoords(sf::Mouse::getPosition(window), main_view), 270);
		///////////////////



		//////////////////

		if (enable_to_change::move_view) {
			if (states::free_vision) {
				MoveViewTo(main_view, window.mapPixelToCoords(sf::Mouse::getPosition(window), main_view), delta_time * 1.0);
			}
			else {
				MoveViewTo(main_view, player, delta_time * player.GetSpeed() * 0.01 * 3);
			}
		}

		window.clear();
		window.setView(main_view);


		DrawMap(window, current_map, player, near_active_objects, near_draw_objects, delta_time, my_settings);


		if (my_settings.show_ui) {
			states::mouse_on_active_ui = 0;
			window.setView(ui_view);
			DrawUI(window, my_settings, fps, player, near_active_objects, map_active_objects);
		}
		if (states::item_throw_confirmation)
			ItemThrowConfirmation(window, my_settings, delta_time);

		window.display();
	}
	return EXIT_SUCCESS;
}

int main_menu()
{
	srand(time(0));
	setlocale(LC_ALL, "rus");

	std::ifstream f("settings.txt", std::ios::in);
	if (!f.eof()) {
		f >> my_settings.language;
		f >> my_settings.player_num;
		f >> my_settings.frame_limit;
	}
	f.close();


	sf::View main_view(sf::FloatRect(0.f, 0.f, my_settings.screen_resolution.x, my_settings.screen_resolution.y));
	ScreenScalerX = my_settings.screen_resolution.x / 1280.0f;
	ScreenScalerY = my_settings.screen_resolution.y / 720.0f;
	ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1.f);
	main_view.zoom(1.f / ScreenScaler);
	window.setView(main_view);
	main_view.setCenter(1700, 1700);


	sf::Clock clock;
	sf::Clock delta_clock;
	float delta_time = 0;


	int fps_counter = 0;
	int fps = 0;

	sf::Time lastFps = sf::seconds(0);
	sf::Time save_timer = sf::seconds(0);


	main_map.push_back(std::vector<std::vector<unsigned short>>());
	LoadMap(main_map[0]);
	std::vector<std::vector<std::vector<unsigned short>>> current_map;
	current_map = main_map;
	sf::Vector2f map_start_pos = { 0,0 };

	std::vector<MapActiveObject*> map_active_objects;
	map_active_objects.reserve(200);

	std::vector<Entity*> map_draw_objects;



	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[3])));
	float rsz = (std::rand() % 2) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 1300.f ,1400.f }, { 0.2f + rsz,0.2f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[3])));
	rsz = (std::rand() % 2) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 1250.f ,1450.f }, { 0.2f + rsz,0.2f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[1])));
	rsz = (std::rand() % 6) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 1120.f ,1450.f }, { 0.6f + rsz,0.6f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[1])));
	rsz = (std::rand() % 6) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 1100.f ,1650.f }, { 0.6f + rsz,0.6f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[0])));
	rsz = (std::rand() % 6) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 1200.f ,1850.f }, { 0.6f + rsz,0.6f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[0])));
	rsz = (std::rand() % 6) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 1150.f ,1890.f }, { 0.6f + rsz,0.6f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[0])));
	rsz = (std::rand() % 6) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 1250.f ,1950.f }, { 0.6f + rsz,0.6f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new Bonfire(*static_cast<Bonfire*>(base_entity[14])));
	map_active_objects[map_active_objects.size() - 1]->Transform({ 1600.f ,1950.f }, { 0.2f + rsz,0.2f + rsz }, 90);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[2])));
	rsz = (std::rand() % 6) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 2250.f ,1420.f }, { 0.6f + rsz,0.6f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[4])));
	rsz = (std::rand() % 6) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 2250.f ,1750.f }, { 0.3f + rsz,0.3f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[4])));
	rsz = (std::rand() % 6) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 2300.f ,1850.f }, { 0.2f + rsz,0.2f + rsz }, -20 + std::rand() % 20);

	map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[5])));
	rsz = (std::rand() % 6) * 0.01f;
	map_active_objects[map_active_objects.size() - 1]->Transform({ 2150.f ,1780.f }, { 0.2f + rsz,0.2f + rsz }, -20 + std::rand() % 20);

	Player player;
	player.SetPosition(main_view.getCenter().x, main_view.getCenter().y + 100);
	player.SetTexture(player_textures[my_settings.player_num], { 20,90 });
	player.SetArmsTexture(player_arm_textures[my_settings.player_num]);
	player.SetSize(1.1, 1.1);

	window.setFramerateLimit(my_settings.frame_limit);
	while (window.isOpen())
	{
		states::left_mouse_pressed = 0;
		states::left_mouse_released = 0;
		states::right_mouse_pressed = 0;
		states::right_mouse_released = 0;
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::Resized) {
				sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
				main_view = sf::View(visibleArea);
				ScreenScalerX = event.size.width / 1280.0f;
				ScreenScalerY = event.size.height / 720.0f;
				ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1.f);
				main_view.zoom(1.f / ScreenScaler);
				main_view.setCenter(1700, 1700);
			}
			else if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Button::Left) {
					states::left_mouse_pressed = 1;
				}
				else if (event.mouseButton.button == sf::Mouse::Button::Right) {
					states::right_mouse_pressed = 1;
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased) {
				if (event.mouseButton.button == sf::Mouse::Button::Left) {
					states::left_mouse_released = 1;
				}
				else if (event.mouseButton.button == sf::Mouse::Button::Right) {
					states::right_mouse_released = 1;
				}
			}
			else if (event.type == sf::Event::MouseWheelScrolled) {
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::F3) {
					if (enable_to_change::show_fps) {
						my_settings.show_fps = !my_settings.show_fps;
						enable_to_change::show_fps = 0;
					}
				}
				if (event.key.code == sf::Keyboard::F4) {
					if (enable_to_change::show_ui) {
						my_settings.show_ui = !my_settings.show_ui;
						enable_to_change::show_ui = 0;
					}
				}
				if (enable_to_change::full_screen) {
					if (!my_settings.full_screen) {
						window.create(sf::VideoMode(sf::VideoMode::getDesktopMode()), "Eat and breathe", sf::Style::Fullscreen);
						if (my_settings.language == 0 || my_settings.language == 2)
							window.setTitle(L"Ешь и дыши");
						main_view.setSize(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);

						ScreenScalerX = sf::VideoMode::getDesktopMode().width / 1280.0;
						ScreenScalerY = sf::VideoMode::getDesktopMode().height / 720.0;
						ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1);
						main_view.zoom(1 / ScreenScaler);

					}
					else {
						window.create(sf::VideoMode(my_settings.screen_resolution.x, my_settings.screen_resolution.y), "Eat and breathe", sf::Style::Default);
						if (my_settings.language == 0 || my_settings.language == 2)
							window.setTitle(L"Ешь и дыши");
						main_view.setSize(my_settings.screen_resolution.x, my_settings.screen_resolution.y);

						ScreenScalerX = my_settings.screen_resolution.x / 1280.0;
						ScreenScalerY = my_settings.screen_resolution.y / 720.0;
						ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1);
						main_view.zoom(1 / ScreenScaler);

					}
					window.setFramerateLimit(my_settings.frame_limit);
					my_settings.full_screen = !my_settings.full_screen;
					enable_to_change::full_screen = 0;
				}
				if (event.key.code == sf::Keyboard::C) {

					if (enable_to_change::move_view) {
						main_view.setCenter(window.mapPixelToCoords(sf::Mouse::getPosition(window), main_view));
						main_view.zoom(1 - 250 * 0.002);
						states::locked_vision_zoom /= 1 - 250 * 0.002;

						enable_to_change::move_view = 0;
						states::locked_vision = 1;
					}
				}

			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::F3) {
					enable_to_change::show_fps = 1;
				}
				if (event.key.code == sf::Keyboard::F4) {
					enable_to_change::show_ui = 1;
				}
				if (event.key.code == sf::Keyboard::F9) {
					enable_to_change::full_screen = 1;
				}
				if (event.key.code == sf::Keyboard::C) {
					enable_to_change::move_view = 1;

					states::locked_vision = 0;
					main_view.zoom(states::locked_vision_zoom);
					states::locked_vision_zoom = 1;
				}
			}
		}
		///
		delta_time = clock.restart().asSeconds();
		//

		for (int i = 0; i < map_active_objects.size(); i++) {
			if (!map_active_objects[i]->ActiveBehaviour(player, delta_time, clock.getElapsedTime())) {
			}
		}

		Button rus(language_button_texture[0], 0.7), eng(language_button_texture[1], 0.7), bel(language_button_texture[2], 0.7);
		rus.SetPosition({ 1600,1400 });
		rus.SetFunc([]() {my_settings.language = 0; });
		eng.SetPosition({ 1710,1400 });
		eng.SetFunc([]() {my_settings.language = 1; });
		bel.SetPosition({ 1820,1400 });
		bel.SetFunc([]() {my_settings.language = 2; });

		rus.Check(window, states::left_mouse_pressed, states::left_mouse_released, sf::Mouse::isButtonPressed(sf::Mouse::Left));
		eng.Check(window, states::left_mouse_pressed, states::left_mouse_released, sf::Mouse::isButtonPressed(sf::Mouse::Left));
		bel.Check(window, states::left_mouse_pressed, states::left_mouse_released, sf::Mouse::isButtonPressed(sf::Mouse::Left));

		Button* active_but = nullptr;
		if (my_settings.language == 0) {
			active_but = &rus;
		}
		else if (my_settings.language == 1) {
			active_but = &eng;
		}
		else {
			active_but = &bel;
		}
		sf::RectangleShape rect({ active_but->GetTextRef().getScale().x * active_but->GetScale() * 110, active_but->GetTextRef().getScale().y * active_but->GetScale() * 100 });
		rect.setOrigin(rect.getSize().x / 2, rect.getSize().y / 2);
		rect.setPosition(active_but->GetPosition());
		rect.setOutlineThickness(11);
		rect.setOutlineColor(sf::Color::Black);
		rect.setFillColor(sf::Color::Transparent);


		Button play("Play", 36, arkhip, 1);
		play.SetPosition({ 1600,1600 });

		if (my_settings.language == 0)
			play.setString(L"Играть");
		else if (my_settings.language == 2)
			play.setString(L"Гуляць");
		play.GetTextRef().setLetterSpacing(2);
		play.GetTextRef().setFillColor(sf::Color(220, 220, 140, 245));
		play.GetTextRef().setOutlineColor(sf::Color::Black);
		play.GetTextRef().setOutlineThickness(5);
		if (play.Check(window, states::left_mouse_pressed, states::left_mouse_released)) {
			main_game();
			break;
		}

		Button quit("Quit", 36, arkhip, 1);
		quit.SetPosition({ 1400,1650 });

		if (my_settings.language == 0)
			quit.setString(L"Выйти");
		else if (my_settings.language == 2)
			quit.setString(L"Выйсци\nНе все переведено на этот язык");
		quit.GetTextRef().setLetterSpacing(2);
		quit.GetTextRef().setFillColor(sf::Color(220, 220, 140, 245));
		quit.GetTextRef().setOutlineColor(sf::Color::Black);
		quit.GetTextRef().setOutlineThickness(5);
		if (quit.Check(window, states::left_mouse_pressed, states::left_mouse_released)) {
			window.close();
			break;
		}

		Button left(arrow_button_texture, 0.7), right(arrow_button_texture, 0.7);
		left.SetPosition({ 1600,1800 });
		left.SetFunc([]() {my_settings.player_num--; if (my_settings.player_num < 0)my_settings.player_num = 2; });
		left.Flip();

		right.SetPosition({ 1800,1800 });
		right.SetFunc([]() {my_settings.player_num++; if (my_settings.player_num > 2)my_settings.player_num = 0; });

		if (left.Check(window, states::left_mouse_pressed, states::left_mouse_released, sf::Mouse::isButtonPressed(sf::Mouse::Left))) {
			player.SetTexture(player_textures[my_settings.player_num], { 20,90 });
			player.SetArmsTexture(player_arm_textures[my_settings.player_num]);
			player.SetSize(1.1, 1.1);
		}
		if (right.Check(window, states::left_mouse_pressed, states::left_mouse_released, sf::Mouse::isButtonPressed(sf::Mouse::Left))) {
			player.SetTexture(player_textures[my_settings.player_num], { 20,90 });
			player.SetArmsTexture(player_arm_textures[my_settings.player_num]);
			player.SetSize(1.1, 1.1);
		}

		save_timer += sf::seconds(delta_time);
		if (save_timer.asSeconds() > 0.3f) {
			std::fstream f("settings.txt", std::ios::out | std::ios::trunc);
			f << my_settings.language;
			f << "\n";
			f << my_settings.player_num;
			f << "\n";
			f << my_settings.frame_limit;
			f.close();
			save_timer = sf::seconds(0);
		}

		///////////////////


		window.clear();
		window.setView(main_view);


		DrawMap(window, current_map, player, map_active_objects, map_draw_objects, delta_time, my_settings);

		window.draw(rect);
		rus.Draw(window, delta_time);
		eng.Draw(window, delta_time);
		bel.Draw(window, delta_time);
		play.Draw(window, delta_time);
		quit.Draw(window, delta_time);
		left.Draw(window, delta_time);
		right.Draw(window, delta_time);

		window.display();
	}
	return EXIT_SUCCESS;
}

int main() {

	std::ifstream f("settings.txt", std::ios::in);
	if (!f.eof()) {
		f >> my_settings.language;
		f >> my_settings.player_num;
		f >> my_settings.frame_limit;
	}
	f.close();

	if (my_settings.full_screen)
		window.create(sf::VideoMode(sf::VideoMode::getDesktopMode()), "Eat and breathe", sf::Style::Fullscreen);
	else
		window.create(sf::VideoMode({ (unsigned)my_settings.screen_resolution.x,(unsigned)my_settings.screen_resolution.y }), "Eat and breathe", sf::Style::Default);
	if (my_settings.language == 0 || my_settings.language == 2)
		window.setTitle(L"Ешь и дыши");

	LoadFonts();

	sf::Text load_text;
	load_text.setLetterSpacing(2);
	load_text.setCharacterSize(40 * ScreenScaler);
	load_text.setFont(you2013);
	load_text.setString("Loading resources");
	if (my_settings.language == 0)
		load_text.setString(L"Загрузка ресурсов");

	window.draw(load_text);
	window.display();

	if (!states::resources_loaded) {
		LoadResources();
		states::resources_loaded = 1;
	}

	main_menu();

	return 0;
}