#pragma once
// Structure de données pour une 2-triangulation orientée (cours p.16) :
//  - un sommet  : ses coordonnées et une de ses arêtes incidentes a
//  - une arête  : sommet initial s1, sommet final s2, triangle gauche tg, triangle droit td
//                 (-1 = vide pour une arête externe)
//  - un triangle: ses 3 arêtes a1, a2, a3 parcourues dans le sens trigonométrique
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include "geometry.h"

struct Sommet   { Vec2 p; int a = -1; };
struct Arete    { int s1, s2; int tg = -1, td = -1; };
struct Triangle { int a1, a2, a3; };

class Triangulation {
public:
    std::vector<Sommet>   sommets;
    std::vector<Arete>    aretes;
    std::vector<Triangle> triangles;

    void vider();

    // Triangulation 2-D incrémentale (cours p.26-27)
    void incrementale(const std::vector<Vec2>& points);

    // Flipping d'arêtes -> triangulation de Delaunay (cours p.30). Renvoie le nombre de flips.
    int flipping();

    // Sommet de départ de l'arête a quand on la parcourt dans le triangle t (sens trigo)
    int origine(int a, int t) const { return aretes[a].tg == t ? aretes[a].s1 : aretes[a].s2; }
    // Les 3 sommets du triangle t dans le sens trigonométrique
    std::array<int, 3> sommetsTriangle(int t) const {
        const Triangle& T = triangles[t];
        return {origine(T.a1, t), origine(T.a2, t), origine(T.a3, t)};
    }

private:
    // table de hachage (min,max) -> indice d'arête, pour retrouver une arête en O(1)
    std::unordered_map<uint64_t, int> index;
    static uint64_t cle(int u, int v) {
        if (u > v) std::swap(u, v);
        return ((uint64_t)(uint32_t)u << 32) | (uint32_t)v;
    }
    int trouverArete(int u, int v) const {
        auto it = index.find(cle(u, v));
        return it == index.end() ? -1 : it->second;
    }
    int lierArete(int u, int v, int t);           // arête u->v parcourue dans t (t=-1 : aucun)
    void ajouterTriangle(int u, int v, int w);    // (u,v,w) orienté trigo
    void ajouterPoint(int q);                     // étape q de l'algorithme incrémental
};
