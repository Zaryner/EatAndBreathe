#pragma once
#include "Entities.h"

#define PI 3.14159265f


float nearest(float a, float b, float target);

float oppositeZoom(float a);

float Distance(const sf::Vector2f& p1, const sf::Vector2f& p2);

void MoveViewTo(sf::View& view, Entity& entity, float speed = 1);

void MoveViewTo(sf::View& view, sf::Vector2f pos, float speed = 1);

void replaceAll(std::string& str, const std::string& from, const std::string& to);

std::string to_lower(std::string s);

void RotateTo(sf::Sprite& s, float x, float y, float offset);

void RotateTo(sf::Sprite& s, const sf::Vector2f& p, float offset);
