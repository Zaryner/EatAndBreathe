#include <SFML/Graphics.hpp>
#include "Entities.h"
#include "EngineFuncs.h"
#include "Resources.h"

#include <iostream>
#include <fstream>
#include <map>


void LoadItems() {
	items.resize(400);
	items[0] = new BaseItem();
	item_id["0"] = 0;
	item_id["nothing"] = 0;
	item_id["empty"] = 0;

	std::ifstream f("resources\\items.conf", std::ios::in);
	while (!f.eof()) {
		std::string line, tmp;
		do {
			std::getline(f, tmp);
			line += tmp;
		} while (line.find("[end]") == std::string::npos && !f.eof());
		sf::Texture item_texture;
		try {
			if (line == "\n" || line == " " || line == "")continue;
			auto icon_pos = line.find("[icon=", 0) + 6;
			item_texture.loadFromFile("resources\\" + (sf::String)(line.substr(icon_pos, line.find("]", icon_pos) - icon_pos)));
			item_texture.setSmooth(1);

			auto id_pos = line.find("[id=", 0) + 4;
			int id = std::stoi(line.substr(id_pos, line.find("]", id_pos) - id_pos));

			auto class_pos = line.find("[class=", 0) + 7;
			std::string class_str = line.substr(class_pos, line.find("]", class_pos) - class_pos);
			class_str = to_lower(class_str);
			if (class_str == "baseitem")
				items[id] = new BaseItem(line, item_texture);
			else if (class_str == "consumable")
				items[id] = new Consumable(line, item_texture);
			else if (class_str == "builditem" || class_str == "building" || class_str == "mapentity" || class_str == "entity")
				items[id] = new BuildItem(line, item_texture);
			else if (class_str == "equipmentitem" || class_str == "equipment" || class_str == "weapon" || class_str == "tool")
				items[id] = new EquipmentItem(line, item_texture);
			else throw std::exception("Error in load item class");

			auto eng_name = items[id]->GetEngName();
			item_id[std::string(eng_name.begin(), eng_name.end())] = id;
		}
		catch (...) {
			std::cout << "Error to load item, line: " << line << "\n";
			continue;
		}
	}
	f.close();
}

void LoadRecipes() {
	recipes.resize(300);

	std::ifstream f("resources\\recipes.conf", std::ios::in);
	while (!f.eof()) {
		std::string line, tmp;
		do {
			std::getline(f, tmp);
			line += tmp;
		} while (line.find("[end]") == std::string::npos && !f.eof());
		sf::Texture item_texture;
		try {
			if (line == "\n" || line == " " || line == "")continue;

			auto id_pos = line.find("[id=", 0) + 4;
			int id = std::stoi(line.substr(id_pos, line.find("]", id_pos) - id_pos));

			auto count_pos = line.find("[count=", 0) + 7;
			int count = std::stoi(line.substr(count_pos, line.find("]", count_pos) - count_pos));

			auto ii_pos = line.find("[item_id=", 0) + 9;
			int ii = std::stoi(line.substr(ii_pos, line.find("]", ii_pos) - ii_pos));

			recipes[id] = new Recipe(items[ii], count, id);
			for (int i = 1; i <= 5; i++) {
				try {
					auto r_pos = line.find("[r" + std::to_string(i) + "=", 0) + 4;
					int r = std::stoi(line.substr(r_pos, line.find("]", r_pos) - r_pos));

					auto c_pos = line.find("[c" + std::to_string(i) + "=", 0) + 4;
					int c = std::stoi(line.substr(c_pos, line.find("]", c_pos) - c_pos));

					recipes[id]->AddMaterial(items[r], c);
				}
				catch (std::exception& e) {
					if (i == 1)throw e;
					break;
				}
			}
		}
		catch (std::exception& e) {
			std::cout << "Error to load recipe, line: " << line << "\n";
			std::cout << e.what();
			continue;
		}
	}
	f.close();
}

///////////////////////// BaseItem \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

BaseItem::BaseItem() {
	ru_name = "ничего";
	eng_name = "nothing";
	bel_name = "нечога";
	id = 0;
	max_count = 99;
	activated = 0;
	ru_description = "0";
	eng_description = "0";
	bel_description = "0";
}

BaseItem::BaseItem(const std::string& s, const sf::Texture& t) {
	auto ru_name_pos = s.find("ru_name=", 0) + 8, eng_name_pos = s.find("eng_name=", 0) + 9, bel_name_pos = s.find("bel_name=", 0) + 9;
	this->ru_name = s.substr(ru_name_pos, s.find("]", ru_name_pos) - ru_name_pos);
	this->eng_name = s.substr(eng_name_pos, s.find("]", eng_name_pos) - eng_name_pos);
	this->bel_name = s.substr(bel_name_pos, s.find("]", bel_name_pos) - bel_name_pos);

	auto id_pos = s.find("[id=", 0) + 4;
	this->id = std::stoi(s.substr(id_pos, s.find("]", id_pos) - id_pos));

	auto max_count_pos = s.find("[max_count=", 0) + 11;
	this->max_count = std::stoi(s.substr(max_count_pos, s.find("]", max_count_pos) - max_count_pos));

	auto activated_pos = s.find("activated=", 0) + 10;
	this->activated = std::stoi(s.substr(activated_pos, s.find("]", activated_pos) - activated_pos));

	auto ru_des_pos = s.find("ru_description=", 0) + 15, eng_des_pos = s.find("eng_description=", 0) + 16, bel_des_pos = s.find("bel_description=", 0) + 16;
	this->ru_description = s.substr(ru_des_pos, s.find("]", ru_des_pos) - ru_des_pos);
	this->eng_description = s.substr(eng_des_pos, s.find("]", eng_des_pos) - eng_des_pos);
	this->bel_description = s.substr(bel_des_pos, s.find("]", bel_des_pos) - bel_des_pos);

	replaceAll(ru_name, "\\n", "\n");
	replaceAll(eng_name, "\\n", "\n");
	replaceAll(bel_name, "\\n", "\n");

	replaceAll(ru_description, "\\n", "\n");
	replaceAll(eng_description, "\\n", "\n");
	replaceAll(bel_description, "\\n", "\n");


	texture = sf::Texture(t);
}

int BaseItem::GetId()const {
	return id;
}
const std::string& BaseItem::GetRuName()const {
	return ru_name;
}
const std::string& BaseItem::GetEngName()const {
	return eng_name;
}
const std::string& BaseItem::GetBelName()const {
	return bel_name;
}
const std::string& BaseItem::GetRuDescription()const {
	return ru_description;
}
const std::string& BaseItem::GetEngDescription()const {
	return eng_description;
}
const std::string& BaseItem::GetBelDescription()const {
	return bel_description;
}
bool BaseItem::isActivated()const {
	return activated;
}
int BaseItem::GetMaxCount()const {
	return max_count;
}
void BaseItem::SetMaxCount(int n) {
	max_count = n;
}

const sf::Texture& BaseItem::GetTexture() const {
	return texture;
}

void BaseItem::Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night)const {

}

///////////////////////// Consumable \\\\\\\\\\\\\\\\\\\\\\\\\

Consumable::Consumable(const std::string& s, const sf::Texture& t) :BaseItem(s, t) {
	this->health = this->hunger = this->tempreture = this->water = 0;
	try {
		auto hp_pos = s.find("[health=", 0) + 8;
		this->health = std::stof(s.substr(hp_pos, s.find("]", hp_pos) - hp_pos));
	}
	catch (...) {}
	try {
		auto hunger_pos = s.find("[hunger=", 0) + 8;
		this->hunger = std::stof(s.substr(hunger_pos, s.find("]", hunger_pos) - hunger_pos));
	}
	catch (...) {}
	try {
		auto t_pos = s.find("[tempreture=", 0) + 12;
		this->tempreture = std::stof(s.substr(t_pos, s.find("]", t_pos) - t_pos));
	}
	catch (...) {}
	try {
		auto w_pos = s.find("[water=", 0) + 7;
		this->water = std::stof(s.substr(w_pos, s.find("]", w_pos) - w_pos));
	}
	catch (...) {}
}
float Consumable::GetEffect(int i)const {
	switch (i)
	{
	case 0:
		return health;
		break;
	case 1:
		return hunger;
		break;
	case 2:
		return tempreture;
		break;
	case 3:
		return water;
		break;
	default:throw std::exception("Errror consumable effect id");
		break;
	}
}
void Consumable::Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night)const {
	player.SetHp(player.GetHp() + health);
	player.SetHunger(player.GetHunger() + hunger);
	player.SetTempreture(player.GetTempreture() + tempreture);
	player.SetWater(player.GetWater() + water);
	player.DecreaseItem(this, 1);
	eat_sound.play();
}

///////////////////////// BuildItem \\\\\\\\\\\\\\\\\\\\\\\\\

BuildItem::BuildItem(const std::string& s, const sf::Texture& t) :BaseItem(s, t) {
	this->entity_id = 0;

	try {
		auto entity_id_pos = s.find("[entity_id=", 0) + 11;
		this->entity_id = std::stoi(s.substr(entity_id_pos, s.find("]", entity_id_pos) - entity_id_pos));
	}
	catch (...) {}
	if (s.find("[always_spawn]") != std::string::npos) {
		always_spawn = 1;
	}
	else {
		always_spawn = 0;
	}
}

void BuildItem::Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night)const {
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
		std::cout << "Invalid build item class\n";
	}

	float px = player.GetPosition().x, py = player.GetPosition().y;
	px = px + 100 * std::cos(player.GetRotation() * PI / 180);
	py = py + 100 * std::sin(player.GetRotation() * PI / 180);
	b->Transform({ px,py }, base_entity[entity_id]->GetSize());
	b->SetId(entity_id);
	if (!(b->flying || always_spawn)) {
		for (auto& obj : near_mao) {
			if (obj->DetectCollision(*b)) return;
		}
	}
	if (night) {
		b->NightChange(night);
	}
	mao.push_back(b);
	player.DecreaseItem(this, 1);
}

int BuildItem::GetEntityId()const { return entity_id; }
int BuildItem::GetAlwaysSpawn()const { return always_spawn; }

