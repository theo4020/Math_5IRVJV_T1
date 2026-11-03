#include "enveloppe.h"
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// Marche de Jarvis : traduction directe du pseudo-code du cours (p.3)
// ---------------------------------------------------------------------------
std::vector<int> marcheJarvis(const std::vector<Vec2>& P) {
    const int n = (int)P.size();
    std::vector<int> env;
    if (n == 0) return env;
    if (n == 1) return {0};

    // recherche du premier sommet : x minimal (puis y minimal)
    int i0 = 0;
    for (int i = 1; i < n; ++i)
        if (P[i].x < P[i0].x || (P[i].x == P[i0].x && P[i].y < P[i0].y)) i0 = i;

    Vec2 v = {0, -1};  // droite d'appui initiale : direction (0,-1)
    int i = i0;
    do {
        env.push_back(i);  // ajout du point pivot

        // initialisation de amin et lmax avec le premier point d'indice différent de i
        int j = (i == 0) ? 1 : 0;
        double amin = angle(v, P[j] - P[i]);
        double lmax = norme(P[j] - P[i]);
        int inew = j;
        // recherche du point le plus proche (en angle) de la droite
        for (j = inew + 1; j < n; ++j) {
            if (j == i) continue;
            double a = angle(v, P[j] - P[i]);
            double l = norme(P[j] - P[i]);
            if (amin > a + EPS || (std::fabs(amin - a) <= EPS && lmax < l)) {
                amin = a; lmax = l; inew = j;
            }
        }
        // mise à jour du pivot et du vecteur directeur
        v = P[inew] - P[i];
        i = inew;
    } while (i != i0 && (int)env.size() <= n);

    return env;
}

// ---------------------------------------------------------------------------
// Graham-Scan (p.17-18)
//  1) origine = barycentre B
//  2) tri suivant l'angle orienté (Ox, BPj), puis la distance à B
//  3) suppression des sommets non convexes dans une liste chaînée circulaire
// ---------------------------------------------------------------------------
std::vector<int> grahamScan(const std::vector<Vec2>& P) {
    const int n = (int)P.size();
    std::vector<int> ordre(n);
    for (int i = 0; i < n; ++i) ordre[i] = i;
    if (n < 3) return ordre;

    // 1) barycentre
    Vec2 B;
    for (const Vec2& p : P) B = B + p;
    B = (1.0 / n) * B;

    // 2) tri angulaire autour de B
    std::vector<double> ang(n), dist(n);
    for (int i = 0; i < n; ++i) {
        ang[i] = std::atan2(P[i].y - B.y, P[i].x - B.x);
        dist[i] = norme(P[i] - B);
    }
    std::sort(ordre.begin(), ordre.end(), [&](int a, int b) {
        if (std::fabs(ang[a] - ang[b]) > 1e-12) return ang[a] < ang[b];
        return dist[a] < dist[b];
    });

    // 3) liste chaînée circulaire L (S.prec / S.suiv) sur les positions 0..n-1
    std::vector<int> prec(n), suiv(n);
    for (int k = 0; k < n; ++k) { prec[k] = (k + n - 1) % n; suiv[k] = (k + 1) % n; }
    int taille = n;
    auto pt = [&](int k) { return P[ordre[k]]; };

    int Sinit = 0;
    int pivot = Sinit;
    bool avance = false;
    do {
        if (taille < 3) break;
        int p = prec[pivot], s = suiv[pivot];
        // pivot convexe <=> angle orienté (Pj Pj-1, Pj Pj+1) > pi <=> virage à gauche strict
        if (orientation(pt(p), pt(pivot), pt(s)) > EPS) {
            pivot = s;
            avance = true;
        } else {
            Sinit = p;
            suiv[p] = s; prec[s] = p; --taille;  // L.supprimer(pivot)
            pivot = Sinit;
            avance = false;
        }
    } while (pivot != Sinit || !avance);

    std::vector<int> env;
    int k = Sinit;
    for (int c = 0; c < taille; ++c) { env.push_back(ordre[k]); k = suiv[k]; }
    return env;
}
