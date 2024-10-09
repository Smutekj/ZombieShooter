#include "Map.h"

#include "GameWorld.h"
#include "Wall.h"

static std::pair<int, int> findTrianglesFromEdge(cdt::Triangulation<cdt::Vector2i> &cdt, cdt::EdgeVInd edge_inds, Edge edge)
{
    auto tri_ind = cdt.findTriangle((edge.from + edge.to()) / 2.f);
    auto &tri = cdt.m_triangles.at(tri_ind);

    auto equals = [](auto v1, auto v2)
    {
        return v1.x == v2.x && v1.y == v2.y;
    };

    for (int k = 0; k < 3; ++k)
    {
        bool cond =
            (equals(tri.verts[k], edge.from) && equals(tri.verts[cdt::next(k)], edge.to())) ||
            (equals(tri.verts[k], edge.to()) && equals(tri.verts[cdt::next(k)], edge.from));
        if (cond)
        {
            return {tri_ind, k};
        }
    }
    return {-1, -1};
}

bool SurfaceManager::readFromMap(std::vector<cdt::EdgeVInd> &edge_inds, std::vector<Edge> &edges, cdt::Triangulation<cdt::Vector2i> &cdt)
{
    auto &world = GameWorld::getWorld();
    std::string wall_name = "Wall0";
    int i = 0;
    while (world.kill(wall_name)) //! destroy all walls
    {
        i++;
        wall_name = "Wall" + std::to_string(i);
    }

    // auto edges = map.extractEdges();

    i = 0;
    for (auto &e : edges)
    {
        auto tri_inds = findTrianglesFromEdge(cdt, edge_inds.at(i), e);

        wall_name = "Wall" + std::to_string(i);

        EdgeId edge_id = {tri_inds.first, tri_inds.second};
        auto ind = edge_inds.at(i);
        // ind.tri_ind

        auto obj = GameWorld::getWorld().addObject<Wall>(wall_name, e.from, e.to(), edge_id);
        i++;
    }

    if (m_surface2regions.count(SurfaceType::Ground) == 0)
    {
        m_surface2regions[SurfaceType::Ground] = {};
    }
    auto &regions = m_surface2regions.at(SurfaceType::Ground);
    for (auto region_id : regions)
    {
        for (auto tri_ind : m_region2tri_inds.at(region_id))
        {
        }
    }
    for (auto &tri : cdt.m_triangles)
    {
        // regions
    }

    using Triangle = cdt::Triangle<cdt::Vector2i>;
    auto triangle_is_within_boundary = [&](Triangle &tri)
    {
        const auto tri_center = tri.getCenter();
        return cdt.isWithinBoundary(tri_center);
    };

    auto count_free_edges = [](const Triangle &tri)
    {
        int n_free_edges = 0;
        for (int k = 0; k < 3; ++k)
        {
            if (!tri.is_constrained[k])
            {
                n_free_edges += 1;
            }
        }
        return n_free_edges;
    };

    auto can_walk_through = [](const Triangle &tri, int ind_in_tri)
    {
        return !tri.is_constrained[ind_in_tri] && tri.neighbours[ind_in_tri] != -1;
    };

    auto &triangles = cdt.m_triangles;

    m_tri2region_id.resize(triangles.size());
    m_tri2surface.resize(triangles.size());
    std::cout << "there are " << std::to_string(triangles.size()) << " in triangulation\n";

    int component_ind = 0;
    std::vector<bool> visited_tris(triangles.size(), false);
    m_tri2region_id.resize(triangles.size());
    m_region2tri_inds.clear();

    auto flood = [&](const cdt::TriInd tri_ind)
    {
        if (visited_tris[tri_ind])
        {
            return;
        }
        std::queue<cdt::TriInd> to_visit({tri_ind});

        while (!to_visit.empty())
        {
            const auto entry_tri_ind = to_visit.front();
            const auto &entry_tri = triangles[entry_tri_ind];
            to_visit.pop();
            if (visited_tris[entry_tri_ind])
            {
                continue;
            }
            visited_tris[entry_tri_ind] = true;
            m_tri2region_id[entry_tri_ind] = component_ind;
            m_region2tri_inds[component_ind].push_back(entry_tri_ind);

            for (int k = 0; k < 3; ++k)
            {
                if (can_walk_through(entry_tri, k))
                {
                    to_visit.push(entry_tri.neighbours[k]);
                }
            }
        }
        component_ind++;
    };

    std::queue<EdgeId> to_visit;
    //! we want to start from all crossroads
    int n_vertices = 0;
    for (cdt::TriInd tri_ind = 0; tri_ind < cdt.m_triangles.size(); ++tri_ind)
    {
        flood(tri_ind);
        const auto &tri = triangles[tri_ind];
        const auto n_free_edges = count_free_edges(tri);
        if (n_free_edges != 2 && cdt.isWithinBoundary(tri.getCenter()))
        { //! crossroads or dead end within boundaries
            for (int ind_in_tri = 0; ind_in_tri < 3; ++ind_in_tri)
            {
                if (can_walk_through(tri, ind_in_tri))
                {
                    to_visit.push({(int)tri_ind, ind_in_tri});
                }
            }
        }
    }

    return true;
}

