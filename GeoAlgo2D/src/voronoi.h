#pragma once
// Diagramme de Voronoï 2D par dualité avec la triangulation de Delaunay
#include <vector>
#include "triangulation.h"

struct SegmentVoronoi { Vec3 a, b, c; };

struct Voronoi {
    std::vector<Vec3> centres;
    std::vector<SegmentVoronoi> aretes;
    std::vector<std::vector<int>> regions;
};

// L : longueur utilisée pour tracer les arêtes infinies (demi-droites / droites)
Voronoi calculerVoronoi(const Triangulation& T, double L);
