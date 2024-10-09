#pragma once

#include <vector>
#include <unordered_map>

#include <Triangulation.h>

#include "MapGrid.h"

enum class SurfaceType
{
    Ground = 0,
    Water,
    Lava,
    Poison,
    Gay,
    Count,
};

using RegionId = std::size_t;
struct SurfaceData
{
    SurfaceType type = SurfaceType::Ground;
    RegionId region = 0;
};

class SurfaceManager
{

public:
    bool readFromMap(std::vector<cdt::EdgeVInd> &edge_inds, std::vector<Edge> &edges, cdt::Triangulation<cdt::Vector2i> &cdt);

    void changeSurface(cdt::Triangulation<cdt::Vector2i> &cdt, utils::Vector2f pos, SurfaceType new_surface);

    SurfaceType getSurfaceAt(const utils::Vector2f &pos);
    bool isSurfaceAt(SurfaceType surface, const utils::Vector2f &pos);

public:
    using RegionMap = std::unordered_map<RegionId, std::vector<cdt::TriInd>>;
    using SurfaceMap = std::unordered_map<SurfaceType, std::vector<RegionId>>;

    RegionMap m_region2tri_inds;
    SurfaceMap m_surface2regions;
    std::vector<SurfaceData> m_tri2surface;
    std::vector<RegionId> m_tri2region_id;

    cdt::Triangulation<cdt::Vector2i> *p_cdt = nullptr;
};

struct SurfaceTable
{
    bool isWalkable(SurfaceType surface) const;

    void toggleWalkable(SurfaceType surface);

private:
    unsigned int m_surface_is_walkable = 0;
};

std::vector<int> findConnectedTriInds(cdt::Triangulation<cdt::Vector2i> &cdt, int start_tri_ind);

class Wall;
class AABB;
std::vector<Wall *> selectWalls(const utils::Vector2f &left_select, const utils::Vector2f &right_select);