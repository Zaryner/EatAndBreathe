#pragma once
#include <vector>
#include <string>
#include <SFML\Graphics.hpp>

class BaseItem;
class Consumable;
struct Item;
class Entity;
class Player;
class IAmMapActiveObject;
class MapActiveObject;
class SolidObject;
class MaterialSource;

class BaseItem {
protected:
	std::string ru_name, eng_name, bel_name;
	std::string ru_description, eng_description, bel_description;
	int id;
	int max_count;
	sf::Texture texture;
	bool activated;
public:

	BaseItem();
	BaseItem(const std::string& s, const sf::Texture& t);

	int GetId()const;
	const std::string& GetRuName()const;
	const std::string& GetEngName()const;
	const std::string& GetBelName()const;
	const std::string& GetRuDescription()const;
	const std::string& GetEngDescription()const;
	const std::string& GetBelDescription()const;
	bool isActivated()const;

	int GetMaxCount()const;
	void SetMaxCount(int n);

	const sf::Texture& GetTexture()const;
	virtual void Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao)const;
};

class Consumable :public BaseItem {
	float hunger;
	float health;
	float tempreture;
	float water;
public:
	Consumable(const std::string& s, const sf::Texture& t);
	virtual void Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao)const;
	float GetEffect(int i)const;
};

class BuildItem :public BaseItem {
	bool refill;
	float hp;
	float def;
	int entity_id;
	float damage;
public:
	BuildItem(const std::string& s, const sf::Texture& t);
	virtual void Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao)const;
	int GetEntityId()const;
};

struct Item {
	BaseItem const* item;
	int count;
	Item();
	Item(int id, int count = 1);
	Item(const BaseItem const* baseItem, int count = 1);
	void Reset();
	virtual void Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao);
};

class Recipe {
	int id;
	BaseItem const* item;
	int count;
	std::vector<BaseItem const*>materials;
	std::vector<int>material_count;
	//bool fire;
	//bool water;
public:
	Recipe();
	Recipe(const BaseItem const* item,int count,int id);
	void AddMaterial(const BaseItem const* m,const int& count);
	bool Available(const Player& p)const;
	const std::vector<BaseItem const*>& GetMaterials()const;
	const std::vector<int>& GetMaterialsCount()const;
	const BaseItem const* GetItem()const;
	const int& GetCount()const;
};

extern void LoadItems();
extern void LoadRecipes();

class Entity {
protected:
	sf::Vector2f position;
	sf::Vector2f size;
	float rotation;

	sf::Sprite sprite;
	virtual void Move(float dx, float dy);
public:
	Entity();
	Entity(const std::string& s, const sf::Texture& t);

	sf::Color base_color;

	virtual void SetSize(const sf::Vector2f& v);
	virtual void SetSize(float x, float y);
	const sf::Vector2f& GetSize();

	virtual void SetPosition(const sf::Vector2f& v);
	virtual void SetPosition(float x, float y);
	const sf::Vector2f& GetPosition();

	virtual void Draw(sf::RenderWindow& window, const float& deltaTime);
	void SetTexture(sf::Texture& texture, const sf::Vector2f& origin_offset = { 0,0 });
	const sf::Sprite& GetSprite()const;
	sf::Sprite& GetSpriteRef();

	virtual void RotateTo(float x, float y, float offset);
	virtual void RotateTo(sf::Vector2f pos, float offset);
	virtual void Rotate(float v);
	virtual void SetRotation(float v);
	float GetRotation()const;

	virtual float Distance(const Entity& e)const;
	virtual void Transform(const sf::Vector2f& p, const sf::Vector2f& s = { 1,1 }, const float& r = 0);

};
class SolidObject :public Entity {
protected:
	sf::FloatRect collider;
	sf::Vector2f collider_local_position;
	sf::Vector2f collider_local_size;
public:
	SolidObject();
	SolidObject(const std::string& s, const sf::Texture& t);
	virtual void SetPosition(const sf::Vector2f& v);
	virtual void SetPosition(float x, float y);

