#pragma once
#include <vector>
#include <string>
#include <SFML\Graphics.hpp>

class BaseItem;
class Consumable;
struct Item;
class Entity;
class Player;
class Animal;
class IAmMapActiveObject;
class MapActiveObject;
class SolidObject;
class MaterialSource;
class MapDamaging;
class Bonfire;
class Fleeing;
class Box;
class InvetoryCarrier;
class Enemy;

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
	virtual void Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night)const;
};

class Consumable :public BaseItem {
	float hunger;
	float health;
	float tempreture;
	float water;
public:
	Consumable(const std::string& s, const sf::Texture& t);
	virtual void Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night)const;
	float GetEffect(int i)const;
};

class BuildItem :public BaseItem {
	int entity_id;
	bool always_spawn;
public:
	BuildItem(const std::string& s, const sf::Texture& t);
	virtual void Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night)const;
	int GetEntityId()const;
	int GetAlwaysSpawn()const;
};

class EquipmentItem :public BaseItem {
	float def;
	float dmg;
	float spd;

	float hit_incr;

	float mining;

	float tempreture_incr;
	float water_incr;
	float hunger_incr;

	bool draw;
	float px, py;
	float sx, sy;
	float rot;

	short slot;
public:
	EquipmentItem(const std::string& s, const sf::Texture& t);
	virtual void Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night)const;

	float GetHunger()const;
	float GetTempreture()const;
	float GetWater()const;

	float GetSpeed()const;
	float GetDef()const;
	float GetDamage()const;
	float GetMining()const;
	short GetSlot()const;

	sf::Vector2f GetLocalPos()const;
	sf::Vector2f GetLocalScale()const;
	float GetLocalRot()const;

	sf::Time GetHitTime()const;

};

struct Item {
	BaseItem const* item;
	int count;
	Item();
	Item(int id, int count = 1);
	Item(const BaseItem* const baseItem, int count = 1);
	void Reset();
	virtual void Use(Player& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night);
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
	Recipe(const BaseItem* const item, int count, int id);
	void AddMaterial(const BaseItem* const m, const int& count);
	bool Available(const Player& p)const;
	const std::vector<BaseItem const*>& GetMaterials()const;
	const std::vector<int>& GetMaterialsCount()const;
	const BaseItem* const GetItem()const;
	const int& GetCount()const;
};

extern void LoadItems();
extern void LoadRecipes();

class Entity {
protected:
	int id;
	sf::Vector2f position;
	sf::Vector2f size;
	float rotation;
	short draw_layer;

	sf::Sprite sprite;
	virtual void Move(float dx, float dy);
public:
	Entity();
	Entity(const std::string& s, const sf::Texture& t);

	sf::Color base_color;

	int GetId()const;
	void SetId(int id);

	short GetDrawLayer()const;
	void SetDrawLayer(short l);

	virtual void SetSize(const sf::Vector2f& v);
	virtual void SetSize(float x, float y);
	const sf::Vector2f& GetSize();

	virtual void SetPosition(const sf::Vector2f& v);
	virtual void SetPosition(float x, float y);
	const sf::Vector2f& GetPosition();

	virtual void NightChange(const bool& night);

	virtual void Draw(sf::RenderWindow& window, const float& deltaTime, bool& night);
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
	bool flying = 0;
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

class MapActiveObject :public SolidObject {
public:
	MapActiveObject();
	MapActiveObject(const std::string& s, const sf::Texture& t);
	virtual bool ActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	virtual bool ClientActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao,
		std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
};

class Animal :public MapActiveObject {
protected:
	// stats
	float hp;
	float max_hp;
	float def;
	float damage;

	sf::Time hit_timer;
	sf::Time hit_cooldown;

	sf::Time idle_anim_time;
	bool is_idle_anim_play;

	sf::Time hurt_anim_time;
	sf::Time last_hurt_time;
	bool is_hurt_anim_play;

	sf::Vector2f b_size;

	// movement
	float speed = 1.f;
	short dx, dy;

	short box_id;

public:
	Animal();
	Animal(const std::string& s, const sf::Texture& t);

	virtual void SetSpeed(float s);
	virtual float GetSpeed()const;

	virtual float GetDef()const;
	virtual void SetDef(float v);

	virtual float GetDamage()const;
	virtual void SetDamage(float v);

	virtual float GetHp()const;
	virtual void SetHp(const float& v);

	virtual float GetMaxHp()const;
	virtual void SetMaxHp(float v);

	virtual sf::Time GetHitCooldown()const;

	virtual void Transform(const sf::Vector2f& p, const sf::Vector2f& s = { 1,1 }, const float& r = 0);
	virtual void SetSize(const sf::Vector2f& v);
	virtual void SetSize(float x, float y);

	virtual void Move(const float& dx, const float& dy);
	void CollideSolidObject(const SolidObject& o);

