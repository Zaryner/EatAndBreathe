#pragma once

#include <vector>
#include <string>
#include <SFML\Graphics.hpp>
#include "Entities.h"

extern sf::Font arkhip;
extern sf::Font you2013;
extern sf::Font mullerExtrabold;
extern sf::Font comfortaa;
extern sf::Font fregat;

extern std::vector<sf::Texture> player_textures;
extern std::vector<sf::Texture> player_arm_textures;

extern std::vector<sf::Texture> tree_textures;
extern sf::Texture bluebluebush_texture;

extern sf::Texture bar_texture;
extern sf::Texture bar_filling_texture[2];
extern sf::Texture icon_placement_texture[6];

extern sf::Texture heart_texture;
extern sf::Texture water_texture;
extern sf::Texture meat_texture;
extern sf::Texture tempreture_texture;

extern std::vector < sf::Texture> bonfire_texture;
extern std::vector < sf::Texture> bonfire_fire_texture;
extern std::vector < sf::Texture> wall_texture;
extern std::vector < sf::Texture> spiky_wall_texture;
extern std::vector < sf::Texture> language_button_texture;
extern sf::Texture arrow_button_texture;

extern sf::Texture inventory_cell_texture[4];

extern std::vector<sf::Texture> cell_textures;
extern std::vector<std::vector<std::vector<unsigned short>>> main_map;
extern sf::Vector2f map_start_pos;

//extern std::vector<MaterialSource*> base_tree;
//extern MaterialSource* base_bluebluebush;
//extern std::vector<MaterialSource*> base_stone;
//extern std::vector<MaterialSource*> base_gold;
//extern std::vector<Building*> base_buildings;
//extern std::vector<SpikyBuilding*> base_spiky_buildings;
//extern Bonfire* base_bonfire;
extern std::vector<Entity*> base_entity;

extern std::vector<BaseItem*> items;
extern std::vector<Recipe*> recipes;
extern std::map<std::string, int>item_id;

extern bool draw_colliders;

extern void LoadCellsTextures();
extern void LoadPlayerTextures();
extern void LoadUITextures();
extern void LoadFonts();
extern void LoadEntityTextures();
extern void LoadResources();
extern void LoadBaseEntities();
extern void LoadMap(std::vector<std::vector<unsigned short>>& m);

extern sf::Color bar_color[4];
extern sf::Color filling_colors_bg[4];
extern sf::Color filling_colors[4];