#pragma once
// Fonctions mathématiques de base
// (cours : "Requêtes géométriques dans R^d" p.5, "Géométrie des triangles" p.12-13,
//  "puissance d'un point" p.28-29)
#include <cmath>
#include <algorithm>

struct Vec2 {
    double x = 0, y = 0;
};

inline Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(double k, Vec2 a) { return {k * a.x, k * a.y}; }

const double EPS = 1e-9;

inline double produitScalaire(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
inline double norme(Vec2 a) { return std::sqrt(produitScalaire(a, a)); }

// Produit vectoriel 2D = déterminant 2x2 |u v|
inline double det2(Vec2 u, Vec2 v) { return u.x * v.y - u.y * v.x; }

// Déterminant 3x3
inline double det3(const double m[3][3]) {
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

// Inversion d'une matrice 2x2 (false si non inversible)
inline bool inverser2x2(const double m[2][2], double inv[2][2]) {
    double d = m[0][0] * m[1][1] - m[0][1] * m[1][0];
    if (std::fabs(d) < EPS) return false;
    inv[0][0] =  m[1][1] / d;  inv[0][1] = -m[0][1] / d;
    inv[1][0] = -m[1][0] / d;  inv[1][1] =  m[0][0] / d;
    return true;
}

// Angle NON orienté entre deux vecteurs non nuls (p.5) : arccos(v.w / |v||w|) dans [0, pi]
inline double angle(Vec2 v, Vec2 w) {
    double c = produitScalaire(v, w) / (norme(v) * norme(w));
    return std::acos(std::clamp(c, -1.0, 1.0));
}

// Orientation de (a,b,c) = 2 x aire algébrique du triangle (p.12)
// > 0 : sens trigonométrique, < 0 : sens horaire, = 0 : alignés
inline double orientation(Vec2 a, Vec2 b, Vec2 c) { return det2(b - a, c - a); }

// Visibilité : l'arête orientée [a,b] (intérieur à gauche) est "vue" depuis p
// ssi p est strictement dans le demi-espace extérieur E- (p.27)
inline bool estVue(Vec2 a, Vec2 b, Vec2 p) { return orientation(a, b, p) < -EPS; }

// Normale extérieure unitaire de l'arête orientée [a,b] dont l'intérieur est à gauche
inline Vec2 normaleExterieure(Vec2 a, Vec2 b) {
    Vec2 d = b - a;
    double l = norme(d);
    return {d.y / l, -d.x / l};
}

// Test d'intériorité d'un point dans un triangle (a,b,c) orienté trigo
inline bool dansTriangle(Vec2 a, Vec2 b, Vec2 c, Vec2 p) {
    return orientation(a, b, p) >= 0 && orientation(b, c, p) >= 0 && orientation(c, a, p) >= 0;
}

// Centre du cercle circonscrit (p.13) : C équidistant des sommets
// -> système linéaire 2x2 résolu par inversion de matrice
inline bool centreCercle(Vec2 p1, Vec2 p2, Vec2 p3, Vec2& c) {
    double m[2][2] = {{p2.x - p1.x, p2.y - p1.y},
                      {p3.x - p1.x, p3.y - p1.y}};
    double b0 = (p2.x * p2.x + p2.y * p2.y - p1.x * p1.x - p1.y * p1.y) / 2;
    double b1 = (p3.x * p3.x + p3.y * p3.y - p1.x * p1.x - p1.y * p1.y) / 2;
    double inv[2][2];
    if (!inverser2x2(m, inv)) return false;
    c = {inv[0][0] * b0 + inv[0][1] * b1, inv[1][0] * b0 + inv[1][1] * b1};
    return true;
}

// Critère de Delaunay (p.28-29) : d est strictement à l'intérieur du cercle circonscrit
// à (a,b,c) <=> puissance de d par rapport au cercle : ||dC||^2 - r^2 < 0
inline bool dansCercleCirconscrit(Vec2 a, Vec2 b, Vec2 c, Vec2 d) {
    Vec2 C;
    if (!centreCercle(a, b, c, C)) return false;
    Vec2 ac = a - C, dc = d - C;
    double r2 = produitScalaire(ac, ac);
    double puissance = produitScalaire(dc, dc) - r2;
    return puissance < -1e-9 * r2;  // tolérance relative (points cocirculaires)
}