	virtual void Draw(sf::RenderWindow& window, const float& deltaTime, bool& night);
	virtual void PlayHurtAnimation(const float& deltaTime, const bool& night);
	virtual void PlayIdleAnimation(const float& deltaTime);
};

class InvetoryCarrier {
protected:
	std::vector<Item> inventory;
public:
	int InventoryCapacity()const;
	Item* FindItem(const BaseItem* const item);
	Item& GetItem(int pos);
	Item const& GetItem(int pos)const;
	const int& GetItemCount(int pos)const;
	void AddItem(const BaseItem* item, int count = 1);
	void DecreaseItem(const BaseItem* item, int count);
	void AddInventory(const std::vector<Item>& v);
	void DropInventory(int item_id, int count, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao);
	bool HaveItem(const BaseItem* item, int count)const;
	bool EmptySlot()const;
	std::vector<Item>& GetInventoryRef();
};

class Player : public Animal, public InvetoryCarrier {
protected:
	float hunger;
	float tempreture;
	float water;

	sf::FloatRect action_collider;

	Item* active_item;
	std::vector<const EquipmentItem*>equipment;


	bool right_facing;

	sf::Sprite left_arm;
	sf::Sprite right_arm;

	sf::Time arm_anim_time;
	bool is_arm_anim_play;
	short left_arm_animated;
	sf::String name;
	int skin_id;

	void Flip();
public:
	bool action;

	float tempreture_decrease;
	float hunger_decrease;
	float water_decrease;
	float water_increase;

	bool near_fire;

	Player();
	float GetState(int id)const;

	float GetHunger()const;
	float GetTempreture()const;
	float GetWater()const;

	void SetHunger(float v);
	void SetTempreture(float v);
	void SetWater(float v);

	void SetName(std::wstring& s);
	const sf::String& GetName()const;

	void SetSkinId(const int& i);
	const int& GetSkinId()const;

	virtual float GetSpeed()const;
	virtual float GetDef()const;
	virtual float GetDamage()const;
	float GetMining()const;
	virtual sf::Time GetHitCooldown()const;
	float GetTemprutureDeacrease()const;
	float GetHungerDeacrease()const;
	float GetWaterDeacrease()const;

	void Craft(const Recipe* r);

	void Dress(const EquipmentItem* item);
	void Undress(const EquipmentItem* item);
	bool isDressed(const BaseItem* const item)const;
	void ChangeActiveItem(Item& item);
	void ChangeActiveItem();
	const Item* const ActiveItem()const;
	void UseActiveItem(std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night);
	void UseItem(const int& id, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const bool& night);
	void SwapItems(const int& id1, const int& id2);
	void DropItem(int item_pos, int count, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao);

	virtual void SetSize(const sf::Vector2f& v);
	virtual void SetSize(const float& sx, const float& sy);
	void SetArmsTexture(sf::Texture& texture);

	virtual void Move(const float& dx, const float& dy);
	void CheckMovement(const float& delta_time, const std::vector<SolidObject*>& so, bool a = 0, bool d = 0, bool w = 0, bool s = 0);
	void CheckControl(const float& delta_time, const bool& right_mouse_pressed, const bool& mouse_on_ui = 0, const bool& mouse_pressed = 0);

	void CheckStats(const float& delta_time, const bool& player_on_water, const bool& night);

	virtual void NightChange(const bool& night);

	void SetPosition(float x, float y);
	void RotateActionCollider(float x, float y, float offset = 0);
	void RotateActionCollider(sf::Vector2f pos, float offset = 0);
	void RotateTo(float x, float y, float offset = 0);
	void RotateTo(sf::Vector2f pos, float offset = 0);

	virtual void Draw(sf::RenderWindow& window, const float& deltaTime, bool& night);
	virtual void PlayArmsAnimation(const float& deltaTime);
	virtual void PlayHurtAnimation(const float& deltaTime, const bool& night);
	virtual void PlayIdleAnimation(const float& deltaTime);

	const sf::FloatRect& GetActionCollider()const;
	void SetActionCollider(sf::FloatRect ac);

	std::vector<const EquipmentItem*>& GetEquipmentRef();
	std::pair<sf::Vector2f, sf::Vector2f> GetArmsPos()const;
	void SetArmsPos(sf::Vector2f a, sf::Vector2f b);
};


class MapEntity :public MapActiveObject {
protected:
	float hp;
	float def =0;

	sf::Time hurt_anim_time;
	bool is_hurt_anim_play;
	sf::Vector2f hurt_pos_movement;
public:
	virtual bool ActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	virtual bool ClientActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao,
		std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	MapEntity(const sf::Texture& t, const float& hp = 0, const float& def = 0, const sf::Vector2f& size = { 1,1 }, const sf::Vector2f& pos = { 1,1 }, float rot = 0);
	MapEntity(const MapEntity& b);
	MapEntity(const std::string& s, const sf::Texture& t);

