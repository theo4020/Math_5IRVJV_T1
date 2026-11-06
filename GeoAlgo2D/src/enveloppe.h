#pragma once
#include <vector>
#include "geometry.h"

// Les deux fonctions renvoient les indices (dans pts) des sommets de l'enveloppe
// convexe, parcourus dans le sens trigonométrique.

// Marche de Jarvis : O(n h)
std::vector<int> marcheJarvis(const std::vector<Vec3>& pts);

// Graham-Scan : O(n log n)
std::vector<int> grahamScan(const std::vector<Vec3>& pts);