bool SurfaceManager::isSurfaceAt(SurfaceType surface, const utils::Vector2f &pos)
{
    if (p_cdt)
    {
        auto tri_ind = p_cdt->findTriangle(pos);
        if (tri_ind != -1)
        {
            return surface == m_tri2surface.at(tri_ind).type;
        }
    }
    return false;
}

SurfaceType SurfaceManager::getSurfaceAt(const utils::Vector2f &pos)
{
    if (p_cdt)
    {
        auto tri_ind = p_cdt->findTriangle(pos);
        if (tri_ind != -1)
        {
            return m_tri2surface.at(tri_ind).type;
        }
    }
    return SurfaceType::Ground;
}

void SurfaceManager::changeSurface(cdt::Triangulation<cdt::Vector2i> &cdt, utils::Vector2f pos, SurfaceType new_surface)
{
    m_tri2surface.resize(cdt.m_triangles.size());

    auto tri_ind = cdt.findTriangle({pos.x, pos.y});
    auto connected_inds = findConnectedTriInds(cdt, tri_ind);

    auto region = m_tri2surface.at(tri_ind).region;
    auto type = m_tri2surface.at(tri_ind).type;
    for (auto &ind : connected_inds)
    {
        auto region_id = m_tri2region_id.at(ind);
        m_tri2surface.at(ind) = {new_surface, region_id};
        m_surface2regions[new_surface].push_back(region_id);
    }
}

std::vector<int> findConnectedTriInds(cdt::Triangulation<cdt::Vector2i> &cdt, int start_tri_ind)
{
    std::vector<int> connected_inds;
    auto &triangles = cdt.m_triangles;
    auto n_triangles = triangles.size();
    std::unordered_set<int> visited(n_triangles);
    std::queue<int> to_visit;
    to_visit.push(start_tri_ind);
    while (!to_visit.empty())
    {
        auto curr = to_visit.front();
        auto &curr_tri = triangles.at(curr);
        to_visit.pop();
        visited.insert(curr);
        connected_inds.push_back(curr);
        for (int i = 0; i < 3; ++i)
        {
            auto neighbour = curr_tri.neighbours[i];
            if (!visited.contains(neighbour) && !curr_tri.is_constrained.at(i))
            {
                to_visit.push(neighbour);
            }
        }
    }
    return connected_inds;
}

std::vector<Wall *> selectWalls(const utils::Vector2f &left_select, const utils::Vector2f &right_select)
{

    utils::Vector2f lower_left = {std::min(left_select.x, right_select.x), std::min(left_select.y, right_select.y)};
    utils::Vector2f upper_right = {std::max(left_select.x, right_select.x), std::max(left_select.y, right_select.y)};
    AABB rect = {lower_left, upper_right};

    auto &collider = GameWorld::getWorld().getCollider();
    auto walls_x = collider.findNearestObjects(ObjectType::Wall, rect);
    std::vector<Wall *> walls;
    for (auto &wall : walls_x)
    {
        walls.push_back(static_cast<Wall *>(wall)); //! scetchy stuff
    }
    return walls;
}


bool SurfaceTable::isWalkable(SurfaceType surface) const
{
    int surface_id = static_cast<int>(surface);
    return (m_surface_is_walkable >> surface_id) & 1u;
}

void SurfaceTable::toggleWalkable(SurfaceType surface)
{
    int surface_id = static_cast<int>(surface);
    m_surface_is_walkable = m_surface_is_walkable | (1u << surface_id);
}