	virtual void Draw(sf::RenderWindow& window, const float& deltaTime, bool& night);
	void PlayHurtAnimation(const float& deltaTime);
	virtual void ChangeStats(const float& hp, const float& def);

	virtual float GetHp()const;
	virtual void SetHp(const float& v);
};
class MapDamaging :public MapEntity {
protected:
	float damage;
	sf::Time damage_timer;
public:
	virtual bool ActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	virtual bool ClientActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao,
		std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	MapDamaging(const sf::Texture& t, const float& hp = 0, const float& def = 0, const float& att = 1, const sf::Vector2f& size = { 1,1 }, const sf::Vector2f& pos = { 1,1 }, float rot = 0);
	MapDamaging(const MapDamaging& b);
	MapDamaging(const std::string& s, const sf::Texture& t);
	virtual void ChangeStats(const float& att);
	const float& GetDamage()const;
	void SetDamage(const float& f);
};
class Bonfire :public MapEntity {
	sf::Texture fire_texture;
	sf::Sprite fire;

	sf::Time idle_anim_time;
	bool is_idle_anim_play;
public:

	Bonfire(const sf::Texture& t, const float& hp = 0, const float& def = 0, const sf::Vector2f& size = { 1,1 }, const sf::Vector2f& pos = { 1,1 }, float rot = 0);
	Bonfire(const Bonfire& b);
	Bonfire(const std::string& s, const sf::Texture& t);
	virtual void NightChange(const bool& night);
	virtual void Draw(sf::RenderWindow& window, const float& deltaTime, bool& night);
	virtual void SetPosition(const sf::Vector2f& v);
	virtual void SetSize(const sf::Vector2f& v);
	virtual void Transform(const sf::Vector2f& p, const sf::Vector2f& s = { 1,1 }, const float& r = 0);
	virtual bool ActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	virtual bool ClientActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao,
		std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	void PlayIdleAnimation(const float& deltaTime);
	const sf::Sprite& GetFireSprite()const;
	sf::Sprite& GetFireSpriteRef();
};
class MaterialSource :public MapActiveObject {
	std::vector<Item> material;
	std::vector<int> loot_per_a_time;
	std::vector<int> restore_count;
	std::vector<int> max_count;
	std::vector<int> chance;
	sf::Time last_restore;
	sf::Time restore_time;

	std::vector<float> mining;

	sf::Time hurt_anim_time;
	bool is_hurt_anim_play;
	sf::Vector2f hurt_pos_movement;
public:

	virtual bool ActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	virtual bool ClientActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao,
		std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	MaterialSource();
	MaterialSource(const sf::Texture& t, const BaseItem& b, const sf::Vector2f& size = { 1,1 }, const sf::Vector2f& pos = { 1,1 }, float rot = 0, int mx_c = 100, int rest_c = 10, float mng = -1);
	MaterialSource(const MaterialSource& s);
	MaterialSource(const std::string& s, const sf::Texture& t);
	virtual void AddItem(const BaseItem& b, int rest_c, int mx_c, int pt = 1, int chance = 100, float mng = 0);
	virtual void Draw(sf::RenderWindow& window, const float& deltaTime, bool& night);
	virtual void Restore(const sf::Time& t);
	void PlayHurtAnimation(const float& deltaTime);
	void operator()(const sf::Vector2f& p, const sf::Vector2f& s = { 1,1 }, const float& r = 0);
};

class Box :public MapEntity, public InvetoryCarrier {
protected:

public:
	Box(const std::string& s, const sf::Texture& t);
	virtual bool ActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	virtual bool ClientActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao,
		std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
};

class Fleeing :public Animal {
protected:
	float fleeing_distance;

	sf::Time walk_timer;
	sf::Time walk_cooldown;
	sf::Vector2f walk;

	sf::Time deadlock_timer;
	sf::Vector2f deadlock;

public:
	Fleeing();
	Fleeing(const sf::Texture& t, const float& max_hp = 10, const float& def = 0, const float& att = 1, const sf::Vector2f& size = { 1,1 }, const sf::Vector2f& pos = { 1,1 }, float rot = 0);
	Fleeing(const std::string& s, const sf::Texture& t);
	virtual bool ActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);
	virtual void Draw(sf::RenderWindow& window, const float& deltaTime, bool& night);
};

class Enemy :public Animal {
protected:
	float aggressive_area;
	float attack_distnce;

	sf::Time walk_timer;
	sf::Time walk_cooldown;
	sf::Vector2f walk;
public:
	Enemy(const std::string& s, const sf::Texture& t);
	virtual bool ActiveBehaviour(std::vector<Player>& player, std::vector<MapActiveObject*>& near_mao, std::vector<MapActiveObject*>& mao, const float& delta_time, const sf::Time& t);

	virtual void Draw(sf::RenderWindow& window, const float& deltaTime, bool& night);
};