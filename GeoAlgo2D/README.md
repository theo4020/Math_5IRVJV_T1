# Géométrie algorithmique – Étape intermédiaire (partie 2D)

C++17, OpenGL + GLFW + Dear ImGui (téléchargés automatiquement par CMake).

## Compiler et lancer (Windows, Visual Studio 2022)
1. Visual Studio > **Fichier > Ouvrir > Dossier…** > choisir le dossier `GeoAlgo2D`.
   Visual Studio détecte le `CMakeLists.txt` et télécharge GLFW et ImGui (internet requis la première fois).
2. Choisir la cible **GeoAlgo2D.exe** dans la liste de démarrage, puis **F5**.

(En ligne de commande : `cmake -S . -B build` puis `cmake --build build --config Release`.)

## Utilisation
- Clic gauche : ajouter un point · Clic droit : supprimer le point le plus proche
- Menu : choix du traitement, génération d'un nuage (rectangle, cercle, disque), listes des
  sommets / arêtes / triangles mises à jour en temps réel.
- Tout est recalculé à chaque ajout ou suppression de point.

## Contenu (algorithmes du cours)
| Point demandé | Fichier | Cours |
|---|---|---|
| Fonctions mathématiques (déterminant, inversion 2x2, produit vectoriel, angle, orientation, visibilité, normale extérieure, test d'intériorité, centre du cercle circonscrit, critère de Delaunay par la puissance d'un point) | `src/geometry.h` | p.5, 12-13, 28-29 |
| Enveloppe convexe : marche de Jarvis | `src/enveloppe.cpp` | Marche de Jarvis p.1-3 |
| Enveloppe convexe : Graham-Scan (barycentre, tri angulaire, liste chaînée circulaire) | `src/enveloppe.cpp` | p.17-18 |
| Comparaison des temps Jarvis / Graham | bouton « Comparer sur ce nuage » | |
| Structure : sommet (coord., a), arête (s1, s2, tg, td), triangle (a1, a2, a3) orienté trigo | `src/triangulation.h` | p.16 |
| Triangulation incrémentale | `src/triangulation.cpp` | p.26-27 |
| Flipping d'arêtes → Delaunay | `src/triangulation.cpp` | p.30 |
| Diagramme de Voronoï par dualité | `src/voronoi.cpp` | p.40-41 |

## Comparaison Jarvis / Graham
Jarvis est en O(n·h) (h = nombre de sommets de l'enveloppe), Graham en O(n log n).
Exemple mesuré (n = 10 000) : nuage dans un rectangle → Jarvis 5,6 ms / Graham 1,3 ms ;
points sur un cercle (h = n) → Jarvis 2 500 ms / Graham 1,1 ms.
Jarvis n'est compétitif que lorsque très peu de points sont sur l'enveloppe ; Graham est
le plus sûr dans le cas général.
