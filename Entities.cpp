#include <SFML/Graphics.hpp>
#include "Entities.h"
#include "EngineFuncs.h"
#include "Resources.h"

#include <iostream>
#include <fstream>
#include <map>


#define PI 3.1415926535

void LoadItems() {
	items.resize(100);
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
			if (class_str == "baseitem" || class_str == "BaseItem")
				items[id] = new BaseItem(line, item_texture);
			else if (class_str == "consumable" || class_str == "Consumable")
				items[id] = new Consumable(line, item_texture);
			else if (class_str == "builditem" || class_str == "BuildItem" || class_str == "building")
				items[id] = new BuildItem(line, item_texture);
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
	recipes.resize(100);

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

void BaseItem::Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao)const {

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
void Consumable::Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao)const {
	player.SetHp(player.GetHp() + health);
	player.SetHunger(player.GetHunger() + hunger);
	player.SetTempreture(player.GetTempreture() + tempreture);
	player.SetWater(player.GetWater() + water);
	player.DecreaseItem(this, 1);
}

///////////////////////// BuildItem \\\\\\\\\\\\\\\\\\\\\\\\\

BuildItem::BuildItem(const std::string& s, const sf::Texture& t) :BaseItem(s, t) {
	this->hp = this->def = this->entity_id = this->damage = this->refill = 0;

	try {
		auto entity_id_pos = s.find("[entity_id=", 0) + 11;
		this->entity_id = std::stoi(s.substr(entity_id_pos, s.find("]", entity_id_pos) - entity_id_pos));
	}
	catch (...) {}

	if (s.find("[r]", 0) != std::string::npos) {
		refill = 1;
	}
	else return;

	try {
		auto hp_pos = s.find("[hp=", 0) + 4;
		this->hp = std::stof(s.substr(hp_pos, s.find("]", hp_pos) - hp_pos));
	}
	catch (...) {}

	try {
		auto def_pos = s.find("[def=", 0) + 5;
		this->def = std::stof(s.substr(def_pos, s.find("]", def_pos) - def_pos));
	}
	catch (...) {}

	try {
		auto damage_pos = s.find("[damage=", 0) + 8;
		this->damage = std::stof(s.substr(damage_pos, s.find("]", damage_pos) - damage_pos));
	}
	catch (...) {}

}

void BuildItem::Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao)const {
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
		std::cout << "Invalid build item class\n";
	}
	float px = player.GetPosition().x, py = player.GetPosition().y;
	px = px + 100 * std::cos(player.GetRotation() * PI / 180);
	py = py + 100 * std::sin(player.GetRotation() * PI / 180);
	b->Transform({ px,py }, base_entity[entity_id]->GetSize());
	for (auto& obj : near_mao) {
		if (obj->DetectCollision(*b)) return;
	}
	mao.push_back(b);
	player.DecreaseItem(this, 1);
}

int BuildItem::GetEntityId()const { return entity_id; }

///////////////////////// Item \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

Item::Item() {
	item = items[0];
	this->count = 0;
}
Item::Item(int id, int count) {
	item = items[id];
	this->count = count;
}
Item::Item(const BaseItem const* baseItem, int count) {
	item = baseItem;
	this->count = count;
}

void Item::Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {
	item->Use(player, near_mao, mao);
}

void Item::Reset() {
	item = items[0];
	count = 0;
}

//////////////////////// Recipe \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

Recipe::Recipe() {
	this->id = 0;
	if (items.size() > 0)
		this->item = items[0];
	this->count = 0;
}

Recipe::Recipe(const BaseItem const* item, int count, int id) {
	this->item = item;
	this->count = count;
	this->id = id;
}

