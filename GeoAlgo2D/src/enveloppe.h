#pragma once
#include <vector>
#include "geometry.h"

// Les deux fonctions renvoient les indices (dans pts) des sommets de l'enveloppe
// convexe, parcourus dans le sens trigonométrique.

// Marche de Jarvis (cours "Marche de Jarvis" p.1-3) : O(n h)
std::vector<int> marcheJarvis(const std::vector<Vec2>& pts);

// Graham-Scan (cours p.17-18) : O(n log n)
std::vector<int> grahamScan(const std::vector<Vec2>& pts);
