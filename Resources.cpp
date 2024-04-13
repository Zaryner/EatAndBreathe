#pragma once
#include "Resources.h"
#include "Entities.h"
#include "EngineFuncs.h"

#include <string>
#include <fstream>
#include <iostream>



sf::Font arkhip;
sf::Font you2013;
sf::Font mullerExtrabold;
sf::Font comfortaa;
sf::Font fregat;

sf::Music music;
sf::Music water_music;
sf::SoundBuffer woodSoundBuffer;
sf::Sound wood_sound;
sf::SoundBuffer eatSoundBuffer;
sf::Sound eat_sound;


sf::Color night_color = sf::Color(50,50,50);

////////////////////////
std::vector<sf::Texture> player_textures;
std::vector<sf::Texture> player_arm_textures;
std::vector<sf::Vector2f>player_texture_pos;

std::vector<sf::Texture> tree_textures;
sf::Texture bluebluebush_texture;
sf::Texture stone_texture;
sf::Texture gold_texture;

sf::Texture bar_texture[2];
sf::Texture bar_filling_texture[2];
sf::Texture icon_placement_texture[6];

sf::Texture heart_texture;
sf::Texture water_texture;
sf::Texture meat_texture;
sf::Texture tempreture_texture;

std::vector < sf::Texture> bonfire_texture;
std::vector < sf::Texture> bonfire_fire_texture;
std::vector < sf::Texture> wall_texture;
std::vector < sf::Texture> spiky_wall_texture;

std::vector < sf::Texture> language_button_texture;
sf::Texture arrow_button_texture;

sf::Texture inventory_cell_texture[4];

std::vector<sf::Texture> cell_textures;
/////////////////////////////////////
std::vector<std::vector<std::vector<unsigned short>>> main_map;
sf::Vector2f map_start_pos = { 0,0 };

std::vector<sf::Texture>entity_textures;
std::vector<Entity*> base_entity;

std::vector<BaseItem*> items;
std::vector<Recipe*> recipes;
std::map<std::string, int>item_id;

bool is_fonts_loaded = 0;
bool is_cells_loaded = 0;
bool is_player_textures_loaded = 0;
bool is_ui_texture_loaded = 0;

bool resources_loaded = 0;

bool draw_colliders = 0;
bool draw_more = 0;


