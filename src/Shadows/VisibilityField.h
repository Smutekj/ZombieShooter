#pragma once

#include <Triangulation.h>
#include <VertexArray.h>

struct VisionCone
{
    cdt::Vector2f left;
    cdt::Vector2f right;
    VisionCone() = default;
    VisionCone(cdt::Vector2f left, cdt::Vector2f right)
        : left(left), right(right) {}
};

class VisionField
{

    struct Walker
    {
        cdt::TriInd prev_tri_ind;
        cdt::TriInd curr_tri_ind;
        cdt::Vector2f left;
        cdt::Vector2f right;
    };

public:
    VisionField(cdt::Triangulation<cdt::Vector2i> &cdt);

    bool isVisible(cdt::Vector2f query) const;

    void contrstuctField(cdt::Vector2f from, cdt::Vector2f look_dir);

    void getDrawVertices(Shader &shader, VertexArray& verts,  Color color = {1,1,1,1}, float max_radius = 30.f) const;

private:
    cdt::Vector2f m_center;
    float m_vision_dist = 100.f;
    float m_min_angle = -60;
    float m_max_angle = +60;

    std::vector<VisionCone> m_vision;
    cdt::Triangulation<cdt::Vector2i> &m_cdt;
};
