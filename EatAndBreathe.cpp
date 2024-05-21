#include <iostream>
#include <vector>
#include <list>
#include <ctime>
#include <thread>
#include <cmath>
#include <fstream>
#include <codecvt>
#include <chrono>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include "EngineFuncs.h"
#include "Entities.h"
#include "Resources.h"
#include "Button.h"
#include "TextField.h"


int main();
int main_menu();
int main_game();
int cooperative_menu();

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
	float send_map_time;
	int player_num = 0;
	bool show_serve_info = 1;
	bool draw_map = 1;
	settings() {
		show_fps = 0;
		full_screen = 0;
		show_ui = 1;
		screen_resolution = { (int)(1280 / 1.f), (int)(720 / 1.f) };
		frame_limit = 9999;
		language = 0;
		draw_cell_distance = 7;
		draw_obj_distance = 1000;
		draw_obj_time = 0.2f;
		send_map_time = 0.0f;
		draw_map = 1;
	}
};

namespace states {
	static bool free_vision = 0;
	static bool locked_vision = 0;
	static float locked_vision_zoom = 1;

	bool left_mouse_pressed = 0;
	bool left_mouse_released = 0;
	bool right_mouse_pressed = 0;
	bool right_mouse_released = 0;

	bool enter_pressed = 0;

	int choosen_item_pos = -1;

	bool item_throw_confirmation = 0;
	int throwing_item_pos;

	bool player_on_water = 0;
	bool mouse_on_active_ui = 0;

	bool night = 0;

	unsigned short my_port;
	sf::IpAddress my_local_ip;
	sf::IpAddress my_public_ip;
	sf::IpAddress server_ip;
	unsigned short server_port;

	std::wstring my_name;
}

void ClearKeyStates() {
	states::left_mouse_pressed = 0;
	states::left_mouse_released = 0;
	states::right_mouse_pressed = 0;
	states::right_mouse_released = 0;

	states::enter_pressed = 0;
}

namespace enable_to_change {
	static bool show_fps = 1;
	static bool show_ui = 1;
	static bool full_screen = 1;
	static bool move_view = 1;
	static bool show_colliders = 1;
	static bool show_more = 1;
	static bool show_map = 1;
};

float ScreenScaler = 1;
float ScreenScalerX = 1;
float ScreenScalerY = 1;

int cell_size = 128;

settings my_settings;

sf::UdpSocket socket;
bool iAmServer = 1;
std::vector<std::pair<std::string, int>>clients;
sf::Text* help_text = nullptr;
std::vector<Player>players;
int my_player = 0;
float normal_player_speed = 300, water_player_speed = 160;

std::vector<MapActiveObject*> map_active_objects;
std::vector<Entity*> map_draw_objects;
std::vector<MapActiveObject*>near_active_objects;
sf::Time last_near_ao_update = sf::seconds(-2);
sf::Time last_map_send = sf::seconds(-2);
std::vector<Entity*>near_draw_objects;

struct PlayerControls {
	bool a, d, w, s;
	bool whf;
	sf::Vector2f mouse_pos;
	bool mouse_pressed;
	bool right_mouse_pressed;
	bool mouse_on_ui;
	PlayerControls() {
		this->a = 0;
		this->d = 0;
		this->w = 0;
		this->s = 0;
		whf = 0;
		mouse_pos = { 0,0 };
		mouse_pressed = 0;
		mouse_on_ui = 0;
		right_mouse_pressed = 0;
	}
	PlayerControls(bool a, bool d, bool w, bool s, bool focus, sf::Vector2f m_pos, bool m_pressed, bool rm_pressed, bool m_on_ui) {
		this->a = a;
		this->d = d;
		this->w = w;
		this->s = s;
		whf = focus;
		mouse_pos = m_pos;
		mouse_pressed = m_pressed;
		mouse_on_ui = m_on_ui;
		right_mouse_pressed = rm_pressed;
	}
};
std::vector<PlayerControls>players_controls;

struct sendingData {
	std::string message;
	std::string command;
	sf::String nick;

	sendingData(sf::String m, sf::String c, sf::String n) {
		message = m;
		command = c;
		nick = n;
	}
	sendingData() {
	}
};
sf::Packet& operator <<(sf::Packet& packet, const sendingData& d)
{
	return packet << d.message << d.command << d.nick;
}

sf::Packet& operator >>(sf::Packet& packet, sendingData& d)
{
	return packet >> d.message >> d.command >> d.nick;
}

