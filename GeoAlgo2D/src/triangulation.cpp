#include "triangulation.h"
#include <algorithm>
#include <cmath>
#include <utility>

void Triangulation::vider() {
    sommets.clear(); aretes.clear(); triangles.clear(); index.clear();
}

// Crée l'arête [u,v] si elle n'existe pas, et la rattache au triangle t
// (t est à GAUCHE de l'arête si on la parcourt de u vers v).
int Triangulation::lierArete(int u, int v, int t) {
    int e = trouverArete(u, v);
    if (e < 0) {
        e = (int)aretes.size();
        aretes.push_back({u, v, t, -1});
        index[cle(u, v)] = e;
        if (sommets[u].a < 0) sommets[u].a = e;
        if (sommets[v].a < 0) sommets[v].a = e;
    } else if (t >= 0) {
        if (aretes[e].s1 == u) aretes[e].tg = t;  // parcourue s1 -> s2 : t à gauche
        else                   aretes[e].td = t;  // parcourue s2 -> s1 : t à droite
    }
    return e;
}

void Triangulation::ajouterTriangle(int u, int v, int w) {
    int t = (int)triangles.size();
    int a1 = lierArete(u, v, t);
    int a2 = lierArete(v, w, t);
    int a3 = lierArete(w, u, t);
    triangles.push_back({a1, a2, a3});
}

// ---------------------------------------------------------------------------
// Triangulation incrémentale (p.26-27)
// ---------------------------------------------------------------------------
void Triangulation::incrementale(const std::vector<Vec2>& points) {
    vider();

    // 1) tri par abscisse croissante (puis ordonnée croissante)
    std::vector<Vec2> P = points;
    std::sort(P.begin(), P.end(), [](Vec2 a, Vec2 b) { return a.x < b.x || (a.x == b.x && a.y < b.y); });
    P.erase(std::unique(P.begin(), P.end(), [](Vec2 a, Vec2 b) { return a.x == b.x && a.y == b.y; }), P.end());
    const int n = (int)P.size();
    for (const Vec2& p : P) sommets.push_back({p, -1});
    if (n < 2) return;

    // 2a) suite de k-1 arêtes colinéaires [P1P2] ... [Pk-1 Pk] avec les k (>= 2) points alignés
    int k = 2;
    while (k < n && std::fabs(orientation(P[0], P[1], P[k])) <= EPS) ++k;
    for (int i = 0; i + 1 < k; ++i) lierArete(i, i + 1, -1);

    // 2b) et 3) ajout des points suivants un par un
    for (int q = k; q < n; ++q) ajouterPoint(q);
}

// Ajoute le point P(q), situé hors du polygone Pq (grâce au tri préalable)
void Triangulation::ajouterPoint(int q) {
    const Vec2 P = sommets[q].p;

    // 3a) recherche des arêtes externes "vues" depuis P.
    // On mémorise chaque arête vue orientée (u,v) de sorte que (u,v,P) soit dans le sens trigo.
    std::vector<std::pair<int, int>> vues;
    for (const Arete& A : aretes) {
        Vec2 p1 = sommets[A.s1].p, p2 = sommets[A.s2].p;
        if (A.tg < 0 && A.td < 0) {
            // cas 2b : arête de la suite colinéaire initiale (aucun triangle)
            if (orientation(p1, p2, P) > 0) vues.push_back({A.s1, A.s2});
            else                            vues.push_back({A.s2, A.s1});
        } else if (A.td < 0) {
            // intérieur à gauche de s1->s2 : vue si P est à droite de s1->s2
            if (estVue(p1, p2, P)) vues.push_back({A.s2, A.s1});
        } else if (A.tg < 0) {
            // intérieur à gauche de s2->s1 : vue si P est à droite de s2->s1
            if (estVue(p2, p1, P)) vues.push_back({A.s1, A.s2});
        }
    }

    // 3b) pour toute arête vue A, ajouter le triangle [A P] ;
    //     les arêtes [S P] et le sommet P sont créés au passage (lierArete)
    for (auto& uv : vues) ajouterTriangle(uv.first, uv.second, q);
}

// ---------------------------------------------------------------------------
// Flipping d'arêtes (p.30)
//
//            c                       c
//          /   \                   / | \
//    ca  / T1    \ bc        ca  /   |   \ bc
//       a ------- b    ==>      a  T1|T2  b
//    ad  \ T2    / db        ad  \   |   / db
//          \   /                   \ | /
//            d                       d
//  arête e = [a,b], T1 = (a,b,c) à gauche, T2 = (b,a,d) à droite
//  après flip : e = [d,c], T1 = (a,d,c), T2 = (d,b,c)
// ---------------------------------------------------------------------------
int Triangulation::flipping() {
    // Ac <- A
    std::vector<int> Ac(aretes.size());
    std::vector<char> dansAc(aretes.size(), 1);
    for (int i = 0; i < (int)aretes.size(); ++i) Ac[i] = i;

    auto opposeA = [&](int t, int u, int v) {
        for (int s : sommetsTriangle(t)) if (s != u && s != v) return s;
        return -1;
    };
    auto remplacerTriangle = [&](int e, int ancien, int nouveau) {
        if (aretes[e].tg == ancien) aretes[e].tg = nouveau;
        else                        aretes[e].td = nouveau;
    };

    int nbFlips = 0;
    while (!Ac.empty()) {
        // retirer l'arête A de Ac
        int e = Ac.back(); Ac.pop_back(); dansAc[e] = 0;
        Arete A = aretes[e];
        if (A.tg < 0 || A.td < 0) continue;  // arête externe : toujours Delaunay

        int T1 = A.tg, T2 = A.td, a = A.s1, b = A.s2;
        int c = opposeA(T1, a, b);
        int d = opposeA(T2, a, b);

        // critère de Delaunay : d ne doit pas être dans le cercle circonscrit à T1
        if (!dansCercleCirconscrit(sommets[a].p, sommets[b].p, sommets[c].p, sommets[d].p)) continue;

        int ad = trouverArete(a, d), db = trouverArete(d, b);
        int bc = trouverArete(b, c), ca = trouverArete(c, a);

        // A <- [S3,S4] (ici [d,c])
        index.erase(cle(a, b));
        aretes[e] = {d, c, T1, T2};
        index[cle(d, c)] = e;

        // T1 <- [ad, A, ca], T2 <- [db, bc, A]
        triangles[T1] = {ad, e, ca};
        triangles[T2] = {db, bc, e};

        // mise à jour des autres liens
        remplacerTriangle(ad, T2, T1);
        remplacerTriangle(bc, T1, T2);
        if (sommets[a].a == e) sommets[a].a = ad;
        if (sommets[b].a == e) sommets[b].a = bc;

        // ajouter A1, A2, A3, A4 à Ac
        for (int x : {ad, db, bc, ca})
            if (!dansAc[x]) { dansAc[x] = 1; Ac.push_back(x); }
        ++nbFlips;
    }
    return nbFlips;
}