void LoadCellsTextures() {
	if (!is_cells_loaded) {
		cell_textures.push_back(sf::Texture());
		cell_textures[0].loadFromFile("resources\\cells\\cell_00.png");
		cell_textures.push_back(sf::Texture());
		cell_textures[1].loadFromFile("resources\\cells\\cell_04.png");
		cell_textures.push_back(sf::Texture());
		cell_textures[2].loadFromFile("resources\\cells\\cell_05.png");
		cell_textures.push_back(sf::Texture());
		cell_textures[3].loadFromFile("resources\\cells\\cell_06.png");
		cell_textures.push_back(sf::Texture());

		is_cells_loaded = 1;
	}
}
void LoadPlayerTextures() {
	if (!is_player_textures_loaded) {
		std::ifstream f("resources\\players.conf", std::ios::in);
		int skip = 0;
		while (!f.eof()) {
			std::string line, tmp;
			do {
				std::getline(f, tmp);
				if (tmp.find("[skip]") != std::string::npos)skip++;
				if (tmp.find("[skip_end]") != std::string::npos) {
					skip--;
					if (skip < 0)skip = 0;
				}
				if (skip <= 0)
					line += tmp;
			} while (tmp.find("[end]") == std::string::npos && !f.eof());

			try {
				if (line == "\n" || line == " " || line == "")continue;
				player_textures.push_back(sf::Texture());
				player_arm_textures.push_back(sf::Texture());
				player_texture_pos.push_back(sf::Vector2f(0, 0));
				try {
					auto px_pos = line.find("[px=", 0) + 4;
					float px = std::stof(line.substr(px_pos, line.find("]", px_pos) - px_pos));
					auto py_pos = line.find("[py=", 0) + 4;
					float py = std::stof(line.substr(py_pos, line.find("]", py_pos) - py_pos));
					player_texture_pos[player_texture_pos.size() - 1] = { px,py };
				}
				catch (...) {}
				try {
					auto sprite_pos = line.find("[arm=", 0) + 5;
					player_arm_textures[player_arm_textures.size() - 1].loadFromFile("resources\\" + (sf::String)(line.substr(sprite_pos, line.find("]", sprite_pos) - sprite_pos)));
					player_arm_textures[player_arm_textures.size() - 1].setSmooth(1);
				}
				catch (...) {}

				auto sprite_pos = line.find("[sprite=", 0) + 8;
				player_textures[player_textures.size() - 1].loadFromFile("resources\\" + (sf::String)(line.substr(sprite_pos, line.find("]", sprite_pos) - sprite_pos)));
				player_textures[player_textures.size() - 1].setSmooth(1);

			}
			catch (...) {
				player_textures.pop_back();
				player_arm_textures.pop_back();
				player_texture_pos.pop_back();
				std::cout << "Error to load player, line: " << line << "\n";
				continue;
			}
		}
		f.close();

		is_player_textures_loaded = 1;
	}
}
void LoadUITextures() {
	if (!is_ui_texture_loaded) {
		bar_texture[0].loadFromFile("resources\\Bar01.png");
		bar_texture[0].setSmooth(1);
		bar_texture[1].loadFromFile("resources\\Bar02.png");
		bar_texture[1].setSmooth(1);
		bar_filling_texture[0].loadFromFile("resources\\BarFilling.png");
		bar_filling_texture[0].setSmooth(1);

		icon_placement_texture[0].loadFromFile("resources\\IconPlacement01.png");
		icon_placement_texture[0].setSmooth(1);


		heart_texture.loadFromFile("resources\\Heart.png");
		heart_texture.setSmooth(1);
		meat_texture.loadFromFile("resources\\Meat.png");
		meat_texture.setSmooth(1);
		tempreture_texture.loadFromFile("resources\\Tempreture.png");
		tempreture_texture.setSmooth(1);
		water_texture.loadFromFile("resources\\Water.png");
		water_texture.setSmooth(1);

		inventory_cell_texture[0].loadFromFile("resources\\InventoryCell01.png");
		inventory_cell_texture[0].setSmooth(1);
		inventory_cell_texture[1].loadFromFile("resources\\InventoryCell02.png");
		inventory_cell_texture[1].setSmooth(1);
		inventory_cell_texture[2].loadFromFile("resources\\InventoryCell03.png");
		inventory_cell_texture[2].setSmooth(1);
		inventory_cell_texture[3].loadFromFile("resources\\InventoryCell04.png");
		inventory_cell_texture[3].setSmooth(1);

		language_button_texture.push_back(sf::Texture());
		language_button_texture[0].loadFromFile("resources\\rubut.png");
		language_button_texture[0].setSmooth(1);
		language_button_texture.push_back(sf::Texture());
		language_button_texture[1].loadFromFile("resources\\engbut.png");
		language_button_texture[1].setSmooth(1);
		language_button_texture.push_back(sf::Texture());
		language_button_texture[2].loadFromFile("resources\\belbut.png");
		language_button_texture[2].setSmooth(1);

		arrow_button_texture.loadFromFile("resources\\arrowbut.png");
		arrow_button_texture.setSmooth(1);

		is_ui_texture_loaded = 1;
	}
}
void LoadFonts() {
	if (!is_fonts_loaded) {
		is_fonts_loaded = 1;
		arkhip.loadFromFile("resources\\fonts\\arkhip.ttf");
		you2013.loadFromFile("resources\\fonts\\you2013.ttf");
		mullerExtrabold.loadFromFile("resources\\fonts\\mullerExtrabold.ttf");
		comfortaa.loadFromFile("resources\\fonts\\Comfortaa.ttf");
		fregat.loadFromFile("resources\\fonts\\Fregat_regular.otf");
	}
}
void LoadEntityTextures() {
	tree_textures.push_back(sf::Texture());
	tree_textures[0].loadFromFile("resources\\tree01.png");
	tree_textures[0].setSmooth(1);

	tree_textures.push_back(sf::Texture());
	tree_textures[1].loadFromFile("resources\\tree02.png");
	tree_textures[1].setSmooth(1);

	tree_textures.push_back(sf::Texture());
	tree_textures[2].loadFromFile("resources\\palm01.png");
	tree_textures[2].setSmooth(1);

	bluebluebush_texture.loadFromFile("resources\\bluebluebush.png");
	bluebluebush_texture.setSmooth(1);

	stone_texture.loadFromFile("resources\\Stone01.png");
	stone_texture.setSmooth(1);
	gold_texture.loadFromFile("resources\\Gold01.png");
	gold_texture.setSmooth(1);

	bonfire_texture.push_back(sf::Texture());
	bonfire_texture[0].loadFromFile("resources\\Bonfire02.png");
	bonfire_texture[0].setSmooth(1);

	bonfire_fire_texture.push_back(sf::Texture());
	bonfire_fire_texture[0].loadFromFile("resources\\Bonfire02c.png");
	bonfire_fire_texture[0].setSmooth(1);

	wall_texture.push_back(sf::Texture());
	wall_texture[0].loadFromFile("resources\\Wall01.png");
	wall_texture[0].setSmooth(1);
	wall_texture.push_back(sf::Texture());
	wall_texture[1].loadFromFile("resources\\Wall02.png");
	wall_texture[1].setSmooth(1);
	wall_texture.push_back(sf::Texture());
	wall_texture[2].loadFromFile("resources\\Wall03.png");
	wall_texture[2].setSmooth(1);

	spiky_wall_texture.push_back(sf::Texture());
	spiky_wall_texture[0].loadFromFile("resources\\SpikyWall01.png");
	spiky_wall_texture[0].setSmooth(1);
	spiky_wall_texture.push_back(sf::Texture());
	spiky_wall_texture[1].loadFromFile("resources\\SpikyWall02.png");
	spiky_wall_texture[1].setSmooth(1);
	spiky_wall_texture.push_back(sf::Texture());
	spiky_wall_texture[2].loadFromFile("resources\\SpikyWall03.png");
	spiky_wall_texture[2].setSmooth(1);
}
//void LoadResources() {          /////// depricated
//	ClearResources();
//
//	LoadCellsTextures();
//	LoadPlayerTextures();
//	LoadUITextures();
//	LoadFonts();
//	LoadEntityTextures();
//
//	LoadItems();
//	LoadRecipes();
//
//	LoadBaseEntities();
//
//	if (!music.openFromFile("resources\\BgBirdMusic.ogg")) {
//		std::cout << "Music load error";
//	}
//
//	music.setVolume(50);         // reduce the volume
//	music.setLoop(true);
//
//	resources_loaded = 1;
//}