void DrawActiveBuilding(sf::RenderWindow& window, const settings& my_settings, Player& player, std::vector<MapActiveObject*>& near_mao, const float& delta_time) {
	if (player.ActiveItem() == nullptr ||
		(typeid(*player.ActiveItem()->item) != typeid(BuildItem)))return;

	int entity_id = static_cast<BuildItem const*>(player.ActiveItem()->item)->GetEntityId();
	MapActiveObject* b;
	if (typeid(*base_entity[entity_id]) == typeid(MapDamaging)) {
		b = new MapDamaging(*static_cast<MapDamaging*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(Bonfire)) {
		b = new Bonfire(*static_cast<Bonfire*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(MapEntity)) {
		b = new MapEntity(*static_cast<MapEntity*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(MapActiveObject)) {
		b = new MapActiveObject(*static_cast<MapActiveObject*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(MaterialSource)) {
		b = new MaterialSource(*static_cast<MaterialSource*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(Animal)) {
		b = new Animal(*static_cast<Animal*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(Fleeing)) {
		b = new Fleeing(*static_cast<Fleeing*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(Enemy)) {
		b = new Enemy(*static_cast<Enemy*>(base_entity[entity_id]));
	}
	else if (typeid(*base_entity[entity_id]) == typeid(Box)) {
		b = new Box(*static_cast<Box*>(base_entity[entity_id]));
	}
	else {
		b = new MapEntity(*static_cast<MapEntity*>(base_entity[entity_id]));
		std::cout << "Invalid build item class(Pre drawing)\n";
	}
	float px = player.GetPosition().x, py = player.GetPosition().y;
	if (iAmServer) {
		px = px + 100.f * std::cos(player.GetRotation() * PI / 180);
		py = py + 100.f * std::sin(player.GetRotation() * PI / 180);
	}
	else {
		px = px + 100.f * std::cos((player.GetRotation() - 270) * PI / 180);
		py = py + 100.f * std::sin((player.GetRotation() - 270) * PI / 180);
	}
	b->Transform({ px,py }, base_entity[entity_id]->GetSize());
	b->GetSpriteRef().setColor(sf::Color(255, 255, 255, 150));
	if (!(b->flying || (static_cast<const BuildItem*>(player.ActiveItem()->item)->GetAlwaysSpawn()))) {
		for (auto& obj : near_mao) {
			if (obj->DetectCollision(*b)) b->GetSpriteRef().setColor(sf::Color(255, 55, 55, 150));
		}
	}
	b->Draw(window, delta_time, states::night);

}

void DrawMap(sf::RenderWindow& window, const std::vector<std::vector<std::vector<unsigned short>>>& current_map, std::vector<Player>& players, std::vector<MapActiveObject*>& activeObjects, std::vector<Entity*>& drawObjects, const float& delta_time, const settings& my_settings, int my_player) {
	sf::Vector2i map_cell_player_pos = { (int)((players[my_player].GetPosition().x - map_start_pos.x) / (cell_size - 2)),(int)((players[my_player].GetPosition().y - map_start_pos.y) / (cell_size - 2)) };
	map_cell_player_pos.x -= my_settings.draw_cell_distance;
	map_cell_player_pos.y -= my_settings.draw_cell_distance;

	if (map_cell_player_pos.x < 0)map_cell_player_pos.x = 0;
	if (map_cell_player_pos.y < 0)map_cell_player_pos.y = 0;

	for (int l = 0; l < current_map.size(); l++) {
		for (int i = map_cell_player_pos.y; i < current_map[l].size() && i < map_cell_player_pos.y + my_settings.draw_cell_distance * 2; i++) {
			for (int j = map_cell_player_pos.x; j < current_map[l][i].size() && j < map_cell_player_pos.x + my_settings.draw_cell_distance * 2; j++) {
				sf::Sprite cell_sprite(cell_textures[current_map[l][i][j]]);
				if (states::night) {
					cell_sprite.setColor(night_color);
				}
				cell_sprite.setScale(1, 1);
				cell_sprite.setPosition(map_start_pos.x + j * (cell_size - 2), map_start_pos.y + i * (cell_size - 2));
				window.draw(cell_sprite);
			}
		}
	}
	std::vector<std::vector<Entity*>>to_draw(201, std::vector<Entity*>());

	for (auto& player : players)
		to_draw[100].push_back(&player);
	for (auto& object : drawObjects) {
		to_draw[object->GetDrawLayer() + 100].push_back(object);
	}
	for (auto& object : activeObjects) {
		to_draw[object->GetDrawLayer() + 100].push_back(object);
	}

	for (auto& layer : to_draw) {
		if (layer.size() > 0) {
			for (auto& object : layer) {
				object->Draw(window, delta_time, states::night);
			}
		}
	}

	DrawActiveBuilding(window, my_settings, players[my_player], activeObjects, delta_time);
}
sf::Color active_inv_color(166, 120, 78, 180);
int active_item_number = -1;

void ItemThrowConfirmation(sf::RenderWindow& window, const settings& my_settings, Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const int& dt) {

	sf::Text confirm;
	confirm.setLetterSpacing(2);
	confirm.setCharacterSize(40 * ScreenScaler);
	confirm.setFont(you2013);
	confirm.setString("Are you sure you want to discard\n" + player.GetItem(states::throwing_item_pos).item->GetEngName() + "?");
	if (my_settings.language == 0)
		confirm.setString(L"Вы уверены что хотите выбросить\n" +
			sf::String::fromUtf8(player.GetItem(states::throwing_item_pos).item->GetRuName().begin(), player.GetItem(states::throwing_item_pos).item->GetRuName().end()) + "?");
	else if (my_settings.language == 2)
		confirm.setString(L"Вы упэунены, што хочаце выкинуць\n" +
			sf::String::fromUtf8(player.GetItem(states::throwing_item_pos).item->GetBelName().begin(), player.GetItem(states::throwing_item_pos).item->GetBelName().end()) + "?");
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
	if (ok1.Check(window, states::left_mouse_pressed, states::left_mouse_released)) {
		if (player.GetItem(states::throwing_item_pos).count <= 1
			&& typeid(*player.GetItem(states::throwing_item_pos).item) == typeid(EquipmentItem)) {
			if (iAmServer)
				player.Undress(static_cast<const EquipmentItem*>(player.GetItem(states::throwing_item_pos).item));
			else {
				std::string msg = "";
				msg += std::to_string(my_player) + "/";
				msg += std::to_string(states::throwing_item_pos) + "/";

				sf::Packet use_packet;
				sendingData use_dat(msg, "player_undress", states::my_name);
				if (!(use_packet << use_dat)) std::cout << "error in packing [undress]\n";
				if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
					std::cout << "error in sending undress]\n";
			}
		}
		if (iAmServer)
			player.DropItem(states::throwing_item_pos, 1, near_mao, mao);
		else {
			std::string msg = "";
			msg += std::to_string(my_player) + "/";
			msg += std::to_string(states::throwing_item_pos) + "/";
			msg += std::to_string(1) + "/";

			sf::Packet use_packet;
			sendingData use_dat(msg, "player_drop", states::my_name);
			if (!(use_packet << use_dat)) std::cout << "error in packing [drop item]\n";
			if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
				std::cout << "error in sending [drop item]\n";
		}
		states::item_throw_confirmation = 0;
	}
	ok1.Draw(window);


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
	if (ok2.Check(window, states::left_mouse_pressed, states::left_mouse_released)) {
		if (typeid(*player.GetItem(states::throwing_item_pos).item) == typeid(EquipmentItem)) {
			if (iAmServer)
				player.Undress(static_cast<const EquipmentItem*>(player.GetItem(states::throwing_item_pos).item));
			else {
				std::string msg = "";
				msg += std::to_string(my_player) + "/";
				msg += std::to_string(states::throwing_item_pos) + "/";

				sf::Packet use_packet;
				sendingData use_dat(msg, "player_undress", states::my_name);
				if (!(use_packet << use_dat)) std::cout << "error in packing [undress]\n";
				if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
					std::cout << "error in sending undress]\n";
			}
		}
		if (iAmServer)
			player.DropItem(states::throwing_item_pos, player.GetItem(states::throwing_item_pos).count, near_mao, mao);
		else {
			std::string msg = "";
			msg += std::to_string(my_player) + "/";
			msg += std::to_string(states::throwing_item_pos) + "/";
			msg += std::to_string(player.GetItem(states::throwing_item_pos).count) + "/";

			sf::Packet use_packet;
			sendingData use_dat(msg, "player_drop", states::my_name);
			if (!(use_packet << use_dat)) std::cout << "error in packing [drop item]\n";
			if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
				std::cout << "error in sending [drop item]\n";
		}
		states::item_throw_confirmation = 0;
	}
	ok2.Draw(window);

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
	cancel.Draw(window);

	if (ok1.MouseOnButton(window) || ok2.MouseOnButton(window) || cancel.MouseOnButton(window))
		states::mouse_on_active_ui = 1;
}

void DrawRecipes(sf::RenderWindow& window, const settings& my_settings, const int& fps, Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {

	int available_recipes = 0;
	for (int i = 0; i < recipes.size(); i++) {
		if (recipes[i] != nullptr && recipes[i]->Available(player)) { available_recipes++; }
	}

	std::vector<sf::Sprite> cell(available_recipes, sf::Sprite(inventory_cell_texture[3]));
	std::vector<sf::Sprite>recipe_sprite;
	sf::Sprite descrSp(inventory_cell_texture[2]);
	descrSp.setScale(0.44 * ScreenScaler, 0.185 * ScreenScaler);
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

			float average = recipes[i]->GetItem()->GetTexture().getSize().x + recipes[i]->GetItem()->GetTexture().getSize().y;
			average /= 2;
			s.setScale(cell[j].getScale().x * 3.5 / average * 150,
				cell[j].getScale().y * 3.5 / average * 150);

			recipe_sprite.push_back(s);
			j++;
		}
	}


	for (int i = 0; i < available_recipes; i++) {
		window.draw(cell[i]);
		window.draw(recipe_sprite[i]);
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
			if (iAmServer)
				player.Craft(recipes[recipe]);
			else {
				std::string msg = "";
				msg += std::to_string(my_player) + "/";
				msg += std::to_string(recipe) + "/";

				sf::Packet use_packet;
				sendingData use_dat(msg, "player_craft", states::my_name);
				if (!(use_packet << use_dat)) std::cout << "error in packing [craft]\n";
				if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
					std::cout << "error in sending [craft]\n";
			}
		}
	}

}

void DrawInventory(sf::RenderWindow& window, const settings& my_settings, const int& fps, Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {

	std::vector<sf::Sprite> inventory_cell(player.InventoryCapacity(), sf::Sprite(inventory_cell_texture[3]));
	sf::Sprite descrSp(inventory_cell_texture[2]);
	descrSp.setScale(0.44 * ScreenScaler, 0.185 * ScreenScaler);
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

	sf::Vector2f inv_cell_pos = { window.getView().getCenter().x - 450 * ScreenScalerX, window.getView().getCenter().y + 270 * ScreenScalerY };
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
					if (iAmServer)
						player.ChangeActiveItem();
					else {
						std::string msg = "";
						msg += std::to_string(my_player) + "/";
						msg += std::to_string(-1) + "/";

						sf::Packet use_packet;
						sendingData use_dat(msg, "change_active_item", states::my_name);
						if (!(use_packet << use_dat)) std::cout << "error in packing [change_active_item]\n";
						if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
							std::cout << "error in sending change_active_item]\n";
					}
				}
				else if (player.GetItem(i).item->isActivated()) {
					if (typeid(*player.GetItem(i).item) == typeid(EquipmentItem) &&
						static_cast<const EquipmentItem*>(player.GetItem(i).item)->GetSlot()) {
						if (player.isDressed(player.GetItem(i).item)) {
							if (iAmServer)
								player.Undress(static_cast<const EquipmentItem*>(player.GetItem(i).item));
							else {
								std::string msg = "";
								msg += std::to_string(my_player) + "/";
								msg += std::to_string(i) + "/";

								sf::Packet use_packet;
								sendingData use_dat(msg, "player_undress", states::my_name);
								if (!(use_packet << use_dat)) std::cout << "error in packing [undress]\n";
								if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
									std::cout << "error in sending undress]\n";
							}
						}
						else {
							if (iAmServer)
								player.Dress(static_cast<const EquipmentItem*>(player.GetItem(i).item));
							else {
								std::string msg = "";
								msg += std::to_string(my_player) + "/";
								msg += std::to_string(i) + "/";

								sf::Packet use_packet;
								sendingData use_dat(msg, "player_dress", states::my_name);
								if (!(use_packet << use_dat)) std::cout << "error in packing [dress]\n";
								if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
									std::cout << "error in sending dress]\n";
							}
						}
					}
					else {
						active_item_number = i;
						if (iAmServer)
							player.ChangeActiveItem(player.GetItem(i));
						else {
							std::string msg = "";
							msg += std::to_string(my_player) + "/";
							msg += std::to_string(i) + "/";

							sf::Packet use_packet;
							sendingData use_dat(msg, "change_active_item", states::my_name);
							if (!(use_packet << use_dat)) std::cout << "error in packing [change_active_item]\n";
							if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
								std::cout << "error in sending change_active_item]\n";
						}
					}
				}
				else {
					if (iAmServer)
						player.UseItem(i, near_mao, mao, states::night);
					else {
						std::string msg = "";
						msg += std::to_string(my_player) + "/";
						msg += std::to_string(i) + "/";

						sf::Packet use_packet;
						sendingData use_dat(msg, "client_use", states::my_name);
						if (!(use_packet << use_dat)) std::cout << "error in packing [use]\n";
						if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
							std::cout << "error in sending use]\n";
					}
				}
			}
			if (states::right_mouse_pressed && player.GetItem(i).item->GetId()) {
				if (player.GetItem(i).item->GetId()) {
					states::choosen_item_pos = i;
				}
			}

		}

		if (active_item_number == i)
			if (player.ActiveItem() != nullptr && player.ActiveItem()->item->GetId())
				inventory_cell[i].setColor(active_inv_color);
			else active_item_number = -1;
		if (player.isDressed(player.GetItem(i).item)) {
			inventory_cell[i].setColor(active_inv_color);
		}

		sf::Sprite s(player.GetItem(i).item->GetTexture());
		s.setOrigin(s.getTexture()->getSize().x / 2, s.getTexture()->getSize().y / 2);
		s.setPosition(inventory_cell[i].getPosition().x + inventory_cell[i].getTexture()->getSize().x / 2 * inventory_cell[i].getScale().x,
			inventory_cell[i].getPosition().y + inventory_cell[i].getTexture()->getSize().y / 2 * inventory_cell[i].getScale().y);
		if (states::choosen_item_pos == i) {
			s.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
		}
		float average = player.GetItem(i).item->GetTexture().getSize().x + player.GetItem(i).item->GetTexture().getSize().y;
		average /= 2;
		s.setScale(inventory_cell[i].getScale().x * 3.5 / average * 150,
			inventory_cell[i].getScale().y * 3.5 / average * 150);

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
		if (states::choosen_item_pos != -1 && mouse_on_cell != -1 && states::choosen_item_pos != mouse_on_cell) {
			if (player.GetItem(states::choosen_item_pos).item == player.GetItem(mouse_on_cell).item) {
				int count = player.GetItem(mouse_on_cell).item->GetMaxCount() - player.GetItem(mouse_on_cell).count;
				player.GetItem(mouse_on_cell).count += std::min(count, player.GetItem(states::choosen_item_pos).count);
				player.GetItem(states::choosen_item_pos).count -= count;
				if (player.GetItem(states::choosen_item_pos).count <= 0)player.GetItem(states::choosen_item_pos).Reset();
			}
			else {
				if (iAmServer)
					player.SwapItems(mouse_on_cell, states::choosen_item_pos);
				else {
					std::string msg = "";
					msg += std::to_string(my_player) + "/";
					msg += std::to_string(mouse_on_cell) + "/";
					msg += std::to_string(states::choosen_item_pos) + "/";

					sf::Packet use_packet;
					sendingData use_dat(msg, "swap_items", states::my_name);
					if (!(use_packet << use_dat)) std::cout << "error in packing [swap_items]\n";
					if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
						std::cout << "error in sending swap_items]\n";
				}
				if (active_item_number != -1 && !player.GetItem(active_item_number).item->isActivated())active_item_number = -1;
			}
		}
		else if (states::choosen_item_pos != -1 && states::choosen_item_pos != mouse_on_cell) {
			states::item_throw_confirmation = 1;
			states::throwing_item_pos = states::choosen_item_pos;
		}
		states::choosen_item_pos = -1;
	}

}

void NightChanges(std::vector<Player>& players, std::vector<MapActiveObject*>& mao, std::vector<Entity*>& draw) {
	for (auto& pl : players)
		pl.NightChange(states::night);
	for (auto& o : mao) {
		o->NightChange(states::night);
	}
	for (auto& o : draw) {
		o->NightChange(states::night);
	}
}

void DrawPlayerStates(sf::RenderWindow& window, const settings& my_settings, const int& fps, Player& player) {
	//                          *******Characteristics bars********

	sf::Sprite bar(bar_texture[0]), bar_filling(bar_filling_texture[0]);
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
	fps_text.setPosition(window.getView().getCenter().x + 520 * ScreenScalerX, window.getView().getCenter().y - 340 * ScreenScalerY);
	fps_text.setString(std::to_string(fps));

	if (my_settings.show_fps) {
		window.draw(fps_text);
	}
}
void DrawServerInfo(sf::RenderWindow& window, settings& my_settings, const int& fps) {
	Button hide(arrow_button_texture, 0.4f * ScreenScaler);
	hide.GetSpriteRef().setColor(sf::Color(255, 255, 255, 140));
	hide.SetPosition({ window.getView().getCenter().x + 580 * ScreenScalerX, window.getView().getCenter().y - 290 * ScreenScalerY });
	if (hide.Check(window, states::left_mouse_pressed, states::left_mouse_released, sf::Mouse::isButtonPressed(sf::Mouse::Left))) {
		my_settings.show_serve_info = !my_settings.show_serve_info;
	}
	if (my_settings.show_serve_info) {
		hide.SetRotation(90);
	}
	else {
		hide.SetRotation(-90);
	}

	if (my_settings.show_serve_info) {
		sf::Text server_info;
		{
			std::string ip = states::server_ip.toString(), port = std::to_string(states::server_port);
			server_info.setString(L"Сервер: " + std::wstring(ip.begin(), ip.end()) + L"\n" +
				std::wstring(port.begin(), port.end()) + L"\n"
			);
			if (my_settings.language == 1) {
				server_info.setString("Server: " + ip + "\n" +
					"Your port: " + port + "\n"
				);
			}
		}
		server_info.setCharacterSize(22 * ScreenScaler);
		server_info.setFont(arkhip);
		server_info.setFillColor(sf::Color(0, 0, 0, 120));
		server_info.setPosition({ window.getView().getCenter().x + 320 * ScreenScalerX, window.getView().getCenter().y - 340 * ScreenScalerY });
		window.draw(server_info);
	}
	hide.Draw(window);
}
void DrawUI(sf::RenderWindow& window, settings& my_settings, const int& fps, Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {
	DrawFps(window, my_settings, fps);
	DrawPlayerStates(window, my_settings, fps, player);
	DrawInventory(window, my_settings, fps, player, near_mao, mao);
	DrawRecipes(window, my_settings, fps, player, near_mao, mao);
	DrawServerInfo(window, my_settings, fps);
}


void listen() {
	//	sf::Clock clock;
		//sf::Time lastReceive = sf::seconds(0);
	for (int iteration = 0; iteration < 100; iteration++) {
		//	if (clock.getElapsedTime().asSeconds() - lastReceive.asSeconds() >= 0) {
		sf::IpAddress sender;
		unsigned short port;
		sf::Packet rp;
		if (socket.receive(rp, sender, port) == sf::Socket::Done) {

			sendingData rData, sData;

			if (rp >> rData) {

			}
			else {
				if (help_text != nullptr)
					help_text->setString("Open packet error\n");
				std::cout << "Open packet error\n";
				continue;
			}

			if (rData.command == "change_server_ip") {
				std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
				states::server_ip = sf::IpAddress(rData.message);
			}
			else if (rData.command == "add_player") {
				std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
				players.push_back(Player());
				players_controls.push_back(PlayerControls());
				std::string msg = rData.message;
				int player_skin_num, player_num;

				auto pos = msg.find("[player_skin_num=", 0) + 17;
				player_skin_num = std::stoi(msg.substr(pos, msg.find("]", pos) - pos));

				players[players.size() - 1].SetSpeed(normal_player_speed);
				players[players.size() - 1].SetSkinId(player_skin_num);
				players[players.size() - 1].SetSize(1, 1);
				players[players.size() - 1].SetPosition(2000 + std::rand() % 2000, 2000 + std::rand() % 2000);
				players[players.size() - 1].AddItem(items[item_id["Wood"]], 3);
				players[players.size() - 1].SetDamage(1);
				players[players.size() - 1].SetDef(0);
				players[players.size() - 1].SetMaxHp(100);
				players[players.size() - 1].SetHp(100);
				auto name = rData.nick.toWideString();
				players[players.size() - 1].SetName(name);
			}
			else if (rData.command == "change_player_num") {
				std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
				my_player = std::stoi(rData.message);
				std::cout << "My player num is: " << my_player << "\n";
			}
			if (iAmServer) {
				if (rData.command == "connect") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					if (!(sender.toString() == clients[0].first && port == clients[0].second))
						clients.push_back({ sender.toString(),port });
					sData = sendingData(states::my_local_ip.toString(), "change_server_ip", "Server");
					sf::Packet sendPacket;
					if (sendPacket << sData) {}
					else {
						std::cout << "Packet error (s)\n";
						if (help_text != nullptr)
							help_text->setString("Packet error (s)\n");
					}
					socket.send(sendPacket, sender, port);
					sendPacket.clear();
					for (auto& pl : players) {
						std::string msg = "";
						msg += "[player_skin_num=" + std::to_string(pl.GetSkinId()) + "]";
						sData = sendingData(msg, "add_player", pl.GetName());
						if (sendPacket << sData) {}
						else {
							std::cout << "Packet error (s)\n";
							if (help_text != nullptr)
								help_text->setString("Packet error (s)\n");
						}
						socket.send(sendPacket, sender, port);
					}
					{
						sf::Packet packet;
						sendingData dat(std::to_string(states::night) + "/", "client_night_changes", "Server");
						if (!(packet << dat)) std::cout << "error in packing [clientnight_chage]\n";
						for (int i = 1; i < clients.size(); i++) {
							if (socket.send(packet, sender, port) != sf::Socket::Done)
								std::cout << "error in sending [client_night_change]\n";
						}
					}
				}
				else if (rData.command == "server_add_player") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					sf::Packet sendPacket;
					sData = sendingData(std::to_string(players.size()), "change_player_num", "server");
					if (sendPacket << sData) {}
					else {
						std::cout << "Packet error (s)\n";
						if (help_text != nullptr)
							help_text->setString("Packet error (s)\n");
					}
					socket.send(sendPacket, sender, port);

					sendPacket.clear();
					sData = sendingData(rData.message, "add_player", rData.nick);
					if (sendPacket << sData) {}
					else {
						std::cout << "Packet error (s)\n";
						if (help_text != nullptr)
							help_text->setString("Packet error (s)\n");
					}
					for (auto client : clients) {
						socket.send(sendPacket, client.first, client.second);
					}
				}
				else if (rData.command == "client_controls") {
					std::string msg = rData.message;
					bool a, d, w, s, focus;
					int pos = 0, end = msg.find("/", pos);

					int pl_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					focus = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					a = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					d = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					w = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					s = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					sf::Vector2f m_pos;
					m_pos.x = std::stof(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					m_pos.y = std::stof(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					bool m_pressed;
					m_pressed = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					bool rm_pressed;
					rm_pressed = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					bool m_on_ui;
					m_on_ui = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					players_controls[pl_num] = PlayerControls(a, d, w, s, focus, m_pos, m_pressed, rm_pressed, m_on_ui);

				}
				else if (rData.command == "client_use") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);

					int pl_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					int num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					auto item = players[pl_num].GetItem(num).item;

					if (players[pl_num].ActiveItem() != nullptr) {
						if (players[pl_num].ActiveItem() == &players[pl_num].GetItem(num)
							&& typeid(*players[pl_num].ActiveItem()->item) == typeid(EquipmentItem)) {
							if (!static_cast<const EquipmentItem*>(players[pl_num].ActiveItem()->item)->GetSlot()) {
								players[pl_num].ChangeActiveItem();

								std::string msg = "-1";

								sf::Packet packet;
								sendingData dat(msg, "change_active_item_number", "Server");
								if (!(packet << dat)) std::cout << "error in packing [cl_use_resp1]\n";
								if (socket.send(packet, clients[pl_num].first, clients[pl_num].second) != sf::Socket::Done)
									std::cout << "error in sending [cl_use_resp1]\n";
							}
						}
						else if (item->isActivated()) {
							if (!static_cast<const EquipmentItem*>(item)->GetSlot()) {
								players[pl_num].ChangeActiveItem(players[pl_num].GetItem(num));

								std::string msg = std::to_string(num);

								sf::Packet packet;
								sendingData dat(msg, "change_active_item_number", "Server");
								if (!(packet << dat)) std::cout << "error in packing [cl_use_resp1]\n";
								if (socket.send(packet, clients[pl_num].first, clients[pl_num].second) != sf::Socket::Done)
									std::cout << "error in sending [cl_use_resp1]\n";
							}
						}
					}
					else if (typeid(*item) == typeid(EquipmentItem)) {
						if (!static_cast<const EquipmentItem*>(item)->GetSlot()) {
							players[pl_num].ChangeActiveItem(players[pl_num].GetItem(num));


							std::string msg = std::to_string(num);

							sf::Packet packet;
							sendingData dat(msg, "change_active_item_number", "Server");
							if (!(packet << dat)) std::cout << "error in packing [cl_use_resp1]\n";
							if (socket.send(packet, clients[pl_num].first, clients[pl_num].second) != sf::Socket::Done)
								std::cout << "error in sending [cl_use_resp1]\n";
						}
					}
					if (typeid(*item) == typeid(EquipmentItem) &&
						static_cast<const EquipmentItem*>(item)->GetSlot()) {
						if (players[pl_num].isDressed(item)) {
							players[pl_num].Undress(static_cast<const EquipmentItem*>(item));
						}
						else {
							players[pl_num].Dress(static_cast<const EquipmentItem*>(item));
						}

					}

					players[pl_num].UseItem(num, near_active_objects, map_active_objects, states::night);

				}
				else if (rData.command == "player_use_active_item") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);

					int pl_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					players[pl_num].UseActiveItem(near_active_objects, map_active_objects, states::night);
				}
				else if (rData.command == "player_undress") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);

					int pl_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					int item_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					players[pl_num].Undress(static_cast<const EquipmentItem*>(players[pl_num].GetItem(item_num).item));
				}
				else if (rData.command == "player_dress") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);

					int pl_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					int item_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					players[pl_num].Dress(static_cast<const EquipmentItem*>(players[pl_num].GetItem(item_num).item));
				}
				else if (rData.command == "player_drop") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);

					int pl_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					int item_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					int item_count = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					players[pl_num].DropItem(item_num, item_count, near_active_objects, map_active_objects);
				}
				else if (rData.command == "player_craft") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);

					int pl_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					int recipe_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);

					players[pl_num].Craft(recipes[recipe_num]);
				}
				else if (rData.command == "swap_items") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);

					int pl_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					int num1 = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					int num2 = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					players[pl_num].SwapItems(num1, num2);
				}
				else if (rData.command == "change_active_item") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);

					int pl_num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					int num = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					if (num == -1) {
						players[pl_num].ChangeActiveItem();
					}
					else players[pl_num].ChangeActiveItem(players[pl_num].GetItem(num));
				}
			}
			else {
				if (rData.command == "client_map_objects") {
					near_draw_objects.clear();
					near_active_objects.clear();
					near_active_objects.reserve(2000);
					near_draw_objects.reserve(2000);
					std::string msg = rData.message;
					int id;
					float px, py, sx, sy, r;
					int pos = 0, end = msg.find("/", pos);
					float arg1, arg2, arg3, arg4;
					while (end != std::string::npos) {
						try {
							id = std::stoi(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);

							px = std::stoi(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							py = std::stoi(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							sx = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							sy = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							r = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);

							arg1 = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							arg2 = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							arg3 = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							arg4 = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
						}
						catch (std::exception& e) {
							std::cout << e.what() << "\n";
						}
						Entity* b;
						bool active_obj = 0;
						if (typeid(*base_entity[id]) == typeid(MapDamaging)) {
							b = new MapDamaging(*static_cast<MapDamaging*>(base_entity[id]));
							active_obj = 1;
						}
						else if (typeid(*base_entity[id]) == typeid(Bonfire)) {
							b = new Bonfire(*static_cast<Bonfire*>(base_entity[id]));
							active_obj = 1;
						}
						else if (typeid(*base_entity[id]) == typeid(MapEntity)) {
							b = new MapEntity(*static_cast<MapEntity*>(base_entity[id]));
							active_obj = 1;
						}
						else if (typeid(*base_entity[id]) == typeid(MapActiveObject)) {
							b = new MapActiveObject(*static_cast<MapActiveObject*>(base_entity[id]));
							active_obj = 1;
						}
						else if (typeid(*base_entity[id]) == typeid(MaterialSource)) {
							active_obj = 1;
							b = new MaterialSource(*static_cast<MaterialSource*>(base_entity[id]));
						}
						else if (typeid(*base_entity[id]) == typeid(Animal)) {
							active_obj = 1;
							b = new Animal(*static_cast<Animal*>(base_entity[id]));
						}
						else if (typeid(*base_entity[id]) == typeid(Fleeing)) {
							active_obj = 1;
							b = new Fleeing(*static_cast<Fleeing*>(base_entity[id]));
						}
						else if (typeid(*base_entity[id]) == typeid(Enemy)) {
							active_obj = 1;
							b = new Enemy(*static_cast<Enemy*>(base_entity[id]));
						}
						else if (typeid(*base_entity[id]) == typeid(Box)) {
							active_obj = 1;
							b = new Box(*static_cast<Box*>(base_entity[id]));
						}
						else {
							active_obj = 0;
							b = new Entity(*static_cast<Entity*>(base_entity[id]));
						}
						if (active_obj) {
							near_active_objects.push_back(static_cast<MapActiveObject*>(b));
						}
						else {
							near_draw_objects.push_back(b);
						}
						near_active_objects[near_active_objects.size() - 1]->Transform({ px,py }, { sx,sy }, r);
						if (typeid(*base_entity[id]) == typeid(Enemy) ||
							typeid(*base_entity[id]) == typeid(Fleeing)) {
							near_active_objects[near_active_objects.size() - 1]->Rotate(base_entity[id]->GetRotation());
						}
						if (typeid(*base_entity[id]) == typeid(Bonfire)) {
							auto o = static_cast<Bonfire*>(near_active_objects[near_active_objects.size() - 1]);
							o->GetFireSpriteRef().setPosition({ arg1,arg2 });
							o->GetFireSpriteRef().setScale({ arg3,arg4 });
						}
						else {
							auto o = near_active_objects[near_active_objects.size() - 1];
							o->GetSpriteRef().setPosition({ arg1,arg2 });
							o->GetSpriteRef().setScale({ arg3,arg4 });
						}
					}
					if (states::night)
						NightChanges(players, near_active_objects, near_draw_objects);
				}
				else if (rData.command == "client_night_changes") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);
					states::night = std::stoi(msg.substr(pos, end - pos));
					pos = end + 1; end = msg.find("/", pos);
					NightChanges(players, near_active_objects, near_draw_objects);
				}
				else if (rData.command == "client_players") {
					//				std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					std::string msg = rData.message;
					int pos = 0, end = msg.find("/", pos);
					for (int i = 0; i < players.size(); i++) {
						int skin_id;
						float hp, hung, tempr, water;
						float px, py, sx, sy, r;
						float acpx, acpy, acsx, acsy;
						bool action;

						hp = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						hung = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						tempr = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						water = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);

						skin_id = std::stoi(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);

						px = std::stoi(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						py = std::stoi(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						sx = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						sy = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						r = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);

						acpx = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						acpy = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						acsx = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						acsy = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						action = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);

						int inv_capacity = std::stoi(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						players[i].GetInventoryRef().clear();
						for (int j = 0; j < inv_capacity; j++) {
							int id = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							int count = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							try {
								players[i].GetInventoryRef().push_back(Item(items[id], count));
							}
							catch (std::exception& e) {
								players[i].GetInventoryRef().push_back(Item(items[1], count));
								std::cout << e.what();
							}
						}
						int eq_capacity = std::stoi(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						players[i].GetEquipmentRef().clear();
						for (int j = 0; j < eq_capacity; j++) {
							int id = std::stof(msg.substr(pos, end - pos));
							pos = end + 1; end = msg.find("/", pos);
							if (id != -1) {
								try {
									players[i].GetEquipmentRef().push_back(static_cast<EquipmentItem*>(items[id]));
								}
								catch (std::exception& e) {
									players[i].GetEquipmentRef().push_back(static_cast<EquipmentItem*>(items[1]));
									std::cout << e.what();
								}
							}
							else {
								players[i].GetEquipmentRef().push_back(nullptr);
							}
						}
						int active_item = std::stoi(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						if (active_item != -1)
							players[i].ChangeActiveItem(*players[i].FindItem(items[active_item]));
						else
							players[i].ChangeActiveItem();

						float rpx, rpy, lpx, lpy;
						rpx = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						rpy = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						lpx = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						lpy = std::stof(msg.substr(pos, end - pos));
						pos = end + 1; end = msg.find("/", pos);
						players[i].SetArmsPos({ rpx,rpy }, { lpx,lpy });

						players[i].SetHp(hp);
						players[i].SetHunger(hung);
						players[i].SetTempreture(tempr);
						players[i].SetWater(water);
						players[i].SetSkinId(skin_id);
						players[i].Transform({ px,py }, { sx,sy }, r + 270);
						players[i].action = action;
						players[i].SetActionCollider(sf::FloatRect(acpx, acpy, acsx, acsy));
					}
				}
				else if (rData.command == "change_active_item_number") {
					active_item_number = std::stoi(rData.message);
				}
				else if (rData.command == "player_death") {
					std::cout << "Received: " << (std::string)rData.command << " from " << sender << " " << port << std::endl;
					cooperative_menu();
				}
			}
		}

		//	lastReceive = clock.getElapsedTime();
	//	}
	}
}

