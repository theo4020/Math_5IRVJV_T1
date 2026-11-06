#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <chrono>
#include <cmath>
#include <random>
#include <vector>

#include "geometry.h"
#include "enveloppe.h"
#include "triangulation.h"
#include "voronoi.h"

enum Mode { 
    JARVIS,
    GRAHAM,
    TRIANG_INC,
    DELAUNAY,
    VORONOI
};

struct App {
    std::vector<Vec3> points;
    int mode = JARVIS;
    bool aJour = false;

    std::vector<int> enveloppe;
    Triangulation tri;
    Voronoi voro;
    int nbFlips = 0;
    double tEnv = 0, tTri = 0, tFlip = 0, tVoro = 0;

    // comparaison Jarvis / Graham
    double bJarvis = -1, bGraham = -1;
    int bN = 0, bH = 0;

    // options
    bool cercles = false, voirDelaunay = true;
    int nbGen = 200, typeGen = 0;
};

template <class F>
static double mesurerMs(F f) {
    auto t0 = std::chrono::steady_clock::now();
    f();
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

// Temps moyen : répète le calcul jusqu'à ~300 ms cumulées (pour des mesures fiables)
template <class F>
static double mesurerMoyenMs(F f) {
    int r = 0;
    double total = 0;
    do { total += mesurerMs(f); ++r; } while (total < 300 && r < 1000);
    return total / r;
}

static void recalculer(App& A) {
    switch (A.mode) {
    case JARVIS: A.tEnv = mesurerMs([&] { A.enveloppe = marcheJarvis(A.points); }); break;
    case GRAHAM: A.tEnv = mesurerMs([&] { A.enveloppe = grahamScan(A.points); }); break;
    default:
        A.tTri = mesurerMs([&] { A.tri.incrementale(A.points); });
        if (A.mode >= DELAUNAY) A.tFlip = mesurerMs([&] { A.nbFlips = A.tri.flipping(); });
        if (A.mode == VORONOI) A.tVoro = mesurerMs([&] { A.voro = calculerVoronoi(A.tri, 5000); });
    }
    A.aJour = true;
}

static void generer(App& A, float W, float H) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> u(0, 1);

    // zone de dessin : à droite du menu
    double x0 = 400, x1 = W - 20, y0 = 20, y1 = H - 20;
    double cx = (x0 + x1) / 2, cy = (y0 + y1) / 2, R = std::min(x1 - x0, y1 - y0) / 2;
    A.points.clear();
    for (int i = 0; i < A.nbGen; ++i) {
        if (A.typeGen == 0) {
            A.points.push_back({x0 + u(rng) * (x1 - x0), y0 + u(rng) * (y1 - y0)});
        } else if (A.typeGen == 1) {
            double a = u(rng) * 2 * 3.14159265358979;
            A.points.push_back({cx + R * std::cos(a), cy + R * std::sin(a)});
        } else {
            double a = u(rng) * 2 * 3.14159265358979, r = R * std::sqrt(u(rng));
            A.points.push_back({cx + r * std::cos(a), cy + r * std::sin(a)});
        }
    }
    A.aJour = false;
}