/*/////////////////////////////////////////////////////////////*/
EquipmentItem::EquipmentItem(const std::string& s, const sf::Texture& t) :BaseItem(s, t) {
	def = dmg = spd = tempreture_incr = water_incr = hunger_incr = mining = 0.f;
	slot = 0;
	try {
		auto def_p = s.find("[def=", 0) + 5;
		def = std::stof(s.substr(def_p, s.find("]", def_p) - def_p));
	}
	catch (...) {};
	try {
		auto dmg_p = s.find("[dmg=", 0) + 5;
		dmg = std::stof(s.substr(dmg_p, s.find("]", dmg_p) - dmg_p));
	}
	catch (...) {};
	try {
		auto spd_p = s.find("[spd=", 0) + 5;
		spd = std::stof(s.substr(spd_p, s.find("]", spd_p) - spd_p));
	}
	catch (...) {};

	try {
		auto temp_p = s.find("[hit_time=", 0) + 10;
		hit_incr = std::stof(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {
		hit_incr = 0;
	};

	try {
		auto mining_p = s.find("[mining=", 0) + 8;
		mining = std::stof(s.substr(mining_p, s.find("]", mining_p) - mining_p));
	}
	catch (...) {};

	try {
		auto temp_p = s.find("[hunger=", 0) + 8;
		hunger_incr = std::stof(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {};
	try {
		auto temp_p = s.find("[water=", 0) + 7;
		water_incr = std::stof(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {};
	try {
		auto temp_p = s.find("[tempreture=", 0) + 12;
		tempreture_incr = std::stof(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {};
	try {
		auto temp_p = s.find("[slot=", 0) + 6;
		slot = std::stoi(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {};

	if (s.find("[draw]") != std::string::npos) {
		draw = 1;
	}
	else {
		draw = 0;
	}

	try {
		auto temp_p = s.find("[px=", 0) + 4;
		px = std::stof(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {
		px = 0;
	};
	try {
		auto temp_p = s.find("[py=", 0) + 4;
		py = std::stof(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {
		py = 0;
	};
	try {
		auto temp_p = s.find("[sx=", 0) + 4;
		sx = std::stof(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {
		sx = 0;
	};
	try {
		auto temp_p = s.find("[sy=", 0) + 4;
		sy = std::stof(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {
		sy = 0;
	};
	try {
		auto temp_p = s.find("[rot=", 0) + 5;
		rot = std::stof(s.substr(temp_p, s.find("]", temp_p) - temp_p));
	}
	catch (...) {
		rot = 0;
	};
}

void EquipmentItem::Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night)const {

}
float EquipmentItem::GetHunger()const {
	return hunger_incr;
}
float EquipmentItem::GetTempreture()const {
	return tempreture_incr;
}
float EquipmentItem::GetWater()const {
	return water_incr;
}

float EquipmentItem::GetSpeed()const {
	return spd;
}
float EquipmentItem::GetDef()const {
	return def;
}
float EquipmentItem::GetDamage()const {
	return dmg;
}
float EquipmentItem::GetMining()const {
	return mining;
}
short EquipmentItem::GetSlot()const {
	return slot;
}
sf::Vector2f EquipmentItem::GetLocalPos()const {
	return{ px,py };
}
sf::Vector2f EquipmentItem::GetLocalScale()const {
	return{ sx,sy };
}
float EquipmentItem::GetLocalRot()const {
	return rot;
}
sf::Time EquipmentItem::GetHitTime()const {
	return sf::seconds(hit_incr);
}
///////////////////////// Item \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

Item::Item() {
	item = items[0];
	this->count = 0;
}
Item::Item(int id, int count) {
	item = items[id];
	this->count = count;
}
Item::Item(const BaseItem* const baseItem, int count) {
	item = baseItem;
	this->count = count;
}

void Item::Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night) {
	item->Use(player, near_mao, mao, night);
}

void Item::Reset() {
	item = items[0];
	count = 0;
}

//////////////////////// Recipe \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

Recipe::Recipe() {
	this->id = 0;
	this->item = nullptr;
	if (items.size() > 0)
		this->item = items[0];
	this->count = 0;
}

Recipe::Recipe(const BaseItem* const item, int count, int id) {
	this->item = item;
	this->count = count;
	this->id = id;
}

void Recipe::AddMaterial(const BaseItem* const m, const int& count) {
	materials.push_back(m);
	material_count.push_back(count);
}
bool Recipe::Available(const Player& p)const {
	bool available = p.EmptySlot();
	for (int i = 0; i < p.InventoryCapacity(); i++) {
		if (p.GetItem(i).item == item && p.GetItem(i).count < item->GetMaxCount()) {
			available = 1;
		}
	}

	for (int i = 0; i < materials.size(); i++) {
		available &= p.HaveItem(materials[i], material_count[i]);
	}
	return available;
}
const std::vector<BaseItem const*>& Recipe::GetMaterials()const {
	return materials;
}
const std::vector<int>& Recipe::GetMaterialsCount()const {
	return material_count;
}
const BaseItem* const Recipe::GetItem()const {
	return item;
}
const int& Recipe::GetCount()const {
	return count;
}
//////////////////////// Entity \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

Entity::Entity() {
	id = 0;

	position = { 0,0 };
	size = { 1,1 };
	rotation = 0;

	draw_layer = 0;

	sprite.setScale(size);
	sprite.setPosition(position);
	sprite.setRotation(rotation);
}
Entity::Entity(const std::string& s, const sf::Texture& t) {
	position = { 0,0 };
	size = { 1,1 };
	rotation = 0;
	auto dl = s.find("[dl=", 0) + 4;
	try {
		draw_layer = std::stoi(s.substr(dl, s.find("]", dl) - dl));
	}
	catch (...) {
		draw_layer = 0;
	}
	auto sx_pos = s.find("[sx=", 0) + 4, sy_pos = s.find("[sy=", 0) + 4;
	try {
		this->size = { std::stof(s.substr(sx_pos, s.find("]", sx_pos) - sx_pos)),std::stof(s.substr(sy_pos, s.find("]", sy_pos) - sy_pos)) };
	}
	catch (...) {}
	auto rot = s.find("[rot=", 0) + 5;
	try {
		this->rotation = std::stof(s.substr(rot, s.find("]", rot) - rot));
	}
	catch (...) {}
	sprite.setTexture(t);
	sprite.setOrigin(t.getSize().x / 2, t.getSize().y / 2);

	sprite.setScale(size);
	sprite.setPosition(position);
	sprite.setRotation(rotation);

}
int Entity::GetId()const {
	return id;
}
void Entity::SetId(int id) {
	this->id = id;
}
short Entity::GetDrawLayer()const { return draw_layer; }
void Entity::SetDrawLayer(short l) { draw_layer = l; }

void Entity::SetSize(const sf::Vector2f& v) {
	size = v;
	sprite.setScale(size);
}
void Entity::SetSize(float x, float y) {
	size = { x,y };
	sprite.setScale(size);
}
const sf::Vector2f& Entity::GetSize() { return size; }

void Entity::SetPosition(const sf::Vector2f& v) {
	position = v;
	sprite.setPosition(position);
}
void Entity::SetPosition(float a, float b) {
	position = { a,b };
	sprite.setPosition(a, b);
}
const sf::Vector2f& Entity::GetPosition() { return position; }

void Entity::Move(float dx, float dy) {
	// dx, dy is the argumants, don't confuze with a fields
	position.x += dx; position.y += dy;
	sprite.move(dx, dy);
}

void Entity::NightChange(const bool& night) {
	if (night) {
		sprite.setColor(night_color);
	}
	else {
		sprite.setColor(sf::Color(255, 255, 255));
	}
}

void Entity::Draw(sf::RenderWindow& window, const float& deltaTime, bool& night) {
	window.draw(sprite);
}
void Entity::SetTexture(sf::Texture& texture, const sf::Vector2f& origin_offset) {
	sprite.setTexture(texture, true);
	sprite.setOrigin(texture.getSize().x / 2 + origin_offset.x, texture.getSize().y / 2 + origin_offset.y);
}
const sf::Sprite& Entity::GetSprite()const {
	return sprite;
}
sf::Sprite& Entity::GetSpriteRef() {
	return sprite;
}
void Entity::RotateTo(float x, float y, float offset = 0) {

	float dx = x - sprite.getPosition().x;
	float dy = y - sprite.getPosition().y;

	float rotation = atan2(dy, dx);
	rotation *= 180 / PI;

	sprite.setRotation(rotation + offset);
	this->rotation = rotation;
}
void Entity::RotateTo(sf::Vector2f pos, float offset = 0) {
	RotateTo(pos.x, pos.y, offset);
}
void Entity::Rotate(float v) {
	rotation += v;
	sprite.rotate(v);
}
void Entity::SetRotation(float v) {
	rotation = v;
	sprite.setRotation(v);
}
float Entity::GetRotation()const {
	return rotation;
}
float Entity::Distance(const Entity& e)const {
	return std::sqrt(std::pow((position.x - e.position.x), 2) + std::pow((position.y - e.position.y), 2));
}
void Entity::Transform(const sf::Vector2f& p, const sf::Vector2f& s, const float& r) {
	SetPosition(p);
	SetSize(s);
	SetRotation(r);
}
///////////////////////////////// liveEntity \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

Animal::Animal() {
	damage = 1;
	max_hp = 100;
	hp = 100;
	def = 0;
	base_color = sf::Color(255, 255, 255, 255);
	dx = 0, dy = 0;

	hit_timer = sf::seconds(0);
	hit_cooldown = sf::seconds(0.8f);
	is_hurt_anim_play = 0;
	is_idle_anim_play = 1;
	b_size = GetSize();
}
Animal::Animal(const std::string& s, const sf::Texture& t) :MapActiveObject(s, t) {
	damage = 1;
	max_hp = 100;
	hp = 100;
	def = 0;
	base_color = sf::Color(255, 255, 255, 255);
	dx = 0, dy = 0;

	hit_timer = sf::seconds(0);
	hit_cooldown = sf::seconds(0.7f);
	is_hurt_anim_play = 0;
	is_idle_anim_play = 1;
	b_size = GetSize();

	auto hp_p = s.find("[hp=", 0) + 4;
	hp = std::stof(s.substr(hp_p, s.find("]", hp_p) - hp_p));
	max_hp = hp;
	auto def_p = s.find("[def=", 0) + 5;
	def = std::stof(s.substr(def_p, s.find("]", def_p) - def_p));
	auto dmg_p = s.find("[dmg=", 0) + 5;
	float dmg = std::stof(s.substr(dmg_p, s.find("]", dmg_p) - dmg_p));
	damage = dmg;
	auto spd_p = s.find("[spd=", 0) + 5;
	speed = std::stof(s.substr(spd_p, s.find("]", spd_p) - spd_p));
	try {
		auto hc_p = s.find("[hit_cooldown=", 0) + 14;
		hit_cooldown = sf::seconds(std::stof(s.substr(hc_p, s.find("]", hc_p) - hc_p)));
	}
	catch (...) {};
	try {
		auto bi_p = s.find("[box_id=", 0) + 8;
		box_id = std::stoi(s.substr(bi_p, s.find("]", bi_p) - bi_p));
	}
	catch (...) {
		box_id = -1;
	};
}
void Animal::SetSpeed(float s) {
	this->speed = s;
}
float Animal::GetSpeed()const {
	return this->speed;
}
float Animal::GetDef() const {
	return def;
}
void Animal::SetDef(float v) {
	this->def = v;
}
float Animal::GetDamage() const {
	return damage;
}
void Animal::SetDamage(float v) {
	this->damage = v;
}
const float& Animal::GetHp() const {
	if (hp < 1.f)return 0.f;
	return hp;
}
void Animal::SetHp(const float& v) {
	if (v < hp)is_hurt_anim_play = 1;

	this->hp = v;
	if (hp < 1.f)hp = 0;
	if (hp > max_hp)hp = max_hp;
}
float Animal::GetMaxHp() const {
	return max_hp;
}
void Animal::SetMaxHp(float v) {
	this->max_hp = v;
}
sf::Time Animal::GetHitCooldown()const {
	return hit_cooldown;
}
void Animal::Transform(const sf::Vector2f& p, const sf::Vector2f& s, const float& r) {
	SetSize(s);
	SetPosition(p);
	SetRotation(r);
}
void Animal::SetSize(const sf::Vector2f& v) {
	this->SolidObject::SetSize(v);
	b_size = v;
}
void Animal::SetSize(float x, float y) {
	this->SetSize({ x,y });
}

void Animal::Move(const float& dx, const float& dy) {
	position.x += dx; position.y += dy;
	sprite.move(dx, dy);

	collider.left += dx;
	collider.top += dy;
}

void Animal::Draw(sf::RenderWindow& window, const float& deltaTime, bool& night) {
	Entity::Draw(window, deltaTime, night);

	if (is_hurt_anim_play)
		PlayHurtAnimation(deltaTime, night);
	if (is_idle_anim_play)
		PlayIdleAnimation(deltaTime);

	if (draw_colliders) {
		sf::RectangleShape collider_rect({ collider.width,collider.height });
		collider_rect.setPosition({ collider.left,collider.top });
		collider_rect.setOutlineThickness(5);
		collider_rect.setOutlineColor(sf::Color::Red);
		collider_rect.setFillColor(sf::Color::Transparent);
		window.draw(collider_rect);
	}

}

void Animal::CollideSolidObject(const SolidObject& o) {
	if (&o == this)return;
	if (o.flying || flying)return;
	if (typeid(o) == typeid(Fleeing) ||
		typeid(o) == typeid(Box) ||
		typeid(o) == typeid(Player) ||
		typeid(o) == typeid(Enemy))return;
	if (o.DetectCollision(*this)) {
		if (o.GetColliderCenterPosition().x < position.x && dx == -1 && (collider.top < o.GetColliderPosition().y + o.GetColliderSize().y - 3 && collider.top + collider.height > o.GetColliderPosition().y + 3))
			dx = 0;
		else if (o.GetColliderCenterPosition().x > position.x && dx == 1 && (collider.top < o.GetColliderPosition().y + o.GetColliderSize().y - 3 && collider.top + collider.height > o.GetColliderPosition().y + 3))
			dx = 0;
		if (o.GetColliderCenterPosition().y < position.y && dy == -1 && (collider.left < o.GetColliderPosition().x + o.GetColliderSize().x - 3 && collider.left + collider.width > o.GetColliderPosition().x + 3))
			dy = 0;
		else if (o.GetColliderCenterPosition().y > position.y && dy == 1 && (collider.left < o.GetColliderPosition().x + o.GetColliderSize().x - 3 && collider.left + collider.width > o.GetColliderPosition().x + 3))
			dy = 0;
	}

}

void Animal::PlayHurtAnimation(const float& deltaTime, const bool& night) {

	sf::Time hurt_ms = sf::milliseconds(200), pause = sf::milliseconds(640);
	sf::Time hurt_delay_ms = sf::milliseconds(10);
	sf::Time max_time = hurt_ms + pause;

	int spd = 9;
	int a_spd = 3;

	if (hurt_anim_time <= hurt_ms && hurt_anim_time - last_hurt_time >= hurt_delay_ms) {
		last_hurt_time = hurt_anim_time;
		auto sp = sf::Color(sprite.getColor());
		if (hurt_anim_time <= hurt_ms / 2.0f) {
			if (sp.g - spd >= 0)sp.g -= spd;
			if (sp.b - spd >= 0)sp.b -= spd;
			if (sp.a - a_spd >= 0)sp.a -= a_spd;
		}
		else {
			if (sp.g < 255 - spd)sp.g += spd;
			if (sp.b < 255 - spd)sp.b += spd;
			if (sp.a < 255 - a_spd)sp.a += a_spd;
		}

		sprite.setColor(sp);
	}


	hurt_anim_time += sf::seconds(deltaTime);

	if (hurt_anim_time >= max_time) {
		hurt_anim_time = sf::milliseconds(0);
		is_hurt_anim_play = 0;
		if (night) {
			sprite.setColor(night_color);
		}
		else sprite.setColor(base_color);
		last_hurt_time = hurt_delay_ms;
	}
}
void Animal::PlayIdleAnimation(const float& deltaTime) {

	sf::Time stretch_ms = sf::milliseconds(2200), wait_ms = sf::milliseconds(100), pause = sf::milliseconds(140);
	sf::Time max_time = stretch_ms + wait_ms + pause;

	float spd = 0.0004f * b_size.x * b_size.y;

	if (idle_anim_time < stretch_ms) {
		if (idle_anim_time < stretch_ms / 2.f) {
			SetSize(GetSize().x + spd, GetSize().y + spd);
		}
		else {
			SetSize(GetSize().x - spd, GetSize().y - spd);
		}
	}


	idle_anim_time += sf::seconds(deltaTime);

	if (idle_anim_time >= max_time) {
		idle_anim_time = sf::milliseconds(0);
		SetSize(b_size);
	}
}


int InvetoryCarrier::InventoryCapacity()const {
	return inventory.size();
}

Item& InvetoryCarrier::GetItem(int pos) {
	return inventory[pos];
}
Item* InvetoryCarrier::FindItem(const BaseItem* const item) {
	for (int i = 0; i < inventory.size(); i++) {
		if (inventory[i].item == item)return &inventory[i];
	}
	return nullptr;
}

Item const& InvetoryCarrier::GetItem(int pos)const {
	return inventory[pos];
}
const int& InvetoryCarrier::GetItemCount(int pos)const {
	return inventory[pos].count;
}

void InvetoryCarrier::AddItem(const BaseItem* item, int count) {
	for (int i = 0; i < inventory.size(); i++) {
		if (item->GetId() == inventory[i].item->GetId() && inventory[i].count < inventory[i].item->GetMaxCount()) {
			if (inventory[i].count + count <= inventory[i].item->GetMaxCount()) {
				inventory[i].count += count;
				return;
			}
			count -= inventory[i].item->GetMaxCount() - inventory[i].count;
			inventory[i].count = inventory[i].item->GetMaxCount();
		}
	}
	for (int i = 0; i < inventory.size(); i++) {
		if (inventory[i].item->GetId() == 0) {
			inventory[i].item = items[item->GetId()];
			if (count <= inventory[i].item->GetMaxCount()) {
				inventory[i].count += count;
				return;
			}
			count -= inventory[i].item->GetMaxCount();
		}
	}
}
void InvetoryCarrier::DecreaseItem(const BaseItem* item, int count) {
	for (int i = inventory.size() - 1; i >= 0; i--) {
		if (item->GetId() == inventory[i].item->GetId()) {
			int t = inventory[i].count;
			inventory[i].count -= count;
			if (inventory[i].count > 0) {
				return;
			}
			inventory[i].Reset();
			if (inventory[i].count == 0) return;
			count -= t;
		}
	}
}
void InvetoryCarrier::AddInventory(const std::vector<Item>& v) {
	for (const auto& i : v) {
		AddItem(i.item, i.count);
	}
}

bool InvetoryCarrier::HaveItem(const BaseItem* item, int count)const {
	int have = 0;
	for (int i = inventory.size() - 1; i >= 0; i--) {
		if (item->GetId() == inventory[i].item->GetId()) {
			have += inventory[i].count;
		}
	}
	return have >= count;
}

bool InvetoryCarrier::EmptySlot()const {
	for (int i = inventory.size() - 1; i >= 0; i--) {
		if (!inventory[i].item->GetId()) {
			return true;
		}
	}
	return false;
}

std::vector<Item>& InvetoryCarrier::GetInventoryRef() {
	return inventory;
}

///////////////////////////////// Player \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void Player::Flip() {
	this->sprite.setScale(this->sprite.getScale().x * -1, this->sprite.getScale().y);
}
Player::Player() :Animal() {
	right_facing = 1;
	max_hp = 100;
	hp = 100;
	hunger = 100;
	tempreture = 100;
	water = 100;
	def = 0;

	hunger_decrease = 3;
	tempreture_decrease = 1.8;
	water_decrease = 1.4;
	water_increase = 5;

	active_item = nullptr;

	box_id = 97;

	base_color = sf::Color(255, 255, 255, 255);

	action = 0;

	hit_cooldown = sf::seconds(0.6);

	idle_anim_time = sf::milliseconds(0);
	is_idle_anim_play = 1;

	arm_anim_time = sf::milliseconds(0);
	left_arm_animated = 0;
	is_arm_anim_play = 0;

	is_hurt_anim_play = 0;
	hurt_anim_time = sf::milliseconds(0);
	last_hurt_time = sf::milliseconds(0);

	inventory.reserve(20);
	inventory.resize(9, { items[0],0 });
	equipment.resize(10, nullptr);
}
float Player::GetState(int id) const {
	switch (id)
	{
	case 0:
		return hp;
		break;
	case 1:
		return hunger;
		break;
	case 2:
		return tempreture;
		break;
	case 3:
		return water;
		break;
	case 4:
		return def;
		break;
	default:
		throw std::exception("Invalid id for player state");
		break;
	}
}

float Player::GetHunger()const {
	return hunger;
}
float Player::GetTempreture()const {
	return tempreture;
}
float Player::GetWater()const {
	return water;
}
void Player::SetHunger(float v) {
	hunger = v;
	if (hunger < 0)hunger = 0;
	if (hunger > 100)hunger = 100;
}
void Player::SetTempreture(float v) {
	tempreture = v;
	if (tempreture < 0)tempreture = 0;
	if (tempreture > 100)tempreture = 100;
}
void Player::SetWater(float v) {
	water = v;
	if (water < 0)water = 0;
	if (water > 100)water = 100;
}
void Player::SetName(std::wstring& s) {
	name = s;
}
const sf::String& Player::GetName()const {
	return name;
}

void Player::SetSkinId(const int& i) {
	skin_id = i;
	SetTexture(player_textures[i], player_texture_pos[i]);
	SetArmsTexture(player_arm_textures[i]);
}
const int& Player::GetSkinId()const {
	return skin_id;
}

float Player::GetSpeed()const {
	float spd = speed;
	for (int i = 0; i < equipment.size(); i++) {
		if (equipment[i] == nullptr)continue;
		spd += equipment[i]->GetSpeed();
	}
	if (active_item != nullptr)
		if (typeid(*active_item->item) == typeid(EquipmentItem)) {
			spd += static_cast<const EquipmentItem*>(active_item->item)->GetSpeed();
		}
	return spd;
}
float Player::GetDef()const {
	float df = this->def;
	for (int i = 0; i < equipment.size(); i++) {
		if (equipment[i] == nullptr)continue;
		df += equipment[i]->GetDef();
	}
	if (active_item != nullptr)
		if (typeid(*active_item->item) == typeid(EquipmentItem)) {
			df += static_cast<const EquipmentItem*>(active_item->item)->GetDef();
		}
	return df;
}
float Player::GetDamage()const {
	float dmg = this->damage;
	for (int i = 0; i < equipment.size(); i++) {
		if (equipment[i] == nullptr)continue;
		dmg += equipment[i]->GetDamage();
	}
	if (active_item != nullptr)
		if (typeid(*active_item->item) == typeid(EquipmentItem)) {
			dmg += static_cast<const EquipmentItem*>(active_item->item)->GetDamage();
		}
	return dmg;
}
float Player::GetMining()const {
	float minig = 1;
	for (int i = 0; i < equipment.size(); i++) {
		if (equipment[i] == nullptr)continue;
		minig += equipment[i]->GetMining();
	}
	if (active_item != nullptr)
		if (typeid(*active_item->item) == typeid(EquipmentItem)) {
			minig += static_cast<const EquipmentItem*>(active_item->item)->GetMining();
		}
	return minig;
}

sf::Time Player::GetHitCooldown()const {
	sf::Time hc = hit_cooldown;
	for (int i = 0; i < equipment.size(); i++) {
		if (equipment[i] == nullptr)continue;
		hc += equipment[i]->GetHitTime();
	}
	if (active_item != nullptr)
		if (typeid(*active_item->item) == typeid(EquipmentItem)) {
			hc += static_cast<const EquipmentItem*>(active_item->item)->GetHitTime();
		}

	return hc;
}

float Player::GetTemprutureDeacrease()const {
	float td = tempreture_decrease;
	for (int i = 0; i < equipment.size(); i++) {
		if (equipment[i] == nullptr)continue;
		td += equipment[i]->GetTempreture();
	}
	if (active_item != nullptr)
		if (typeid(*active_item->item) == typeid(EquipmentItem)) {
			td += static_cast<const EquipmentItem*>(active_item->item)->GetTempreture();
		}

	return td;
}
float Player::GetHungerDeacrease()const {
	float hg = hunger_decrease;
	for (int i = 0; i < equipment.size(); i++) {
		if (equipment[i] == nullptr)continue;
		hg += equipment[i]->GetHunger();
	}
	if (active_item != nullptr)
		if (typeid(*active_item->item) == typeid(EquipmentItem)) {
			hg += static_cast<const EquipmentItem*>(active_item->item)->GetHunger();
		}

	return hg;
}
float Player::GetWaterDeacrease()const {
	float wt = tempreture_decrease;
	for (int i = 0; i < equipment.size(); i++) {
		if (equipment[i] == nullptr)continue;
		wt += equipment[i]->GetWater();
	}
	if (active_item != nullptr)
		if (typeid(*active_item->item) == typeid(EquipmentItem)) {
			wt += static_cast<const EquipmentItem*>(active_item->item)->GetWater();
		}

	return wt;
}

void Player::Craft(const Recipe* r) {
	if (r->Available(*this)) {
		for (int i = 0; i < r->GetMaterials().size(); i++) {
			DecreaseItem(r->GetMaterials()[i], r->GetMaterialsCount()[i]);
		}
		AddItem(r->GetItem(), r->GetCount());
	}
}
void Player::Dress(const EquipmentItem* item) {
	equipment[item->GetSlot()] = item;
}
void Player::Undress(const EquipmentItem* item) {
	equipment[item->GetSlot()] = nullptr;
}
bool Player::isDressed(const BaseItem* const item)const {
	for (int i = 0; i < equipment.size(); i++) {
		if (equipment[i] == item)return true;
	}
	return false;

}
void Player::ChangeActiveItem(Item& item) {
	active_item = &item;
}

void Player::ChangeActiveItem() {
	active_item = nullptr;
}
const Item* const Player::ActiveItem()const {
	return active_item;
}
void Player::UseActiveItem(std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night) {
	if (active_item != nullptr) {
		active_item->Use(*this, near_mao, mao, night);
		if (!active_item->item->GetId())active_item = nullptr;
	}
}
void Player::UseItem(const int& id, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night) {
	if (id >= 0 && id < inventory.size() && inventory[id].item != nullptr) {
		inventory[id].Use(*this, near_mao, mao, night);
	}
}
void Player::SwapItems(const int& id1, const int& id2) {
	std::swap(inventory[id1], inventory[id2]);
}
void Player::DropItem(int item_pos, int count, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {
	if (GetItem(item_pos).count < count)return;
	MapActiveObject* b;
	b = new Box(*static_cast<Box*>(base_entity[box_id]));
	b->Transform(GetPosition(), base_entity[box_id]->GetSize());
	static_cast<Box*>(b)->AddItem(GetItem(item_pos).item, count);
	mao.push_back(b);
	DecreaseItem(GetItem(item_pos).item, count);
}
void Player::SetSize(const sf::Vector2f& v) {
	size = v;
	sprite.setScale(0.6 / 300 * 72 * size.x, 0.6 / 300 * 72 * size.y);
	left_arm.setScale(2.3 / 300 * 72 * size.x, 2.3 / 300 * 72 * size.y);
	right_arm.setScale(2.3 / 300 * 72 * size.x, 2.3 / 300 * 72 * size.y);

	collider.width = 54 * v.x;
	collider.height = 54 * v.y;

	action_collider.width = 52 * v.x;
	action_collider.height = 52 * v.y;
}
void Player::SetSize(const float& sx, const float& sy) { SetSize({ sx,sy }); }

void Player::SetArmsTexture(sf::Texture& texture) {
	left_arm.setTexture(texture, true);
	left_arm.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);

	right_arm.setTexture(texture, true);
	right_arm.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
}

void Player::Move(const float& dx, const float& dy) {
	position.x += dx; position.y += dy;
	sprite.move(dx, dy);

	collider.left += dx;
	collider.top += dy;

	action_collider.left += dx;
	action_collider.top += dy;
}

void Player::CheckMovement(const float& delta_time, const std::vector<SolidObject*>& so, bool a, bool d, bool w, bool s) {
	this->dx = 0; this->dy = 0;
	if (a)this->dx = -1;
	if (d)this->dx = 1;
	if (w)this->dy = -1;
	if (s)this->dy = 1;

	for (const auto& s_obj : so)
		CollideSolidObject(*s_obj);
	this->Move(dx * GetSpeed() * delta_time, dy * GetSpeed() * delta_time);
}
void Player::CheckControl(const float& delta_time, const bool& right_mouse_pressed, const bool& mouse_on_ui, const bool& mouse_pressed) {
	hit_timer -= sf::seconds(delta_time);
	action = 0;
	near_fire = 0;
	if (mouse_pressed && (active_item == nullptr || typeid(*active_item->item) != typeid(BuildItem)) && !mouse_on_ui) {
		if (hit_timer <= sf::seconds(0)) {
			action = 1;
			hit_timer = GetHitCooldown();

			is_idle_anim_play = 0;
			is_arm_anim_play = 1;
			//arm_anim_time = sf::milliseconds(0);
			//right_arm.setPosition(180, 500);
			//left_arm.setPosition(480, 500);
		}
	}
	if (right_mouse_pressed && active_item != nullptr && typeid(*active_item->item) == typeid(BuildItem)) {
		active_item = nullptr;
	}
}

void Player::CheckStats(const float& delta_time, const bool& player_on_water, const bool& night) {
	SetHunger(hunger - GetHungerDeacrease() * delta_time);
	SetTempreture(tempreture - GetTemprutureDeacrease() * delta_time - GetTemprutureDeacrease() * delta_time * night * 1 * !near_fire);
	if (player_on_water) {
		SetWater(water + GetWaterDeacrease() * 2.f * delta_time);
	}
	else SetWater(water - GetWaterDeacrease() * delta_time);

	if (hunger <= 0 || tempreture <= 0 || water <= 0) {
		SetHp(hp - 1.4f * delta_time);
	}

	if (GetHunger() >= 75 && GetWater() >= 75 && GetTempreture() >= 75) {
		SetHp(hp + 0.9f * delta_time);
	}

}

void Player::NightChange(const bool& night) {
	Entity::NightChange(night);
	if (night) {
		left_arm.setColor(night_color);
		right_arm.setColor(night_color);
	}
	else {
		left_arm.setColor(sf::Color::White);
		right_arm.setColor(sf::Color::White);
	}
}


void Player::RotateActionCollider(float x, float y, float offset) {

	float dx = x - sprite.getPosition().x;
	float dy = y - sprite.getPosition().y;

	float rotation = atan2(dy, dx);
	rotation *= 180 / PI;
	this->rotation = rotation;

	action_collider.left = position.x + 30 * std::cos(rotation * PI / 180) - action_collider.width / 2;
	action_collider.top = position.y + 30 * std::sin(rotation * PI / 180) - action_collider.height / 2;

	action_collider.width = 52 * size.x + 30 * std::abs(std::cos(rotation * PI / 180)) * size.x;
	action_collider.height = 52 * size.y + 30 * std::abs(std::sin(rotation * PI / 180)) * size.y;
}
void Player::RotateActionCollider(sf::Vector2f pos, float offset) {
	RotateActionCollider(pos.x, pos.y, offset);
}
void Player::RotateTo(float x, float y, float offset) {

	float dx = x - sprite.getPosition().x;
	float dy = y - sprite.getPosition().y;

	float rotation = atan2(dy, dx);
	rotation *= 180 / PI;

	sprite.setRotation(rotation + offset);
	RotateActionCollider(x, y, offset);
}
void Player::RotateTo(sf::Vector2f pos, float offset) {
	RotateTo(pos.x, pos.y, offset);
}
void Player::SetPosition(float x, float y) {
	position.x += x; position.y += y;
	sprite.setPosition(x, y);
	right_arm.setPosition(200, 500);
	left_arm.setPosition(460, 500);

	collider.left = x - 30;
	collider.top = y - 30;
}

void Player::Draw(sf::RenderWindow& window, const float& deltaTime, bool& night) {
	Animal::Draw(window, deltaTime, night);

	if (is_arm_anim_play)
		PlayArmsAnimation(deltaTime);
	window.draw(left_arm, sprite.getTransform() * left_arm.getTransform());
	window.draw(right_arm, sprite.getTransform() * right_arm.getTransform());

	if (active_item != nullptr &&
		typeid(*active_item->item) == typeid(EquipmentItem)) {
		sf::Sprite item_sprite;
		auto item = static_cast<const EquipmentItem*>(active_item->item);

		item_sprite.setTexture(item->GetTexture());
		item_sprite.setOrigin(item_sprite.getTexture()->getSize().x / 2, item_sprite.getTexture()->getSize().x / 2);

		item_sprite.setPosition(item->GetLocalPos());
		item_sprite.setScale(item->GetLocalScale());
		item_sprite.setRotation(item->GetLocalRot());

		if (night)item_sprite.setColor(night_color);
		window.draw(item_sprite, sprite.getTransform() * right_arm.getTransform() * item_sprite.getTransform());
	}

	for (int i = 0; i < this->equipment.size(); i++) {
		if (equipment[i] == nullptr)continue;
		sf::Sprite item_sprite;
		auto item = equipment[i];

		item_sprite.setTexture(item->GetTexture());
		item_sprite.setOrigin(item_sprite.getTexture()->getSize().x / 2, item_sprite.getTexture()->getSize().x / 2);

		item_sprite.setPosition(item->GetLocalPos());
		item_sprite.setScale(item->GetLocalScale());
		item_sprite.setRotation(item->GetLocalRot());

		if (night)item_sprite.setColor(night_color);
		window.draw(item_sprite, sprite.getTransform() * item_sprite.getTransform());
	}
	sf::Text name_t;
	name_t.setString(name);

	name_t.setCharacterSize(26);
	name_t.setFont(arkhip);
	name_t.setFillColor(sf::Color(0, 0, 0, 200));
	name_t.setPosition({ -8.f * name.getSize() + GetPosition().x,-85 + GetPosition().y });
	window.draw(name_t);

	if (draw_colliders) {
		sf::RectangleShape acollider_rect({ action_collider.width,action_collider.height });
		acollider_rect.setPosition({ action_collider.left,action_collider.top });
		acollider_rect.setOutlineColor(sf::Color::Black);
		acollider_rect.setOutlineThickness(5);
		acollider_rect.setFillColor(sf::Color::Transparent);
		window.draw(acollider_rect);

		sf::CircleShape s;
		s.setOutlineThickness(1);
		s.setOutlineColor(sf::Color::Black);
		s.setFillColor(sf::Color(188, 34, 78, 245));
		s.setRadius(3);
		s.setPosition(position.x - s.getRadius(), position.y - s.getRadius());
		window.draw(s);
	}

}
void Player::PlayArmsAnimation(const float& deltaTime) {

	sf::Time attack_ms = sf::milliseconds(220), pause = hit_cooldown - attack_ms;
	sf::Time max_time = attack_ms + pause;

	if (arm_anim_time == sf::milliseconds(0)) {
		right_arm.setPosition(200, 500);
		left_arm.setPosition(460, 500);
	}

	double spd = 3.0 * deltaTime * 1000;

	if (arm_anim_time < attack_ms) {
		if (left_arm_animated) {
			if (arm_anim_time < attack_ms / 2.f) left_arm.move(0, spd);
			else left_arm.move(0, -spd);
		}
		else {
			if (arm_anim_time < attack_ms * 0.5f) right_arm.move(0, spd);
			else right_arm.move(0, -spd);
		}
	}


	arm_anim_time += sf::seconds(deltaTime);

	if (arm_anim_time >= max_time) {
		arm_anim_time = sf::milliseconds(0);
		is_arm_anim_play = 0;
		is_idle_anim_play = 1;
		idle_anim_time = sf::milliseconds(0);
		left_arm_animated = !left_arm_animated;
		right_arm.setPosition(200, 500);
		left_arm.setPosition(460, 500);
	}
}

void Player::PlayHurtAnimation(const float& deltaTime, const bool& night) {

	sf::Time hurt_ms = sf::milliseconds(200), pause = sf::milliseconds(640);
	sf::Time hurt_delay_ms = sf::milliseconds(10);
	sf::Time max_time = hurt_ms + pause;

	int spd = 9;
	int a_spd = 3;

	if (hurt_anim_time <= hurt_ms && hurt_anim_time - last_hurt_time >= hurt_delay_ms) {
		last_hurt_time = hurt_anim_time;
		auto la = sf::Color(left_arm.getColor()), ra = sf::Color(right_arm.getColor()), sp = sf::Color(sprite.getColor());
		if (hurt_anim_time <= hurt_ms / 2.0f) {
			if (sp.g - spd >= 0)sp.g -= spd;
			if (sp.b - spd >= 0)sp.b -= spd;
			if (sp.a - a_spd >= 0)sp.a -= a_spd;

			if (la.g - spd >= 0)la.g -= spd;
			if (la.b - spd >= 0)la.b -= spd;
			if (la.a - a_spd >= 0)la.a -= a_spd;

			if (ra.g - spd >= 0)ra.g -= spd;
			if (ra.b - spd >= 0)ra.b -= spd;
			if (ra.a < -a_spd >= 0)ra.a -= a_spd;
		}
		else {
			if (sp.g < 255 - spd)sp.g += spd;
			if (sp.b < 255 - spd)sp.b += spd;
			if (sp.a < 255 - a_spd)sp.a += a_spd;

			if (la.g < 255 - spd)la.g += spd;
			if (la.b < 255 - spd)la.b += spd;
			if (la.a < 255 - a_spd)la.a += a_spd;

			if (ra.g < 255 - spd)ra.g += spd;
			if (ra.b < 255 - spd)ra.b += spd;
			if (ra.a < 255 - a_spd)ra.a += a_spd;
		}

		sprite.setColor(sp);
		left_arm.setColor(la);
		right_arm.setColor(ra);
	}


	hurt_anim_time += sf::seconds(deltaTime);

	if (hurt_anim_time >= max_time) {
		hurt_anim_time = sf::milliseconds(0);
		is_hurt_anim_play = 0;
		if (!night) {
			sprite.setColor(base_color);
			left_arm.setColor(base_color);
			right_arm.setColor(base_color);
		}
		else {
			sprite.setColor(night_color);
			left_arm.setColor(night_color);
			right_arm.setColor(night_color);
		}
		last_hurt_time = hurt_delay_ms;
	}
}
void Player::PlayIdleAnimation(const float& deltaTime) {

	sf::Time stretch_ms = sf::milliseconds(2200), wait_ms = sf::milliseconds(100), pause = sf::milliseconds(140);
	sf::Time max_time = stretch_ms + wait_ms + pause;

	double spd = 0.03 * deltaTime * 1000;

	if (idle_anim_time < stretch_ms) {
		if (idle_anim_time < stretch_ms / 2.f) {
			left_arm.move(spd, 0);
			right_arm.move(-spd, 0);
		}
		else {
			left_arm.move(-spd, 0);
			right_arm.move(spd, 0);
		}
	}


	idle_anim_time += sf::seconds(deltaTime);

	if (idle_anim_time >= max_time) {
		idle_anim_time = sf::milliseconds(0);

		right_arm.setPosition(200, 500);
		left_arm.setPosition(460, 500);
	}
}

const sf::FloatRect& Player::GetActionCollider()const {
	return action_collider;
}

void Player::SetActionCollider(sf::FloatRect ac) {
	action_collider = ac;
}

std::vector<const EquipmentItem*>& Player::GetEquipmentRef() {
	return equipment;
}

std::pair<sf::Vector2f, sf::Vector2f> Player::GetArmsPos()const {
	return { {right_arm.getPosition().x,right_arm.getPosition().y},{left_arm.getPosition().x,left_arm.getPosition().y} };
}

void Player::SetArmsPos(sf::Vector2f a, sf::Vector2f b) {
	right_arm.setPosition(a);
	left_arm.setPosition(b);
}

/////////////////////// MapActiveObject \\\\\\\\\\\\\\\\\\\\\\\

MapActiveObject::MapActiveObject() :SolidObject() {};

/////////////////////// SolidObject \\\\\\\\\\\\\\\\\\\\\\\

SolidObject::SolidObject() :Entity() {
	collider_local_position = { 0,0 };
	collider_local_size = { 80,80 };
	flying = 0;
}
SolidObject::SolidObject(const std::string& s, const sf::Texture& t) : Entity(s, t) {
	collider_local_position = { 0,0 };
	collider_local_size = { 80,80 };

	if (s.find("[flying]") != std::string::npos) {
		flying = 1;
	}
	else {
		flying = 0;
	}

	auto clpx = s.find("[clpx=", 0) + 6, clpy = s.find("[clpy=", 0) + 6;
	try {
		this->collider_local_position = { std::stof(s.substr(clpx, s.find("]", clpx) - clpx)),std::stof(s.substr(clpy, s.find("]", clpy) - clpy)) };
	}
	catch (...) {}

	auto clsx = s.find("[clsx=", 0) + 6, clsy = s.find("[clsy=", 0) + 6;
	try {
		this->collider_local_size = { std::stof(s.substr(clsx, s.find("]", clsx) - clsx)),std::stof(s.substr(clsy, s.find("]", clsy) - clsy)) };
	}
	catch (...) {}

	if (s.find("[sprite_bounds]") != std::string::npos) {
		this->collider_local_size = { sprite.getLocalBounds().width,sprite.getLocalBounds().height };
		this->collider_local_position = { sprite.getLocalBounds().left - sprite.getLocalBounds().width / 2,
			sprite.getLocalBounds().top - sprite.getLocalBounds().height / 2 };
	}


}
void SolidObject::SetSize(const sf::Vector2f& v) {
	this->Entity::SetSize(v);
	SetColliderSize(v);
}
void SolidObject::SetSize(float x, float y) {
	this->SetSize({ x,y });
}
void SolidObject::SetColliderSize(const sf::Vector2f& v) {
	collider.width = collider_local_size.x * v.x; collider.height = collider_local_size.y * v.y;
}
void SolidObject::SetColliderLocalSize(const sf::Vector2f& v) {
	collider_local_size = v;
	SetColliderSize(size);
}
const sf::Vector2f SolidObject::GetColliderSize()const {
	return { collider.width,collider.height };
}
const sf::Vector2f SolidObject::GetColliderLocalSize()const {
	return collider_local_size;
}
const sf::FloatRect& SolidObject::GetCollider()const {
	return collider;
}
void SolidObject::SetPosition(const sf::Vector2f& v) {
	this->Entity::SetPosition(v);
	SetColliderPosition(v);
}
void SolidObject::SetPosition(float a, float b) {
	this->SetPosition({ a,b });
}
void SolidObject::SetColliderPosition(const sf::Vector2f& p) {
	collider.left = collider_local_position.x * size.x + p.x; collider.top = collider_local_position.y * size.y + p.y;
}
void SolidObject::SetColliderLocalPosition(const sf::Vector2f& p) {
	collider_local_position = { p.x,p.y };
	SetColliderPosition(position);
}

void SolidObject::Transform(const sf::Vector2f& p, const sf::Vector2f& s, const float& r) {
	SetSize(s);
	SetPosition(p);
	SetRotation(r);
}

const bool& SolidObject::DetectCollision(const SolidObject& so)const {
	return collider.intersects(so.collider);
}

const sf::Vector2f SolidObject::GetColliderLocalPosition()const {
	return collider_local_position;
}
const sf::Vector2f SolidObject::GetColliderPosition()const {
	return { collider.left,collider.top };
}
const sf::Vector2f SolidObject::GetColliderCenterPosition()const {
	return { collider.left + collider.width / 2.f,collider.top + collider.height / 2.f };
}

/////////////////////////// MapActiveObject \\\\\\\\\\\\\\\\\\\\\\\\

bool MapActiveObject::ActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) { return true; }
bool MapActiveObject::ClientActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) { return true; }
MapActiveObject::MapActiveObject(const std::string& s, const sf::Texture& t) :SolidObject(s, t) {};

/////////////////////// Building \\\\\\\\\\\\\\\\\\\\\\\\\

MapEntity::MapEntity(const sf::Texture& t, const float& hp, const float& def, const sf::Vector2f& size, const sf::Vector2f& pos, float rot) {
	SetPosition(pos);
	SetSize(size);
	SetRotation(rot);

	this->sprite.setTexture(t);
	sprite.setOrigin(t.getSize().x / 2, t.getSize().y / 2);

	collider_local_position = -collider_local_size / 2.f;

	this->hp = hp;
	this->def = def;

	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;
}
MapEntity::MapEntity(const MapEntity& b) {
	SetPosition(b.position);
	SetSize(b.size);
	SetRotation(b.rotation);
	this->sprite.setTexture(*b.sprite.getTexture());
	sprite.setOrigin(b.GetSprite().getOrigin());

	SetColliderLocalPosition(b.GetColliderLocalPosition());
	SetColliderLocalSize(b.GetColliderLocalSize());

	this->hp = b.hp;
	this->def = b.def;

	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;
	id = b.GetId();
}
MapEntity::MapEntity(const std::string& s, const sf::Texture& t) :MapActiveObject(s, t) {
	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;

	auto hp_p = s.find("[hp=", 0) + 4;
	hp = std::stof(s.substr(hp_p, s.find("]", hp_p) - hp_p));
	auto def_p = s.find("[def=", 0) + 5;
	def = std::stof(s.substr(def_p, s.find("]", def_p) - def_p));
}
void MapEntity::ChangeStats(const float& hp, const float& def) {
	this->hp = hp;
	this->def = def;
}

const float& MapEntity::GetHp() const {
	if (hp < 1.f)return 0.f;
	return hp;
}
void  MapEntity::SetHp(const float& v) {
	this->hp = v;
	if (hp < 1.f)hp = 0;
}

bool MapEntity::ActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	for (auto& player : players) {
		if (player.GetActionCollider().intersects(collider)) {
			if (player.action) {
				is_hurt_anim_play = 1;
				wood_sound.play();
				hurt_pos_movement = player.GetPosition();
				hurt_anim_time = sf::milliseconds(0);
				float damage = player.GetDamage() - this->def;
				if (damage < 0)damage = 0;
				this->hp -= damage;
				if (this->hp <= 0)return false;
			}
		}
	}
	return true;
}

bool MapEntity::ClientActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao,
	std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	return true;
}
void MapEntity::Draw(sf::RenderWindow& window, const float& deltaTime, bool& night) {
	if (is_hurt_anim_play)
		PlayHurtAnimation(deltaTime);
	Entity::Draw(window, deltaTime, night);
	if (draw_colliders) {
		sf::RectangleShape collider_rect({ collider.width,collider.height });
		collider_rect.setPosition({ collider.left,collider.top });
		collider_rect.setOutlineThickness(5);
		collider_rect.setOutlineColor(sf::Color::Red);
		collider_rect.setFillColor(sf::Color::Transparent);
		window.draw(collider_rect);
	}
}

void MapEntity::PlayHurtAnimation(const float& deltaTime) {
	sf::Time movement_s = sf::seconds(0.5), pause_s = sf::seconds(0.1);
	sf::Time max_time = movement_s + pause_s;

	double spd = 0.03 * deltaTime * 1000;

	if (hurt_anim_time < movement_s) {
		if (hurt_anim_time >= movement_s / 2.f)spd *= -1;
		if (hurt_pos_movement.x >= GetColliderCenterPosition().x) {
			sprite.move(-spd, 0);
		}
		else {
			sprite.move(spd, 0);
		}
		if (hurt_pos_movement.y >= GetColliderCenterPosition().y) {
			sprite.move(0, -spd);
		}
		else {
			sprite.move(0, spd);
		}
	}



	hurt_anim_time += sf::seconds(deltaTime);

	if (hurt_anim_time >= max_time) {
		is_hurt_anim_play = 0;
		hurt_anim_time = sf::milliseconds(0);
		sprite.setPosition(position.x, position.y);
	}
}


bool MapDamaging::ActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	damage_timer += sf::seconds(delta_time);
	for (auto& player : players) {
		if (player.GetCollider().intersects(collider)) {
			if (damage_timer.asSeconds() >= 0.5f) {
				float att = damage - player.GetDef() * 0.2;
				if (att < 0)att = 0;
				player.SetHp(player.GetHp() - att);
				damage_timer = sf::seconds(0);
			}
		}

		if (player.GetActionCollider().intersects(collider)) {
			if (player.action) {
				float att = damage - player.GetDef();
				if (att < 0)att = 0;
				player.SetHp(player.GetHp() - att);

				is_hurt_anim_play = 1;
				hurt_pos_movement = player.GetPosition();
				float damage = player.GetDamage() - this->def;
				if (damage < 0)damage = 0;
				this->hp -= damage;
				if (this->hp <= 0)return false;
			}
		}
	}
	return 1;
}
bool MapDamaging::ClientActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao,
	std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {

	return true;
}
MapDamaging::MapDamaging(const sf::Texture& t, const float& hp, const float& def, const float& att, const sf::Vector2f& size, const sf::Vector2f& pos, float rot) :MapEntity(t, hp, def, size, pos, rot) {
	damage = att;
}
MapDamaging::MapDamaging(const MapDamaging& b) : MapEntity(b) {
	damage = b.damage;
}
MapDamaging::MapDamaging(const std::string& s, const sf::Texture& t) : MapEntity(s, t) {
	damage_timer = sf::seconds(0);
	auto dmg_p = s.find("[dmg=", 0) + 5;
	float dmg = std::stof(s.substr(dmg_p, s.find("]", dmg_p) - dmg_p));
	damage = dmg;
}
void MapDamaging::ChangeStats(const float& att) {
	damage = att;
}
const float& MapDamaging::GetDamage()const { return damage; }
void MapDamaging::SetDamage(const float& f) { damage = f; }

/////////////////////// Bonfire \\\\\\\\\\\\\\\\\\\\\\\

Bonfire::Bonfire(const sf::Texture& t, const float& hp, const float& def, const sf::Vector2f& size, const sf::Vector2f& pos, float rot) :MapEntity(t, hp, def, size, pos, rot) {
	fire.setTexture(bonfire_fire_texture[0]);
	fire.setOrigin(fire.getTexture()->getSize().x / 2, fire.getTexture()->getSize().y / 2);
	idle_anim_time = sf::seconds(0);
	is_idle_anim_play = 1;
	draw_layer = -5;
};
Bonfire::Bonfire(const Bonfire& b) :MapEntity(b) {
	fire = b.fire;
	idle_anim_time = sf::seconds(0);
	is_idle_anim_play = 1;
	draw_layer = -5;
}
void Bonfire::NightChange(const bool& night) {

}
Bonfire::Bonfire(const std::string& s, const sf::Texture& t) : MapEntity(s, t) {
	fire.setTexture(bonfire_fire_texture[0]);
	fire.setOrigin(fire.getTexture()->getSize().x / 2, fire.getTexture()->getSize().y / 2);
	idle_anim_time = sf::seconds(0);
	is_idle_anim_play = 1;
	draw_layer = -5;

	auto f_p = s.find("[fire_sprite=", 0) + 13;
	if (f_p != std::string::npos + 13) {
		fire_texture.loadFromFile("resources\\" + (sf::String)(s.substr(f_p, s.find("]", f_p) - f_p)));
		fire_texture.setSmooth(1);
		fire.setTexture(fire_texture, 1);
		fire.setOrigin(fire.getTexture()->getSize().x / 2, fire.getTexture()->getSize().y / 2);
	}

}
void Bonfire::Draw(sf::RenderWindow& window, const float& deltaTime, bool& night) {
	if (is_hurt_anim_play)
		PlayHurtAnimation(deltaTime);
	window.draw(sprite);

	if (is_idle_anim_play)
		PlayIdleAnimation(deltaTime);
	window.draw(fire);
	if (draw_colliders) {
		sf::CircleShape s;
		s.setOutlineThickness(5);
		s.setOutlineColor(sf::Color::Yellow);
		s.setFillColor(sf::Color::Transparent);
		s.setRadius(sprite.getScale().x * sprite.getTexture()->getSize().x / 2);
		s.setPosition(position.x - s.getRadius(), position.y - s.getRadius());
		window.draw(s);

		sf::RectangleShape collider_rect({ collider.width,collider.height });
		collider_rect.setPosition({ collider.left,collider.top });
		collider_rect.setOutlineThickness(5);
		collider_rect.setOutlineColor(sf::Color::Red);
		collider_rect.setFillColor(sf::Color::Transparent);
		window.draw(collider_rect);
	}

}
void Bonfire::SetPosition(const sf::Vector2f& v) {
	SolidObject::SetPosition(v);
	fire.setPosition(v);
}
void Bonfire::SetSize(const sf::Vector2f& v) {
	SolidObject::SetSize(v);
	fire.setScale(v);
}
void Bonfire::Transform(const sf::Vector2f& p, const sf::Vector2f& s, const float& r) {
	SolidObject::Transform(p, s, r);
	fire.setPosition(p);
	fire.setScale(s);
	fire.setRotation(r);
}
bool Bonfire::ActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	bool r = true;
	r &= this->MapEntity::ActiveBehaviour(players, near_mao, mao, delta_time, t);
	for (auto& player : players) {
		if (player.Distance(*this) <= sprite.getScale().x * sprite.getTexture()->getSize().x / 2) {
			player.SetTempreture(player.GetTempreture() + delta_time * player.tempreture_decrease * 2);
			player.near_fire = 1;
		}
	}
	this->hp -= delta_time;
	if (this->hp <= 0)return false;
	return r;
}
bool Bonfire::ClientActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao,
	std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	return true;
}
void Bonfire::PlayIdleAnimation(const float& deltaTime) {
	sf::Time stretch = sf::seconds(2), wait = sf::seconds(0.1);
	sf::Time max_time = stretch + wait;

	double spd = 0.08 * deltaTime;

	if (idle_anim_time < stretch) {
		if (idle_anim_time < stretch / 2.f) {
			fire.setScale(fire.getScale().x + spd, fire.getScale().y + spd);
		}
		else {
			fire.setScale(fire.getScale().x - spd, fire.getScale().y - spd);
		}
	}


	idle_anim_time += sf::seconds(deltaTime);

	if (idle_anim_time >= max_time) {
		idle_anim_time = sf::milliseconds(0);

		fire.setScale(GetSize());
	}
}

const sf::Sprite& Bonfire::GetFireSprite()const {
	return fire;
}
sf::Sprite& Bonfire::GetFireSpriteRef() {
	return fire;
}
/////////////////////// MaterialSourse \\\\\\\\\\\\\\\\\\\\\\\\\

MaterialSource::MaterialSource() {
	position.x = 0; position.y = 0;
	is_hurt_anim_play = 0;
}
MaterialSource::MaterialSource(const sf::Texture& t, const BaseItem& b, const sf::Vector2f& size, const sf::Vector2f& pos, float rot, int mx_c, int rest_c, float mng) {
	SetPosition(pos);
	SetSize(size);
	SetRotation(rot);
	this->sprite.setTexture(t);
	sprite.setOrigin(t.getSize().x / 2, t.getSize().y / 2);
	collider_local_position = -collider_local_size / 2.f;

	material.push_back(Item(&b, mx_c));
	max_count.push_back(mx_c);
	restore_count.push_back(rest_c);
	loot_per_a_time.push_back(1);
	this->chance.push_back(1000);
	mining.push_back(mng);

	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;

	restore_time = sf::seconds(30);
	last_restore = sf::seconds(0);

}
MaterialSource::MaterialSource(const std::string& s, const sf::Texture& t) :MapActiveObject(s, t) {
	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;

	restore_time = sf::seconds(30);
	last_restore = sf::seconds(0);
	for (int i = 1; i <= 9; i++) {
		try {
			auto m_p = s.find("[material_id" + std::to_string(i) + "=", 0) + 14;
			int m = std::stoi(s.substr(m_p, s.find("]", m_p) - m_p));

			auto mc_p = s.find("[material_count" + std::to_string(i) + "=", 0) + 17;
			int mc = std::stoi(s.substr(mc_p, s.find("]", mc_p) - mc_p));

			auto lpt_p = s.find("[lpt" + std::to_string(i) + "=", 0) + 6;
			int lpt = std::stoi(s.substr(lpt_p, s.find("]", lpt_p) - lpt_p));

			auto rc_p = s.find("[retore_count" + std::to_string(i) + "=", 0) + 15;
			int rc = std::stoi(s.substr(rc_p, s.find("]", rc_p) - rc_p));
			auto ch_p = s.find("[chance" + std::to_string(i) + "=", 0) + 9;
			int ch = std::stoi(s.substr(ch_p, s.find("]", ch_p) - ch_p));

			auto mining_p = s.find("[mining" + std::to_string(i) + "=", 0) + 9;
			int mining_ = std::stoi(s.substr(mining_p, s.find("]", mining_p) - mining_p));


			material.push_back(Item(items[m], mc));
			loot_per_a_time.push_back(lpt);
			restore_count.push_back(rc);
			max_count.push_back(mc);
			chance.push_back(ch);
			mining.push_back(mining_);
		}
		catch (std::exception& e) {
			if (i == 1)throw e;
			break;
		}
	}
	try {
		auto rt_p = s.find("[rt=", 0) + 4;
		int rt = std::stof(s.substr(rt_p, s.find("]", rt_p) - rt_p));
		restore_time = sf::seconds(rt);
	}
	catch (std::exception& e) {}
}
MaterialSource::MaterialSource(const MaterialSource& s) {
	SetPosition(s.position);
	SetSize(s.size);
	SetRotation(s.rotation);
	this->sprite.setTexture(*s.sprite.getTexture());
	sprite.setOrigin(s.GetSprite().getOrigin());

	SetColliderLocalPosition(s.GetColliderLocalPosition());
	SetColliderLocalSize(s.GetColliderLocalSize());

	material = s.material;
	max_count = s.max_count;
	restore_count = s.restore_count;
	loot_per_a_time = s.loot_per_a_time;
	chance = s.chance;
	mining = s.mining;

	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;

	restore_time = s.restore_time;
	last_restore = s.last_restore;
	id = s.GetId();
}
bool MaterialSource::ActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	Restore(t);
	for (auto& player : players) {
		if (player.GetActionCollider().intersects(collider)) {
			if (player.action) {
				is_hurt_anim_play = 1;
				hurt_pos_movement = player.GetPosition();
				wood_sound.play();
				for (int j = 0; j < material.size(); j++) {
					if (material[j].count > 0) {
						if (std::rand() % 1000 <= chance[j]) {
							if (player.GetMining() - mining[j] <= 0)continue;
							int loot;
							if (mining[j] == -1)
								loot = 1;
							else
								loot = loot_per_a_time[j] * player.GetMining() - mining[j];
							player.AddItem(material[j].item, loot);
							material[j].count -= loot;
							if (material[j].count < 0)material[j].count = 0;
						}
					}
					else {
						//		std::cout << "no materials\n";
					}
				}
			}
		}
	}
	return true;
}
bool MaterialSource::ClientActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao,
	std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	return 1;
}
void MaterialSource::Draw(sf::RenderWindow& window, const float& deltaTime, bool& night) {
	if (is_hurt_anim_play)
		PlayHurtAnimation(deltaTime);
	Entity::Draw(window, deltaTime, night);
	if (draw_colliders) {
		sf::RectangleShape collider_rect({ collider.width,collider.height });
		collider_rect.setPosition({ collider.left,collider.top });
		collider_rect.setOutlineThickness(5);
		collider_rect.setOutlineColor(sf::Color::Red);
		collider_rect.setFillColor(sf::Color::Transparent);
		window.draw(collider_rect);
	}
}

void MaterialSource::Restore(const sf::Time& t) {
	if (t > last_restore + restore_time) {
		for (int i = 0; i < material.size(); i++) {
			material[i].count += restore_count[i];
			if (material[i].count > max_count[i])material[i].count = max_count[i];
		}
		last_restore = t;
	}
}

void MaterialSource::PlayHurtAnimation(const float& deltaTime) {
	sf::Time movement_s = sf::seconds(0.5), pause_s = sf::seconds(0.1);
	sf::Time max_time = movement_s + pause_s;

	double spd = 0.03 * deltaTime * 1000;
	if (hurt_anim_time < movement_s) {
		if (hurt_anim_time >= movement_s / 2.f)spd *= -1;
		if (hurt_pos_movement.x >= GetColliderCenterPosition().x) {
			sprite.move(-spd, 0);
		}
		else {
			sprite.move(spd, 0);
		}
		if (hurt_pos_movement.y >= GetColliderCenterPosition().y) {
			sprite.move(0, -spd);
		}
		else {
			sprite.move(0, spd);
		}
	}



	hurt_anim_time += sf::seconds(deltaTime);

	if (hurt_anim_time >= max_time) {
		is_hurt_anim_play = 0;
		hurt_anim_time = sf::milliseconds(0);
		sprite.setPosition(position.x, position.y);
	}
}
void MaterialSource::operator()(const sf::Vector2f& p, const sf::Vector2f& s, const float& r) {
	position = p;
	size = s;
	rotation = r;
}
void MaterialSource::AddItem(const BaseItem& b, int rest_c, int mx_c, int pt, int chance, float mng) {
	material.push_back(Item(&b, mx_c));
	max_count.push_back(mx_c);
	restore_count.push_back(rest_c);
	loot_per_a_time.push_back(1);
	this->chance.push_back(chance);
	mining.push_back(mng);
}


Fleeing::Fleeing() :Animal() {
	hp = 10;
	max_hp = 10;
	damage = 0;
	def = 0;

	speed = 1.f;

	deadlock_timer = sf::seconds(0);
	walk_cooldown = sf::seconds(0.01f * (std::rand() % 99));
}

Fleeing::Fleeing(const sf::Texture& t, const float& max_hp, const float& def, const float& att, const sf::Vector2f& size, const sf::Vector2f& pos, float rot) :Animal() {
	this->hp = max_hp;
	this->max_hp = max_hp;
	this->damage = att;
	this->def = def;

	speed = 1.f;

	is_hurt_anim_play = 0;
	is_idle_anim_play = 1;
	deadlock_timer = sf::seconds(0);
	walk_cooldown = sf::seconds(0.01f * (std::rand() % 9));

	SetPosition(pos);
	SetSize(size);
	SetRotation(rot);

	b_size = GetSize();

	this->sprite.setTexture(t);
	sprite.setOrigin(t.getSize().x / 2, t.getSize().y / 2);

	collider_local_position = -collider_local_size / 2.f;

}
Fleeing::Fleeing(const std::string& s, const sf::Texture& t) :Animal(s, t) {
	try {
		auto fd_p = s.find("[fd=", 0) + 4;
		fleeing_distance = std::stoi(s.substr(fd_p, s.find("]", fd_p) - fd_p));
	}
	catch (...) {
		fleeing_distance = 160;
	}

	is_hurt_anim_play = 0;
	is_idle_anim_play = 1;
	deadlock_timer = sf::seconds(0);
	walk_cooldown = sf::seconds(0.01f * (std::rand() % 9));
	b_size = GetSize();
}
bool Fleeing::ActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	bool r = true;
	this->dx = this->dy = 0;
	for (auto& player : players) {
		if (player.GetActionCollider().intersects(collider)) {
			if (player.action) {

				float att = player.GetDamage() - def;
				if (att < 0)att = 0;
				SetHp(GetHp() - att);
				r &= GetHp() > 0;
				if (GetHp() <= 0 && box_id >= 0) {
					MapActiveObject* b;
					b = new Box(*static_cast<Box*>(base_entity[box_id]));
					b->Transform(GetPosition(), base_entity[box_id]->GetSize(), GetRotation() + base_entity[box_id]->GetRotation());
					mao.push_back(b);
					return 0;
				}
			}
		}
	}

	Player* player = &players[0];
	for (auto& pl : players) {
		if (pl.Distance(*this) < player->Distance(*this)) {
			player = &pl;
		}
	}
	if (player->Distance(*this) <= fleeing_distance) {
		if (std::abs(player->GetPosition().x - GetPosition().x) > 30) {
			this->dx = ((player->GetPosition().x - GetPosition().x) > 0 ? -1 : 1);
		}
		if (std::abs(player->GetPosition().y - GetPosition().y) > 30) {
			this->dy = ((player->GetPosition().y - GetPosition().y) > 0 ? -1 : 1);
		}
		float t_dx = dx, t_dy = dy;
		if (deadlock_timer.asSeconds() > 0) {
			deadlock_timer -= sf::seconds(delta_time);
			dx = deadlock.x;
			dy = deadlock.y;
		}
		for (auto obj : near_mao) {
			CollideSolidObject(*obj);
		}
		if (!dx && !dy && deadlock_timer.asSeconds() <= 0) {
			deadlock.x = -t_dx;
			deadlock.y = -t_dy;
			deadlock_timer = sf::seconds(0.8f + 0.1f * (std::rand() % 18));
		}

		Move(dx * speed * delta_time, dy * speed * delta_time);
		RotateTo(GetPosition().x + dx * speed * delta_time, GetPosition().y + dy * speed * delta_time, base_entity[id]->GetRotation());
		walk_cooldown = sf::seconds(0);
		walk.x = dx;
		walk.y = dy;
	}
	else {
		if (walk_timer.asSeconds() > 0 && walk_cooldown.asSeconds() <= 0) {
			dx = walk.x;
			dy = walk.y;
			for (auto obj : near_mao) {
				CollideSolidObject(*obj);
			}
			Move(dx * speed * delta_time, dy * speed * delta_time);
			if (player->Distance(*this) <= fleeing_distance) {
				Move(-dx * speed * delta_time, -dy * speed * delta_time);
			}
			else {
				RotateTo(GetPosition().x + dx * speed * delta_time, GetPosition().y + dy * speed * delta_time, base_entity[id]->GetRotation());
			}
			walk_timer -= sf::seconds(delta_time);
		}
		else if (walk_cooldown.asSeconds() <= 0) {
			walk_cooldown = sf::seconds(2);
			walk_timer = sf::seconds(0.2f);

			walk.x = std::rand() % 3 - 1;
			walk.y = std::rand() % 3 - 1;
		}
		else {
			walk_cooldown -= sf::seconds(delta_time);
		}
	}
	return r;
}


void Fleeing::Draw(sf::RenderWindow& window, const float& deltaTime, bool& night) {
	Animal::Draw(window, deltaTime, night);

	if (draw_colliders && draw_more) {
		sf::CircleShape aa;
		aa.setOutlineThickness(4);
		aa.setOutlineColor(sf::Color(2, 2, 2, 105));
		aa.setFillColor(sf::Color::Transparent);
		aa.setRadius(fleeing_distance);
		aa.setPosition(GetPosition().x - aa.getRadius(), GetPosition().y - aa.getRadius());
		window.draw(aa);
	}
}



Box::Box(const std::string& s, const sf::Texture& t) :MapEntity(s, t) {
	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;
	inventory.resize(9, Item(items[0], 0));

	for (int i = 1; i <= 9; i++) {
		try {
			auto item_pos = s.find("[item_id" + std::to_string(i) + "=", 0) + 10;
			int item_id = std::stoi(s.substr(item_pos, s.find("]", item_pos) - item_pos));

			auto c_pos = s.find("[c" + std::to_string(i) + "=", 0) + 4;
			int c = std::stoi(s.substr(c_pos, s.find("]", c_pos) - c_pos));

			AddItem(items[item_id], c);
		}
		catch (std::exception& e) {
			if (i == 1)throw e;
			break;
		}
	}
};

bool Box::ActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	bool r = true;
	for (auto& player : players) {
		if (player.GetActionCollider().intersects(GetCollider())) {
			if (player.action) {
				is_hurt_anim_play = 1;
				hurt_pos_movement = player.GetPosition();

				float att = player.GetDamage() - def;
				if (att < 0)att = 0;
				SetHp(GetHp() - att);
				r &= GetHp() > 0;
				if (GetHp() <= 0) {
					player.AddInventory(inventory);
					return 0;
				}
			}
		}
	}
	return r;
}

bool Box::ClientActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao,
	std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	for (auto& player : players) {
		if (player.GetActionCollider().intersects(collider)) {
			if (player.action) {
				is_hurt_anim_play = 1;
				hurt_pos_movement = player.GetPosition();
			}
		}
	}
	return true;
}

Enemy::Enemy(const std::string& s, const sf::Texture& t) :Animal(s, t) {
	try {
		auto aa_p = s.find("[aa=", 0) + 4;
		aggressive_area = std::stof(s.substr(aa_p, s.find("]", aa_p) - aa_p));
	}
	catch (...) {
		aggressive_area = 100;
	}

	try {
		auto ad_p = s.find("[attack_distance=", 0) + 17;
		attack_distnce = std::stof(s.substr(ad_p, s.find("]", ad_p) - ad_p));
	}
	catch (...) {
		attack_distnce = 10;
	}

	is_hurt_anim_play = 0;
	is_idle_anim_play = 1;
	walk_cooldown = sf::seconds(0.01f * (std::rand() % 9));
	b_size = GetSize();
}
bool Enemy::ActiveBehaviour(std::vector<Player>& players, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t) {
	bool r = true;
	this->dx = this->dy = 0;
	for (auto& player : players) {
		if (player.GetActionCollider().intersects(collider)) {
			if (player.action) {

				float att = player.GetDamage() - def;
				if (att < 0)att = 0;
				SetHp(GetHp() - att);
				r &= GetHp() > 0;
				if (GetHp() <= 0 && box_id >= 0) {
					MapActiveObject* b;
					b = new Box(*static_cast<Box*>(base_entity[box_id]));
					b->Transform(GetPosition(), base_entity[box_id]->GetSize(), GetRotation() + base_entity[box_id]->GetRotation());
					mao.push_back(b);
					return 0;
				}
			}
		}
	}
	Player* player = &players[0];
	for (auto& pl : players) {
		if (pl.Distance(*this) < player->Distance(*this)) {
			player = &pl;
		}
	}
	if (player->Distance(*this) <= aggressive_area) {
		if (std::abs(player->GetPosition().x - GetPosition().x) > 20) {
			this->dx = ((player->GetPosition().x - GetPosition().x) > 0 ? 1 : -1);
		}
		if (std::abs(player->GetPosition().y - GetPosition().y) > 20) {
			this->dy = ((player->GetPosition().y - GetPosition().y) > 0 ? 1 : -1);
		}
		for (auto obj : near_mao) {
			CollideSolidObject(*obj);
		}

		Move(dx * speed * delta_time, dy * speed * delta_time);
		RotateTo(player->GetPosition(), base_entity[id]->GetRotation());
		walk_cooldown = sf::seconds(0);
		walk.x = dx;
		walk.y = dy;
	}
	else {
		if (walk_timer.asSeconds() > 0 && walk_cooldown.asSeconds() <= 0) {
			dx = walk.x;
			dy = walk.y;
			for (auto obj : near_mao) {
				CollideSolidObject(*obj);
			}
			Move(dx * speed * delta_time, dy * speed * delta_time);
			RotateTo(GetPosition().x + dx * speed * delta_time, GetPosition().y + dy * speed * delta_time, base_entity[id]->GetRotation());

			walk_timer -= sf::seconds(delta_time);
		}
		else if (walk_cooldown.asSeconds() <= 0) {
			walk_cooldown = sf::seconds(2);
			walk_timer = sf::seconds(0.2f);

			walk.x = std::rand() % 3 - 1;
			walk.y = std::rand() % 3 - 1;
		}
		else {
			walk_cooldown -= sf::seconds(delta_time);
		}
	}
	if (player->Distance(*this) <= attack_distnce && hit_timer <= sf::seconds(0)) {
		float att = damage - player->GetDef();
		if (att < 0)att = 0;
		player->SetHp(player->GetHp() - att);
		hit_timer = hit_cooldown;
	}
	if (hit_timer > sf::seconds(0))
		hit_timer -= sf::seconds(delta_time);

	return r;
}

void Enemy::Draw(sf::RenderWindow& window, const float& deltaTime, bool& night) {
	Animal::Draw(window, deltaTime, night);

	if (draw_colliders) {
		sf::CircleShape ad;
		ad.setOutlineThickness(5);
		ad.setOutlineColor(sf::Color(1, 1, 1, 175));
		ad.setFillColor(sf::Color::Transparent);
		ad.setRadius(attack_distnce);
		ad.setPosition(GetPosition().x - ad.getRadius(), GetPosition().y - ad.getRadius());
		window.draw(ad);

		if (draw_more) {
			sf::CircleShape aa;
			aa.setOutlineThickness(4);
			aa.setOutlineColor(sf::Color(2, 2, 2, 105));
			aa.setFillColor(sf::Color::Transparent);
			aa.setRadius(aggressive_area);
			aa.setPosition(GetPosition().x - aa.getRadius(), GetPosition().y - aa.getRadius());
			window.draw(aa);
		}
	}

}