int main_game() {
	srand(time(0));
	setlocale(LC_ALL, "rus");

	sf::View main_view(sf::FloatRect(0.f, 0.f, my_settings.screen_resolution.x, my_settings.screen_resolution.y));
	sf::View ui_view(sf::FloatRect(0.f, 0.f, my_settings.screen_resolution.x, my_settings.screen_resolution.y));
	ScreenScalerX = my_settings.screen_resolution.x / 1280.0f;
	ScreenScalerY = my_settings.screen_resolution.y / 720.0f;
	ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1.f);
	main_view.zoom(1.f / ScreenScaler);
	window.setView(main_view);

	players.clear();

	sf::Clock clock;
	sf::Clock delta_clock;
	float delta_time = 0;


	int fps_counter = 0;
	int fps = 0;

	sf::Time lastFps = sf::seconds(0);
	sf::Time night_chage_time = sf::seconds(320);
	sf::Time night_change_timer = night_chage_time;

	last_near_ao_update = sf::seconds(-2);
	last_map_send = sf::seconds(-2);

	main_view.setCenter({ 5000,4000 });

	{
		sf::Packet packet;
		std::string msg = "";
		msg += "[player_skin_num=" + std::to_string(my_settings.player_num) + "]";
		sendingData dat(msg, "server_add_player", states::my_name);
		if (!(packet << dat)) std::cout << "error in packing [create_player]\n";
		if (socket.send(packet, states::server_ip, states::server_port) != sf::Socket::Done)
			std::cout << "error in sending [create_player]\n";
	}


	std::vector<std::vector<std::vector<unsigned short>>> current_map;
	current_map = main_map;
	sf::Vector2f map_start_pos = { 0,0 };

	map_active_objects.clear();
	map_draw_objects.clear();
	near_active_objects.clear();
	near_draw_objects.clear();

	map_active_objects.reserve(5000);
	map_draw_objects.reserve(5000);
	near_active_objects.reserve(2000);
	near_draw_objects.reserve(2000);


	std::this_thread::sleep_for(std::chrono::milliseconds(5));
	//

	// creatng map
	if (iAmServer) {
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
			map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 }, { 0.6f + rsz,0.6f + rsz }, -20 + std::rand() % 20);
		}
		for (int i = 0; i < 20; i++) {
			map_active_objects.push_back(new MaterialSource(*static_cast<MaterialSource*>(base_entity[2])));
			float rsz = (std::rand() % 6) * 0.01f;
			map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 }, { 0.6f + rsz,0.6f + rsz }, -20 + std::rand() % 20);
		}
		for (int i = 0; i < 120; i++) {
			map_active_objects.push_back(new Fleeing(*static_cast<Fleeing*>(base_entity[99])));
			map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 }, base_entity[99]->GetSize(), -20 + std::rand() % 20);
		}
		for (int i = 0; i < 80; i++) {
			map_active_objects.push_back(new Enemy(*static_cast<Enemy*>(base_entity[100])));
			map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 }, base_entity[100]->GetSize(), -20 + std::rand() % 20);
		}
		for (int i = 0; i < 50; i++) {
			map_active_objects.push_back(new Enemy(*static_cast<Enemy*>(base_entity[102])));
			map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 }, base_entity[102]->GetSize(), -20 + std::rand() % 20);
		}
		for (int i = 0; i < 1; i++) {
			map_active_objects.push_back(new Enemy(*static_cast<Enemy*>(base_entity[101])));
			map_active_objects[map_active_objects.size() - 1]->Transform({ 800.f + std::rand() % 8000,800.f + std::rand() % 7400 }, base_entity[101]->GetSize(), -20 + std::rand() % 20);
		}
	}
	//creatng walls
	if (iAmServer) {
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

	}
	window.setFramerateLimit(my_settings.frame_limit);
	while (window.isOpen())
	{

		listen();

		ClearKeyStates();
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
					if (!states::mouse_on_active_ui) {
						if (iAmServer)
							players[my_player].UseActiveItem(near_active_objects, map_active_objects, states::night);
						else {
							std::string msg = "";
							msg += std::to_string(my_player) + "/";

							sf::Packet use_packet;
							sendingData use_dat(msg, "player_use_active_item", states::my_name);
							if (!(use_packet << use_dat)) std::cout << "error in packing [player_use_active_item]\n";
							if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
								std::cout << "error in sending player_use_active_item]\n";
						}
					}

				}
				if (event.mouseButton.button == sf::Mouse::Button::Right) {
					states::right_mouse_pressed = 1;
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased) {
				if (event.mouseButton.button == sf::Mouse::Button::Left) {
					states::left_mouse_released = 1;
				}
				if (event.mouseButton.button == sf::Mouse::Button::Right) {
					states::right_mouse_released = 1;
				}
			}
			else if (event.type == sf::Event::MouseWheelScrolled) {
				if (states::locked_vision) {
					main_view.zoom(1 - event.mouseWheelScroll.delta * 0.1);
					states::locked_vision_zoom /= 1 - event.mouseWheelScroll.delta * 0.1;
				}
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::F1) {
					if (enable_to_change::show_more) {
						draw_more = !draw_more;
						enable_to_change::show_more = 0;
					}
				}
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
				/*if (event.key.code == sf::Keyboard::F10) {
					if (enable_to_change::show_map && iAmServer) {
						my_settings.draw_map = !my_settings.draw_map;
						enable_to_change::show_map= 0;
					}
				}*/
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
					auto item = players[my_player].GetItem(event.key.code - sf::Keyboard::Num1).item;
					if (iAmServer) {
						if (players[my_player].ActiveItem() != nullptr) {
							if (players[my_player].ActiveItem() == &players[my_player].GetItem(event.key.code - sf::Keyboard::Num1)
								&& typeid(*players[my_player].ActiveItem()->item) == typeid(EquipmentItem)) {
								if (!static_cast<const EquipmentItem*>(players[my_player].ActiveItem()->item)->GetSlot()) {
									players[my_player].ChangeActiveItem();
									active_item_number = -1;
								}
							}
							else if (item->isActivated()) {
								if (!static_cast<const EquipmentItem*>(item)->GetSlot()) {
									players[my_player].ChangeActiveItem(players[my_player].GetItem(event.key.code - sf::Keyboard::Num1));
									active_item_number = event.key.code - sf::Keyboard::Num1;
								}
							}
						}
						else if (typeid(*item) == typeid(EquipmentItem)) {
							if (!static_cast<const EquipmentItem*>(item)->GetSlot()) {
								players[my_player].ChangeActiveItem(players[my_player].GetItem(event.key.code - sf::Keyboard::Num1));
								active_item_number = event.key.code - sf::Keyboard::Num1;
							}
						}
						if (typeid(*item) == typeid(EquipmentItem) &&
							static_cast<const EquipmentItem*>(item)->GetSlot()) {
							if (players[my_player].isDressed(item)) {
								players[my_player].Undress(static_cast<const EquipmentItem*>(item));
							}
							else {
								players[my_player].Dress(static_cast<const EquipmentItem*>(item));
							}

						}
						players[my_player].UseItem(event.key.code - sf::Keyboard::Num1, near_active_objects, map_active_objects, states::night);
					}
					else {
						std::string msg = "";
						msg += std::to_string(my_player) + "/";
						msg += std::to_string(event.key.code - sf::Keyboard::Num1) + "/";

						sf::Packet use_packet;
						sendingData use_dat(msg, "client_use", states::my_name);
						if (!(use_packet << use_dat)) std::cout << "error in packing [use]\n";
						if (socket.send(use_packet, states::server_ip, states::server_port) != sf::Socket::Done)
							std::cout << "error in sending use]\n";
					}
				}
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::F1) {
					enable_to_change::show_more = 1;
				}
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
				if (event.key.code == sf::Keyboard::F10) {
					enable_to_change::show_map = 1;
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

		if (iAmServer) {
			night_change_timer -= sf::seconds(delta_time);
			if (night_change_timer <= sf::seconds(0)) {
				states::night = !states::night;
				NightChanges(players, map_active_objects, map_draw_objects);
				night_change_timer = night_chage_time;
				sf::Packet packet;
				sendingData dat(std::to_string(states::night) + "/", "client_night_changes", "Server");
				if (!(packet << dat)) std::cout << "error in packing [clientnight_chage]\n";
				for (int i = 1; i < clients.size(); i++) {
					if (socket.send(packet, clients[i].first, clients[i].second) != sf::Socket::Done)
						std::cout << "error in sending [client_night_change]\n";
				}
			}
			if (clock.getElapsedTime().asSeconds() - last_near_ao_update.asSeconds() >= my_settings.draw_obj_time) {
				near_active_objects.clear();
				near_draw_objects.clear();
				std::vector<std::string> msg(players.size(), "");
				for (auto& activeObject : map_active_objects) {
					bool pb = 0;
					for (int j = 0; j < players.size(); j++) {
						if (players[j].Distance(*activeObject) < my_settings.draw_obj_distance) {
							if (!pb) {
								near_active_objects.push_back(activeObject);
								pb = 1;
							}
						}
					}
				}
				for (auto& obj : map_draw_objects) {
					bool pb = 0;
					for (int j = 0; j < players.size(); j++) {
						if (players[j].Distance(*obj) < my_settings.draw_obj_distance) {
							if (!pb) {
								near_draw_objects.push_back(obj);
								pb = 1;
							}
						}
					}

					last_near_ao_update = clock.getElapsedTime();
				}
			}
			//
			if (clock.getElapsedTime().asSeconds() - last_map_send.asSeconds() >= my_settings.send_map_time) {
				std::vector<std::vector<std::string>> msg(players.size(), std::vector<std::string>(1, ""));
				for (auto& activeObject : near_active_objects) {
					for (int j = 1; j < players.size(); j++) {
						int msg_n = 0;
						if (players[j].Distance(*activeObject) < my_settings.draw_obj_distance) {
							msg[j][msg_n] += std::to_string(activeObject->GetId()) + "/";
							msg[j][msg_n] += std::to_string((int)(activeObject->GetPosition().x)) + "/";
							msg[j][msg_n] += std::to_string((int)(activeObject->GetPosition().y)) + "/";
							msg[j][msg_n] += std::to_string(activeObject->GetSize().x) + "/";
							msg[j][msg_n] += std::to_string(activeObject->GetSize().y) + "/";
							msg[j][msg_n] += std::to_string(activeObject->GetRotation()) + "/";

							if (typeid(*activeObject) == typeid(Bonfire)) {
								auto ao = static_cast<Bonfire*>(activeObject);
								msg[j][msg_n] += std::to_string(ao->GetFireSprite().getPosition().x) + "/";
								msg[j][msg_n] += std::to_string(ao->GetFireSprite().getPosition().y) + "/";
								msg[j][msg_n] += std::to_string(ao->GetFireSprite().getScale().x) + "/";
								msg[j][msg_n] += std::to_string(ao->GetFireSprite().getScale().y) + "/";
							}
							else {
								msg[j][msg_n] += std::to_string(activeObject->GetSprite().getPosition().x) + "/";
								msg[j][msg_n] += std::to_string(activeObject->GetSprite().getPosition().y) + "/";
								msg[j][msg_n] += std::to_string(activeObject->GetSprite().getScale().x) + "/";
								msg[j][msg_n] += std::to_string(activeObject->GetSprite().getScale().y) + "/";
							}
							if (msg[j][msg_n].size() >= 40000) {
								msg[j].push_back({ "" });
								msg_n++;
							}
						}
					}
				}
				for (auto& obj : map_draw_objects) {
					for (int j = 1; j < players.size(); j++) {
						int msg_n = 0;
						if (players[j].Distance(*obj) < my_settings.draw_obj_distance) {
							msg[j][msg_n] += std::to_string(obj->GetId()) + "/";
							msg[j][msg_n] += std::to_string((int)(obj->GetPosition().x)) + "/";
							msg[j][msg_n] += std::to_string((int)(obj->GetPosition().y)) + "/";
							msg[j][msg_n] += std::to_string(obj->GetSize().x) + "/";
							msg[j][msg_n] += std::to_string(obj->GetSize().y) + "/";
							msg[j][msg_n] += std::to_string(obj->GetRotation()) + "/";

							msg[j][msg_n] += std::to_string(obj->GetSprite().getPosition().x) + "/";
							msg[j][msg_n] += std::to_string(obj->GetSprite().getPosition().y) + "/";
							msg[j][msg_n] += std::to_string(obj->GetSprite().getScale().x) + "/";
							msg[j][msg_n] += std::to_string(obj->GetSprite().getScale().y) + "/";
						}
						if (msg[j][msg_n].size() >= 40000) {
							msg[j].push_back({ "" });
							msg_n++;
						}

					}

					last_map_send = clock.getElapsedTime();
				}
				for (int i = 1; i < players.size(); i++) {
					for (int j = 0; j < msg[i].size(); j++) {
						sf::Packet obj_packet;
						sendingData obj_dat(msg[i][j], "client_map_objects", "Server");
						if (!(obj_packet << obj_dat)) std::cout << "error in packing [client_map_objects" << j << "]\n";
						if (socket.send(obj_packet, clients[i].first, clients[i].second) != sf::Socket::Done)
							std::cout << "error in sending [client_map_objects" << j << "]\n";
					}
					std::string pl_msg = "";
					for (int j = 0; j < players.size(); j++) {
						pl_msg += std::to_string(players[j].GetHp()) + "/";
						pl_msg += std::to_string(players[j].GetHunger()) + "/";
						pl_msg += std::to_string(players[j].GetTempreture()) + "/";
						pl_msg += std::to_string(players[j].GetWater()) + "/";

						pl_msg += std::to_string(players[j].GetSkinId()) + "/";

						pl_msg += std::to_string(players[j].GetPosition().x) + "/";
						pl_msg += std::to_string(players[j].GetPosition().y) + "/";
						pl_msg += std::to_string(players[j].GetSize().x) + "/";
						pl_msg += std::to_string(players[j].GetSize().y) + "/";
						pl_msg += std::to_string(players[j].GetRotation()) + "/";

						pl_msg += std::to_string(players[j].GetActionCollider().left) + "/";
						pl_msg += std::to_string(players[j].GetActionCollider().top) + "/";
						pl_msg += std::to_string(players[j].GetActionCollider().width) + "/";
						pl_msg += std::to_string(players[j].GetActionCollider().height) + "/";
						pl_msg += std::to_string(players[j].action) + "/";

						pl_msg += std::to_string(players[j].InventoryCapacity()) + "/";
						for (int i = 0; i < players[j].InventoryCapacity(); i++) {
							pl_msg += std::to_string(players[j].GetItem(i).item->GetId()) + "/";
							pl_msg += std::to_string(players[j].GetItem(i).count) + "/";
						}

						pl_msg += std::to_string(players[j].GetEquipmentRef().size()) + "/";
						for (int i = 0; i < players[j].GetEquipmentRef().size(); i++) {
							if (players[j].GetEquipmentRef()[i] != nullptr)
								pl_msg += std::to_string(players[j].GetEquipmentRef()[i]->GetId()) + "/";
							else {
								pl_msg += std::to_string(-1) + "/";
							}
						}
						if (players[j].ActiveItem() != nullptr)
							pl_msg += std::to_string(players[j].ActiveItem()->item->GetId()) + "/";
						else pl_msg += std::to_string(-1) + "/";

						pl_msg += std::to_string(players[j].GetArmsPos().first.x) + "/";
						pl_msg += std::to_string(players[j].GetArmsPos().first.y) + "/";
						pl_msg += std::to_string(players[j].GetArmsPos().second.x) + "/";
						pl_msg += std::to_string(players[j].GetArmsPos().second.y) + "/";
					}
					sf::Packet pl_packet;
					sendingData pl_dat(pl_msg, "client_players", "Server");
					if (!(pl_packet << pl_dat)) std::cout << "error in packing [players]\n";
					if (socket.send(pl_packet, clients[i].first, clients[i].second) != sf::Socket::Done)
						std::cout << "error in sending [players]\n";
				}
			}
			//

			for (int i = 0; i < near_active_objects.size(); i++) {
				if (!near_active_objects[i]->ActiveBehaviour(players, near_active_objects, map_active_objects, delta_time, clock.getElapsedTime())) {
					auto obj = std::find(map_active_objects.begin(), map_active_objects.end(), near_active_objects[i]);

					if (map_active_objects.size() >= 50000) {
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
			std::vector<sf::Vector2i> map_cell_player_pos;
			std::vector<bool>player_on_water;
			for (int i = 0; i < players.size(); i++) {
				map_cell_player_pos.push_back({ (int)((players[i].GetPosition().x - map_start_pos.x) / (cell_size - 2)), (int)((players[i].GetPosition().y - map_start_pos.y) / (cell_size - 2)) });
			}
			for (int i = 0; i < players.size(); i++) {
				if (map_cell_player_pos[i].y < current_map[0].size() && map_cell_player_pos[i].x < current_map[0][map_cell_player_pos[i].y].size()) {
					if (current_map[0][map_cell_player_pos[i].y][map_cell_player_pos[i].x] == 2) {
						player_on_water.push_back(1);
						players[i].SetSpeed(water_player_speed);
						states::player_on_water = 1;
					}
					else {
						player_on_water.push_back(0);
						players[i].SetSpeed(normal_player_speed);
						states::player_on_water = 0;
					}
				}
			}
			if (player_on_water[my_player]) {
				
				if (water_music.getStatus()!=water_music.Playing) {
					water_music.play();
				}
			}
			else {
				if (water_music.getStatus() == water_music.Playing) {
					water_music.stop();
				}
			}

			for (int i = 0; i < players.size(); i++) {
				if (players[i].GetHp() <= 0) {
					if (i == my_player) {
						enable_to_change::move_view = 1;

						states::locked_vision = 0;
						main_view.zoom(states::locked_vision_zoom);
						states::locked_vision_zoom = 1;
					}
					if (i) {
						std::string msg = "";
						sf::Packet death_packet;
						sendingData death_dat(msg, "player_death", states::my_name);
						if (!(death_packet << death_dat)) std::cout << "error in packing [death]\n";
						if (socket.send(death_packet, clients[i].first, clients[i].second) != sf::Socket::Done)
							std::cout << "error in sending death]\n";
						clients.erase(clients.begin() + i);
						players.erase(players.begin() + i);
						players_controls.erase(players_controls.begin() + i);
						for (int j = i; j < players.size(); j++) {
							sf::Packet num_packet;
							sendingData num_dat(std::to_string(j), "change_player_num", states::my_name);
							if (!(num_packet << num_dat)) std::cout << "error in packing [num]\n";
							if (socket.send(num_packet, clients[i].first, clients[i].second) != sf::Socket::Done)
								std::cout << "error in sending num]\n";
						}
					}
					else {
						for (int j = 1; j < players.size(); j++) {
							std::string msg = "";
							sf::Packet death_packet;
							sendingData death_dat(msg, "player_death", states::my_name);
							if (!(death_packet << death_dat)) std::cout << "error in packing [death]\n";
							if (socket.send(death_packet, clients[j].first, clients[j].second) != sf::Socket::Done)
								std::cout << "error in sending death]\n";
						}
						main_menu();
						return 0;
					}
				}
			}
			players_controls[0].right_mouse_pressed = states::right_mouse_pressed;
			players_controls[0].mouse_pressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
			players_controls[0].mouse_on_ui = states::mouse_on_active_ui;
			players_controls[0].mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), main_view);
			players_controls[0].a = sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
			players_controls[0].d = sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
			players_controls[0].w = sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
			players_controls[0].s = sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
			for (int i = 0; i < players.size(); i++) {
				players[i].CheckStats(delta_time, player_on_water[i], states::night);

				if (players_controls[i].whf || (i == 0 && window.hasFocus())) {
					players[i].CheckMovement(delta_time, std::vector<SolidObject*>(near_active_objects.begin(), near_active_objects.end()), players_controls[i].a,
						players_controls[i].d, players_controls[i].w, players_controls[i].s);
					players[i].CheckControl(delta_time, players_controls[i].right_mouse_pressed, players_controls[0].mouse_on_ui, players_controls[i].mouse_pressed);
				}

				players[i].RotateTo(players_controls[i].mouse_pos, 270);
			}
		}


		if (!iAmServer) {
			//	for (int i = 0; i < near_active_objects.size(); i++) {
			//		near_active_objects[i]->ClientActiveBehaviour(players, near_active_objects, map_active_objects, delta_time, clock.getElapsedTime());
			//	}

			//////////////////

			players[my_player].CheckControl(delta_time, states::right_mouse_pressed, states::mouse_on_active_ui);

			std::string msg = "";
			msg += std::to_string(my_player) + "/";
			msg += std::to_string(window.hasFocus()) + "/";
			msg += std::to_string(sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) + "/";
			msg += std::to_string(sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) + "/";
			msg += std::to_string(sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) + "/";
			msg += std::to_string(sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) + "/";
			msg += std::to_string(window.mapPixelToCoords(sf::Mouse::getPosition(window), main_view).x) + "/";
			msg += std::to_string(window.mapPixelToCoords(sf::Mouse::getPosition(window), main_view).y) + "/";
			msg += std::to_string(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) + "/";
			msg += std::to_string(states::right_mouse_pressed) + "/";
			msg += std::to_string(states::mouse_on_active_ui) + "/";

			sf::Packet controls_packet;
			sendingData controls_dat(msg, "client_controls", states::my_name);
			if (!(controls_packet << controls_dat)) std::cout << "error in packing [client_controls]\n";
			if (socket.send(controls_packet, states::server_ip, states::server_port) != sf::Socket::Done)
				std::cout << "error in sending [client_controls]\n";

		}

		if (enable_to_change::move_view) {
			float camera_speed = players[my_player].GetSpeed() * 0.01;
			if (camera_speed < my_player)camera_speed *= -1;
			if (states::free_vision) {
				MoveViewTo(main_view, window.mapPixelToCoords(sf::Mouse::getPosition(window), main_view), delta_time * 1.0);
			}
			else {
				MoveViewTo(main_view, players[my_player], delta_time * camera_speed * 3);
			}
		}


		window.clear();
		window.setView(main_view);

		if (my_settings.draw_map)
			DrawMap(window, current_map, players, near_active_objects, near_draw_objects, delta_time, my_settings, my_player);

		if (my_settings.show_ui) {
			states::mouse_on_active_ui = 0;
			window.setView(ui_view);
			DrawUI(window, my_settings, fps, players[my_player], near_active_objects, map_active_objects);
		}



		if (states::item_throw_confirmation)
			ItemThrowConfirmation(window, my_settings, players[my_player], near_active_objects, map_active_objects, delta_time);

		window.display();
	}
	return EXIT_SUCCESS;
}

