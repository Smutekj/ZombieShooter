#pragma once

#include "Utils/Grid.h"

#include <Triangulation.h>
#include <Utils/Vector2.h>

#include <unordered_map>


using TriVertex = utils::Vector2i ;
using Edge = cdt::EdgeI<TriVertex>;

class MapGrid : public utils::Grid
{

public:
    enum class Tile
    {
        Ground,
        Wall,
    };

    enum class Direction
    {
        Up,
        Down,
        Left,
        Right
    };

public:
    MapGrid(utils::Vector2i box_size, utils::Vector2i n_cells);

    void changeTiles(Tile new_tile, utils::Vector2i lower_left, utils::Vector2i size);

    std::vector<Edge> extractEdges() const;

private:
    bool isAtBoundary(int ix, int iy) const;
    bool isWall(Direction dir, int ix, int iy) const;

private:
    utils::Vector2i m_size;
    std::vector<Tile> m_tiles;
};

class MapGridDiagonal : public utils::Grid
{

public:
    enum class Tile
    {
        Ground,
        Wall,
    };

    //! this is where the normal points
    enum class Direction
    {
        Up,
        Down,
        Left,
        Right,
        RightUp,
        RightDown,
        LeftUp,
        LeftDown,
    };

public:
    

    MapGridDiagonal(utils::Vector2i box_size, utils::Vector2i n_cells);

    void changeTiles(Tile new_tile, utils::Vector2i lower_left, utils::Vector2i size);

    std::vector<Edge> extractEdges() const;
    
    auto& getTiles()
    {
        return m_tiles;
    }
    void transformCorners();
    void extractBoundaries();


private:
    bool isAtBoundary(int ix, int iy) const;
    bool isWall(Direction dir, int ix, int iy) const;


    enum class BoundaryTile
    {
        Wall,
        Water,
    };

    struct BoundaryData
    {
        Direction dir;
        BoundaryTile type;
    };

    int deltaInd(Direction dir)
    {
        return delta_inds.at(dir);
    }

private:
    utils::Vector2i m_size;
    std::vector<Tile> m_tiles;
    std::unordered_map<int, BoundaryData> m_boundaries;

    std::unordered_map<Direction, int> delta_inds;
};
