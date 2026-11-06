#pragma once
// Fonctions mathématiques de base
// ("Requêtes géométriques dans R^d", "Géométrie des triangles", "puissance d'un point")
#include <cmath>
#include <algorithm>

struct Vec3 {
    double x = 0, y = 0, z = 0;
};

inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z + b.z}; }
inline Vec3 operator*(double k, Vec3 a) { return { k * a.x, k * a.y, k * a.z }; }

const double EPS = 1e-9;

inline double produitScalaire(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline double norme(Vec3 a) { return std::sqrt(produitScalaire(a, a)); }

// Produit vectoriel 2D = déterminant 2x2 |u v|
inline double det2(Vec3 u, Vec3 v) { return u.x * v.y - u.y * v.x; }

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

// Angle NON orienté entre deux vecteurs non nuls : arccos(v.w / |v||w|) dans [0, pi]
inline double angle(Vec3 v, Vec3 w) {
    double c = produitScalaire(v, w) / (norme(v) * norme(w));
    return std::acos(std::clamp(c, -1.0, 1.0));
}

// Orientation de (a,b,c) = 2 x aire algébrique du triangle
// > 0 : sens trigonométrique,
// < 0 : sens horaire,
// = 0 : alignés
inline double orientation(Vec3 a, Vec3 b, Vec3 c) { return det2(b - a, c - a); }

// Visibilité : l'arête orientée [a,b] (intérieur à gauche) est "vue" depuis p
// ssi p est strictement dans le demi-espace extérieur E-
inline bool estVue(Vec3 a, Vec3 b, Vec3 p) { return orientation(a, b, p) < -EPS; }

// Normale extérieure unitaire de l'arête orientée [a,b] dont l'intérieur est à gauche
inline Vec3 normaleExterieure(Vec3 a, Vec3 b) {
    Vec3 d = b - a;
    double l = norme(d);
    return {d.y / l, -d.x / l};
}

// Test d'intériorité d'un point dans un triangle (a,b,c) orienté trigo
inline bool dansTriangle(Vec3 a, Vec3 b, Vec3 c, Vec3 p) {
    return orientation(a, b, p) >= 0 && orientation(b, c, p) >= 0 && orientation(c, a, p) >= 0;
}

// Centre du cercle circonscrit : C équidistant des sommets
// résolu par inversion de matrice
inline bool centreCercle(Vec3 p1, Vec3 p2, Vec3 p3, Vec3& c) {
    double m[2][2] = {{p2.x - p1.x, p2.y - p1.y},
                      {p3.x - p1.x, p3.y - p1.y}};
    double b0 = (p2.x * p2.x + p2.y * p2.y - p1.x * p1.x - p1.y * p1.y) / 2;
    double b1 = (p3.x * p3.x + p3.y * p3.y - p1.x * p1.x - p1.y * p1.y) / 2;
    double inv[2][2];
    if (!inverser2x2(m, inv)) return false;
    c = {inv[0][0] * b0 + inv[0][1] * b1, inv[1][0] * b0 + inv[1][1] * b1};
    return true;
}

// Critère de Delaunay : d est strictement à l'intérieur du cercle circonscrit
// à (a,b,c) <=> puissance de d par rapport au cercle : ||dC||^2 - r^2 < 0
inline bool dansCercleCirconscrit(Vec3 a, Vec3 b, Vec3 c, Vec3 d) {
    Vec3 C;
    if (!centreCercle(a, b, c, C)) return false;
    Vec3 ac = a - C, dc = d - C;
    double r2 = produitScalaire(ac, ac);
    double puissance = produitScalaire(dc, dc) - r2;
    return puissance < -1e-9 * r2;  // tolérance relative (points cocirculaires)
}