void Recipe::AddMaterial(const BaseItem const* m, const int& count) {
	materials.push_back(m);
	material_count.push_back(count);
}
bool Recipe::Available(const Player& p)const {
	bool available = p.EmptySlot();
	for (int i = 0; i < p.InventoryCapacity(); i++) {
		if (p.GetItem(i).item == item&&p.GetItem(i).count<item->GetMaxCount()) {
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
const BaseItem const* Recipe::GetItem()const {
	return item;
}
const int& Recipe::GetCount()const {
	return count;
}
//////////////////////// Entity \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

Entity::Entity() {
	position = { 0,0 };
	size = { 1,1 };
	rotation = 0;
}
Entity::Entity(const std::string& s, const sf::Texture& t) {
	position = { 0,0 };
	size = { 1,1 };
	rotation = 0;
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
}
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

void Entity::Draw(sf::RenderWindow& window, const float& deltaTime) {
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
	max_hp = 100;
	hp = 100;
	def = 0;
	base_color = sf::Color(255, 255, 255, 255);
	dx = 0, dy = 0;
}
void Animal::SetSpeed(float s) {
	this->speed = s;
}
float Animal::GetSpeed() {
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
float Animal::GetHp() const {
	if (hp < 1.f)return 0.f;
	return hp;
}
void Animal::SetHp(float v) {
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

///////////////////////////////// Player \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void Player::Flip() {
	this->sprite.setScale(this->sprite.getScale().x * -1, this->sprite.getScale().y);
}
Player::Player() {
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

	base_color = sf::Color(255, 255, 255, 255);

	action = 0;

	idle_anim_time = sf::milliseconds(0);
	is_idle_anim_play = 1;

	arm_anim_time = sf::milliseconds(0);
	left_arm_animated = 0;
	is_arm_anim_play = 0;

	is_hurt_anim_play = 0;
	hurt_anim_time = sf::milliseconds(0);
	last_hurt_time = sf::milliseconds(0);

	inventory.reserve(20);
	inventory.resize(7, { items[0],0 });
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
void Player::SetHp(float v) {
	this->hp = v;
	if (hp < 1.f)hp = 0;
	if (hp > max_hp)hp = max_hp;

	if (v < hp)is_hurt_anim_play = 1;
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

int Player::InventoryCapacity()const {
	return inventory.size();
}

Item& Player::GetItem(int pos) {
	return inventory[pos];
}
Item const& Player::GetItem(int pos)const {
	return inventory[pos];
}
const int& Player::GetItemCount(int pos)const {
	return inventory[pos].count;
}

void Player::AddItem(const BaseItem* item, int count) {
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
void Player::DecreaseItem(const BaseItem* item, int count) {
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

bool Player::HaveItem(const BaseItem* item, int count)const {
	int have = 0;
	for (int i = inventory.size() - 1; i >= 0; i--) {
		if (item->GetId() == inventory[i].item->GetId()) {
			have += inventory[i].count;
		}
	}
	return have >= count;
}

bool Player::EmptySlot()const {
	for (int i = inventory.size() - 1; i >= 0; i--) {
		if (!inventory[i].item->GetId()) {
			return true;
		}
	}
	return false;
}

void Player::Craft(const Recipe* r) {
	if (r->Available(*this)) {
		for (int i = 0; i < r->GetMaterials().size(); i++) {
			DecreaseItem(r->GetMaterials()[i], r->GetMaterialsCount()[i]);
		}
		AddItem(r->GetItem(), r->GetCount());
	}
}

void Player::ChangeActiveItem(Item& item) {
	active_item = &item;
}
void Player::ChangeActiveItem() {
	active_item = nullptr;
}
const Item const* Player::ActiveItem()const {
	return active_item;
}
void Player::UseActiveItem(std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {
	if (active_item != nullptr) {
		active_item->Use(*this, near_mao, mao);
		if (!active_item->item->GetId())active_item = nullptr;
	}
}
void Player::UseItem(const int& id, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao) {
	if (id >= 0 && id < inventory.size() && inventory[id].item != nullptr) {
		inventory[id].Use(*this, near_mao, mao);
	}
}
void Player::SwapItems(const int& id1, const int& id2) {
	std::swap(inventory[id1], inventory[id2]);
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

void Player::CheckMovement(const float& delta_time, const std::vector<SolidObject*>& so) {
	this->dx = 0; this->dy = 0;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))this->dx = -1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))this->dx = 1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))this->dy = -1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))this->dy = 1;

	for (const auto& s_obj : so)
		CollideSolidObject(*s_obj);
	this->Move(dx * speed * delta_time, dy * speed * delta_time);
}
void Player::CheckControl(const float& delta_time) {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		if (!is_arm_anim_play) {
			action = 1;

			is_arm_anim_play = 1;
			is_idle_anim_play = 0;
			arm_anim_time = sf::milliseconds(0);
			right_arm.setPosition(180, 500);
			left_arm.setPosition(480, 500);
		}
	}
	else if (!is_arm_anim_play) {
		action = 0;
	}
}

void Player::CheckStats(const float& delta_time, const bool& player_on_water) {
	SetHunger(hunger - hunger_decrease * delta_time);
	SetTempreture(tempreture - tempreture_decrease * delta_time);
	if (player_on_water) {
		SetWater(water + water_increase * delta_time);
	}
	else SetWater(water - water_decrease * delta_time);

	if (hunger <= 0 || tempreture <= 0 || water <= 0) {
		SetHp(hp - 1.4f * delta_time);
	}

	if (GetHunger() >= 75 && GetWater() >= 75 && GetTempreture() >= 75) {
		SetHp(hp + 0.9f * delta_time);
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

void Player::Draw(sf::RenderWindow& window, const float& deltaTime) {
	window.draw(sprite);

	if (is_arm_anim_play)
		PlayArmsAnimation(deltaTime);
	if (is_hurt_anim_play)
		PlayHurtAnimation(deltaTime);
	if (is_idle_anim_play)
		PlayIdleAnimation(deltaTime);

	window.draw(left_arm, sprite.getTransform() * left_arm.getTransform());
	window.draw(right_arm, sprite.getTransform() * right_arm.getTransform());

	if (draw_colliders) {
		sf::RectangleShape collider_rect({ collider.width,collider.height });
		collider_rect.setPosition({ collider.left,collider.top });
		collider_rect.setOutlineThickness(5);
		collider_rect.setOutlineColor(sf::Color::Red);
		collider_rect.setFillColor(sf::Color::Transparent);
		window.draw(collider_rect);
		sf::RectangleShape acollider_rect({ action_collider.width,action_collider.height });
		acollider_rect.setPosition({ action_collider.left,action_collider.top });
		acollider_rect.setOutlineColor(sf::Color::Black);
		acollider_rect.setOutlineThickness(5);
		acollider_rect.setFillColor(sf::Color::Transparent);
		window.draw(acollider_rect);
	}

}
void Player::PlayArmsAnimation(const float& deltaTime) {

	sf::Time attack_ms = sf::milliseconds(220), pause = sf::milliseconds(140);
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

void Player::PlayHurtAnimation(const float& deltaTime) {

	sf::Time hurt_ms = sf::milliseconds(200), pause = sf::milliseconds(640);
	sf::Time hurt_delay_ms = sf::milliseconds(10);
	sf::Time max_time = hurt_ms + pause;

	int spd = 12;
	int a_spd = 5;

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
		sprite.setColor(base_color);
		left_arm.setColor(base_color);
		right_arm.setColor(base_color);
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


void Player::CollideSolidObject(const SolidObject& o) {
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

const sf::FloatRect& Player::GetActionCollider()const {
	return action_collider;
}

/////////////////////// MapActiveObject \\\\\\\\\\\\\\\\\\\\\\\

MapActiveObject::MapActiveObject() :SolidObject() {};

/////////////////////// SolidObject \\\\\\\\\\\\\\\\\\\\\\\

SolidObject::SolidObject() :Entity() {
	collider_local_position = { 0,0 };
	collider_local_size = { 80,80 };
}
SolidObject::SolidObject(const std::string& s, const sf::Texture& t) :Entity(s, t) {
	collider_local_position = { 0,0 };
	collider_local_size = { 80,80 };
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

bool MapActiveObject::ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t) { return true; }
MapActiveObject::MapActiveObject(const std::string& s, const sf::Texture& t) :SolidObject(s, t) {};

/////////////////////// Building \\\\\\\\\\\\\\\\\\\\\\\\\

Building::Building(const sf::Texture& t, const float& hp, const float& def, const sf::Vector2f& size, const sf::Vector2f& pos, float rot) {
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
	hit_timer = sf::seconds(0);
}
Building::Building(const Building& b) {
	SetPosition(b.position);
	SetSize(b.size);
	SetRotation(b.rotation);
	this->sprite.setTexture(*b.sprite.getTexture());
	sprite.setOrigin(b.GetSprite().getOrigin());

	SetColliderLocalPosition(b.GetColliderLocalPosition());
	SetColliderLocalSize(b.GetColliderLocalSize());

	this->hp = b.hp;
	this->def = b.def;

	hit_timer = sf::seconds(0);
	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;

}
Building::Building(const std::string& s, const sf::Texture& t) :MapActiveObject(s, t) {
	hit_timer = sf::seconds(0);
	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;

	auto hp_p = s.find("[hp=", 0) + 4;
	hp = std::stoi(s.substr(hp_p, s.find("]", hp_p) - hp_p));
	auto def_p = s.find("[def=", 0) + 5;
	def = std::stoi(s.substr(def_p, s.find("]", def_p) - def_p));
}
void Building::ChangeStats(const float& hp, const float& def) {
	this->hp = hp;
	this->def = def;
}
bool Building::ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t) {
	hit_timer += sf::seconds(delta_time);
	if (player.GetActionCollider().intersects(collider)) {
		if (player.action && hit_timer.asSeconds() >= 0.7f) {
			is_hurt_anim_play = 1;
			hurt_pos_movement = player.GetPosition();
			float damage = player.GetDamage() - this->def;
			if (damage < 0)damage = 0;
			this->hp -= damage;
			if (this->hp <= 0)return false;
			hit_timer = sf::seconds(0);
		}
	}
	return true;
}
void Building::Draw(sf::RenderWindow& window, const float& deltaTime) {
	if (is_hurt_anim_play)
		PlayHurtAnimation(deltaTime);
	window.draw(sprite);
	if (draw_colliders) {
		sf::RectangleShape collider_rect({ collider.width,collider.height });
		collider_rect.setPosition({ collider.left,collider.top });
		collider_rect.setOutlineThickness(5);
		collider_rect.setOutlineColor(sf::Color::Red);
		collider_rect.setFillColor(sf::Color::Transparent);
		window.draw(collider_rect);
	}
}

void Building::PlayHurtAnimation(const float& deltaTime) {
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

/////////////////////// SpikyBuilding:: \\\\\\\\\\\\\\\\\\\\\\\\\

bool SpikyBuilding::ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t) {
	damage_timer += sf::seconds(delta_time);
	if (player.GetCollider().intersects(collider)) {
		if (damage_timer.asSeconds() >= 0.5f) {
			float att = damage - player.GetDef() * 0.2;
			if (att < 0)att = 0;
			player.SetHp(player.GetHp() - att);
			damage_timer = sf::seconds(0);
		}
	}
	hit_timer += sf::seconds(delta_time);
	if (player.GetActionCollider().intersects(collider)) {
		if (player.action && hit_timer.asSeconds() >= 0.7f) {
			float att = damage - player.GetDef();
			if (att < 0)att = 0;
			player.SetHp(player.GetHp() - att);

			is_hurt_anim_play = 1;
			hurt_pos_movement = player.GetPosition();
			float damage = player.GetDamage() - this->damage;
			if (damage < 0)damage = 0;
			this->hp -= damage;
			if (this->hp <= 0)return false;
			hit_timer = sf::seconds(0);
		}
	}
	return 1;
}
SpikyBuilding::SpikyBuilding(const sf::Texture& t, const float& hp, const float& def, const float& att, const sf::Vector2f& size, const sf::Vector2f& pos, float rot) :Building(t, hp, def, size, pos, rot) {
	damage = att;
}
SpikyBuilding::SpikyBuilding(const SpikyBuilding& b) :Building(b) {
	damage = b.damage;
}
SpikyBuilding::SpikyBuilding(const std::string& s, const sf::Texture& t) :Building(s, t) {
	damage_timer = sf::seconds(0);
	auto dmg_p = s.find("[dmg=", 0) + 5;
	float dmg = std::stof(s.substr(dmg_p, s.find("]", dmg_p) - dmg_p));
	damage = dmg;
}
void SpikyBuilding::ChangeStats(const float& att) {
	damage = att;
}
const float& SpikyBuilding::GetDamage()const { return damage; }
void SpikyBuilding::SetDamage(const float& f) { damage = f; }

/////////////////////// Bonfire \\\\\\\\\\\\\\\\\\\\\\\

Bonfire::Bonfire(const sf::Texture& t, const float& hp, const float& def, const sf::Vector2f& size, const sf::Vector2f& pos, float rot) :Building(t, hp, def, size, pos, rot) {
	fire.setTexture(bonfire_fire_texture[0]);
	fire.setOrigin(fire.getTexture()->getSize().x / 2, fire.getTexture()->getSize().y / 2);
	idle_anim_time = sf::seconds(0);
	is_idle_anim_play = 1;
};
Bonfire::Bonfire(const Bonfire& b) :Building(b) {
	fire = b.fire;
	idle_anim_time = sf::seconds(0);
	is_idle_anim_play = 1;
}
Bonfire::Bonfire(const std::string& s, const sf::Texture& t) :Building(s, t) {
	fire.setTexture(bonfire_fire_texture[0]);
	fire.setOrigin(fire.getTexture()->getSize().x / 2, fire.getTexture()->getSize().y / 2);
	idle_anim_time = sf::seconds(0);
	is_idle_anim_play = 1;

	auto f_p = s.find("[fire_sprite=", 0) + 13;
	if (f_p != std::string::npos + 13) {
		fire_texture.loadFromFile("resources\\" + (sf::String)(s.substr(f_p, s.find("]", f_p) - f_p)));
		fire_texture.setSmooth(1);
		fire.setTexture(fire_texture);
		fire.setOrigin(fire.getTexture()->getSize().x / 2, fire.getTexture()->getSize().y / 2);
	}

}
void Bonfire::Draw(sf::RenderWindow& window, const float& deltaTime) {
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
bool Bonfire::ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t) {
	bool r = true;
	r &= this->Building::ActiveBehaviour(player, delta_time, t);
	if (player.Distance(*this) <= sprite.getScale().x * sprite.getTexture()->getSize().x / 2) {
		player.SetTempreture(player.GetTempreture() + delta_time * player.tempreture_decrease * 2);
	}
	this->hp -= delta_time;
	if (this->hp <= 0)return false;
	return r;
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

/////////////////////// MaterialSourse \\\\\\\\\\\\\\\\\\\\\\\\\

MaterialSource::MaterialSource() {
	position.x = 0; position.y = 0;
}
MaterialSource::MaterialSource(const sf::Texture& t, const BaseItem& b, const sf::Vector2f& size, const sf::Vector2f& pos, float rot, int mx_c, int rest_c) {
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

	collect_timer = sf::seconds(0);

	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;

	restore_time = sf::seconds(30);
	last_restore = sf::seconds(0);

}
MaterialSource::MaterialSource(const std::string& s, const sf::Texture& t) :MapActiveObject(s, t) {
	collect_timer = sf::seconds(0);

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

			material.push_back(Item(items[m], mc));
			loot_per_a_time.push_back(lpt);
			restore_count.push_back(rc);
			max_count.push_back(mc);
			chance.push_back(ch);
		}
		catch (std::exception& e) {
			if (i == 1)throw e;
			break;
		}
	}
	try {
		auto rt_p = s.find("[rt=", 0) + 4;
		int rt = std::stoi(s.substr(rt_p, s.find("]", rt_p) - rt_p));
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

	collect_timer = sf::seconds(0);

	hurt_anim_time = sf::seconds(0);
	is_hurt_anim_play = 0;

	restore_time = s.restore_time;
	last_restore = s.last_restore;
}
bool MaterialSource::ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t) {
	Restore(t);
	collect_timer += sf::seconds(delta_time);
	if (player.GetActionCollider().intersects(collider)) {
		if (player.action && collect_timer.asSeconds() >= 0.7f) {
			is_hurt_anim_play = 1;
			hurt_pos_movement = player.GetPosition();
			for (int j = 0; j < material.size(); j++) {
				if (material[j].count > 0) {
					if (1 + std::rand() % 1000 <= chance[j]) {
						player.AddItem(material[j].item, loot_per_a_time[j]);
						material[j].count -= loot_per_a_time[j];
						if (material[j].count < 0)material[j].count = 0;
					}

					collect_timer = sf::seconds(0);
				}
				else {
					//		std::cout << "no materials\n";
				}
			}
		}
	}
	return true;
}
void MaterialSource::Draw(sf::RenderWindow& window, const float& deltaTime) {
	if (is_hurt_anim_play)
		PlayHurtAnimation(deltaTime);
	window.draw(sprite);
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
void MaterialSource::AddItem(const BaseItem& b, int rest_c, int mx_c, int pt, int chance) {
	material.push_back(Item(&b, mx_c));
	max_count.push_back(mx_c);
	restore_count.push_back(rest_c);
	loot_per_a_time.push_back(1);
	this->chance.push_back(chance);
}