int cooperative_menu()
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
	if (my_settings.full_screen)
		main_view.setSize(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);

	ScreenScalerX = my_settings.screen_resolution.x / 1280.0f;
	ScreenScalerY = my_settings.screen_resolution.y / 720.0f;
	ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1.f);
	main_view.zoom(1.f / ScreenScaler);
	window.setView(main_view);
	main_view.setCenter(1000, 1000);

	clients.clear();
	clients.push_back({ sf::IpAddress::getLocalAddress().toString(),states::my_port });

	sf::Clock clock;
	sf::Clock delta_clock;
	float delta_time = 0;

	states::night = 0;

	int fps_counter = 0;
	int fps = 0;

	sf::Time lastFps = sf::seconds(0);
	sf::Time save_timer = sf::seconds(0);

	std::vector<std::vector<std::vector<unsigned short>>> current_map;
	current_map = main_map;
	sf::Vector2f map_start_pos = { 0,0 };

	std::vector<MapActiveObject*> map_active_objects;
	map_active_objects.reserve(200);

	std::vector<Entity*> map_draw_objects;

	TextField name_field(sf::String(states::my_name), 22, arkhip, { 850,780,270,40 }, 1, sf::Color(245, 245, 245, 245));
	TextField ip_field(sf::String(states::server_ip.toString()), 22, arkhip, { 850,870,270,40 }, 1, sf::Color(245, 245, 245, 245));
	TextField port_field(sf::String(std::to_string(states::server_port)), 22, arkhip, { 850,960,270,40 }, 1, sf::Color(245, 245, 245, 245));

	std::vector<TextField*>text_fields;
	text_fields.push_back(&name_field);
	text_fields.push_back(&ip_field);
	text_fields.push_back(&port_field);

	sf::Text output_text;
	{
		std::string local_ip = states::my_local_ip.toString(), public_ip = states::my_public_ip.toString(), port = std::to_string(states::my_port);
		output_text.setString(L"Ваш публичный айпи: " + std::wstring(public_ip.begin(), public_ip.end()) + L"\n" +
			L"Ваш локальный айпи: " + std::wstring(local_ip.begin(), local_ip.end()) + L"\n" +
			L"Ваш порт: " + std::wstring(port.begin(), port.end()) + L"\n"
		);
		if (my_settings.language == 1) {
			output_text.setString("Your public ip: " + public_ip + "\n" +
				"Your local ip: " + local_ip + "\n" +
				"Your port: " + port + "\n"
			);
		}
	}
	std::string base_output_text = output_text.getString();

	output_text.setCharacterSize(22);
	output_text.setFont(arkhip);
	output_text.setFillColor(sf::Color::Black);
	output_text.setPosition({ 500,1150 });

	window.setFramerateLimit(my_settings.frame_limit);
	while (window.isOpen())
	{
		ClearKeyStates();
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::TextEntered) {
				if (event.text.unicode < 128) {
					//std::cout << (char)event.text.unicode;
				}
				if (event.text.unicode >= '0' && event.text.unicode <= '9') {
					if (port_field.isActive)
						port_field.PutChar(event.text.unicode);
				}
				if ((event.text.unicode >= 'a' && event.text.unicode <= 'z') ||
					(event.text.unicode >= '0' && event.text.unicode <= '9') ||
					(event.text.unicode >= L'а' && event.text.unicode <= L'я') || event.text.unicode == ' ' ||
					event.text.unicode == '.' || event.text.unicode == '\\' || event.text.unicode == '/' ||
					event.text.unicode == ',' || event.text.unicode == '<' || event.text.unicode == '>' ||
					event.text.unicode == '(' || event.text.unicode == ')' || event.text.unicode == '[' ||
					event.text.unicode == ']' || event.text.unicode == '@' || event.text.unicode == '#' ||
					event.text.unicode == '$' || event.text.unicode == '?' || event.text.unicode == '!' ||
					event.text.unicode == '-' || event.text.unicode == '_' || event.text.unicode == '=' ||
					event.text.unicode == '+' || event.text.unicode == '%' || event.text.unicode == '\'' ||
					event.text.unicode == '\"' || event.text.unicode == ':' || event.text.unicode == ';' ||
					(event.text.unicode >= 'A' && event.text.unicode <= 'Z') ||
					(event.text.unicode >= L'А' && event.text.unicode <= L'Я')
					) {
					for (int i = 0; i < text_fields.size(); i++) {
						if (text_fields[i]->isActive) {
							if (text_fields[i] == &port_field)continue;
							text_fields[i]->PutChar(event.text.unicode);
						}
					}
				}
			}
			else if (event.type == sf::Event::Resized) {
				sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
				main_view = sf::View(visibleArea);
				ScreenScalerX = event.size.width / 1280.0f;
				ScreenScalerY = event.size.height / 720.0f;
				ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1.f);
				main_view.zoom(1.f / ScreenScaler);
				main_view.setCenter(1000, 1000);
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
				if (event.key.code == sf::Keyboard::Backspace) {
					for (int i = 0; i < text_fields.size(); i++) {
						if (text_fields[i]->isActive) {
							text_fields[i]->EraseChar();
						}
					}
				}
				if (event.key.code == sf::Keyboard::Enter) {
					states::enter_pressed = 1;
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


		Button back(L"Назад", 36, arkhip, 1);
		back.SetPosition({ 600,700 });

		if (my_settings.language == 1)
			back.setString("Back");

		back.GetTextRef().setLetterSpacing(2);
		back.GetTextRef().setFillColor(sf::Color(220, 220, 140, 245));
		back.GetTextRef().setOutlineColor(sf::Color::Black);
		back.GetTextRef().setOutlineThickness(5);
		if (back.Check(window, states::left_mouse_pressed, states::left_mouse_released)) {
			auto nm = std::wstring(name_field.GetTextRef().getString().begin(), name_field.GetTextRef().getString().end());
			if (!nm.size()) nm = L"Unnamed" + std::to_wstring(states::my_port);
			states::my_name = nm;
			main_menu();
			break;
		}

		/*Button run_server(L"Создать", 36, arkhip, 1);
		run_server.SetPosition({ 830,1070 });
		if (my_settings.language == 1)
			run_server.setString("Create");
		run_server.GetTextRef().setLetterSpacing(2);
		run_server.GetTextRef().setFillColor(sf::Color(220, 220, 140, 245));
		run_server.GetTextRef().setOutlineColor(sf::Color::Black);
		run_server.GetTextRef().setOutlineThickness(5);
		if (run_server.Check(window, states::left_mouse_pressed, states::left_mouse_released)) {
			auto nm = std::wstring(name_field.GetTextRef().getString().begin(), name_field.GetTextRef().getString().end());
			if (!nm.size()) nm = L"Unnamed" + std::to_wstring(states::my_port);
			states::my_name = nm;
			states::server_ip = states::my_local_ip;
			states::server_port = states::my_port;
			main_game();
			break;
		}*/

		Button join_server(L"Присоединится", 36, arkhip, 1);
		join_server.SetPosition({ 830,1010 });
		if (my_settings.language == 1)
			join_server.setString("Join");
		join_server.GetTextRef().setLetterSpacing(2);
		join_server.GetTextRef().setFillColor(sf::Color(220, 220, 140, 245));
		join_server.GetTextRef().setOutlineColor(sf::Color::Black);
		join_server.GetTextRef().setOutlineThickness(5);
		if (join_server.Check(window, states::left_mouse_pressed, states::left_mouse_released)) {
			sf::IpAddress send_ip;
			unsigned short send_port;
			try {
				send_ip = sf::IpAddress(ip_field.GetTextRef().getString());
				send_port = std::stoi(std::string(port_field.GetTextRef().getString()));
			}
			catch (...) {
				if (my_settings.language == 1)
					output_text.setString(base_output_text + "Enter server ip and port correctly!\n");
				else
					output_text.setString(std::wstring(base_output_text.begin(), base_output_text.end()) + L"Введите айпи и порт сервера правильно!\n");
				continue;
			}
			states::server_ip = send_ip;
			states::server_port = send_port;
			auto nm = std::wstring(name_field.GetTextRef().getString().begin(), name_field.GetTextRef().getString().end());
			if (!nm.size()) nm = L"Unnamed" + std::to_wstring(states::my_port);
			states::my_name = nm;
			if ((states::server_ip == states::my_local_ip || states::server_ip == sf::IpAddress::Broadcast) && states::server_port == states::my_port)
				iAmServer = 1;
			else
				iAmServer = 0;
			sf::Packet packet;
			sendingData dat(states::my_name + L" has joined the server\n", "connect", states::my_name);
			if (packet << dat) {

			}
			else {
				if (my_settings.language == 1)
					output_text.setString(base_output_text + "Error in packing data!\n");
				else
					output_text.setString(std::wstring(base_output_text.begin(), base_output_text.end()) + L"Ошипка упаковки данных!\n");
			}
			if (socket.send(packet, states::server_ip, states::server_port) != sf::Socket::Done)
			{
				if (my_settings.language == 1)
					output_text.setString(base_output_text + "Sending error!\n");
				else
					output_text.setString(std::wstring(base_output_text.begin(), base_output_text.end()) + L"Ошипка отправки!\n");
			}
			main_game();
		}

		for (int i = 0; i < text_fields.size(); i++) {
			text_fields[i]->Check(window, states::left_mouse_pressed, states::enter_pressed, sf::Mouse::isButtonPressed(sf::Mouse::Left));
		}

		///////////////////


		window.clear();
		window.setView(main_view);


		DrawMap(window, current_map, *(new std::vector<Player>(1, *(new Player()))), map_active_objects, map_draw_objects, delta_time, my_settings, 0);


		back.Draw(window);
		join_server.Draw(window);
		//	run_server.Draw(window);
		window.draw(output_text);
		for (int i = 0; i < text_fields.size(); i++) {
			text_fields[i]->Draw(window);
		}

		window.display();
	}
	return EXIT_SUCCESS;
}

