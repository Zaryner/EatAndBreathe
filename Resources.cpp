#pragma once
#include "Resources.h"
#include "Entities.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <SFML\Graphics.hpp>


sf::Font arkhip;
sf::Font you2013;
sf::Font mullerExtrabold;
sf::Font comfortaa;
sf::Font fregat;

////////////////////////
std::vector<sf::Texture> player_textures;
std::vector<sf::Texture> player_arm_textures;

std::vector<sf::Texture> tree_textures;
sf::Texture bluebluebush_texture;
sf::Texture stone_texture;
sf::Texture gold_texture;

sf::Texture bar_texture;
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
//std::vector<MaterialSource*> base_tree;
//MaterialSource* base_bluebluebush;
//std::vector<MaterialSource*> base_stone;
//std::vector<MaterialSource*> base_gold;
//std::vector<Building*> base_buildings;
//std::vector<SpikyBuilding*> base_spiky_buildings;
//Bonfire* base_bonfire;

std::vector<BaseItem*> items;
std::vector<Recipe*> recipes;
std::map<std::string, int>item_id;

bool is_fonts_loaded = 0;
bool is_cells_loaded = 0;
bool is_player_textures_loaded = 0;
bool is_ui_texture_loaded = 0;

bool draw_colliders = 0;

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
		player_textures.push_back(sf::Texture());
		player_textures[0].loadFromFile("resources\\Player01.png");
		player_textures[0].setSmooth(1);

		player_arm_textures.push_back(sf::Texture());
		player_arm_textures[0].loadFromFile("resources\\Arms01.png");
		player_arm_textures[0].setSmooth(1);


		player_textures.push_back(sf::Texture());
		player_textures[1].loadFromFile("resources\\Player02.png");
		player_textures[1].setSmooth(1);

		player_arm_textures.push_back(sf::Texture());
		player_arm_textures[1].loadFromFile("resources\\Arms02.png");
		player_arm_textures[1].setSmooth(1);


		player_textures.push_back(sf::Texture());
		player_textures[2].loadFromFile("resources\\Player03.png");
		player_textures[2].setSmooth(1);

		player_arm_textures.push_back(sf::Texture());
		player_arm_textures[2].loadFromFile("resources\\Arms03.png");
		player_arm_textures[2].setSmooth(1);

		is_player_textures_loaded = 1;
	}
}
void LoadUITextures() {
	if (!is_ui_texture_loaded) {
		bar_texture.loadFromFile("resources\\Bar01.png");
		bar_texture.setSmooth(1);
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
void LoadResources() {
	LoadCellsTextures();
	LoadPlayerTextures();
	LoadUITextures();
	LoadFonts();
	LoadEntityTextures();

	LoadItems();
	LoadRecipes();

	LoadBaseEntities();
}

void LoadBaseEntities() {
	base_entity.reserve(220);
	entity_textures.resize(220);

	sf::Vector2f brs = { 390,400 }, brp = { -210, -215 };

	base_entity.push_back(new MaterialSource(tree_textures[0], *items[1], { 0.6,0.6 }, { 1,1 }, 20, 30));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 160,150 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition({ -80, -80 });

	base_entity.push_back(new MaterialSource(tree_textures[1], *items[1], { 0.6,0.6 }, { 1,1 }, 20, 20));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 190,170 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition({ -100, -90 });

	base_entity.push_back(new MaterialSource(tree_textures[2], *items[1], { 0.6,0.6 }, { 1,1 }, 20, 20));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 150,140 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition({ -75, -75 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->AddItem(*items[3], 3, 6, 1, 40);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->AddItem(*items[4], 3, 6, 1, 30);
	////////////
	base_entity.push_back(new MaterialSource(bluebluebush_texture, *items[2], { 0.3,0.3 }, { 1,1 }, 0, 8, 4));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 160,160 });
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition({ -80,-80 });
	///////////
	base_entity.push_back(new MaterialSource(stone_texture, *items[5], { 0.2,0.2 }, { 1,1 }, 15, 15));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);

	base_entity.push_back(new MaterialSource(stone_texture, *items[5], { 0.3,0.3 }, { 1,1 }, 50, 25));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	///////////
	base_entity.push_back(new MaterialSource(gold_texture, *items[6], { 0.2,0.2 }, { 1,1 }, 15, 10));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);

	base_entity.push_back(new MaterialSource(gold_texture, *items[6], { 0.3,0.3 }, { 1,1 }, 40, 20));
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<MaterialSource*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	////////////
	base_entity.push_back(new Building(wall_texture[0], 8, 0, { 0.2f,0.2f }));
	static_cast<Building*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<Building*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity.push_back(new Building(wall_texture[1], 16, 0, { 0.2f,0.2f }));
	static_cast<Building*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<Building*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity.push_back(new Building(wall_texture[2], 20, 1, { 0.2f,0.2f }));
	static_cast<Building*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<Building*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	////////////
	base_entity.push_back(new SpikyBuilding(spiky_wall_texture[0], 8, 0, 3, { 0.2f,0.2f }));
	static_cast<SpikyBuilding*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<SpikyBuilding*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity.push_back(new SpikyBuilding(spiky_wall_texture[1], 16, 0, 3.5, { 0.2f,0.2f }));
	static_cast<SpikyBuilding*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<SpikyBuilding*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	base_entity.push_back(new SpikyBuilding(spiky_wall_texture[2], 20, 1, 4, { 0.2f,0.2f }));
	static_cast<SpikyBuilding*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize(brs);
	static_cast<SpikyBuilding*>(base_entity[base_entity.size() - 1])->SetColliderLocalPosition(brp);
	/////////
	base_entity.push_back(new Bonfire(bonfire_texture[0], 18.f, 0.f, { 0.25f,0.25f }));
	static_cast<Bonfire*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 0,0 });
	static_cast<Bonfire*>(base_entity[base_entity.size() - 1])->SetColliderLocalSize({ 90,90 });
	//////////

	for (int i = 0; i < 100; i++) {
		base_entity.push_back(nullptr);
	}

	std::ifstream f("resources\\entities.conf", std::ios::in);
	while (!f.eof()) {
		std::string line, tmp;
		do {
			std::getline(f, tmp);
			line += tmp;
		} while (line.find("[end]") == std::string::npos && !f.eof());

		try {
			if (line == "\n" || line == " " || line == "")continue;

			auto id_pos = line.find("[id=", 0) + 4;
			int id = std::stoi(line.substr(id_pos, line.find("]", id_pos) - id_pos));

			auto sprite_pos = line.find("[sprite=", 0) + 8;
			entity_textures[id].loadFromFile("resources\\" + (sf::String)(line.substr(sprite_pos, line.find("]", sprite_pos) - sprite_pos)));
			entity_textures[id].setSmooth(1);

			auto class_pos = line.find("[class=", 0) + 7;
			std::string class_str = line.substr(class_pos, line.find("]", class_pos) - class_pos);
			if (class_str == "entity" || class_str == "Entity")
				base_entity[id] = new Entity(line, entity_textures[id]);
			else if (class_str == "SolidObject" || class_str == "solidobject" || class_str == "so")
				base_entity[id] = new SolidObject(line, entity_textures[id]);
			else if (class_str == "materialsource" || class_str == "MaterialSource")
				base_entity[id] = new MaterialSource(line, entity_textures[id]);
			else if (class_str == "Building" || class_str == "building")
				base_entity[id] = new Building(line, entity_textures[id]);
			else if (class_str == "SpikyBuilding" || class_str == "spikybuilding" || class_str == "spiky")
				base_entity[id] = new SpikyBuilding(line, entity_textures[id]);
			else if (class_str == "Bonfire" || class_str == "bonfire" || class_str == "fire")
				base_entity[id] = new Bonfire(line, entity_textures[id]);
			else throw std::exception("Error in load entity class");

		}
		catch (...) {
			std::cout << "Error to load entity, line: " << line << "\n";
			continue;
		}
	}
	f.close();


}
void LoadMap(std::vector<std::vector<unsigned short>>& m) {
	try {
		std::ifstream f("resources\\main.map", std::ios::in);
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