	virtual void SetSize(const sf::Vector2f& v);
	virtual void SetSize(float x, float y);
	void SetColliderSize(const sf::Vector2f& s);
	void SetColliderLocalSize(const sf::Vector2f& v);

	virtual void Transform(const sf::Vector2f& p, const sf::Vector2f& s = { 1,1 }, const float& r = 0);

	const bool& DetectCollision(const SolidObject& so)const;

	virtual const sf::Vector2f GetColliderPosition()const;
	virtual const sf::Vector2f GetColliderCenterPosition()const;
	virtual const sf::Vector2f GetColliderLocalPosition()const;
	void SetColliderPosition(const sf::Vector2f& p);
	void SetColliderLocalPosition(const sf::Vector2f& p);

	const sf::Vector2f GetColliderSize()const;
	const sf::Vector2f GetColliderLocalSize()const;
	const sf::FloatRect& GetCollider()const;
};
class Animal :public SolidObject {
protected:
	// stats
	float hp;
	float max_hp;
	float def;
	float damage;

	// movement
	float speed = 1.f;
	short dx, dy;

public:
	Animal();

	virtual void SetSpeed(float s);
	virtual float GetSpeed();

	virtual float GetDef()const;
	virtual void SetDef(float v);

	virtual float GetDamage()const;
	virtual void SetDamage(float v);

	virtual float GetHp()const;
	virtual void SetHp(float v);

	virtual float GetMaxHp()const;
	virtual void SetMaxHp(float v);
};
class Player : public Animal {
protected:
	float hunger;
	float tempreture;
	float water;

	sf::FloatRect action_collider;

	std::vector<Item> inventory;
	Item* active_item;

	bool right_facing;

	sf::Time idle_anim_time;
	bool is_idle_anim_play;

	sf::Sprite left_arm;
	sf::Sprite right_arm;

	sf::Time arm_anim_time;
	bool is_arm_anim_play;
	short left_arm_animated;

	sf::Time hurt_anim_time;
	sf::Time last_hurt_time;
	bool is_hurt_anim_play;

	void Flip();
public:
	bool action;

	float tempreture_decrease;
	float hunger_decrease;
	float water_decrease;
	float water_increase;

	Player();
	float GetState(int id)const;

	float GetHunger()const;
	float GetTempreture()const;
	float GetWater()const;

	virtual void SetHp(float v);
	void SetHunger(float v);
	void SetTempreture(float v);
	void SetWater(float v);

	int InventoryCapacity()const;
	Item& GetItem(int pos);
	Item const& GetItem(int pos)const;
	const int& GetItemCount(int pos)const;
	void AddItem(const BaseItem* item, int count = 1);
	void DecreaseItem(const BaseItem* item, int count);
	bool HaveItem(const BaseItem* item, int count)const;

	bool EmptySlot()const;
	void Craft(const Recipe* r);

	void ChangeActiveItem(Item& item);
	void ChangeActiveItem();
	const Item const* ActiveItem()const;
	void UseActiveItem(std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao);
	void UseItem(const int& id, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao);
	void SwapItems(const int& id1, const int& id2);

	virtual void SetSize(const sf::Vector2f& v);
	virtual void SetSize(const float& sx, const float& sy);
	void SetArmsTexture(sf::Texture& texture);

	virtual void Move(const float& dx, const float& dy);
	void CheckMovement(const float& delta_time, const std::vector<SolidObject*>& so);
	void CheckControl(const float& delta_time);

	void CheckStats(const float& delta_time, const bool& player_on_water);

	void SetPosition(float x, float y);
	void RotateActionCollider(float x, float y, float offset = 0);
	void RotateActionCollider(sf::Vector2f pos, float offset=0);
	void RotateTo(float x, float y, float offset=0);
	void RotateTo(sf::Vector2f pos, float offset=0);

	virtual void Draw(sf::RenderWindow& window, const float& deltaTime);
	void PlayArmsAnimation(const float& deltaTime);
	void PlayHurtAnimation(const float& deltaTime);
	void PlayIdleAnimation(const float& deltaTime);