void LoadResources(sf::RenderWindow& window, sf::Text& s, const int& language, const float& ScreenScaler) {
	sf::Text load_text;
	load_text.setLetterSpacing(2);
	load_text.setCharacterSize(40 * ScreenScaler);
	load_text.setFont(you2013);
	load_text.setString("Loading resources");
	if (language == 0)
		load_text.setString(L"Загрузка ресурсов");

	s.setString(L"Согреваем солышком песок");
	if (language == 1)
		s.setString("Warm sand with the sun");

	window.clear();
	window.draw(load_text);
	window.draw(s);
	window.display();

	ClearResources();

	LoadCellsTextures();
	LoadMainMap();
	LoadPlayerTextures();
	LoadUITextures();
	LoadFonts();
	LoadEntityTextures();

	s.setString(L"Сейчас точно делаем что-то важное!");
	if (language == 1)
		s.setString("Now we are really do someting important");
	window.clear();
	window.draw(load_text);
	window.draw(s);
	window.display();

	LoadItems();
	LoadRecipes();

	LoadBaseEntities();

	if (!music.openFromFile("resources\\BgBirdMusic.ogg")) {
		std::cout << "Music load error";
	}

	music.setVolume(50);         // reduce the volume
	music.setLoop(true);

	if (!woodSoundBuffer.loadFromFile("resources\\korotkiy-chtkiy-rezkiy-udar-po-derevu.wav"))
		std::cout << "wood sound load error";
	wood_sound.setBuffer(woodSoundBuffer);
	wood_sound.setVolume(40);

	if (!water_music.openFromFile("resources\\water.wav")) {
		std::cout << "Music load error";
	}

	water_music.setVolume(30);         // reduce the volume
	water_music.setLoop(true);

	if (!eatSoundBuffer.loadFromFile("resources\\ukus.wav"))
		std::cout << "eat sound load error";
	eat_sound.setBuffer(eatSoundBuffer);
	eat_sound.setVolume(40);

	resources_loaded = 1;
}