// ---------------------------------------------------------------------------
// Menu et listes
// ---------------------------------------------------------------------------
static void menu(App& A, float W, float H) {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(370, H - 20), ImGuiCond_FirstUseEver);
    ImGui::Begin("Menu");
    ImGui::TextWrapped("Clic gauche : ajouter un point\nClic droit : supprimer un point");

    ImGui::SeparatorText("Traitement");
    const char* noms[] = {"Enveloppe convexe - Marche de Jarvis", "Enveloppe convexe - Graham-Scan",
                          "Triangulation incrementale", "Triangulation de Delaunay (flipping)",
                          "Diagramme de Voronoi"};
    for (int m = 0; m < 5; ++m)
        if (ImGui::RadioButton(noms[m], &A.mode, m)) A.aJour = false;

    ImGui::SeparatorText("Nuage de points");
    ImGui::Text("%d points", (int)A.points.size());
    ImGui::SliderInt("Nombre", &A.nbGen, 3, 20000, "%d", ImGuiSliderFlags_Logarithmic);
    ImGui::Combo("Type", &A.typeGen, "Aleatoire (rectangle)\0Sur un cercle\0Aleatoire (disque)\0");
    if (ImGui::Button("Generer")) generer(A, W, H);
    ImGui::SameLine();
    if (ImGui::Button("Effacer")) { A.points.clear(); A.aJour = false; }

    ImGui::SeparatorText("Resultats");
    if (A.mode == JARVIS || A.mode == GRAHAM) {
        ImGui::Text("Sommets de l'enveloppe : %d", (int)A.enveloppe.size());
        ImGui::Text("Temps de calcul : %.3f ms", A.tEnv);

        ImGui::SeparatorText("Comparaison Jarvis / Graham");
        if (ImGui::Button("Comparer sur ce nuage")) {
            A.bJarvis = mesurerMoyenMs([&] { marcheJarvis(A.points); });
            A.bGraham = mesurerMoyenMs([&] { grahamScan(A.points); });
            A.bN = (int)A.points.size();
            A.bH = (int)grahamScan(A.points).size();
        }
        if (A.bJarvis >= 0) {
            ImGui::Text("n = %d, h = %d sommets sur l'enveloppe", A.bN, A.bH);
            ImGui::Text("Jarvis : %.4f ms", A.bJarvis);
            ImGui::Text("Graham : %.4f ms", A.bGraham);
            ImGui::Text("-> plus rapide : %s", A.bJarvis < A.bGraham ? "Jarvis" : "Graham");
            ImGui::TextWrapped("Jarvis est en O(n h) et Graham en O(n log n) : Jarvis est interessant "
                               "quand peu de points sont sur l'enveloppe (h petit, ex. nuage dans un "
                               "rectangle), Graham quand beaucoup le sont (ex. points sur un cercle).");
        }
    } else {
        ImGui::Text("Sommets : %d  Aretes : %d  Triangles : %d", (int)A.tri.sommets.size(),
                    (int)A.tri.aretes.size(), (int)A.tri.triangles.size());
        ImGui::Text("Triangulation incrementale : %.3f ms", A.tTri);
        if (A.mode >= DELAUNAY) {
            ImGui::Text("Flipping : %d flips, %.3f ms", A.nbFlips, A.tFlip);
            ImGui::Checkbox("Cercles circonscrits", &A.cercles);
        }
        if (A.mode == VORONOI) {
            ImGui::Text("Voronoi (dualite) : %.3f ms", A.tVoro);
            ImGui::Checkbox("Afficher Delaunay", &A.voirDelaunay);
            ImGui::TextWrapped("Survoler un site met sa region en evidence.");
        }
    }

    ImGui::SeparatorText("Listes");
    const ImGuiTableFlags tf = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (A.mode == JARVIS || A.mode == GRAHAM) {
        if (ImGui::CollapsingHeader("Enveloppe (sens trigonometrique)") &&
            ImGui::BeginTable("env", 2, tf, ImVec2(0, 200))) {
            ImGui::TableSetupColumn("Point"); ImGui::TableSetupColumn("Coordonnees");
            ImGui::TableHeadersRow();
            for (int i : A.enveloppe) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%d", i);
                ImGui::TableNextColumn(); ImGui::Text("(%.0f, %.0f)", A.points[i].x, A.points[i].y);
            }
            ImGui::EndTable();
        }
    } else {
        const Triangulation& T = A.tri;
        if (ImGui::CollapsingHeader("Sommets") && ImGui::BeginTable("som", 3, tf, ImVec2(0, 180))) {
            ImGui::TableSetupColumn("Sommet"); ImGui::TableSetupColumn("Coordonnees"); ImGui::TableSetupColumn("a");
            ImGui::TableHeadersRow();
            ImGuiListClipper c; c.Begin((int)T.sommets.size());
            while (c.Step()) for (int i = c.DisplayStart; i < c.DisplayEnd; ++i) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%d", i);
                ImGui::TableNextColumn(); ImGui::Text("(%.0f, %.0f)", T.sommets[i].p.x, T.sommets[i].p.y);
                ImGui::TableNextColumn(); ImGui::Text("%d", T.sommets[i].a);
            }
            ImGui::EndTable();
        }
        if (ImGui::CollapsingHeader("Aretes") && ImGui::BeginTable("ar", 5, tf, ImVec2(0, 180))) {
            for (const char* h : {"Arete", "s1", "s2", "tg", "td"}) ImGui::TableSetupColumn(h);
            ImGui::TableHeadersRow();
            ImGuiListClipper c; c.Begin((int)T.aretes.size());
            while (c.Step()) for (int i = c.DisplayStart; i < c.DisplayEnd; ++i) {
                const Arete& e = T.aretes[i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%d", i);
                ImGui::TableNextColumn(); ImGui::Text("%d", e.s1);
                ImGui::TableNextColumn(); ImGui::Text("%d", e.s2);
                ImGui::TableNextColumn(); if (e.tg < 0) ImGui::Text("-"); else ImGui::Text("%d", e.tg);
                ImGui::TableNextColumn(); if (e.td < 0) ImGui::Text("-"); else ImGui::Text("%d", e.td);
            }
            ImGui::EndTable();
        }
        if (ImGui::CollapsingHeader("Triangles") && ImGui::BeginTable("tr", 4, tf, ImVec2(0, 180))) {
            for (const char* h : {"Triangle", "a1", "a2", "a3"}) ImGui::TableSetupColumn(h);
            ImGui::TableHeadersRow();
            ImGuiListClipper c; c.Begin((int)T.triangles.size());
            while (c.Step()) for (int i = c.DisplayStart; i < c.DisplayEnd; ++i) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%d", i);
                ImGui::TableNextColumn(); ImGui::Text("%d", T.triangles[i].a1);
                ImGui::TableNextColumn(); ImGui::Text("%d", T.triangles[i].a2);
                ImGui::TableNextColumn(); ImGui::Text("%d", T.triangles[i].a3);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

// ---------------------------------------------------------------------------
// Dessin (ImGui background draw list ; repère écran = y vers le bas)
// ---------------------------------------------------------------------------
static void dessiner(const App& A, float H, Vec3 souris) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    auto ecran = [H](Vec3 p) { return ImVec2((float)p.x, H - (float)p.y); };
    const ImU32 cPoint = IM_COL32(240, 240, 240, 255), cEnv = IM_COL32(255, 170, 0, 255);
    const ImU32 cTri = IM_COL32(80, 170, 255, 255), cVoro = IM_COL32(255, 90, 120, 255);
    const ImU32 cCercle = IM_COL32(120, 255, 120, 90);
    const Triangulation& T = A.tri;

    if (A.mode == JARVIS || A.mode == GRAHAM) {
        int m = (int)A.enveloppe.size();
        for (int i = 0; i < m; ++i)
            dl->AddLine(ecran(A.points[A.enveloppe[i]]), ecran(A.points[A.enveloppe[(i + 1) % m]]), cEnv, 2.0f);
        for (int i : A.enveloppe) dl->AddCircle(ecran(A.points[i]), 6, cEnv, 0, 2.0f);
    } else {
        bool voirTri = A.mode != VORONOI || A.voirDelaunay;
        if (voirTri) {
            ImU32 c = A.mode == VORONOI ? IM_COL32(80, 170, 255, 110) : cTri;
            for (const Arete& e : T.aretes) dl->AddLine(ecran(T.sommets[e.s1].p), ecran(T.sommets[e.s2].p), c, 1.5f);
        }
        if (A.mode >= DELAUNAY && A.cercles) {
            for (int t = 0; t < (int)T.triangles.size(); ++t) {
                auto s = T.sommetsTriangle(t);
                Vec3 C;
                if (centreCercle(T.sommets[s[0]].p, T.sommets[s[1]].p, T.sommets[s[2]].p, C))
                    dl->AddCircle(ecran(C), (float)norme(T.sommets[s[0]].p - C), cCercle, 64);
            }
        }
        if (A.mode == VORONOI) {
            for (const SegmentVoronoi& s : A.voro.aretes) dl->AddLine(ecran(s.a), ecran(s.b), cVoro, 2.0f);
            // région du site survolé
            int best = -1; double dmin = 1e18;
            for (int i = 0; i < (int)T.sommets.size(); ++i) {
                double d = norme(T.sommets[i].p - souris);
                if (d < dmin) { dmin = d; best = i; }
            }
            if (best >= 0 && dmin < 15) {
                for (int e : A.voro.regions[best])
                    dl->AddLine(ecran(A.voro.aretes[e].a), ecran(A.voro.aretes[e].b), IM_COL32(255, 255, 0, 255), 4.0f);
                dl->AddCircle(ecran(T.sommets[best].p), 8, IM_COL32(255, 255, 0, 255), 0, 2.0f);
            }
        }
    }
    for (const Vec3& p : A.points) dl->AddCircleFilled(ecran(p), 3.5f, cPoint);
}

int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1400, 900, "Geometrie algorithmique 2D - etape intermediaire", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    App A;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();
        float W = io.DisplaySize.x, H = io.DisplaySize.y;
        Vec3 souris = {io.MousePos.x, H - io.MousePos.y};  // repère mathématique

        // Saisie à la souris (hors du menu)
        if (!io.WantCaptureMouse) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                bool existe = false;
                for (const Vec3& p : A.points) if (norme(p - souris) < 2) existe = true;
                if (!existe) { A.points.push_back(souris); A.aJour = false; }
            }
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                int best = -1; double dmin = 10;
                for (int i = 0; i < (int)A.points.size(); ++i) {
                    double d = norme(A.points[i] - souris);
                    if (d < dmin) { dmin = d; best = i; }
                }
                if (best >= 0) { A.points.erase(A.points.begin() + best); A.aJour = false; }
            }
        }

        if (!A.aJour) recalculer(A);
        menu(A, W, H);
        dessiner(A, H, souris);

        ImGui::Render();
        int fw, fh;
        glfwGetFramebufferSize(window, &fw, &fh);
        glViewport(0, 0, fw, fh);
        glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
