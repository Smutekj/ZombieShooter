#pragma once
#include <Vector2.h>
#include <Transform.h>

#include <vector>

#include "core.h"

struct Polygon : public Transform
{
  std::vector<utils::Vector2f> points;

  Polygon(int n_points = 3, utils::Vector2f at = {0, 0});

  AABB getBoundingRect() const
  {
    auto r = getPosition();
    return {r - getScale(), r + getScale()};
  }

  utils::Vector2f getCenter();

  std::vector<utils::Vector2f> getPointsInWorld();
  void move(utils::Vector2f by);
  void rotate(float by);
  void update(float dt);

  bool isCircle() const
  {
    return points.size() < 3;
  }
  utils::Vector2f getMVTOfSphere(utils::Vector2f center, float radius);

};