void ClearResources() {
	player_textures.clear();
	player_arm_textures.clear();
	tree_textures.clear();
	bonfire_texture.clear();
	bonfire_fire_texture.clear();
	wall_texture.clear();
	spiky_wall_texture.clear();
	language_button_texture.clear();
	cell_textures.clear();
	main_map.clear();
	entity_textures.clear();
	base_entity.clear();
	items.clear();
	recipes.clear();
	item_id.clear();
	resources_loaded = 0;
	is_fonts_loaded = 0;
	is_cells_loaded = 0;
	is_player_textures_loaded = 0;
	is_ui_texture_loaded = 0;
}

void LoadBaseEntities() {
	base_entity.reserve(220);
	entity_textures.resize(220);

	sf::Vector2f brs = { 390,400 }, brp = { -210, -215 };

	base_entity.push_back(new MaterialSource(tree_textures[0], *items[1], { 0.6,0.6 }, { 1,1 }, 0, 20, 30, 0));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 160,150 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition({ -80, -80 });
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);

	base_entity.push_back(new MaterialSource(tree_textures[1], *items[1], { 0.6,0.6 }, { 1,1 }, 0, 20, 20, 0));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 190,170 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition({ -100, -90 });
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);

	base_entity.push_back(new MaterialSource(tree_textures[2], *items[1], { 0.6,0.6 }, { 1,1 }, 0, 20, 20, 0));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 150,140 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition({ -75, -75 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->AddItem(*items[3], 3, 6, 1, 40);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->AddItem(*items[4], 3, 6, 1, 30);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	////////////
	base_entity.push_back(new MaterialSource(bluebluebush_texture, *items[2], { 0.3,0.3 }, { 1,1 }, 0, 8, 4, -1));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 160,160 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition({ -80,-80 });
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	///////////
	base_entity.push_back(new MaterialSource(stone_texture, *items[5], { 0.2,0.2 }, { 1,1 }, 0, 15, 15, 1));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);

	base_entity.push_back(new MaterialSource(stone_texture, *items[5], { 0.3,0.3 }, { 1,1 }, 0, 50, 25, 1));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	///////////
	base_entity.push_back(new MaterialSource(gold_texture, *items[6], { 0.2,0.2 }, { 1,1 }, 0, 15, 10, 2));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);

	base_entity.push_back(new MaterialSource(gold_texture, *items[6], { 0.3,0.3 }, { 1,1 }, 0, 40, 20, 2));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	////////////
	base_entity.push_back(new MapEntity(wall_texture[0], 8, 0, { 0.2f,0.2f }));
	static_cast<MapEntity*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MapEntity*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	base_entity.push_back(new MapEntity(wall_texture[1], 16, 0, { 0.2f,0.2f }));
	static_cast<MapEntity*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MapEntity*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	base_entity.push_back(new MapEntity(wall_texture[2], 20, 1, { 0.2f,0.2f }));
	static_cast<MapEntity*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MapEntity*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	////////////
	base_entity.push_back(new MapDamaging(spiky_wall_texture[0], 8, 0, 3, { 0.2f,0.2f }));
	static_cast<MapDamaging*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MapDamaging*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	base_entity.push_back(new MapDamaging(spiky_wall_texture[1], 16, 0, 3.5, { 0.2f,0.2f }));
	static_cast<MapDamaging*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MapDamaging*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	base_entity.push_back(new MapDamaging(spiky_wall_texture[2], 20, 1, 4, { 0.2f,0.2f }));
	static_cast<MapDamaging*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MapDamaging*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	/////////
	base_entity.push_back(new Bonfire(bonfire_texture[0], 18.f, 0.f, { 0.25f,0.25f }));
	static_cast<Bonfire*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 0,0 });
	static_cast<Bonfire*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 90,90 });
	base_entity[base_entity.size() - 1]->SetDrawLayer(-5);
	base_entity[base_entity.size() - 1]->SetId(base_entity.size() - 1);
	//////////

	for (int i = 0; i < 401; i++) {
		base_entity.push_back(nullptr);
	}

	std::ifstream f("resources\\entities.conf", std::ios::in);
	int skip = 0;
	while (!f.eof()) {
		std::string line, tmp;
		do {
			std::getline(f, tmp);
			if (tmp.find("[skip]") != std::string::npos)skip++;
			if (tmp.find("[skip_end]") != std::string::npos) {
				skip--;
				if (skip < 0)skip = 0;
			}
			if (skip <= 0)
				line += tmp;
		} while (tmp.find("[end]") == std::string::npos && !f.eof());

		try {
			if (line == "\n" || line == " " || line == "")continue;

			auto id_pos = line.find("[id=", 0) + 4;
			int id = std::stoi(line.substr(id_pos, line.find("]", id_pos) - id_pos));

			auto sprite_pos = line.find("[sprite=", 0) + 8;
			entity_textures[id].loadFromFile("resources\\" + (sf::String)(line.substr(sprite_pos, line.find("]", sprite_pos) - sprite_pos)));
			entity_textures[id].setSmooth(1);

			auto class_pos = line.find("[class=", 0) + 7;
			std::string class_str = line.substr(class_pos, line.find("]", class_pos) - class_pos);
			class_str = to_lower(class_str);
			if (class_str == "entity")
				base_entity[id] = new Entity(line, entity_textures[id]);
			else if (class_str == "solidobject" || class_str == "so")
				base_entity[id] = new SolidObject(line, entity_textures[id]);
			else if (class_str == "materialsource" || class_str == "ms")
				base_entity[id] = new MaterialSource(line, entity_textures[id]);
			else if (class_str == "mapactiveobject" || class_str == "mao")
				base_entity[id] = new MapActiveObject(line, entity_textures[id]);
			else if (class_str == "building" || class_str == "mapentity")
				base_entity[id] = new MapEntity(line, entity_textures[id]);
			else if (class_str == "spikybuilding" || class_str == "spiky" || class_str == "mapdamaging" || class_str == "damaging")
				base_entity[id] = new MapDamaging(line, entity_textures[id]);
			else if (class_str == "bonfire" || class_str == "fire")
				base_entity[id] = new Bonfire(line, entity_textures[id]);
			else if (class_str == "animal")
				base_entity[id] = new Animal(line, entity_textures[id]);
			else if (class_str == "fleeing" || class_str == "fleeinganimal")
				base_entity[id] = new Fleeing(line, entity_textures[id]);
			else if (class_str == "enemy" || class_str == "aggressive" || class_str == "enemyganimal")
				base_entity[id] = new Enemy(line, entity_textures[id]);
			else if (class_str == "box" || class_str == "lootbox")
				base_entity[id] = new Box(line, entity_textures[id]);
			else throw std::exception("Error in load entity class");
			base_entity[id]->SetId(id);
		}
		catch (...) {
			std::cout << "Error to load entity, line: " << line << "\n";
			continue;
		}
	}
	f.close();


}
void LoadMainMap() {
	try {
		std::ifstream f("resources\\main.map", std::ios::in);
		main_map.push_back(std::vector<std::vector<unsigned short>>());
		int line = -1;
		while (!f.eof()) {
			std::string s;
			getline(f, s);
			main_map[0].push_back(std::vector<unsigned short>());
			line++;

			std::string tmp = "";
			for (char i : s) {
				if (i == ' ' || i == '\n') {
					main_map[0][line].push_back(std::stoi(tmp));
					tmp = "";
				}
				tmp += i;
			}

		}
		f.close();
	}
	catch (std::exception e) {
		std::cout << "Error to load map: " << e.what() << "\n";
	}

}
void LoadMap(std::vector<std::vector<unsigned short>>& m, const std::string& s) {
	try {
		std::ifstream f(s, std::ios::in);
		m.push_back(std::vector<unsigned short>());
		int line = -1;
		while (!f.eof()) {
			std::string s;
			getline(f, s);
			m.push_back(std::vector<unsigned short>());
			line++;

			std::string tmp = "";
			for (char i : s) {
				if (i == ' ' || i == '\n') {
					m[line].push_back(std::stoi(tmp));
					tmp = "";
				}
				tmp += i;
			}

		}
		f.close();
	}
	catch (std::exception e) {
		std::cout << "Error to load map: " << e.what() << "\n";
	}

}
sf::Color bar_color[4] = {
	sf::Color(220, 42, 42, 100),
	sf::Color(240, 128, 41, 100),
	sf::Color(210, 252, 243, 100),
	sf::Color(36, 155, 206, 100)
};
sf::Color filling_colors_bg[4] = {
	sf::Color(33, 33, 33, 33),
	sf::Color(33, 33, 33, 33),
	sf::Color(33, 33, 33, 33),
	sf::Color(33, 33, 33, 33)
};
sf::Color filling_colors[4] = {
	sf::Color(220, 42, 42, 150),
	sf::Color(240, 128, 41, 150),
	sf::Color(210, 252, 243, 230),
	sf::Color(36, 155, 206, 150)
};