#pragma once
// Diagramme de Voronoï 2D par dualité avec la triangulation de Delaunay (cours p.40-41)
#include <vector>
#include "triangulation.h"

struct SegmentVoronoi { Vec2 a, b; };

struct Voronoi {
    std::vector<Vec2> centres;               // C_T : centre du cercle circonscrit à chaque triangle T
    std::vector<SegmentVoronoi> aretes;      // A* : arête duale de chaque arête A de la triangulation
    std::vector<std::vector<int>> regions;   // R : pour chaque sommet S, les A* duales des arêtes incidentes à S
};

// L : longueur utilisée pour tracer les arêtes infinies (demi-droites / droites)
Voronoi calculerVoronoi(const Triangulation& T, double L);
