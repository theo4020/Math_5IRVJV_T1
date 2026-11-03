#include "voronoi.h"

Voronoi calculerVoronoi(const Triangulation& T, double L) {
    Voronoi V;
    V.regions.assign(T.sommets.size(), {});
    V.aretes.resize(T.aretes.size());

    if (T.triangles.empty()) {
        // Triangulation dégénérée (tous les points alignés) :
        // n-1 droites parallèles, chacune médiatrice d'une arête [Pi Pj]
        for (int e = 0; e < (int)T.aretes.size(); ++e) {
            Vec2 p1 = T.sommets[T.aretes[e].s1].p, p2 = T.sommets[T.aretes[e].s2].p;
            Vec2 m = 0.5 * (p1 + p2);
            Vec2 n = normaleExterieure(p1, p2);
            V.aretes[e] = {m - L * n, m + L * n};
        }
    } else {
        // pour chaque triangle T : centre C_T du cercle circonscrit
        V.centres.resize(T.triangles.size());
        for (int t = 0; t < (int)T.triangles.size(); ++t) {
            auto s = T.sommetsTriangle(t);
            centreCercle(T.sommets[s[0]].p, T.sommets[s[1]].p, T.sommets[s[2]].p, V.centres[t]);
        }
        // pour chaque arête A : arête duale A*
        for (int e = 0; e < (int)T.aretes.size(); ++e) {
            const Arete& A = T.aretes[e];
            if (A.tg >= 0 && A.td >= 0) {
                // arête interne : segment entre les centres des deux triangles incidents
                V.aretes[e] = {V.centres[A.tg], V.centres[A.td]};
            } else {
                // arête externe : demi-droite issue du centre du triangle incident,
                // perpendiculaire à A (passant par son milieu), dirigée vers l'extérieur
                int t = A.tg >= 0 ? A.tg : A.td;
                Vec2 u = T.sommets[A.tg >= 0 ? A.s1 : A.s2].p;  // A orientée intérieur à gauche
                Vec2 v = T.sommets[A.tg >= 0 ? A.s2 : A.s1].p;
                Vec2 C = V.centres[t];
                V.aretes[e] = {C, C + L * normaleExterieure(u, v)};
            }
        }
    }
    // pour chaque sommet S : région R = liste des A* duales des arêtes incidentes à S
    for (int e = 0; e < (int)T.aretes.size(); ++e) {
        V.regions[T.aretes[e].s1].push_back(e);
        V.regions[T.aretes[e].s2].push_back(e);
    }
    return V;
}
