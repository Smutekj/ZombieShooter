#include "Polygon.h"


  Polygon::Polygon(int n_points, utils::Vector2f at) : points(n_points)
  {
    for (int i = 0; i < n_points; ++i)
    {
      utils::Vector2f new_pos;
      new_pos.x = std::cos(i * 2.f * M_PI / n_points + M_PI/4.f);
      new_pos.y = std::sin(i * 2.f * M_PI / n_points + M_PI/4.f);
      points[i] = new_pos;
    }

    // setScale({radius, radius});
    setPosition(at);
  }

  std::vector<utils::Vector2f> Polygon::getPointsInWorld() 
  {
    auto n_points = points.size();
    std::vector<utils::Vector2f> world_points = points;
    for (int i = 0; i < n_points; ++i)
    {
      transform(world_points[i]);
    }
    return world_points;
  }

  void Polygon::move(utils::Vector2f by)
  {
    setPosition(getPosition() + by);
  }

  void Polygon::rotate(float by)
  {
    setRotation(getRotation() + by * 180. / M_PI);
  }


  utils::Vector2f Polygon::getMVTOfSphere(utils::Vector2f center, float radius)
  {

    const auto &points_world = getPointsInWorld();

    int next = 1;
    const auto n_points1 = points.size();

    float min_overlap = std::numeric_limits<float>::max();
    utils::Vector2f min_axis;
    for (int curr = 0; curr < n_points1; ++curr)
    {
      auto t1 = points_world.at(next) - points_world[curr]; //! line perpendicular to current polygon edge
      utils::Vector2f n1 = {t1.y, -t1.x};
      n1 /= norm(n1);
      auto proj1 = projectOnAxis(n1, points_world);
      float proj_sphere = dot(n1, center);
      Projection1D proj2({proj_sphere - radius, proj_sphere + radius});

      if (!overlap1D(proj1, proj2))
      {
        return {0, 0};
      }
      else
      {
        auto overlap = calcOverlap(proj1, proj2);
        if (overlap < min_overlap)
        {
          min_overlap = overlap;
          min_axis = n1;
        }
      }
      next++;
      if (next == n_points1)
      {
        next = 0;
      }
    }
    return min_axis;
  }