	void CollideSolidObject(const SolidObject& o);

	const sf::FloatRect& GetActionCollider()const;
};


class MapActiveObject :public SolidObject{
public:
	MapActiveObject();
	MapActiveObject(const std::string& s, const sf::Texture& t);
	virtual bool ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t);
};

class Building :public MapActiveObject {
protected:
	float hp;
	float def;

	sf::Time hit_timer;

	sf::Time hurt_anim_time;
	bool is_hurt_anim_play;
	sf::Vector2f hurt_pos_movement;

public:
	virtual bool ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t);
	Building(const sf::Texture& t, const float& hp = 0, const float& def = 0, const sf::Vector2f& size = { 1,1 }, const sf::Vector2f& pos = { 1,1 }, float rot = 0);
	Building(const Building& b);
	Building(const std::string& s,const sf::Texture& t);
	virtual void Draw(sf::RenderWindow& window, const float& deltaTime);
	void PlayHurtAnimation(const float& deltaTime);
	virtual void ChangeStats(const float& hp, const float& def);
};
class SpikyBuilding :public Building {
protected:
	float damage;
	sf::Time damage_timer;
public:
	virtual bool ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t);
	SpikyBuilding(const sf::Texture& t, const float& hp = 0, const float& def = 0, const float& att = 1, const sf::Vector2f& size = { 1,1 }, const sf::Vector2f& pos = { 1,1 }, float rot = 0);
	SpikyBuilding(const SpikyBuilding& b);
	SpikyBuilding(const std::string& s, const sf::Texture& t);
	virtual void ChangeStats(const float& att);
	const float& GetDamage()const;
	void SetDamage(const float& f);
};
class Bonfire :public Building {
	sf::Texture fire_texture;
	sf::Sprite fire;
	sf::Time idle_anim_time;
	bool is_idle_anim_play;
public:
	Bonfire(const sf::Texture& t, const float& hp = 0, const float& def = 0, const sf::Vector2f& size = { 1,1 }, const sf::Vector2f& pos = { 1,1 }, float rot = 0);
	Bonfire(const Bonfire& b);
	Bonfire(const std::string& s, const sf::Texture& t);
	virtual void Draw(sf::RenderWindow& window, const float& deltaTime);
	virtual void SetPosition(const sf::Vector2f& v);
	virtual void SetSize(const sf::Vector2f& v);
	virtual void Transform(const sf::Vector2f& p, const sf::Vector2f& s = { 1,1 }, const float& r = 0);
	virtual bool ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t);
	void PlayIdleAnimation(const float& deltaTime);
};
class MaterialSource :public MapActiveObject {
	std::vector<Item> material;
	std::vector<int> loot_per_a_time;
	std::vector<int> restore_count;
	std::vector<int> max_count;
	std::vector<int> chance;
	sf::Time collect_timer;
	sf::Time last_restore;
	sf::Time restore_time;

	sf::Time hurt_anim_time;
	bool is_hurt_anim_play;
	sf::Vector2f hurt_pos_movement;

public:
	virtual bool ActiveBehaviour(Player& player, const float& delta_time, const sf::Time& t);
	MaterialSource();
	MaterialSource(const sf::Texture& t, const BaseItem& b, const sf::Vector2f& size = { 1,1 }, const sf::Vector2f& pos = { 1,1 }, float rot = 0, int mx_c = 100, int rest_c = 10);
	MaterialSource(const MaterialSource& s);
	MaterialSource(const std::string& s, const sf::Texture& t);
	virtual void AddItem(const BaseItem& b, int rest_c, int mx_c, int pt = 1, int chance = 100);
	virtual void Draw(sf::RenderWindow& window, const float& deltaTime);
	virtual void Restore(const sf::Time& t);
	void PlayHurtAnimation(const float& deltaTime);
	void operator()(const sf::Vector2f& p, const sf::Vector2f& s = { 1,1 }, const float& r = 0);
};