int main_menu()
{
	srand(time(0));
	setlocale(LC_ALL, "rus");

	clients.clear();
	clients.push_back({ sf::IpAddress::getLocalAddress().toString(),states::my_port });
	players.clear();
	map_active_objects.clear();
	map_draw_objects.clear();
	near_active_objects.clear();
	near_draw_objects.clear();

	std::ifstream f("settings.txt", std::ios::in);
	if (!f.eof()) {
		f >> my_settings.language;
		f >> my_settings.player_num;
		f >> my_settings.frame_limit;
	}
	f.close();

	std::wfstream wf("settings.txt", std::ios::in);
	wf.imbue(std::locale(wf.getloc(), new std::codecvt_utf8_utf16<wchar_t, 0x10ffff, std::codecvt_mode(std::consume_header | std::generate_header)>));
	for (int i = 0; i < 4; i++)
		wf >> states::my_name;
	wf.close();


	sf::View main_view(sf::FloatRect(0.f, 0.f, my_settings.screen_resolution.x, my_settings.screen_resolution.y));
	if (my_settings.full_screen)
		main_view.setSize(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);

	ScreenScalerX = my_settings.screen_resolution.x / 1280.0f;
	ScreenScalerY = my_settings.screen_resolution.y / 720.0f;
	ScreenScaler = nearest(ScreenScalerX, ScreenScalerY, 1.f);
	main_view.zoom(1.f / ScreenScaler);
	window.setView(main_view);
	main_view.setCenter(1700, 1700);


	sf::Clock clock;
	sf::Clock delta_clock;
	float delta_time = 0;

	states::night = 0;

	int fps_counter = 0;
	int fps = 0;

	sf::Time lastFps = sf::seconds(0);
	sf::Time save_timer = sf::seconds(0);

	TextField name_field(sf::String(states::my_name), 22, arkhip, { 1560,1550,270,40 }, 1, sf::Color(245, 245, 245, 245));

	std::vector<std::vector<std::vector<unsigned short>>> current_map;
	current_map = main_map;
	sf::Vector2f map_start_pos = { 0,0 };

	std::vector<MapActiveObject*> map_active_objects;
	map_active_objects.reserve(200);

	std::vector<Entity*> map_draw_objects;


	{
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
	}
	Player player;
	player.SetPosition(main_view.getCenter().x, main_view.getCenter().y + 100);
	player.SetTexture(player_textures[my_settings.player_num], { 20,90 });
	player.SetArmsTexture(player_arm_textures[my_settings.player_num]);
	player.SetSize(1.1, 1.1);

	window.setFramerateLimit(my_settings.frame_limit);
	while (window.isOpen())
	{
		ClearKeyStates();
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
			else if (event.type == sf::Event::TextEntered) {
				if ((event.text.unicode >= 'a' && event.text.unicode <= 'z') ||
					(event.text.unicode >= '0' && event.text.unicode <= '9') ||
					(event.text.unicode >= L'а' && event.text.unicode <= L'я') || event.text.unicode == ' ' ||
					event.text.unicode == '.' || event.text.unicode == '\\' || event.text.unicode == '/' ||
					event.text.unicode == ',' || event.text.unicode == '<' || event.text.unicode == '>' ||
					event.text.unicode == '(' || event.text.unicode == ')' || event.text.unicode == '[' ||
					event.text.unicode == ']' || event.text.unicode == '@' || event.text.unicode == '#' ||
					event.text.unicode == '$' || event.text.unicode == '?' || event.text.unicode == '!' ||
					event.text.unicode == '-' || event.text.unicode == '_' || event.text.unicode == '=' ||
					event.text.unicode == '+' || event.text.unicode == '%' || event.text.unicode == '\'' ||
					event.text.unicode == '\"' || event.text.unicode == ':' || event.text.unicode == ';' ||
					(event.text.unicode >= 'A' && event.text.unicode <= 'Z') ||
					(event.text.unicode >= L'А' && event.text.unicode <= L'Я')
					) {
					if (name_field.isActive) {
						name_field.PutChar(event.text.unicode);
					}
				}

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
				if (event.key.code == sf::Keyboard::Backspace) {
					if (name_field.isActive) {
						name_field.EraseChar();
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
			if (!map_active_objects[i]->ActiveBehaviour(*(new std::vector<Player>(1, player)), map_active_objects, map_active_objects, delta_time, clock.getElapsedTime())) {
			}
		}

		name_field.Check(window, states::left_mouse_pressed, states::enter_pressed, sf::Mouse::isButtonPressed(sf::Mouse::Left));

		Button rus(language_button_texture[0], 0.7f), eng(language_button_texture[1], 0.7f), bel(language_button_texture[2], 0.7f);
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
		play.SetPosition({ 1600,1650 });

		if (my_settings.language == 0)
			play.setString(L"Играть");
		else if (my_settings.language == 2)
			play.setString(L"Гуляць");
		play.GetTextRef().setLetterSpacing(2);
		play.GetTextRef().setFillColor(sf::Color(220, 220, 140, 245));
		play.GetTextRef().setOutlineColor(sf::Color::Black);
		play.GetTextRef().setOutlineThickness(5);
		if (play.Check(window, states::left_mouse_pressed, states::left_mouse_released)) {
			auto nm = std::wstring(name_field.GetTextRef().getString().begin(), name_field.GetTextRef().getString().end());
			if (!nm.size()) nm = L"Unnamed" + std::to_wstring(states::my_port);
			states::my_name = nm;
			states::server_ip = states::my_local_ip;
			states::server_port = states::my_port;
			iAmServer = 1;
			main_game();
			break;
		}

		Button cooperative("Cooperative", 36, arkhip, 1);
		cooperative.SetPosition({ 1800,1650 });

		if (my_settings.language == 0)
			cooperative.setString(L"Кооператив");
		else if (my_settings.language == 2)
			cooperative.setString(L"Кооператив");
		cooperative.GetTextRef().setLetterSpacing(2);
		cooperative.GetTextRef().setFillColor(sf::Color(220, 220, 140, 245));
		cooperative.GetTextRef().setOutlineColor(sf::Color::Black);
		cooperative.GetTextRef().setOutlineThickness(5);
		if (cooperative.Check(window, states::left_mouse_pressed, states::left_mouse_released)) {
			auto nm = std::wstring(name_field.GetTextRef().getString().begin(), name_field.GetTextRef().getString().end());
			if (!nm.size()) nm = L"Unnamed" + std::to_wstring(states::my_port);
			states::my_name = nm;
			cooperative_menu();
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
		left.SetFunc([]() {my_settings.player_num--; if (my_settings.player_num < 0)my_settings.player_num = player_textures.size() - 1; });
		left.Flip();

		right.SetPosition({ 1800,1800 });
		right.SetFunc([]() {my_settings.player_num++; if (my_settings.player_num >= player_textures.size())my_settings.player_num = 0; });

		if (left.Check(window, states::left_mouse_pressed, states::left_mouse_released, sf::Mouse::isButtonPressed(sf::Mouse::Left))) {
			player.SetSkinId(my_settings.player_num);
			player.SetSize(1.1, 1.1);
		}
		if (right.Check(window, states::left_mouse_pressed, states::left_mouse_released, sf::Mouse::isButtonPressed(sf::Mouse::Left))) {
			player.SetSkinId(my_settings.player_num);
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
			f << "\n";
			f.close();
			states::my_name = name_field.GetTextRef().getString();
			std::wfstream wf("settings.txt", std::ios::out | std::ios::app);
			wf.imbue(std::locale(wf.getloc(), new std::codecvt_utf8_utf16<wchar_t, 0x10ffff, std::codecvt_mode(std::consume_header | std::generate_header)>));
			wf << states::my_name << L"\n";
			wf.close();
			save_timer = sf::seconds(0);
		}

		///////////////////


		window.clear();
		window.setView(main_view);


		DrawMap(window, current_map, *(new std::vector<Player>(1, player)), map_active_objects, map_draw_objects, delta_time, my_settings, 0);

		window.draw(rect);
		cooperative.Draw(window);
		rus.Draw(window);
		eng.Draw(window);
		bel.Draw(window);
		play.Draw(window);
		quit.Draw(window);
		left.Draw(window);
		right.Draw(window);
		name_field.Draw(window);
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

	sf::Text loading;
	loading.setLetterSpacing(2);
	loading.setCharacterSize(40 * ScreenScaler);
	loading.setFont(you2013);
	loading.setPosition(50, 200);

	if (!resources_loaded) {
		LoadResources(window, loading, my_settings.language, ScreenScaler);
		resources_loaded = 1;
	}
	////////////////////////////////////////////////////////////////////

// bind the socket to a port
	if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Done)
	{
		std::cout << "ERROR TO BIND SOCKET TO ANY PORT\n";
	}
	states::my_port = socket.getLocalPort();
	states::my_local_ip = sf::IpAddress::getLocalAddress();
	states::my_public_ip = sf::IpAddress::getPublicAddress();
	states::server_port = states::my_port;
	states::server_ip = states::my_local_ip;
	//states::server_ip = sf::IpAddress::Broadcast;

	if (states::my_name.size() == 0)states::my_name = L"Unnamed" + std::to_wstring(states::my_port);

	std::cout << "local: " << sf::IpAddress::getLocalAddress().toString() << "\n";
	std::cout << "public: " << sf::IpAddress::getPublicAddress().toString() << "\n";

	socket.setBlocking(false);
	////////////////////////////////////////////////////////////////////////

	clients.clear();
	clients.push_back({ sf::IpAddress::getLocalAddress().toString(),states::my_port });

	music.play();
	main_menu();

	return 0;
}