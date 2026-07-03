#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <conio.h>
#include <direct.h>

#define ERRORC() errorc(__FILE__, __LINE__) 
#define QUICK_GROESSE  6
#define INV_GROESSE    50
#define MAX_MONSTER    30
#define MAX_MONSTER_DB 100
#define MAX_ITEM_DB    100
#define MAX_SLOT       4
#define BLTUNG_Ladung  10

#define EFF_VIG(p) l_eff_stat(p, 0)
#define EFF_END(p) l_eff_stat(p, 1)
#define EFF_DEX(p) l_eff_stat(p, 2)
#define EFF_STR(p) l_eff_stat(p, 3)

#define SAVE_PFAD "Game\\save\\savegame.dat"
#define SAVE_MAGIC 0x4E616D65  // Name
#define SAVE_VERSION 1
// ---- System ---------------------------------------------------
void Cursor_aus();
void cursor_an();
void cls();
void credit();
void spielanleitung();
void errorc(const char* datei, int zeile);
int  menu_c(int wahl, int size, const char* menu[], const char* menu_name[]);
void l_AF(const char* ziel_name);
void l_PrCe(const char* text);
void l_tt(const char* farbe, const char* text, int delay_ms);

int l_launch_game();
int l_create_folder(const char* name);
int l_folder_exists(const char* name);
int l_create_file(const char* file_path);
int l_read_file(const char* path, char* buffer, size_t size);
int l_file_exists(const char* path);
int l_write_data(const char* file_path, int ID);
// ---- Settings -------------------------------------------------
int l_tutorial = 0;
int l_beta = 0;
int l_debug = 0;
int l_esmg = 1;
int l_schwierigkeit = 2;
int l_mauer_design = 35;

int l_hohe = 27; // y
int l_breite;   // x
// ---- struct ---------------------------------------------------
typedef enum { KF, KR, AL, AR, HL, HR, BL, BR, FL, FR, ANZAHL_ZONEN } l_zonen;
typedef enum {
    TYP_LEER, TYP_BUFF, TYP_VERBRAUCH, TYP_GEGENSTAND,
    TYP_GROSSSCHWERT, TYP_SCHWERT, TYP_MESSER, TYP_BOGEN,
    TYP_HELM, TYP_RUESTUNG, TYP_HOSE, TYP_SCHUHE
} l_item_typ;

typedef struct {
    int feuer;
    int frost;
    int wasser;
    int blitz;
    int gift;
} l_elementar;
typedef struct {
    // 0=nein, zahl=wie stark (oder wie viele Runden noch);
    // Status & Fl che
    int l_fluch;      // Fluch (Senkt maximale HP oder sofortiger Tod bei Max)
    int l_blutung;    // Blutung (Staut sich auf, dann massiver Schaden)
    int l_frost;      // Frostbite (Zieht Ausdauer ab)
    int l_krankheit;  // Krankheit (Senkt tempor r die Max-Stats wie Vigor oder Str)

    // Physische Einschr nkungen (Crowd Control)
    int l_paralyse;   // Bet ubung (Aussetzen f r X Runden)
    int l_blind;      // Blindheit (Trefferchance sinkt massiv / Sichtfeld klein)
    int l_verwirrt;   // Verwirrung (Die wirklichen optionen sind vertauscht (man steht es nicht))
    int l_schlaf;     // Schlaf (Kann nicht angreifen, n chster Treffer macht kritischen Schaden)

    // Positive Effekte (Buffs)
    int l_regen;      // Lebensregeneration pro Runde
    int l_steinhaut;  // Erlittener Schaden wird verkleinert
    int l_raserei;    // Eigener Schaden stark erh ht, aber man nimmt auch mehr Schaden

    // Ring Bonus
    int l_bonus_vig;
    int l_bonus_end;
    int l_bonus_dex;
    int l_bonus_str;
} l_effekte;
typedef struct {
    int l_id;
    int l_typ;
    int l_staerke;
    int l_menge;
    int l_haltbarkeit;

    l_elementar elemente;
    l_effekte   effekte;

    char l_name[32];
    char l_beschreibung[256];
} l_item;
typedef struct {
    l_item slots[INV_GROESSE];
    l_item quick[QUICK_GROESSE];
    l_item buff[MAX_SLOT];
    l_item arrow[MAX_SLOT];
    l_item waffen_slots[MAX_SLOT];

    l_item l_helm;
    l_item l_koerper;
    l_item l_hose;
    l_item l_schuhe;
    l_item l_waffe;
} l_inventar;
typedef struct {
    int l_level;
    int l_runen;

    int l_vig;    // Lebne
    int l_end;   // Ausdauer
    int l_dex;  // Geschicklichkeit
    int l_str; // Staerke

    int l_max_vig[ANZAHL_ZONEN];
    int l_cur_vig[ANZAHL_ZONEN];
    int l_cur_end;

    int l_pos_h;
    int l_pos_b;
    int l_buffer_h;
    int l_buffer_b;

    l_effekte status;               // Ist er blind? Ist er gelaehmt?
    l_elementar elementar_aktuell; // Erleidet er gerade Schaden  ber Zeit? (z.B. gift = 3)
    l_elementar resistenzen;      // Sein Schutz gegen Elemente (durch Level oder Buffs)

    l_inventar inventar;
} l_player;
typedef struct {
    int l_id;
    int l_typ; // 1=boden-nah-kampf, 2=boden-fern-kampf,  3=luft-nah-kampf, 4=luft-fern-kampf, 5=Truhe, 6=Mimic,
    int drop_chance; // R stung oder ein andere iteam
    int runen;

    int pos_max;
    int pos_min;
    int pos_buffer_b;
    int pos_buffer_h;
    int pos_b;
    int pos_h;
    int flucht_cooldown;

    int l_intelligent;
    int l_vig;
    int l_dex;
    int l_str;

    int l_cur_vig;
    int l_cur_dex;

    char l_name[32];
    char l_beschreibung[256];
    char l_ascii_art_datei[64];

    l_item l_helm;
    l_item l_koerper;
    l_item l_hose;
    l_item l_schuhe;
    l_item l_waffe;

    l_effekte status;
    l_elementar elementar_aktuell;
    l_elementar resistenzen;
} l_monster;
typedef struct {
    // Raum Funkktion: 0 = Nichts, 1 = Wand, 2 = Spieler, 3 = Ausgang,
    int** l_map;
    int** l_ME;
    int** l_MW;
    int l_ebenen;
    int l_mw_max;
    int l_exit_b;
    int l_exit_h;

    l_item    item_c[MAX_ITEM_DB];
    l_player  player;
    l_monster monster[MAX_MONSTER];
    l_monster monster_c[MAX_MONSTER_DB];
} l_infozentrum;
typedef struct {
    char name[50];
    int status;
} l_achievement;
// ---- Game Logik -----------------------------------------------
void l_start(l_infozentrum* liz);
void l_tutorial_story();
void l_stutorial();
int  Labyrinth(l_infozentrum* liz);

void l_vorbereitung(l_infozentrum* liz);
void l_map_erstellen(l_infozentrum* liz, int type);
void l_berechne_mw_max(l_infozentrum* liz);
int  l_death_scren();
void l_freigeben(l_infozentrum* liz);

void l_p_AT(l_infozentrum* liz);
int  l_berechne_hp(int vig);
int  l_berechne_stamina(int end);
int  l_berechne_treffer(int dex);
int  l_berechne_schaden(int str);
int  l_berechne_runen_kosten(int level);
void l_berechne_spieler_werte(l_player* p);
void l_berechne_spieler(l_infozentrum* liz) {
    l_berechne_spieler_werte(&liz->player);
}
int  l_eff_stat(l_player* p, int stat_typ);

int  l_ESP_menu(l_infozentrum* liz);
void l_p_inventory(l_infozentrum* liz);
void l_equipment(l_infozentrum* liz);
void l_equip_item(l_item* ziel, l_item* quelle);
void l_iteam_samlung(l_infozentrum* liz);
void l_addon_iteam_laden(l_infozentrum* liz);

void l_battle(l_infozentrum* liz, int m_index);
void l_draw_bar(int cur, int max, int breite) {
    if (max <= 0) max = 1;
    int voll = (cur * breite) / max;
    if (voll < 0) voll = 0;
    if (voll > breite) voll = breite;
    printf("[");
    printf("\033[1;31m");
    for (int i = 0; i < voll; i++) printf("#");
    printf("\033[90m");
    for (int i = 0; i < breite - voll; i++) printf("-");
    printf("\033[0m]");
}
int  l_battle_p(l_infozentrum* liz, int m_index);
void l_battle_logic_p(l_infozentrum* liz, int m_index);
int  l_berechne_gesamtschaden(int a_str, int a_dex, l_item* waffe, l_elementar* ziel_res, l_elementar* ziel_elem_akt, l_effekte* ziel_status, l_item* helm, l_item* koerper, l_item* hose, l_item* schuhe);
void l_battle_logic_m(l_infozentrum* liz, int m_index);
int  l_berechne_monster_gesamtschaden(int m_str, l_item* waffe, l_elementar* ziel_res, l_elementar* ziel_elem_akt, l_effekte* ziel_status, l_item* helm, l_item* koerper, l_item* hose, l_item* schuhe);
int  l_ziel_zone_waehlen(l_player* p, int m_dex);
void l_status_effekte_auswerten(l_player* p);
void l_bosskampf_artorias(l_infozentrum* liz, int m_index);

void l_monster_samlung(l_infozentrum* liz);
void l_monster_spawnen(l_infozentrum* liz);
void l_truhe_spawnen(l_infozentrum* liz);
void l_truhe_oeffnen(l_infozentrum* liz, int m_index);
void l_addon_monster_laden(const char* dateipfad, l_monster* m);
void l_monster_intelligent(l_infozentrum* liz, int m_index);
void l_monster_drop(l_infozentrum* liz, int m_index);
int  l_freien_slot(l_inventar* inv);
// ---- Menu -----------------------------------------------------
void l_E_MG();
void l_E_SW();
void l_E_DN();
void l_achievements();
void l_spielstand_speichern(l_infozentrum* liz);
int  l_spielstand_laden(l_infozentrum* liz);
void l_addons();
// ---- Main -----------------------------------------------------
int main() {
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    if (l_launch_game()) return 1;
    int wahl = 0;
    const char* menu[] = {
        " Start Game",
        " Einstellung",
        " Achievement",
        " Addons",
        " Spielanleitung",
        " Credits",
        " Exit"
    };
    const char* menu_name[] = { "Labyrinth", };
    int size = sizeof(menu) / sizeof(menu[0]);
    while (1) {
        wahl = menu_c(wahl, size, menu, menu_name);

        if (wahl == 0) {

            int swahl = 0;
            const char* smenu[] = {
                " New Game",
                " Spiel Laden",
                " Zurueck"
            };
            const char* smenu_name[] = { "Game", };
            int ssize = sizeof(smenu) / sizeof(smenu[0]);
            swahl = menu_c(swahl, ssize, smenu, smenu_name);

            l_infozentrum liz = { 0 };

            if (swahl == 0) {
                l_start(&liz);
            }
            else if (swahl == 1) {
                if (l_spielstand_laden(&liz)) {
                Ebene:
                    int LRS = Labyrinth(&liz);
                    if (liz.l_ebenen == 2 && liz.player.l_cur_vig[KR] > 0 && liz.player.l_cur_vig[KF] > 0) {
                        cls();
                        l_map_erstellen(&liz, 2);
                        l_monster_spawnen(&liz);
                        l_truhe_spawnen(&liz);
                        liz.player.l_pos_h = 1; liz.player.l_pos_b = 1;
                        liz.player.l_buffer_h = liz.player.l_pos_h; liz.player.l_buffer_b = liz.player.l_pos_b;
                        liz.l_map[liz.player.l_pos_b][liz.player.l_pos_h] = 2;
                        goto Ebene;
                    }
                    else if (liz.l_ebenen == 3 && liz.player.l_cur_vig[KR] > 0 && liz.player.l_cur_vig[KF] > 0) {
                        cls();
                        l_map_erstellen(&liz, 3);
                        liz.player.l_pos_h = 1; liz.player.l_pos_b = 1;
                        liz.player.l_buffer_h = liz.player.l_pos_h; liz.player.l_buffer_b = liz.player.l_pos_b;
                        liz.l_map[liz.player.l_pos_b][liz.player.l_pos_h] = 2;

                        int a_b = l_breite - 3;
                        int a_h = l_hohe - 3;
                        while (liz.l_map[a_b][a_h] != 0 && a_h > 0) {
                            a_b--;
                            if (a_b <= 0) {
                                a_b = l_breite - 2;
                                a_h--;
                            }
                        }

                        liz.monster[0] = liz.monster_c[30];
                        liz.monster[0].pos_b = a_b;
                        liz.monster[0].pos_h = a_h;
                        liz.monster[0].pos_buffer_b = a_b;
                        liz.monster[0].pos_buffer_h = a_h;

                        liz.l_ME[a_b][a_h] = -1;
                        goto Ebene;
                    }
                    l_freigeben(&liz);
                    if (LRS == 1) l_start(&liz);
                }
            }

        }
        else if (wahl == 1) {

            int ewahl = 0;
            const char* emenu[] = {
            " Tutorial",
            " Beta Modus",
            " Debug Modus",
            " Map Groesse",
            " Schwierigkeit",
            " Design",
            "   Zurueck"
            };
            int esize = sizeof(emenu) / sizeof(emenu[0]);
            int etaste;

            while (1) {
                cls();
                printf("\n\n\n\t\t\t\t=========================================================\n");
                l_PrCe("Einstellung");
                printf("\t\t\t\t=========================================================\n\n");
                const char* tcheckbox = (l_tutorial == 1) ? "[\033[32mX\033[0m]" : "[ ]";
                const char* beheckbox = (l_beta == 1) ? "[\033[32mX\033[0m]" : "[ ]";
                const char* deheckbox = (l_debug == 1) ? "[\033[32mX\033[0m]" : "[ ]";
                const char* hoheckbox =
                    (l_esmg == 0) ? "[\033[32mK\033[0m]" :
                    (l_esmg == 1) ? "[\033[33mM\033[0m]" :
                    (l_esmg == 2) ? "[\033[31mG\033[0m]" : "\033[30;47m[C]\033[0m";
                const char* swheckbox =
                    (l_schwierigkeit == 1) ? "[\033[32mE\033[0m]" :
                    (l_schwierigkeit == 2) ? "[\033[33mM\033[0m]" :
                    (l_schwierigkeit == 3) ? "[\033[31mS\033[0m]" :
                    (l_schwierigkeit == 4) ? "\033[30;47m[H]\033[0m" : "[\033[31mE\033[0m]";
                const char* dnheckbox = (l_mauer_design == 35) ? "[\033[33mS\033[0m]" : "\033[30;47m[C]\033[0m";
                const char* checkboxes[] = { tcheckbox, beheckbox, deheckbox, hoheckbox, swheckbox, dnheckbox, "   " };

                for (int i = 0; i < esize; i++) {
                    const char* prefix = (i < esize - 1) ? checkboxes[i] : " ";
                    if (i == ewahl) printf("\t\t\t\t\t\t-> %s%s\n", prefix, emenu[i]);
                    else printf("\t\t\t\t\t\t %s%s\n", prefix, emenu[i]);
                }

                etaste = _getch();
                if (etaste == 224) {
                    etaste = _getch();
                    if (etaste == 72 && ewahl > 0)         ewahl--;
                    if (etaste == 80 && ewahl < esize - 1) ewahl++;
                }
                else if (etaste == 13 || etaste == ' ') {
                    if (ewahl == 0) l_tutorial = !l_tutorial;
                    if (ewahl == 1) l_beta = !l_beta;
                    if (ewahl == 2) l_debug = !l_debug;
                    if (ewahl == 3) l_E_MG();
                    if (ewahl == 4) l_E_SW();
                    if (ewahl == 5) l_E_DN();
                    if (ewahl == 6) break;
                }
                else {
                    if ((etaste == 'w' || etaste == 'W') && ewahl > 0)         ewahl--;
                    if ((etaste == 's' || etaste == 'S') && ewahl < esize - 1) ewahl++;
                    int e_gz = etaste - '1';
                    if (e_gz >= 0 && e_gz < esize) { ewahl = e_gz; }
                }
            }
        }
        else if (wahl == 2) { l_achievements(); }
        else if (wahl == 3) { l_addons(); }
        else if (wahl == 4) { spielanleitung(); }
        else if (wahl == 5) { credit(); }
        else if (wahl == 6) {
            cls();
            printf("\n\n\n\n\n\n\n\n\t\t\t\t\t\033[1;37m__________________________________________\033[0m\n\n");
            printf("\t\t\t\t\t\033[1;37m         >  Danke fuers Spielen!  <\033[0m\n\n");
            printf("\t\t\t\t\t\033[1;37m__________________________________________\033[0m\n\n\n\n\n\n\n\n\n\n\n\n\n");
            return 0;
        }
        else { ERRORC(); return 1; }
    }

    return 1;

}
// ---- System ---------------------------------------------------
void Cursor_aus() {
    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(hcon, &cci);
    cci.bVisible = FALSE;
    SetConsoleCursorInfo(hcon, &cci);
}
void cursor_an() {
    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(hcon, &cci);
    cci.bVisible = TRUE;
    SetConsoleCursorInfo(hcon, &cci);
}
void cls() {
    // printf("\033[H\033[J");
    system("cls");
    /*
    printf("\033[H\033[J");
    \033  = ESC          -> "Steuerbefehl kommt"
    [     = CSI-Einleit. -> "Control Sequence Introducer"
    H     = Befehl       -> "Move cursor to position"
    --------------------------------------------------------------
    \033  = ESC
    [     = CSI
    J     = Befehl -> "Erase in Display"
    --------------------------------------------------------------
    Schritt 1: \033[H -> Cursor springt nach oben links
    Schritt 2: \033[J -> Alles ab Cursor (= alles) wird gel scht

    COORD home = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), home);

    */
}
void credit() {

    cls();
    printf("\n\n\t\t\t\t  =====================================================\n");
    l_PrCe("Credits");
    printf("\t\t\t\t  =====================================================\n\n\n"); Sleep(400);
    printf("\t\t\t\t\t------------------------------------\n");                      Sleep(100);
    printf("\t\t\t\t\t        Projektinformationen\n");                              Sleep(100);
    printf("\t\t\t\t\t------------------------------------\n\n");                    Sleep(100);
    printf("\t\t\t\t\tTitel: Labyrinth\n");                                          Sleep(100);
    printf("\t\t\t\t\tVersion: 1.1.0\n");                                            Sleep(100);
    printf("\t\t\t\t\tKurzbeschreibung: Das ist ein Labyrinth Spiel\n\n\n");         Sleep(100);

    printf("\t\t\t\t\t------------------------------------\n");                     Sleep(100);
    printf("\t\t\t\t\t         Entwicklerteam\n");                                  Sleep(100);
    printf("\t\t\t\t\t------------------------------------\n\n");                   Sleep(100);
    printf("\t\t\t\t\tProgrammierer: Name\n");                                      Sleep(100);
    printf("\t\t\t\t\tDesigner: Name\n\n");                                         Sleep(100);

    printf("\t\t\t\t\t------------------------------------\n");                     Sleep(100);
    printf("\t\t\t\t\t        Ressourcen & Tools\n");                               Sleep(100);
    printf("\t\t\t\t\t------------------------------------\n\n");                   Sleep(100);
    printf("\t\t\t\t\tProgrammiersprachen: C\n");                                   Sleep(100);
    printf("\t\t\t\t\tEngines: Visual Studio\n");                                   Sleep(100);
    printf("\t\t\t\t\tTools: text-image.com, asciiart.eu\n");                       Sleep(100);
    printf("\t\t\t\t\tBibliotheken: stdio.h, stdlib.h,\n");
    printf("\t\t\t\t\t              time.h, windows.h,\n");
    printf("\t\t\t\t\t              conio.h, direct.h\n\n");                        Sleep(100);

    printf("\t\t\t\t\t------------------------------------\n");                     Sleep(100);
    printf("\t\t\t\t\t            Danksagungen\n");                                 Sleep(100);
    printf("\t\t\t\t\t------------------------------------\n\n");                   Sleep(100);
    printf("\t\t\t\t\tNoch Nimand\n\n");                                            Sleep(100);

    system("pause");

}
void spielanleitung() {
    cls();
    printf("\n\n\t\t\t\t  =====================================================\n");
    l_PrCe("Spielanleitung");
    printf("\t\t\t\t  =====================================================\n\n");
    Sleep(200);

    printf("\t\t\t\t\t------------------------------------\n");       Sleep(80);
    printf("\t\t\t\t\t             Ziel des Spiels\n");               Sleep(80);
    printf("\t\t\t\t\t------------------------------------\n\n");     Sleep(80);
    printf("\t\t\t\t\tFinde den Ausgang des Labyrinths!\n");          Sleep(80);
    printf("\t\t\t\t\tDer Ausgang ist immer so weit wie\n");          Sleep(80);
    printf("\t\t\t\t\tmoeglich von dir entfernt.\n\n\n");             Sleep(80);

    printf("\t\t\t\t\t------------------------------------\n");       Sleep(80);
    printf("\t\t\t\t\t              Steuerung\n");                    Sleep(80);
    printf("\t\t\t\t\t------------------------------------\n\n");     Sleep(80);
    printf("\t\t\t\t\tBewegen:    Pfeiltasten\n");                    Sleep(80);
    printf("\t\t\t\t\t            oder W A S D\n\n");                 Sleep(80);
    printf("\t\t\t\t\tVerlassen:  ESC\n\n\n");                        Sleep(80);

    printf("\t\t\t\t\t------------------------------------\n");       Sleep(80);
    printf("\t\t\t\t\t               Legende\n");                     Sleep(80);
    printf("\t\t\t\t\t------------------------------------\n\n");     Sleep(80);
    printf("\t\t\t\t\t \033[1;31mX\033[0m  = Du (Spieler)\n");        Sleep(80);
    printf("\t\t\t\t\t \033[1;33mX\033[0m  = Ausgang\n");             Sleep(80);
    printf("\t\t\t\t\t #  = Wand\n");                                 Sleep(80);
    printf("\t\t\t\t\t    = Freier Weg\n\n\n");                       Sleep(80);

    printf("\t\t\t\t\t------------------------------------\n");       Sleep(80);
    printf("\t\t\t\t\t                Tipps\n");                      Sleep(80);
    printf("\t\t\t\t\t------------------------------------\n\n");     Sleep(80);
    printf("\t\t\t\t\t- Halte dich an einer Wand entlang,\n");        Sleep(80);
    printf("\t\t\t\t\t  um nicht die Orientierung zu verlieren.\n\n"); Sleep(80);
    printf("\t\t\t\t\t- Je groesser die Map, desto laenger\n");       Sleep(80);
    printf("\t\t\t\t\t  der Weg zum Ausgang.\n\n");                   Sleep(80);
    printf("\t\t\t\t\t- Einstellungen findest du im Hauptmenue\n");   Sleep(80);
    printf("\t\t\t\t\t  unter 'Einstellung'.\n\n\n");                 Sleep(80);

    printf("\t\t\t\t  ====================================================\n");
    printf("\t\t\t\t   Viel Erfolg!  Druecke eine Taste zum Fortfahren...\n");
    printf("\t\t\t\t  ====================================================\n\n");

    system("pause");
}
void errorc(const char* datei, int zeile) {
    cls();
    printf("\n\n\n\n\n\n\n\n");
    printf("\t\t\t\t=========================================================\n");
    printf("\t\t\t\tLeider ist ein Fehler aufgetreten.\n");
    printf("\t\t\t\tBitte versuchen Sie es erneut.\n");
    printf("\t\t\t\tFehlercode in:\n");
    printf("\t\t\t\tZeile %d\n", zeile);
    printf("\t\t\t\tDatei %s\n", datei);
    printf("\t\t\t\t=========================================================\n\n\n\n");
    system("pause");
}
int  menu_c(int wahl, int size, const char* menu[], const char* menu_name[]) {
    int taste;
    while (1) {
        cls();
        printf("\n\n\n\t\t\t\t=========================================================\n");
        l_PrCe(menu_name[0]);
        printf("\t\t\t\t=========================================================\n\n");
        for (int i = 0; i < size; i++) {
            if (i == wahl) printf("\t\t\t\t\t\t\033[1;37m%s\033[0m\n", menu[i]);
            else            printf("\t\t\t\t\t\t \033[90;40m%s\033[0m\n", menu[i]);
        }                                                     //
                                                             //
        taste = _getch();                                   //
        if (taste == 224) {                                // 224 steht f r Prefix ist daf r da sonst pfeil taste hoch/runter machen kann
            taste = _getch();                             //
            if (taste == 72 && wahl > 0)        wahl--;  // 72 steht f r pfeil taste hoch
            if (taste == 80 && wahl < size - 1) wahl++; // 80 steht f r pfeil taste runter
        }                                              //
        else if (taste == 13 || taste == ' ') {       // 13 ist Enter
            return wahl;                             //
        }                                           //
        else {                                     //
            if ((taste == 'w' || taste == 'W') && wahl > 0)        wahl--;
            if ((taste == 's' || taste == 'S') && wahl < size - 1) wahl++;
            int e_gz = taste - '1';
            if (e_gz >= 0 && e_gz < size) {
                wahl = e_gz;
            }
        }
    }
}
void l_AF(const char* ziel_name) {
    FILE* datei;
    errno_t err = fopen_s(&datei, "Game\\achievements.dat", "r");
    if (err != 0 || datei == NULL) {
        printf("Fehler: Achievements-Datei nicht gefunden.\n");
        system("pause");
        return;
    }

    l_achievement liste[20];
    char zeile[150];
    int anzahl = 0;

    while (fgets(zeile, sizeof(zeile), datei) != NULL) {
        if (anzahl >= 20) {
            break;
        }

        zeile[strcspn(zeile, "\n")] = 0;
        if (sscanf_s(zeile, "Achievement: %[^:]: %d", liste[anzahl].name, (unsigned)sizeof(liste[anzahl].name), &liste[anzahl].status) == 2) {
            if (strcmp(liste[anzahl].name, ziel_name) == 0) {
                if (liste[anzahl].status == 0) {
                    liste[anzahl].status = 1;
                    cls();
                    printf("\n\t\t\t\t\033[1;37m       ___________________________________________\033[0m\n\n");
                    printf("\t\t\t\t\033[1;37m              [ACHIEVEMENT FREIGESCHALTET]\033[0m\n\n");
                    l_PrCe(ziel_name);
                    printf("\t\t\t\t\033[1;37m       ___________________________________________\033[0m\n\n");
                    system("pause");
                }
            }
            anzahl++;
        }
    }
    fclose(datei);

    err = fopen_s(&datei, "Game\\achievements.dat", "w");
    if (err != 0 || datei == NULL) {
        printf("Fehler beim Speichern des Achievements!\n");
        ERRORC();
        return;
    }
    for (int i = 0; i < anzahl; i++) {
        fprintf(datei, "Achievement: %s: %d\n", liste[i].name, liste[i].status);
    }
    fclose(datei);


    err = fopen_s(&datei, "Game\\achievements.dat", "r");
    if (err != 0 || datei == NULL) {
        printf("Fehler: Achievements-Datei nicht gefunden.\n");
        return;
    }
    fseek(datei, 0, SEEK_END);
    long groesse = ftell(datei);
    if (groesse == 0) {
        fclose(datei);
        return;
    }
    rewind(datei);
    char name[50];
    int status;
    int alles_erfuellt = 1;
    int final_schon_fertig = 0;
    while (fgets(zeile, sizeof(zeile), datei) != NULL) {
        zeile[strcspn(zeile, "\n")] = 0;
        if (sscanf_s(zeile, "Achievement: %[^:]: %d", name, (unsigned)sizeof(name), &status) == 2) {
            if (strcmp(name, "Alle Erfolge Abschliessen") == 0) {
                if (status == 1) { final_schon_fertig = 1; break; }
            }
            else {
                if (status == 0) { alles_erfuellt = 0; break; }
            }
        }
    }
    fclose(datei);
    if (alles_erfuellt == 1 && final_schon_fertig == 0) {
        l_AF("Alle Erfolge Abschliessen");
    }
}
void l_PrCe(const char* text) {
    int breite = 39;
    int len = (int)strlen(text);
    if (len > breite) len = breite;
    int links = (breite - len) / 2;
    int rechts = breite - len - links;
    printf("\t\t\t\t\033[1;37m       > %*s%s%*s <\033[0m\n", links, "", text, rechts, "");

    /*
    int breite = 39;
    int len = (int)strlen(text);
    if (len > breite) len = breite;
    int links = (breite - len) / 2;
    int rechts = breite - len - links;
    printf("\t\t\t\t\033[1;37m       > %*s%s%*s <\033[0m\n", links, "", text, rechts, "");
    ----------------------------------------------------------------------------------------
    int len = 0;
    int esc = 0;

    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\033' || text[i] == '\x1b') {
            esc = 1;
            continue;
        }
        if (esc) {
            if (text[i] == 'm') esc = 0;
            continue;
        }
        len++;
    }
    int pad = (120 - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) { printf(" "); }
    printf("%s\033[0m\n", text);
    */
}
void l_tt(const char* farbe, const char* text, int delay_ms) {
    printf("%s", farbe);
    for (int i = 0; text[i] != '\0'; i++) {
        if (_kbhit()) {
            int t = _getch();
            if (t == 27 || t == 13 || t == ' ') {
                printf("%s", text + i);
                break;
            }
        }
        putchar(text[i]);
        Sleep(delay_ms);
    }
    printf("\033[0m\n");
}

int l_launch_game() {
    l_breite = (l_hohe * 2) + 1;
    const char* folder[] = {
        "Game",
        "Game\\Addon",
        "Game\\Addon\\Monster",
        "Game\\Addon\\Waffen",
        "Game\\Addon\\Buff",
        "Game\\Addon\\Verbrauch",
        "Game\\Addon\\Helm",
        "Game\\Addon\\Rustung",
        "Game\\Addon\\Hose",
        "Game\\Addon\\Schuhe",
        "Game\\Standard",
        "Game\\Standard\\Monster",
        "Game\\save"
    };
    int anzahl_ordner = sizeof(folder) / sizeof(folder[0]);
    for (int i = 0; i < anzahl_ordner; i++) {
        if (!l_folder_exists(folder[i])) l_create_folder(folder[i]);
        if (!l_folder_exists(folder[i])) {
            ERRORC();
            return 1;
        }
    }

    const char* l_achivment_file = "Game\\achievements.dat";
    if (!l_file_exists(l_achivment_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_achivment_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_achivment_file, 0);
    }

    const char* l_monster_1_file = "Game\\Standard\\Monster\\Soul_of_cinder.dat";
    if (!l_file_exists(l_monster_1_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_1_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_1_file, 1);
    }

    const char* l_monster_2_file = "Game\\Standard\\Monster\\Der_Waechter.dat";
    if (!l_file_exists(l_monster_2_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_2_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_2_file, 2);
    }

    const char* l_monster_3_file = "Game\\Standard\\Monster\\Monster_3.dat";
    if (!l_file_exists(l_monster_3_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_3_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_3_file, 3);
    }

    const char* l_monster_4_file = "Game\\Standard\\Monster\\Monster_4.dat";
    if (!l_file_exists(l_monster_4_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_4_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_4_file, 4);
    }

    const char* l_monster_5_file = "Game\\Standard\\Monster\\Monster_5.dat";
    if (!l_file_exists(l_monster_5_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_5_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_5_file, 5);
    }

    const char* l_monster_6_file = "Game\\Standard\\Monster\\Monster_6.dat";
    if (!l_file_exists(l_monster_6_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_6_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_6_file, 6);
    }

    const char* l_monster_7_file = "Game\\Standard\\Monster\\Siegmeyer.dat";
    if (!l_file_exists(l_monster_7_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_7_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_7_file, 7);
    }

    const char* l_monster_8_file = "Game\\Standard\\Monster\\Arkanor.dat";
    if (!l_file_exists(l_monster_8_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_8_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_8_file, 8);
    }

    const char* l_monster_9_file = "Game\\Standard\\Monster\\Mimic.dat";
    if (!l_file_exists(l_monster_9_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_9_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_9_file, 9);
    }

    const char* l_monster_10_file = "Game\\Standard\\Monster\\Artorias.dat";
    if (!l_file_exists(l_monster_10_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_10_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_10_file, 10);
    }

    const char* l_monster_11_file = "Game\\Standard\\Monster\\Monster_11.dat";
    if (!l_file_exists(l_monster_11_file)) {
        FILE* file = NULL;
        fopen_s(&file, l_monster_11_file, "w");
        if (file != NULL) { fclose(file); }
        l_write_data(l_monster_11_file, 11);
    }

    Cursor_aus();

    return 0;
}
int l_create_folder(const char* name) {
    if (CreateDirectoryA(name, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) { return 1; }
    return 0;
}
int l_folder_exists(const char* name) {
    DWORD attr = GetFileAttributesA(name);
    if (attr == INVALID_FILE_ATTRIBUTES) { return 0; }
    if (attr & FILE_ATTRIBUTE_DIRECTORY) { return 1; }
    return 0;
}
int l_create_file(const char* file_path) {
    FILE* file = NULL;
    errno_t err = fopen_s(&file, file_path, "w");
    if (err != 0 || file == NULL) { return 0; }
    fclose(file);
    return 1;
}
int l_read_file(const char* path, char* buffer, size_t size) {
    FILE* f = NULL;

    if (fopen_s(&f, path, "r") != 0 || f == NULL) { return 0; }
    if (fgets(buffer, (int)size, f) == NULL) { fclose(f); return 0; }

    fclose(f);
    return 1;
}
int l_file_exists(const char* path) {
    FILE* f = NULL;
    errno_t err = fopen_s(&f, path, "r");

    if (err != 0 || f == NULL) { return 0; }

    fclose(f);
    return 1;
}
int l_write_data(const char* file_path, int ID) {
    FILE* file = NULL;
    errno_t err = fopen_s(&file, file_path, "a");
    if (err != 0 || file == NULL) { return 0; }

    if (ID == 0) {
        fprintf(file, "Achievement: Spiel durchgespielt in Einfach: 0\n");
        fprintf(file, "Achievement: Spiel durchgespielt in Mittel: 0\n");
        fprintf(file, "Achievement: Spiel durchgespielt in Schwer: 0\n");
        fprintf(file, "Achievement: Spiel durchgespielt in Hardcore: 0\n");
        fprintf(file, "Achievement: Erstes mal gestorben: 0\n");
        fprintf(file, "Achievement: Erstes mal Monster getoetet: 0\n");
        fprintf(file, "Achievement: Tutorial Abgeschlossen: 0\n");
        fprintf(file, "Achievement: Alle Erfolge Abschliessen: 0");
    }
    else if (ID == 1) {
        fprintf(file, "                                       :~??\n");
        fprintf(file, "                               .      ^JGGP^^..\n");
        fprintf(file, "                                    ^YPGB####B#YJY:\n");
        fprintf(file, "                              ..:^.Y&#GP#&##BGBBB#?\n");
        fprintf(file, "                            ..::^7Y&#B&##BBPGBG#BG?\n");
        fprintf(file, "      . ..    .      .....^Y5PGBGG@@##&@@&#GBBB#PP?.\n");
        fprintf(file, "  ....:^~^:.:::....:^~7?5P&&#@@@@@@#&&#G&&&&BBBBPP7\n");
        fprintf(file, ". .::::!?5Y?5P5??YPJYPGB&&#GBB&&&BG&&&&#G&&&BBG5YJ.\n");
        fprintf(file, "?J7!^?JGP5PPGB#B5?G&B&&&&&BG#BG#@B&@@&@@&#@@&##Y~\n");
        fprintf(file, "^!7?YYB&#J!JJYP&@&G#&&@@@&BGB&&B&@&&&@&&&#B##Y?:\n");
        fprintf(file, "JPG##B5G#&#GGB##&@@&&&&&&&#G&@@@#&@@@@&&#GJ!:\n");
        fprintf(file, "&&&#B&G5YG#&&&&&&&&@@@@&&##&&@@@#B&@@&##BJ~\n");
        fprintf(file, "&&&#PP5GB#&&&@@@&&&&&&######&&&&&&&&@@&&@&&J.\n");
        fprintf(file, "@@&&#GPG#&&&&&&&###BPYPG#GB&&&&#&&&&&&&@&&&B5!             .~.\n");
        fprintf(file, "@&&#B###&&&&&&@&###&#&BBGP#&&BYYGBG#&&&@&&&#BG!.          ^G&P.\n");
        fprintf(file, "&BPPG&&@@@@@@@&BBB&&B5YJ?B&&GYYGBG#&&&&&&&&&B#G?.        P&&#B5~:\n");
        fprintf(file, "&&&BGB&&@@&&&#5JG#&GJ??5B&&#BB&##&&&&&&&&&&&###Y^       Y&&&#GY#B.\n");
        fprintf(file, "&#B#&&&&&&&&&&PGBB#BPBB##&&&&&&BB&&&&&&&&&&###BY^      .&&&&#5G&5\n");
        fprintf(file, "@&&&B&&&&B##&&#&&&&#B###&&&&&&B&&&&&@@&&#&##&&##5.    :P&@@&&&&G.\n");
        fprintf(file, "GPG#@@&@&B#B?G@##&@BBB#&@&@&&@&&&&&&&#&&&B#&BBGB&G7!75B&&&@@@&#?\n");
        fprintf(file, "PGGP#@#?JP?::5@##&@&B#&@@&&&@&&&&&&#J5G#&&&&B5B#PGB##B#@@@@#&BY^\n");
        fprintf(file, "PG#&#&?^7.:5&&&&&@@&B&&&@&&&&#&&G5Y!~^?&GJ!&##&#B&#B&&@@@G!!Y5Y7:\n");
        fprintf(file, "B55B#&&BJ~J&&&&#B#B#GB&&&BB&&&B!. ..:!Y: .:~JG@##&&&&@@@5  ..^^~:..\n");
        fprintf(file, "&&&&&&#~.:B&&&BGPGB#&&&&#P&&&&P:  .::.  . ... !B&@@@@&&5.  ....^..:.\n");
        fprintf(file, "&#&@@G~ ^P&&GB&@&#PBB#&#5Y#&&&@7   ...         .?PPY~^.  .......:.:.\n");
        fprintf(file, "#&&&:  .Y@&&#GG&@@@@&&@&&#GGBB@&G!. ..            .....          ...\n");
        fprintf(file, "&BB#~.JB#&&&@@&##&&@@@@&&@@@&&@@@@#: .           .. ...\n");
        fprintf(file, "&##&GP?G&@@@@@@@@&&&&@@#&&@@@@@@@@@G                  ..     ..    .\n");
        fprintf(file, "@&&&@@&#@@@@@@@@@@@@@@#&@@@@@@@@#B&G:\n");
        fprintf(file, "@@&&&&&@@@@@@@@@@@@@@&#@@@@@@@@@&&&PP5!^:^~.");
    }
    else if (ID == 2) {
        fprintf(file, "                   ~G&B#&&&&&&&&&BPYJ?JYJ!?5!~!7~..\n");
        fprintf(file, "                   G&BB#&&&#B&&&&&&#555YJ!.\n");
        fprintf(file, "                  ^&5B##BBB#B#&&&&&B5J!:\n");
        fprintf(file, "                 .JJ:PBGB&&##&&B#&&#5.:^:  .:\n");
        fprintf(file, "                .!.~J5B###&&BGB&####&Y::Y~7BY:\n");
        fprintf(file, "                .^:.?#BBB&#GPB&&&&&GB55G&###GY!!: :~\n");
        fprintf(file, "                ^^:.:PGB#BBB#&@@&&&&@&&@&####B5YY5J.\n");
        fprintf(file, "               .J!^~??PPB#&@@@@@&#@&@@@&B###&BPP#B7~:\n");
        fprintf(file, "     :B.      ..JY#&@&#B&@@@@@@@&#@@@@&&B#GB&##&#BB5Y7:\n");
        fprintf(file, "      5~   ~~ ~^YPGB#&&&#P#@&@&&&&&&&&&&&BB&&#&&##BGBBP7::\n");
        fprintf(file, "       ....5&5PGGGPGB#&&#P5&&#&&&@@&&&&&BBBGPB####BBBBGP#P\n");
        fprintf(file, "      ..!GGGBGPPPPG#&##&&&P#&&&&&&&&&##GG#GB###&&&PYY5G#B#G^\n");
        fprintf(file, "       .~JPGBGP5!:!75#B##&##&&&&#&&&&B5P&&#&#&&&&&&G5P&BPB@#   .^?JJJ~  .\n");
        fprintf(file, "       ..^75B#PJ?77^~YYP#&#&&&&&#&&&BPB#&#&&&&&&&&&&#&#B#@@B~:^B@&&@&B^\n");
        fprintf(file, "       .7P#&#B##Y^.:~?55P&&@@@&&&&@&#BBP#&&&&&&&@@@@&#&@@@@@@@@@@@&&&&#^\n");
        fprintf(file, ".....^?PB&#B##B#BB###BBP5BB#&@@&#&&&B#####&@@@@@@@@@@&&&&&&&&@@@@@&&@&&&G\n");
        fprintf(file, "... ^Y?7~JBPB###&@@@&5B&BPPB&&&@@&##&&BB&&&&&&&@@@@@@&BB##&&&&&&&@@&&@&&@\n");
        fprintf(file, ".. .!!!~.:##BBBGB&&&#B&#&&#B#&&&@@#&&##&&&&&&&&&&@@@@@&####&@@&&&&B&&&&&&\n");
        fprintf(file, ".. :~~~..^#&#B#B#B#BB&@@#5??5PPGGB##&&&&&&@@@&&&@##@@@@&&&&&&&&@&5YG&&#P#\n");
        fprintf(file, "...755YJ!Y######&&&&&@@@P55PPPGGB#&&&&&&@@@@&&&@&:.?&@@@@@@&&##&BYPG##P5B\n");
        fprintf(file, "..:GG57!Y&&&&&@@@&&&@@@&B&##&&&&&&&&&&@@@@@@@@@@7   ^@@@@@@&&######&##BG#\n");
        fprintf(file, "...Y&#GY5#&&&&&@&&&&@@@&&&#&&&&&&&&&&@@@@@@@@@&J     :~?&@&BB&&&&#&&#&#B&\n");
        fprintf(file, "....J@@@@&&#B#@@&&&&&&&@&&&&&&&@&&&&@@&&&&&&@@P         !B&&#&@&&&&&&&&&&\n");
        fprintf(file, ".... J@&BB###&@&@#7P@@@@&&&&&&&&&##&&&&&@@&&@&!    ..    ^P#&&&@&&&&@@@&&\n");
        fprintf(file, "....^B&BJPPB&&#&&. .J7#@@@@&&&&BPG#&@@@&@&&@@@~  :7?^ ..  .:7B&&@@@&&&@@@\n");
        fprintf(file, "....G@&&#BB&&&#B~...7 .&@@@@&&#&&@@&BB&@&&@@@@G:77.  .!. .   .^7G@@@#@@@@\n");
        fprintf(file, "...!&@@&&&&@@@@P ...?^.J&@@@&@@@@@&BGB&&@@&&&&&#7.  .::...... .:~~!J^B&&@\n");
        fprintf(file, ".:!#@@&&&&@@@@&Y::?5##5P@@@@@&&&&&&&&&&@@&&&&&&&BJ7~??:.....:^^:.    ...\n");
        fprintf(file, ".^7G&&&&&&###&&B7?P#&&&#&@@@@&&&##G5PBB#&&&@&&&&@@&GBY::^^~~~:........\n");
        fprintf(file, ".::.~P&&&&B#&&@&5?JGG#&&@@@&&&@@@@&&&##&&@@&&&&&&&&#&&J^^!!~^::.........");
    }
    else if (ID == 3) {
        fprintf(file, "                          .. .^7J7~.  .\n");
        fprintf(file, "                          :~5##&###GG5!.\n");
        fprintf(file, "                         ^:~?P?JJYP55JY?^..                  .7J?^.\n");
        fprintf(file, "                        :. :JJ~?J!::?###P~.     .:..^?JJ?J~.~?J!:\n");
        fprintf(file, "                      .^...^~?Y!~^.^&&BBJ!:: .:!?!7::^?Y7:\n");
        fprintf(file, "                      B5 .::.:~:.!!^#&&&&&#&#&&G#?\n");
        fprintf(file, "                      55:^...::~PB5Y&&&&&&&B5J??Y7^:\n");
        fprintf(file, "                      ?J?J!...~GG?!&@&&&&P7^~!7YG##&B~         ..\n");
        fprintf(file, "                     ^?J!^?7!YGGP5B@&&@#~::^~^!B#B&&&#~      .:.\n");
        fprintf(file, "                    ^~^7~^!?BB7^Y#&&@@&Y..:~~?G#BBBPBB#P7^~:.\n");
        fprintf(file, "                   :777??!Y#G###&&&&&@&^..~GY55P##BGGGY!..\n");
        fprintf(file, "                     7YP7JG##&#&&&&&&@Y^!GYYBBPPG5Y#BBG#7\n");
        fprintf(file, "                      ...:7YJ7?J?B#P@&BP#P~Y#Y~GB#&&##&BB!     ^\n");
        fprintf(file, "                     .:.:  ...~!:7PYP&#&#~J&&&B&@@@#GB&P~::  .:.\n");
        fprintf(file, "                       .~ .  . .::7PGGBB&@@&#&&B&&&B#BB&#?GJ75\n");
        fprintf(file, "                  .  ^YG!....:~?~~PGBBBYP#&&5~B&#PJ##GBGGBB#&~\n");
        fprintf(file, "                   .!JG&BJ7J5!~^77PGGGBGPGBG^  ^5! ?&GP!~B5Y#?\n");
        fprintf(file, "               .  :7:?G&###&##&BB##BPGBB##J     .. :P#G7JB#GGJ\n");
        fprintf(file, "                 !!!?P&&GJ7?P#&&&##&&&#B#7.       ::~PBB#BP5B~\n");
        fprintf(file, "             .~!YY7~7!?7.:755B5YB?YPP5PBBPJ!^   ~5^  .J#BYJY&~     .?#\n");
        fprintf(file, "             7??55PG?     ~YBPJJYJYPBB#&#B&@#~ Y&^     JB5GB#!   .5&&P\n");
        fprintf(file, "             ~YYP55J. . .:YPB#BBGB#&&&&&&G#&B?55.      .GGBP5!:~P&&?.\n");
        fprintf(file, "               .:^..:.7PYYG#GPGB&&&BY!YG&&G&#^    7~:. :5PYYJPB&#7.\n");
        fprintf(file, "           .^~:  ..!J7?5P!?7?JYPGG###PPB#@##&Y   ..:YG5GBBGY#GGY.\n");
        fprintf(file, "         .^:P@B7??#B5:..:^!G&&#YGGB#P#&&&&&YBB^     ?#B#PBBPBB#?\n");
        fprintf(file, "            7PJ#G#&&Y~?GPB&&&G577755PPG#&&#BG&G   ~??7B&BBGG##7\n");
        fprintf(file, "    7!.:. .::^7PGBGG#BB##&@&J!~7?J77#&&#GB#&B#&: :^5PGGP&&BG?.\n");
        fprintf(file, ".^7G&&BJ:.^!~ ....~JB&##&GPGYYJ~7PJ5JP#&&BGB#YJ:~GGGB#B#5!5G7\n");
        fprintf(file, "!!JG##BY?Y!^7!~7J7~^~~7Y55JJ?~~7YGP77~5##GGJ~:?B&&&&#5!:   ?5!\n");
        fprintf(file, " .?YPGGBB#BY?75PY!!PB#B5!:.^7PG?BG55~^?G#5~^?#&&&&#?       ..");
    }
    else if (ID == 4) {
        fprintf(file, "                    .: .\n");
        fprintf(file, "              ...^!7?JY~:.  :..^:.^:.\n");
        fprintf(file, "             .?5G#&&&&#GPJYGBPY?:  .\n");
        fprintf(file, "              .Y&&#B555?5B#7YYPJ:.::..\n");
        fprintf(file, "               ^#Y?5!^YGGB5.. .!^:~: ::\n");
        fprintf(file, "             :^YB~~Y!^YPP5~::..\n");
        fprintf(file, "           .:^^~J5B#B#&&#P!:::.\n");
        fprintf(file, "        ...!Y!^77?GB&&&#57^:~77:\n");
        fprintf(file, "       .!^?5G&&PPJ?JJ5PPPPG&#PJ::.\n");
        fprintf(file, "       7?JG&&&?^~^:. .77?Y##&#BY77.\n");
        fprintf(file, "       J##&&@#5J~~~^!5PYJP&&&&B577.\n");
        fprintf(file, "       7&&&&@&&#5J?7~JG5G#&&&&&&&5.\n");
        fprintf(file, "      :B#&&@#GG#GJ?77YGGBG&&#B###P.\n");
        fprintf(file, "      7&##&&!GPGG5PP5GGB&&&&5###&#^.\n");
        fprintf(file, "      ?&##&P.!5GGPYJPBB##&&J Y###!^!:\n");
        fprintf(file, "     .Y&&&#~ !#BB&#55PB#&&#B^.&&P:.^~:\n");
        fprintf(file, "    .?##B&P  ^BG#&&57JPG&&&&?.B&Y!!^~~\n");
        fprintf(file, "   .~G&&G#Y~:YBG&&&GJ5GB&&&&G^B&PG5JJ?^\n");
        fprintf(file, "   .?#&BBG5#BGYP&#BB###&&&&&&&#&PGY5PP?.\n");
        fprintf(file, "   .J##B#YG&#G5B&BGGGGBB&&&&&@&#BGBGG#Y.\n");
        fprintf(file, "   .Y#&BG:7#BB#&###BGGGB#&&&@B^?&B#B##7\n");
        fprintf(file, "   .7#&&7.GBGB##BB#&#&&&&&&&@G !&&&&#B~\n");
        fprintf(file, "   ..^P&~^GGGB###&#J:G&###&&@G.5##&&&G^\n");
        fprintf(file, "   .^75#77GGYPB&&&7 .#BP5YYB#Y ^~~&#B?.\n");
        fprintf(file, "    .~#G:7BB5GB&&J  .BB7YG5B#J.  ^!..\n");
        fprintf(file, "   . ^P~.~5###&@Y    7#PBB?B&!\n");
        fprintf(file, "     ~!..^^?#&&&^    ~##J~:?P.\n");
        fprintf(file, "  . .?::!77?P&#P.    :#5~!~YJ.\n");
        fprintf(file, "   .7Y. !5PBGP#&7    ~#GGP5G?.\n");
        fprintf(file, "  .:5!  .YPPP5GBY.  .J!?55G#J.");
    }
    else if (ID == 5) {
        fprintf(file, "                .:~^..~^\n");
        fprintf(file, "              5&&BB###B##7.\n");
        fprintf(file, "           ~5&@B#&#GGGGG#&&G\n");
        fprintf(file, "           5&@&GB&#GGBPPG#&@^\n");
        fprintf(file, "             JBP#BPBGP5YY5B@P\n");
        fprintf(file, "          :77:P5P#B5Y#GB5G&@~\n");
        fprintf(file, "          7&BPGGPJP5JG5BP@&@:\n");
        fprintf(file, "       :755P#PP5##JYB??5G&@@P\n");
        fprintf(file, "     .YPYY5Y5GG5!J&G###PYBG&&G\n");
        fprintf(file, "    .P5JJ??YJYY5GJ55JY5GB??#B&^\n");
        fprintf(file, "    JG5YYYYJJY#5?!7??5JYP?BPP#?\n");
        fprintf(file, "  .PP55YPBG#&#J?!?!~~~~?!~?PPGP.\n");
        fprintf(file, "  .#&&&&&@&&&YJ!~7J~~!?J!~~B&&&?\n");
        fprintf(file, " ~###&@&77BG5?G5J!7??J7J?JPB&&@B^\n");
        fprintf(file, "?#GPPB&G  .GPJGB57?!7Y7?!5&&&BPY5Y!~^.\n");
        fprintf(file, "7#GGGG&B   ##BB#BB&GYYP5&B#Y#JYYYPGYYPJ:\n");
        fprintf(file, ".#####&P  Y&&@&&##&PY5PY#&&G#B5J55B#P5GP?\n");
        fprintf(file, " P&&##&#P?##&&##&&&#??5JB&@#GBG&&&&&@B5PG7\n");
        fprintf(file, " ?@@@@@@G#GY5YY5P55GPB&&&&&&#&#@@@@@#GPGG5\n");
        fprintf(file, " 7@@@@@&B#PJJJ??????7!7Y#@@&&@&B#GBBPB#BB:\n");
        fprintf(file, "  :77?JBP@&&&&&&&&#Y?&&&@@@@&&@&BBBBBG5?.\n");
        fprintf(file, "        7&B#BB#&@B.  !&@@@&#PG#&!!!~:.\n");
        fprintf(file, "       J@&#B#BB&!      !@@@#GB#&.\n");
        fprintf(file, "       G@@@&@&@@7      ~@@@@&&&@!\n");
        fprintf(file, "       7@@&@@&~.       :7B@@@@&#J\n");
        fprintf(file, "      ~&&&&&&P           G#&&#&&&.\n");
        fprintf(file, "      G&B###@G           J&&@#B##.\n");
        fprintf(file, "      P&&&#&5.            ~&@&#&&\n");
        fprintf(file, "     :&@@&&&              .&@&&&@7.\n");
        fprintf(file, "    7@@@@@@&              7@@@@@@@@5\n");
        fprintf(file, "    :!!!!!~:               .........");
    }
    else if (ID == 6) {
        fprintf(file, "          ^:\n");
        fprintf(file, "         .5P7~\n");
        fprintf(file, "         .G5^.\n");
        fprintf(file, "       ::!GP\n");
        fprintf(file, "  .:   ..:JBP   .\n");
        fprintf(file, "  YP??  ^?5G#5  !!   .^~^^::.\n");
        fprintf(file, " ?#BP#^.5BYYG#G:7? 7GBBBBGGGPY?:\n");
        fprintf(file, "  ^PGG!:5#PGGGBGP. .BBGGPP555GBBY:                 ...\n");
        fprintf(file, "    .!??5BBPG&#^   7BBGGP55PGGG#&&:                 .::.\n");
        fprintf(file, "        . !B#&#   ~##BBBGPGGBBGG&&.                .::^.\n");
        fprintf(file, "           PB#&! ~B#BB#GBYY5GGPB&&:               .:::!~.\n");
        fprintf(file, "           ^#&&#JB#BBBG5GPYPGGG#&&G~             .7~:.^^:\n");
        fprintf(file, "            5##@#BGPGBBBP5!^!P##P#BBY.            :~^....\n");
        fprintf(file, "             ?BB&GGBB#BPPGJ!75GG5&BGP.              .    . .\n");
        fprintf(file, "            .7P5BPGG5YGG55GJ!?YBB&#5:             .    .:77:\n");
        fprintf(file, "           ^P5!~7?PGPPP55J5G5!?P#G#BG7.          ...  :?PY:\n");
        fprintf(file, "         .JGB7^~7JPP5Y?7!?5BBBYP##BBBBG?.        :..:^!G7.\n");
        fprintf(file, "        ~GBGP~~!?PBGGJ75GBGPB#BB&BBB##BBB7      :^~7?7!^\n");
        fprintf(file, "       ^BBGG!^~Y#B&GBB##GY7?PP#B&BPP#&####~  .^~~75~.\n");
        fprintf(file, "      .PBGGJ~7B@@5#@&&B?!!!?PGPY##G5#&&&##G??77YPG^\n");
        fprintf(file, "     .5BGP??7P&@@#G@@&&GY7?P##PG#&&G#@&&B5PJYGP##7\n");
        fprintf(file, "     P&BGGJJY#&&@@5B@&&##GG####@&&&&&@&&BPBBB#&&&#J:\n");
        fprintf(file, "      7BBBG##&&@@@BY&&&&&&&#&&&@&&&&&@&&#GB&&&&&&&&&P~\n");
        fprintf(file, "      ^BB###&&@@&&@P#&&@&&&&&&&@&&@&&@&&&#B#&@@@@&&&&#P:\n");
        fprintf(file, "      ?BG###&@@&&B&@#&@@&&&&&&&@&&@@@@@&&&&##&&&@@@&&&&#7\n");
        fprintf(file, "     .GBG#&BGB@&&#G@#B@@&&@@&@@@&&&&&@@@@&&&&#####&&&&&&&P\n");
        fprintf(file, "     J#GBG@&P5#&&&B#&5&&&&&&&&@@&&&&@@@@@@@&&&&&&&&######&J\n");
        fprintf(file, "    ^BBGBP&@&5P&&&&B@PG&&&#&&&&&@@@@@@@@@@@@@&&&&##&&&&####J.\n");
        fprintf(file, "    5BBBBP#@@&P5B&&#&#J#&##&&&&&&@&@@@@@@@@@@@&&BY!^:.^~7?YGG~\n");
        fprintf(file, "   :BBBBBGB@@&#5YP&&&&5G&###&&&&&@@&@@@@@@@@@@@&&&&B57:     .~.");
    }
    else if (ID == 7) {
        fprintf(file, "                            .^JGGBP~.       :.\n");
        fprintf(file, "                       .!Y5Y7!~~7JYJ~.   .7...   :~JP.\n");
        fprintf(file, "                     .J#B!!?~~~~~~~!JP?   .:!:  ~@@@@:\n");
        fprintf(file, "                    :#&#5JG5~~7?!!?!!5GG.  ..::J&@@P!\n");
        fprintf(file, "                   .&&&&#&#GPP5Y5B#BB#GB&.   ?@@@B^\n");
        fprintf(file, "                   P@&P7!??~~~~~~~~!!!?Y#P ^B@@&?\n");
        fprintf(file, "                   ^&#P7!J5~~~~~~~7Y~~~^?&5@@&P.\n");
        fprintf(file, "           .7YYJJ?7.Y@#GPG#PJ???JYGG77JP#&@&@P\n");
        fprintf(file, "          .&&B5Y@@@&##&&&##&#BBBB#&#B##5!!Y#@&B5~.\n");
        fprintf(file, "          #@&####&&@&###&&###&&&&&BG57~^~~^?&J7?J?JP!~\n");
        fprintf(file, "         !@#&&###&&#&&&&&&#GJ?7777777??JJ5#&BP5Y?7PP!JY\n");
        fprintf(file, "        ~@&####&&@&#GPPGGBBBBBBBBBBBBG5J7#@######B5~~~Y!\n");
        fprintf(file, "        J&###&@&&@&@#P55J?7777777!!!~~~^!@#######BP57~!G.\n");
        fprintf(file, "         B@@&&##&&B#@&B5~^~~~~~~~~~~~~!Y#@&########BGPGJG.\n");
        fprintf(file, "        ?&&#####&@#&##&&#5?!~~~~~!7?5B&&#&@@@&######BGP5&.\n");
        fprintf(file, "       .&@&&&&&&&&@&&&##@&&#BBBBB#&&&###&&&BB&@@&&&&&&&GYY\n");
        fprintf(file, "      .&@@@@@@@&&&###&&&@&&&&&&&&&&&&&###J!~!?G&@&&&&&&&&&^\n");
        fprintf(file, "     ?&&&&&&@@#&&##BGGB&GGGPPBB55YYJ?7!~YG~~~~!P&&@@@&&&&@@.\n");
        fprintf(file, "     &&#####@&#&#BP55P&GY?!~~P?^~~~~~~~~^G5~~~~7&&@#J~~~!7GB\n");
        fprintf(file, "     ?&#####&@#@#G5P5##Y!~~~~B7~~~~~~~~~~JB~~~~!&@&G!~~~~~B?\n");
        fprintf(file, "      #######&&@&#G55#GY~~~~~#!~~~~~~~~~~5P^~!J&@##5~~~~~BY\n");
        fprintf(file, "      !&&&&&&&@@@@&#G#B57~~~!#~~~~~~~~~~~#YYB&@@&@#5!~~^PB\n");
        fprintf(file, "       B&####&&&@@@@@@@&#G5JY&7777??J5PG&@&@@@@@&@@&G5YG#.\n");
        fprintf(file, "       :&&&&@@#&###&&@@@@@@@@@@@@@@@@@@@&@&PP@&@@&&&PYJPP\n");
        fprintf(file, "       7@&&&@#&#######GGGP5P&BGGGPPP5PB&#&&Y~PG@@&&##BB&5\n");
        fprintf(file, "     .G@@@&#@&&#####&P5P5~~~B~~~~~~~~~~5Y?5J~^~G@@&&&&#P.\n");
        fprintf(file, "    ~&@@&&@@&&@####&#GP5PJ~7P~~~~~~~~~~~P7^~!75###&#B:.\n");
        fprintf(file, "  .5@@&&&@&@&&##&#&&##BGPPYPY~~~~~~~~~~!?#J5G##PY?!JB\n");
        fprintf(file, " :#@&&&@&7J##@&#BB&@#####BG#B5YJ???JYY5GB@##B5?!7J5P#\n");
        fprintf(file, "~@@&&@@P. 5@#GB&&#&&&&&&&&&&&&&###&&&&&BGGPJ77?55Y?7#.");
    }
    else if (ID == 8) {
        fprintf(file, "                         :^:\n");
        fprintf(file, "                     :7JY?!!^:^^J!\n");
        fprintf(file, "                   .P&@@@#GJJGB5P^\n");
        fprintf(file, "                   P@@@@&&&&#B?^?:\n");
        fprintf(file, "                   Y@@@@@@&&&P?!J~\n");
        fprintf(file, "                   ~&@@&@@@@&#BGBY\n");
        fprintf(file, "                   !@@@@@@@@@@@&&G^\n");
        fprintf(file, "                 ~B@@@@@@@@@@&##&&B^\n");
        fprintf(file, "               ~&@@@@@@&##&&&@@&BBB?\n");
        fprintf(file, "             :J#&@@@@@@&&&###&@@&&&P^\n");
        fprintf(file, "             &@&BG#&@&@@@@@&&&@&@@@&G7.\n");
        fprintf(file, "            J@@@&&#B@@@@@@@&&&@&&&#&&&~\n");
        fprintf(file, "           ^@@@@&&GP&@@@@@&&&&&#BGPBG&:\n");
        fprintf(file, "           5@&&&B55G&@@@@@@@@&&B#G##::\n");
        fprintf(file, "          7@@@&BPPB#&@@@@@@@@#&##B&&7\n");
        fprintf(file, "          J@&&&#BB#@@@@@@@@@@#&&##&&J\n");
        fprintf(file, "          P@@&#PG#@@@@@@@@@@&#&##B&&&7     :~:\n");
        fprintf(file, "        .B@@&&&&&@@@@@@@@@@@&#&&##&&&#~.~7P&@&B?.\n");
        fprintf(file, "     ....&@@&&&&&@@@@&&@@@@&@@@&&&&YJG##G&@@@@@@&B!\n");
        fprintf(file, "    .....P@@&&##&@@@@&@@@@@@@@@&&&#5P55B&&@@@@@@@@@&J.\n");
        fprintf(file, " .....:::!@@@&##@@@@@@@@@@@@@@@@&#&###BB#&&@@@@@@@@@@@P^\n");
        fprintf(file, "....::^^~~&@&&#&@@@@@@@@@@@@@@@@&&#&&&&&&&#&&@@@@@@@@@@&7\n");
        fprintf(file, "...:^^~!!!&@@&#&@&@@@@@@@@@@@@@@@&#BP#&#&&&&&#BB&@@@@@&#P:\n");
        fprintf(file, ".:^7~~!7??#@@@&@@@@@@@@@@@@@@@@@@@@&BPG######G5PG#@@@@@&G~\n");
        fprintf(file, ".~&&Y7??YY&@@&&@@@@@@@@@@@&@#&@@@@@@@@&###&&GYPPB&@@@@@@#!\n");
        fprintf(file, ".^JJYG##GG@@@&&&@@@@@@@@@&&&BB@@@&&@@@@@@@&&@&&#&@@@@@@@&7\n");
        fprintf(file, ".:^!7JG#&&@@&&&&@@@@@@@@&&&&PG@@@@@@@@@@@@@@&@@@@@@@@@@@@B\n");
        fprintf(file, ":^~7?Y5PG#&@@&@&&@@@@@@@&&&&5B&@@@@@@@&&&#####&@@@@@@@@@@@7\n");
        fprintf(file, ":^~7JYPGB##&@@@@@&&@&&&&@&@&5B&@@@@@&&#BGP55YY55G#&@@@@@@@^\n");
        fprintf(file, ":^~7JYPGB#&&&@@@@@&&&&&&B#&&Y#@@@@&#BPYJ?!~~~~~!!7?YPGBGY:");
    }
    else if (ID == 9) {
        fprintf(file, "          ........................................\n");
        fprintf(file, "       :: :                                      :  ...\n");
        fprintf(file, "     :^.    ............:..::..:...............^.       ^.\n");
        fprintf(file, "         ...................................        .:.....\n");
        fprintf(file, "   !  ~: .~  7  ?  ~. :~  7  7  ~: .!  ?  !. .   ~:  .~     7\n");
        fprintf(file, "  ~   ^: .~  !  !  ~. .^  !  !  ^. .~  !  ~.    ^:    .~     !\n");
        fprintf(file, "   ...........................................       ~   ^.  .:\n");
        fprintf(file, "   .:  ^   ~  .:  :. ^.  ~  :.  ^  ..  :.  ^  ..  .   ~:  !:.!\n");
        fprintf(file, "    !:!.   ~. 7: .!  .! :~  .! :~   ?  7 . 7  7.   :^  :~  ::\n");
        fprintf(file, "     ^   .: 7~:.57    .7! ^! :!!  ^ :~!    .!7.     .~.  ~:  ~.\n");
        fprintf(file, "            .^      :~ .  ^   .  .:  ..     .:     .  ^^  :::.\n");
        fprintf(file, "                    .^  .........   .................. .~.   :\n");
        fprintf(file, "                   ... ..                            ..  :^  7\n");
        fprintf(file, "         ..    :::......::...       ................     ..  .\n");
        fprintf(file, "         7  .^.             ......::.  ..............::.^:  ^\n");
        fprintf(file, "       .. .~.       .::.            .^:  ...........   ^   :^\n");
        fprintf(file, "     .^: ^^       .^:       .::::::    ^:          .^    .   .\n");
        fprintf(file, "     .  ~:       ^:       .^...   ..    ^.        ::      .  7\n");
        fprintf(file, " .^. ..:~       !        ^:..:^^^^^^^^^^^^^^^^. :^  .^.      ^\n");
        fprintf(file, " 7.    ?       !.      .~   .      .           .. .^.  .    ..\n");
        fprintf(file, " !    .!       7      .!.!  !. ~.  7  ~.  !. ~   ^.  .^.  .^.\n");
        fprintf(file, "   .. :^      .:      7.!.. .!:~ . ^^:~ : .!:~ .   .^.  .^.\n");
        fprintf(file, "    . ::             ~:      .:     .:     .^  7 .^.  .^.\n");
        fprintf(file, "      ^:            .!                         7    .^.\n");
        fprintf(file, "      ~.    7       7  ....................    7  .:.\n");
        fprintf(file, "     .^    !.      !........................   7  .\n");
        fprintf(file, "   .^:   .^       ~                            !\n");
        fprintf(file, " ::.            :^...........................  7\n");
        fprintf(file, "~.            .:.                              ~");
    }
    else if (ID == 10) {
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  . *-##@@@@@@.  . #+#-           .@===+*@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ -@@*@@@@@@@@.. :@-@##+@#  +:      .*--==#@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  @@=@@@@@@@@:@  #. =#@@.@   .   .  --::-=*@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  @@ @@@@@@@@-   #@@*.*@+=@#@-  =    ..:.::=*@@@@@@@@@\n");
        fprintf(file, "@@@@#*:+@@@@@@@@@@@@@@@@@@@@@@@@@@@@  @@ @@@@@@@@*+..@--+=:== @=-@  .@  .  ....-+@@@@@@@@@\n");
        fprintf(file, "+#**@@+.   @@@@@@@@@@@@@@@@@@@@@@@@@. @+.@@@@@@@: *@@.  +@-+          @      .:-+@@@@@@@@@\n");
        fprintf(file, "#@:-- @@@@    @@@@@@@@@@@@@@@@@@@@@@:-@=*@@@@@: :=@@@@##   @@-   ..         ..:=*@@@@@@@@@\n");
        fprintf(file, " #@@@:-@@@@@@    @@@@@@@@@@@@@@@@@@@- #@@@@@@@*  -@@@@@@@@= *@@@@.   .        .:=#@@@@@@@@\n");
        fprintf(file, "@+  -#@    @@@@-.   @@@@@@@@@@@@@@@@:=@@@@@@@@@. =:-@###-:*-.:.@@@@@:   .    -...-###@@@@@\n");
        fprintf(file, "@@@@@-*@@@@==@@@@@    ..*@@@@@@@@@@@+@@@#@##**+*@@*@@@*++:=.-: @@@@- .+ .  *:..-*#@@@@@@@@\n");
        fprintf(file, "@@@@@@@  -*@    +@@@#    . @@@@@@@@*=@@@**+---=*#@@#@@= +..  .  @@@@@ .  @@#=--=*@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@   ... +=@@@@-       @@@@@@####@***-:*@#+@=.-.-  .   @+*.:@=@@@#***##@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@.   :@@@##@@@@- .     :#@@@*+*######@#.. .* :  :   @@@  @@@@#@###@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@..  @@  +@@@@@@@*  === #@@#*+=-*==#*::--*  .   ::@@  @@@@@@@#@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@:  ..-    := +*@@@=*#*.-*@*=:-#@@#*#*    ..   .@@ *@@@@@@@@@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@- .@@   @=..  =@@#@  ..@@@@@=-*@@*@@-.       @@  @@@@@@@@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@**- .**   ::++==.:* :+@@+ @@@@@@@@* +#-      @@-   #@@@@@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@@@@+  :@@+  : -@# -@@@:   . :@@:+ .@##+=.+. @@@@ +  :@@@@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@@@@@@@@##+@ .       :  .=  +.=   ...*. .    -@@- +    @@@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@=:@@..: .- :.:.:.  .+-@@@-  :@=+@@@:#@  . . @ -@@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  *:    @. #.  +. -.@   .:@@@ @@@#     @    @@@@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@@@@=.+##@@@@@@@@#.    #@.  =@@ .@# :++      .   ..@+     #@@@@@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@@@@@@==.::--:.+#@@@@@@@*+=@=@  +-@@    .@: :=@               .    +@@@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@@@@#*#*#@=.      ... +@@@@@@@@ .+*@@@@#         +@.@-@  @  .@@.  .:@  .*@@@@@@@\n");
        fprintf(file, "@@@@@@@@@@@#-:--- ..        :..   .=@@@@@@.  *@ .    . -= .*@      +@. @..    @    *@@@@@@\n");
        fprintf(file, "@@@@@@@@=...:...:-.              ::+#@@@@@@@@ #@  * . .: .  *@#-.    @@:.@@: . +@==##@@@@@\n");
        fprintf(file, "@@@@@@*=*-  .  .        .   ...--=:-#@@@@@@#@@.+@@ .=@@@@@@@        @-  .  .@@=   . .@@@@@\n");
        fprintf(file, "@@@@@@*-+  -:.               =     -#@@#+@@@+   .#@@@@@@::@@@@   @#. .@   *@@@@@   .+#@@@@\n");
        fprintf(file, "@@@@@@..= .   .-        .   .-     .+@@##@@@@*  ..-@.   ..      :   +   .@@@@@@@@@@@@@@@@@\n");
        fprintf(file, "@@@*..      -:              .   ..  :+++=*###. - .=..=:..  - .  ..=.    . .   ----+@@@@@@@\n");
    }
    else {
        fprintf(file, "                              .:~^..~^\n");
        fprintf(file, "                            5&&BB###B##7.\n");
        fprintf(file, "                         ~5&@B#&#GGGGG#&&G\n");
        fprintf(file, "                         5&@&GB&#GGBPPG#&@\n");
        fprintf(file, "                           JBP#BPBGP5YY5B@P\n");
        fprintf(file, "                        :77:P5P#B5Y#GB5G&@~\n");
        fprintf(file, "                        7&BPGGPJP5JG5BP@&@:\n");
        fprintf(file, "                     :755P#PP5##JYB??5G&@@P\n");
        fprintf(file, "                   .YPYY5Y5GG5!J&G###PYBG&&G\n");
        fprintf(file, "                  .P5JJ??YJYY5GJ55JY5GB??#B&^\n");
        fprintf(file, "                  JG5YYYYJJY#5?!7??5JYP?BPP#?\n");
        fprintf(file, "                .PP55YPBG#&#J?!?!~~~~?!~?PPGP.\n");
        fprintf(file, "                .#&&&&&@&&&YJ!~7J~~!?J!~~B&&&?\n");
        fprintf(file, "               ~###&@&77BG5?G5J!7??J7J?JPB&&@B^\n");
        fprintf(file, "              ?#GPPB&G  .GPJGB57?!7Y7?!5&&&BPY5Y!~^.\n");
        fprintf(file, "              7#GGGG&B   ##BB#BB&GYYP5&B#Y#JYYYPGYYPJ:\n");
        fprintf(file, "              .#####&P  Y&&@&&##&PY5PY#&&G#B5J55B#P5GP?\n");
        fprintf(file, "               P&&##&#P?##&&##&&&#??5JB&@#GBG&&&&&@B5PG7\n");
        fprintf(file, "               ?@@@@@@G#GY5YY5P55GPB&&&&&&#&#@@@@@#GPGG5\n");
        fprintf(file, "               7@@@@@&B#PJJJ??????7!7Y#@@&&@&B#GBBPB#BB:\n");
        fprintf(file, "                :77?JBP@&&&&&&&&#Y?&&&@@@@&&@&BBBBBG5?.\n");
        fprintf(file, "                      7&B#BB#&@B.  !&@@@&#PG#&!!!~:.\n");
        fprintf(file, "                     J@&#B#BB&!      !@@@#GB#&.\n");
        fprintf(file, "                     G@@@&@&@@7      ~@@@@&&&@!\n");
        fprintf(file, "                     7@@&@@&~.       :7B@@@@&#J\n");
        fprintf(file, "                    ~&&&&&&P           G#&&#&&&.\n");
        fprintf(file, "                    G&B###@G           J&&@#B##.\n");
        fprintf(file, "                    P&&&#&5.            ~&@&#&&\n");
        fprintf(file, "                   :&@@&&&              .&@&&&@7.\n");
        fprintf(file, "                  7@@@@@@&              7@@@@@@@@5\n");
        fprintf(file, "                  :!!!!!~:               .........\n");
    }

    fclose(file);
    return 1;
}
// ---- Game Logik -----------------------------------------------
void l_start(l_infozentrum* liz) {
LGNS:
    l_vorbereitung(liz);
    if (l_tutorial) l_tutorial_story();
Ebene:
    int LRS = Labyrinth(liz);
    if (liz->l_ebenen == 2 && liz->player.l_cur_vig[KR] > 0 && liz->player.l_cur_vig[KF] > 0) {
        cls();
        l_map_erstellen(liz, 2);
        l_monster_spawnen(liz);
        l_truhe_spawnen(liz);
        liz->player.l_pos_h = 1; liz->player.l_pos_b = 1;
        liz->player.l_buffer_h = liz->player.l_pos_h; liz->player.l_buffer_b = liz->player.l_pos_b;
        liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h] = 2;
        goto Ebene;
    }
    else if (liz->l_ebenen == 3 && liz->player.l_cur_vig[KR] > 0 && liz->player.l_cur_vig[KF] > 0) {
        cls();
        l_map_erstellen(liz, 3);
        liz->player.l_pos_h = 1; liz->player.l_pos_b = 1;
        liz->player.l_buffer_h = liz->player.l_pos_h; liz->player.l_buffer_b = liz->player.l_pos_b;
        liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h] = 2;

        int a_b = l_breite - 3;
        int a_h = l_hohe - 3;
        while (liz->l_map[a_b][a_h] != 0 && a_h > 0) {
            a_b--;
            if (a_b <= 0) {
                a_b = l_breite - 2;
                a_h--;
            }
        }

        liz->monster[0] = liz->monster_c[30];
        liz->monster[0].pos_b = a_b;
        liz->monster[0].pos_h = a_h;
        liz->monster[0].pos_buffer_b = a_b;
        liz->monster[0].pos_buffer_h = a_h;

        liz->l_ME[a_b][a_h] = -1;
        goto Ebene;
    }
    l_freigeben(liz);
    if (LRS) goto LGNS;
}
void l_tutorial_story() {
    cls();
    printf("\n\n\n");
    Sleep(300);

    l_tt("  Schritte.....", "\033[37m", 35);
    Sleep(400);
    l_tt("  Er haelt eine Fackel in der Hand.", "\033[37m", 35);

    printf("\n");
    Sleep(400);
    l_tt("  \"Du lebst ja noch.\"", "\033[1;33m", 55);
    Sleep(600);
    l_tt("  \"Ich dachte du waerst schon tot.\"", "\033[1;33m", 45);
    Sleep(700);

    printf("\n");
    l_tt("  [ Druecke ENTER ]", "\033[90m", 30);
    while (_getch() != 13);

    cls();
    printf("\n\n\n");
    Sleep(300);

    printf("\n");
    l_tt("  \"Ich kann dich leider nicht mitnehmen.\"", "\033[1;33m", 45);
    Sleep(300);
    l_tt("  \"Ich habe nur einen Teleportstein.\"", "\033[1;33m", 45);
    Sleep(300);
    l_tt("  \"Und ich brauche ihn selbst um hier rauszukommen.\"", "\033[1;33m", 40);
    Sleep(700);

    printf("\n");
    l_tt("  [ Druecke ENTER ]", "\033[90m", 30);
    while (_getch() != 13);

    cls();
    printf("\n\n\n");
    Sleep(300);

    l_tt("  Er greift in seine Tasche.", "\033[37m", 35);
    Sleep(600);
    l_tt("  Sie zieht ein altes Buch heraus.", "\033[37m", 35);
    Sleep(400);

    printf("\n");
    l_tt("  \"Hier. Nimm das.\"", "\033[1;33m", 55);
    Sleep(300);
    l_tt("  \"Es gehoerte dem letzten der es hier rausgeschafft hat.\"", "\033[1;33m", 40);
    Sleep(400);
    l_tt("  \"Da steht alles drin was du wissen musst.\"", "\033[1;33m", 40);
    Sleep(700);

    printf("\n");
    l_tt("  * Bekommst ein Altes buch *.", "\033[37m", 40);
    l_tt("  Der Einband ist abgenutzt. Seiten sind vollgekritzelt.", "\033[37m", 35);
    Sleep(500);

    printf("\n");
    l_tt("  [ Druecke ENTER ]", "\033[90m", 30);
    while (_getch() != 13);

    cls();
    printf("\n\n\n");
    Sleep(300);

    l_tt("  \"Ich muss jetzt gehen.\"", "\033[1;33m", 50);
    Sleep(300);
    l_tt("  \"Das Labyrinth... es veraendert sich staendig.\"", "\033[1;33m", 40);
    Sleep(400);
    l_tt("  \"Warte nicht zu lange.\"", "\033[1;33m", 50);
    Sleep(800);

    printf("\n");
    l_tt("  Er haelt den Teleportstein hoch.", "\033[37m", 35);
    Sleep(400);
    l_tt("  Ein helles Licht blendet dich.", "\033[37m", 35);
    Sleep(600);

    printf("\n");
    Sleep(400);
    l_tt("  Er verschwindet.", "\033[37m", 55);
    Sleep(800);

    printf("\n");
    l_tt("  [ Druecke ENTER um das Buch zu oeffnen ]", "\033[90m", 25);
    while (_getch() != 13);

    cls();
    printf("\n\n\n");
    Sleep(200);

    printf("\n");
    l_tt("  ~ Das alte Buch ~.", "\033[37m", 55);
    Sleep(300);

    l_tt("  Du schlaegs es auf", "\033[37m", 55);
    l_tt("  Die erste Seite:", "\033[37m", 55);
    Sleep(600);

    l_tt("  [ Druecke ENTER ]", "\033[90m", 25);
    while (_getch() != 13);
    l_stutorial();
}
void l_stutorial() {
    const int SEITEN = 8;
    int seite = 0;

    while (1) {
        cls();
        printf(" +=========================================================+\n");
        printf(" |                                                         |\n");
        printf(" |            >  TUTORIAL  (Seite %-2d / %-2d)  <              |\n", seite + 1, SEITEN);
        printf(" |                                                         |\n");
        printf(" +=========================================================+\n\n");

        switch (seite) {

        case 0:
            printf("  \033[1;37m  Wie du aus denn Labyrinth Kommst\033[0m\n\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  Dein Ziel:                                 |\n");
            printf("  |                                             |\n");
            printf("  |  Finde den Ausgang des Labyrinths!          |\n");
            printf("  |  Er ist immer so weit wie moeglich          |\n");
            printf("  |  von dir entfernt.                          |\n");
            printf("  |                                             |\n");
            printf("  |  Besiege Monster, sammle Runen,             |\n");
            printf("  |  steige im Level auf und ueberlebe.         |\n");
            printf("  +---------------------------------------------+\n\n");
            printf("  Druecke \033[1;37m[D]\033[0m oder \033[1;37m[ENTER]\033[0m"
                " um weiterzugehen.\n");
            printf("  \033[90m[ESC] Tutorial ueberspringen\033[0m\n");
            break;

        case 1:
            printf("  \033[1;37m  Steuerung\033[0m\n\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  Bewegen:                                   |\n");
            printf("  |                                             |\n");
            printf("  |    W  / Pfeil Hoch    ->  Oben              |\n");
            printf("  |    S  / Pfeil Runter  ->  Unten             |\n");
            printf("  |    A  / Pfeil Links   ->  Links             |\n");
            printf("  |    D  / Pfeil Rechts  ->  Rechts            |\n");
            printf("  |                                             |\n");
            printf("  |  Menues oeffnen:                            |\n");
            printf("  |                                             |\n");
            printf("  |    ESC  ->  Spielmenue (Equipment, Stats)   |\n");
            printf("  |    I    ->  Inventar                        |\n");
            printf("  |    U    ->  Level Up                        |\n");
            printf("  |                                             |\n");
            printf("  +---------------------------------------------+\n");
            break;

        case 2:
            printf("  \033[1;37m  Die Karte\033[0m\n\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  Legende:                                   |\n");
            printf("  |                                             |\n");
            printf("  |   \033[1;34mX\033[0m   =  Du (Spieler)                       |\n");
            printf("  |   \033[30;47mX\033[0m   =  Ausgang  <-- Dein Ziel!            |\n");
            printf("  |   \033[1;31mX\033[0m   =  Monster                            |\n");
            printf("  |   \033[1;33m?\033[0m   =  Item auf dem Boden                 |\n");
            printf("  |   #   =  Wand (nicht begehbar)              |\n");
            printf("  |       =  Freier Weg                         |\n");
            printf("  |                                             |\n");
            printf("  |  Tipp: Der Ausgang ist immer am weitesten   |\n");
            printf("  |  von deinem Startpunkt entfernt.            |\n");
            printf("  |  Folge langen Korridoren!                   |\n");
            printf("  +---------------------------------------------+\n");
            break;

        case 3:
            printf("  +---------------------------------------------+\n");
            printf("  |  Deine Kampf-Aktionen:                      |\n");
            printf("  |                                             |\n");
            printf("  |  \033[1;37mAngriff\033[0m                                    |\n");
            printf("  |    Greife das Monster an.                   |\n");
            printf("  |    Verbraucht Ausdauer.                     |\n");
            printf("  |    Trefferchance: abhaengig von DEX.        |\n");
            printf("  |                                             |\n");
            printf("  |  \033[1;37mQuick Slot\033[0m                                 |\n");
            printf("  |    Benutze Heiltrank oder Buff.             |\n");
            printf("  |    Items vorher im Equipment einlegen!      |\n");
            printf("  |                                             |\n");
            printf("  |  \033[1;37mAusweichen\033[0m                                 |\n");
            printf("  |    Monster greift diese Runde nicht an.     |\n");
            printf("  |    Kostet 20%% der max. Ausdauer.            |\n");
            printf("  |                                             |\n");
            printf("  |  \033[1;37mRuhen\033[0m                                      |\n");
            printf("  |    Regeneriert 33%% Ausdauer.                |\n");
            printf("  |    Monster greift danach an!                |\n");
            printf("  |                                             |\n");
            printf("  |  \033[1;37mFlucht\033[0m                                     |\n");
            printf("  |    Fliehe aus dem Kampf.                    |\n");
            printf("  |    Auf Einfach: immer erfolgreich.          |\n");
            printf("  |    Auf Schwer/Hardcore: kann fehlschlagen.  |\n");
            printf("  +---------------------------------------------+\n");
            break;

        case 4:
            printf("  \033[1;37m  Koerperteile & Schaden\033[0m\n\n");
            printf("  Dein Koerper hat 10 Zonen mit eigenen HP.\n");
            printf("  \033[1;31mWenn Kopf ODER Koerper auf 0 faellt -> Tod!\033[0m\n\n");

            printf("  +----------------------+  +---------------------+\n");
            printf("  |    Koerper-Karte     |  |  Kritische Zonen:   |\n");
            printf("  |                      |  |                     |\n");
            printf("  |      [ \033[1;31mKopf\033[0m ]        |  | [\033[1;31mKopf\033[0m]  -> sofort   |\n");
            printf("  |  [AL][\033[1;31mKoerper\033[0m][AR]   |  | [\033[1;31mKoerper\033[0m] -> sofort |\n");
            printf("  |  [HL]        [HR]    |  |                     |\n");
            printf("  |  [BL]        [BR]    |  |  Andere Zonen:      |\n");
            printf("  |  [FL]        [FR]    |  |  nur Nachteile      |\n");
            printf("  |                      |  |                     |\n");
            printf("  +----------------------+  +---------------------+\n\n");
            printf("  AL/AR = Arm L/R   HL/HR = Hand L/R\n");
            printf("  BL/BR = Bein L/R  FL/FR = Fuss L/R\n\n");
            printf("  Tipp: Heile im Inventar um alle Zonen zu heilen.\n");
            break;

        case 5:
            printf("  \033[1;37m  Inventar & Equipment\033[0m\n\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  \033[1;37mInventar  [I-Taste]:\033[0m                       |\n");
            printf("  |                                             |\n");
            printf("  |  Zeigt alle Items die du traegst.           |\n");
            printf("  |  Mit ENTER kannst du Verbrauch-Items        |\n");
            printf("  |  direkt benutzen (heilen).                  |\n");
            printf("  |  Mit A/D kannst du Kategorien filtern.      |\n");
            printf("  |                                             |\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  \033[1;37mEquipment  [ESC -> Equipment]:\033[0m             |\n");
            printf("  |                                             |\n");
            printf("  |  Waffe    -> Bestimmt deinen Schaden        |\n");
            printf("  |  Ruestung -> Reduziert erlittenen Schaden   |\n");
            printf("  |  Ringe    -> Passive Stat-Boni  (+STR..)    |\n");
            printf("  |  Quick    -> Heiltranke fuer den Kampf      |\n");
            printf("  |  W-Slots  -> Waffen-Wechsel per TAB         |\n");
            printf("  |                                             |\n");
            printf("  |  Tipp: Alle 4 Ruestungsteile tragen fuer    |\n");
            printf("  |  maximalen Schutz!                          |\n");
            printf("  +---------------------------------------------+\n");
            break;

        case 6:
            printf("  \033[1;37m  Level Up & Attribute\033[0m\n\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  \033[1;37m[U-Taste] -> Level Up Menue\033[0m                |\n");
            printf("  |                                             |\n");
            printf("  |  Kosten: Runen  (von Monstern erhalten)     |\n");
            printf("  |  Jedes Level kostet mehr Runen.             |\n");
            printf("  |                                             |\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  Die 4 Attribute:                           |\n");
            printf("  |                                             |\n");
            printf("  |  \033[1;33mVIGOR\033[0m      ->  Max. HP erhoehen            |\n");
            printf("  |  \033[1;33mENDURANCE\033[0m  ->  Max. Ausdauer erhoehen      |\n");
            printf("  |  \033[1;33mDEXTERITY\033[0m ->  Trefferchance erhoehen       |\n");
            printf("  |  \033[1;33mSTRENGTH\033[0m   ->  Schaden erhoehen            |\n");
            printf("  |                                             |\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  Waffen-Empfehlung:                         |\n");
            printf("  |                                             |\n");
            printf("  |  Schwert/Grossschwert  ->  STR maxieren     |\n");
            printf("  |  Bogen                 ->  DEX maxieren     |\n");
            printf("  |  Ueberleben            ->  VIG/END maxieren |\n");
            printf("  +---------------------------------------------+\n");
            break;

        case 7:
            printf("  \033[1;37m  Monster-Typen\033[0m\n\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  Typ 1  --  Boden-Nah-Kampf                 |\n");
            printf("  |    Alle Waffen treffen.                     |\n");
            printf("  |                                             |\n");
            printf("  |  Typ 2  --  Boden-Fern-Kampf                |\n");
            printf("  |    Alle Waffen treffen.                     |\n");
            printf("  |    Kann bei gescheiterter Flucht angreifen. |\n");
            printf("  |                                             |\n");
            printf("  |  Typ 3  --  \033[1;33mLuft-Nah-Kampf  [!]\033[0m             |\n");
            printf("  |    Nur Bogen oder Grossschwert trifft!      |\n");
            printf("  |                                             |\n");
            printf("  |  Typ 4  --  \033[1;31mLuft-Fern-Kampf [!!]\033[0m            |\n");
            printf("  |    Nur Bogen trifft!                        |\n");
            printf("  |    Kann bei gescheiterter Flucht angreifen. |\n");
            printf("  +---------------------------------------------+\n");
            printf("  |  KI-Typen (Bewegung auf der Map):           |\n");
            printf("  |                                             |\n");
            printf("  |  1 = Zufaellig        2 = Folgt dir nah     |\n");
            printf("  |  3 = Bewacht Ausgang  4 = Verfolgt immer    |\n");
            printf("  +---------------------------------------------+\n\n");
            break;
        }
        printf("\n");
        printf(" +=========================================================+\n");
        printf(" |  ");
        if (seite > 0) printf("\033[90m[A/Links] Zurueck\033[0m        ");
        else printf("                         ");
        if (seite < SEITEN - 1) printf("  \033[90m[D/ENTER/Rechts] Weiter\033[0m   ");
        else printf("     \033[1;32m[ENTER] Spielen!       \033[0m");
        printf("  |\n");
        printf(" +=========================================================+");
        int t = _getch();
        if (t == 27) return;
        if (t == 224) {
            t = _getch();
            if (t == 75 && seite > 0) { seite--; continue; }
            if (t == 77) {
                if (seite < SEITEN - 1) { seite++; continue; }
                else return;
            }
        }
        if ((t == 'a' || t == 'A') && seite > 0) { seite--; continue; }
        if (t == 'd' || t == 'D' || t == 13 || t == ' ') {
            if (seite < SEITEN - 1) seite++;
            else {
                l_AF("Tutorial Abgeschlossen");
                return;
            }
        }
    }
}
int  Labyrinth(l_infozentrum* liz) {
    ULONGLONG lastEnemyMove = GetTickCount64();
    ULONGLONG lastPlayerMove = GetTickCount64();
    ULONGLONG lastPlayerend = GetTickCount64();
    int end_reg_geschwindigkeit = 300;
    int artorias_getriggert = 0;
    int artorias_m_index = -1;
    cls();
    while (1) {
        int taste = 0;
        int monster_darf_bewegen = 0;
        int spieler_geschwindigkeit = 100;
        if (l_schwierigkeit != 1) {
            if (liz->player.l_cur_vig[BL] == 0) spieler_geschwindigkeit += 100;
            else if (liz->player.l_cur_vig[BL] < (liz->player.l_max_vig[BL] / 2)) spieler_geschwindigkeit += 50;
            if (liz->player.l_cur_vig[BR] == 0) spieler_geschwindigkeit += 100;
            else if (liz->player.l_cur_vig[BR] < (liz->player.l_max_vig[BR] / 2)) spieler_geschwindigkeit += 50;
            if (liz->player.l_cur_vig[FL] == 0) spieler_geschwindigkeit += 50;
            else if (liz->player.l_cur_vig[FL] < (liz->player.l_max_vig[FL] / 2)) spieler_geschwindigkeit += 25;
            if (liz->player.l_cur_vig[FR] == 0) spieler_geschwindigkeit += 50;
            else if (liz->player.l_cur_vig[FR] < (liz->player.l_max_vig[FR] / 2)) spieler_geschwindigkeit += 25;
        }

        while (1) {
            COORD home = { 0, 0 };
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), home);
            if ((GetTickCount64() - lastPlayerend >= end_reg_geschwindigkeit) && liz->player.l_cur_end < l_berechne_stamina(liz->player.l_end)) {
                lastPlayerend = GetTickCount64();
                liz->player.l_cur_end += 1;
                if (liz->player.l_cur_end > l_berechne_stamina(liz->player.l_end))  liz->player.l_cur_end = l_berechne_stamina(liz->player.l_end);
            }
            char render_buffer[70750];
            int b_idx = 0;
            for (int y = 0; y < l_hohe; y++) {
                render_buffer[b_idx++] = '\n';
                for (int x = 0; x < l_breite; x++) {
                    if (liz->l_map[x][y] == 1) { render_buffer[b_idx++] = (char)l_mauer_design; }
                    else if (liz->l_map[x][y] == 2) { b_idx += sprintf_s(&render_buffer[b_idx], 70750 - b_idx, "\033[1;34mX\033[0m"); }
                    else if (liz->l_map[x][y] == 3) { b_idx += sprintf_s(&render_buffer[b_idx], 70750 - b_idx, "\033[30;47mX\033[0m"); }
                    else if (liz->l_ME[x][y] > 0) { b_idx += sprintf_s(&render_buffer[b_idx], 70750 - b_idx, "\033[1;33m?\033[0m"); }
                    else if (liz->l_ME[x][y] < 0 && (liz->monster[-liz->l_ME[x][y] - 1].l_typ == 5 || liz->monster[-liz->l_ME[x][y] - 1].l_typ == 6)) {
                        b_idx += sprintf_s(&render_buffer[b_idx], 70750 - b_idx, "\033[1;33m?\033[0m");
                    }
                    else if (liz->l_ME[x][y] < 0) { b_idx += sprintf_s(&render_buffer[b_idx], 70750 - b_idx, "\033[1;31mX\033[0m"); }
                    else if (liz->l_map[x][y] != 1) { render_buffer[b_idx++] = ' '; }
                }
            }
            render_buffer[b_idx] = '\0';
            if (l_debug) {
                printf("Bein Links/Rechts: %d,%d|Fuss Links/Rechts: %d,%d", liz->player.l_cur_vig[BL], liz->player.l_cur_vig[BR], liz->player.l_cur_vig[FL], liz->player.l_cur_vig[FR]);
                printf("Check Ausdauer: %d/%d\n", liz->player.l_cur_end, l_berechne_stamina(liz->player.l_end));
            }
            printf("%s", render_buffer);
            if (liz->player.l_cur_vig[KF] < 1 || liz->player.l_cur_vig[KR] < 1 || liz->l_ME[liz->player.l_pos_b][liz->player.l_pos_h] < 0) break;
            if (GetTickCount64() - lastEnemyMove >= 700) {
                lastEnemyMove = GetTickCount64();
                monster_darf_bewegen = 1;
                taste = 0;
                break;
            }
            if (liz->l_ebenen == 3 && !artorias_getriggert) {
                for (int m = 0; m < MAX_MONSTER; m++) {
                    if (liz->monster[m].l_id == 30) {
                        int dist = abs(liz->player.l_pos_b - liz->monster[m].pos_b) + abs(liz->player.l_pos_h - liz->monster[m].pos_h);
                        if (dist <= 5) {
                            artorias_getriggert = 1;
                            artorias_m_index = m;
                            taste = 200;
                            break;
                        }
                    }
                }
                if (artorias_getriggert) break;
            }
            if (_kbhit()) {
                taste = _getch();

                if (taste == 27) {
                    if (l_ESP_menu(liz) == 27) {
                        taste = 27;
                        break;
                    }
                    else {
                        taste = 0;
                        lastEnemyMove = GetTickCount64();
                        cls();
                    }
                }
                else if (taste == 73 || taste == 105) {
                    l_p_inventory(liz);
                    taste = 0;
                    lastEnemyMove = GetTickCount64();
                    cls();
                }
                else if (taste == 85 || taste == 117) {
                    l_p_AT(liz);
                    taste = 0;
                    lastEnemyMove = GetTickCount64();
                    cls();
                }
                else if (taste == 224) {
                    taste = _getch();
                    if (GetTickCount64() - lastPlayerMove >= spieler_geschwindigkeit) {
                        if (taste == 72 && liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h - 1] != 1) { liz->player.l_pos_h--; lastPlayerMove = GetTickCount64(); break; }
                        if (taste == 80 && liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h + 1] != 1) { liz->player.l_pos_h++; lastPlayerMove = GetTickCount64(); break; }
                        if (taste == 75 && liz->l_map[liz->player.l_pos_b - 1][liz->player.l_pos_h] != 1) { liz->player.l_pos_b--; lastPlayerMove = GetTickCount64(); break; }
                        if (taste == 77 && liz->l_map[liz->player.l_pos_b + 1][liz->player.l_pos_h] != 1) { liz->player.l_pos_b++; lastPlayerMove = GetTickCount64(); break; }
                    }
                    taste = 0;
                }
                else {
                    if (GetTickCount64() - lastPlayerMove >= spieler_geschwindigkeit) {
                        if ((taste == 'w' || taste == 'W') && liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h - 1] != 1) { liz->player.l_pos_h--; lastPlayerMove = GetTickCount64(); break; }
                        if ((taste == 's' || taste == 'S') && liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h + 1] != 1) { liz->player.l_pos_h++; lastPlayerMove = GetTickCount64(); break; }
                        if ((taste == 'a' || taste == 'A') && liz->l_map[liz->player.l_pos_b - 1][liz->player.l_pos_h] != 1) { liz->player.l_pos_b--; lastPlayerMove = GetTickCount64(); break; }
                        if ((taste == 'd' || taste == 'D') && liz->l_map[liz->player.l_pos_b + 1][liz->player.l_pos_h] != 1) { liz->player.l_pos_b++; lastPlayerMove = GetTickCount64(); break; }
                    }
                    taste = 0;
                }

            }

            Sleep(15);
        }

        if (liz->player.l_cur_vig[KF] < 1 || liz->player.l_cur_vig[KR] < 1) {
            int geladen = 0;
            l_AF("Erstes mal gestorben");
            while (1) {
                int wahl = l_death_scren();
                if (wahl == 0) {
                    if (l_spielstand_laden(liz) == 1) { geladen = 1; break; }
                    else { liz->l_ebenen = 0; return 0; }
                }
                else if (wahl == 1) {
                    liz->l_ebenen = 1;
                    return 1;
                }
                else {
                    liz->l_ebenen = 1;
                    return 0;
                }
            }
            if (geladen == 1) {
                artorias_getriggert = 0;
                artorias_m_index = -1;
                cls();
                continue;
            }
        }
        if (taste == 27) { liz->l_ebenen = 0; break; }

        if (taste == 200 && artorias_m_index >= 0) {
            int m_idx = artorias_m_index;
            l_bosskampf_artorias(liz, m_idx);
            if (liz->player.l_cur_vig[KF] > 0 && liz->player.l_cur_vig[KR] > 0) {
                liz->monster[m_idx].l_cur_vig = 0;
                l_AF("Erstes mal Monster getoetet");
                l_monster_drop(liz, m_idx);
                liz->player.l_runen += liz->monster[m_idx].runen;
                liz->l_ME[liz->monster[m_idx].pos_b][liz->monster[m_idx].pos_h] = 0;
                liz->monster[m_idx].l_id = 0;
            }
            else {
                liz->player.l_pos_b = liz->player.l_buffer_b;
                liz->player.l_pos_h = liz->player.l_buffer_h;
                lastEnemyMove = GetTickCount64();
            }
            cls();
            continue;
        }

        if (liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h] == 3) {
            liz->l_ebenen++;
            if (liz->l_ebenen == 4) {
                cls();
                printf("\n\n\n");
                l_PrCe("Du hast aus dem Labyrinth geschafft");
                system("pause");
                cls();
            }
            if (l_schwierigkeit == 1 && liz->l_ebenen == 4) { l_AF("Spiel durchgespielt in Einfach"); }
            else if (l_schwierigkeit == 2 && liz->l_ebenen == 4) { l_AF("Spiel durchgespielt in Mittel"); }
            else if (l_schwierigkeit == 3 && liz->l_ebenen == 4) { l_AF("Spiel durchgespielt in Schwer"); }
            else if (l_schwierigkeit == 4 && liz->l_ebenen == 4) { l_AF("Spiel durchgespielt in Hardcore"); }
            break;
        }
        if (liz->l_ME[liz->player.l_pos_b][liz->player.l_pos_h] < 0) {
            int map_wert = liz->l_ME[liz->player.l_pos_b][liz->player.l_pos_h];
            int m_index = -map_wert - 1;

            if (liz->monster[m_index].l_typ == 5) {
                l_truhe_oeffnen(liz, m_index);
                liz->l_ME[liz->player.l_pos_b][liz->player.l_pos_h] = 0;
                liz->monster[m_index].l_id = 0;
            }
            else {
                if (liz->l_ebenen == 3 && liz->monster[m_index].l_id == 30) {
                    l_bosskampf_artorias(liz, m_index);
                    if (liz->player.l_cur_vig[KF] > 0) { liz->monster[m_index].l_cur_vig = 0; }
                }
                else { l_battle(liz, m_index); }

                if (liz->monster[m_index].l_cur_vig <= 0) {
                    l_AF("Erstes mal Monster getoetet");
                    l_monster_drop(liz, m_index);
                    liz->player.l_runen += liz->monster[m_index].runen;
                    liz->l_ME[liz->player.l_pos_b][liz->player.l_pos_h] = 0;
                    liz->monster[m_index].l_id = 0;
                }
                else {
                    liz->player.l_pos_b = liz->player.l_buffer_b;
                    liz->player.l_pos_h = liz->player.l_buffer_h;
                    lastEnemyMove = GetTickCount64();
                    cls();
                    continue;
                }
            }
            cls();
        }
        if (liz->player.l_buffer_h != liz->player.l_pos_h || liz->player.l_buffer_b != liz->player.l_pos_b) {
            liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h] = 2;
            liz->l_map[liz->player.l_buffer_b][liz->player.l_buffer_h] = 0;
            liz->player.l_buffer_h = liz->player.l_pos_h;
            liz->player.l_buffer_b = liz->player.l_pos_b;
        }
        if (monster_darf_bewegen == 1) {
            for (int m = 0; m < MAX_MONSTER; m++) {
                if (liz->monster[m].l_id != 0) {
                    l_monster_intelligent(liz, m);
                }
            }
        }
    }
    return 0;
}

void l_vorbereitung(l_infozentrum* liz) {
    srand((unsigned)time(NULL));
    cls();
    liz->l_map = NULL;
    liz->l_ME = NULL;
    liz->l_MW = NULL;
    liz->l_ebenen = 1;

    // Map int zu int 2D ary
    liz->l_map = (int**)calloc(l_breite, sizeof(int*));
    if (liz->l_map == NULL) return;
    for (int i = 0; i < l_breite; i++) {
        liz->l_map[i] = (int*)calloc(l_hohe, sizeof(int));
        if (liz->l_map[i] == NULL) {
            l_freigeben(liz);
            return;
        }
    }

    // ME int zu int 2D ary
    liz->l_ME = (int**)calloc(l_breite, sizeof(int*));
    if (liz->l_ME == NULL) {
        l_freigeben(liz);
        return;
    }
    for (int i = 0; i < l_breite; i++) {
        liz->l_ME[i] = (int*)calloc(l_hohe, sizeof(int));
        if (liz->l_ME[i] == NULL) {
            l_freigeben(liz);
            return;
        }
    }

    // MW int zu int 2D ary
    liz->l_MW = (int**)calloc(l_breite, sizeof(int*));
    if (liz->l_MW == NULL) {
        l_freigeben(liz);
        return;
    }
    for (int i = 0; i < l_breite; i++) {
        liz->l_MW[i] = (int*)calloc(l_hohe, sizeof(int));
        if (liz->l_MW[i] == NULL) {
            l_freigeben(liz);
            return;
        }
    }

    // -------------------------------------------------------

    // Map erstellen
    l_map_erstellen(liz, 1);

    // -------------------------------------------------------

    l_iteam_samlung(liz);
    l_monster_samlung(liz);
    l_monster_spawnen(liz);
    l_truhe_spawnen(liz);

    // -------------------------------------------------------

    // Vorbereitung Ende
    memset(&liz->player, 0, sizeof(l_player));
    liz->player.l_level = 1;
    liz->player.inventar.slots[0] = liz->item_c[81];
    liz->player.inventar.slots[1] = liz->item_c[20];
    liz->player.inventar.slots[2] = liz->item_c[21];
    liz->player.inventar.slots[3] = liz->item_c[22];
    liz->player.inventar.slots[4] = liz->item_c[23];
    liz->player.inventar.slots[5] = liz->item_c[10];
    liz->player.inventar.slots[6] = liz->item_c[1];
    liz->player.inventar.slots[7] = liz->item_c[2];
    l_equip_item(&liz->player.inventar.l_waffe, &liz->player.inventar.slots[5]);
    l_equip_item(&liz->player.inventar.l_helm, &liz->player.inventar.slots[1]);
    l_equip_item(&liz->player.inventar.l_koerper, &liz->player.inventar.slots[2]);
    l_equip_item(&liz->player.inventar.l_hose, &liz->player.inventar.slots[3]);
    l_equip_item(&liz->player.inventar.l_schuhe, &liz->player.inventar.slots[4]);
    l_equip_item(&liz->player.inventar.quick[0], &liz->player.inventar.slots[6]);
    l_equip_item(&liz->player.inventar.quick[1], &liz->player.inventar.slots[7]);

    if (l_schwierigkeit == 1) {
        liz->player.l_runen = 2000;
        liz->player.l_vig = 8;
        liz->player.l_end = 6;
        liz->player.l_dex = 6;
        liz->player.l_str = 5;
    }
    else if (l_schwierigkeit == 2) {
        liz->player.l_runen = 1000;
        liz->player.l_vig = 5;
        liz->player.l_end = 4;
        liz->player.l_dex = 4;
        liz->player.l_str = 3;
    }
    else if (l_schwierigkeit == 3) {
        liz->player.l_runen = 300;
        liz->player.l_vig = 3;
        liz->player.l_end = 2;
        liz->player.l_dex = 2;
        liz->player.l_str = 2;
    }
    else if (l_schwierigkeit == 4) {
        liz->player.l_runen = 0;
        liz->player.l_vig = 1;
        liz->player.l_end = 1;
        liz->player.l_dex = 1;
        liz->player.l_str = 1;
    }

    l_berechne_spieler_werte(&liz->player);
    liz->player.l_pos_h = 1; liz->player.l_pos_b = 1;
    liz->player.l_buffer_h = liz->player.l_pos_h; liz->player.l_buffer_b = liz->player.l_pos_b;
    liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h] = 2;
    liz->player.l_cur_end = l_berechne_stamina(liz->player.l_end);
    for (int z = 0; z < ANZAHL_ZONEN; z++) liz->player.l_cur_vig[z] = liz->player.l_max_vig[z];

    // Debug
    if (l_debug) {
        // -------------------------------------------------------
        FILE* datei;
        errno_t err = fopen_s(&datei, "Game\\debug.dat", "w");
        if (err != 0) {
            printf("Fehler beim  oeffnen der Datei\n");
            system("pause");
        }
        else {
            fprintf(datei, "\t\t\t\t=========================================================\n");
            fprintf(datei, "\t\t\t\t                  >  Labyrinth normal  <                 \n");
            fprintf(datei, "\t\t\t\t=========================================================\n\n");

            for (int y = 0; y < l_hohe; y++) {
                fprintf(datei, "\n\t\t\t\t");
                for (int x = 0; x < l_breite; x++) {
                    if (liz->l_map[x][y] == 1) { fprintf(datei, "#"); }
                    else if (liz->l_map[x][y] == 2) { fprintf(datei, "X"); }
                    else if (liz->l_map[x][y] == 3) { fprintf(datei, "+"); }
                    else if (liz->l_ME[x][y] > 0) { fprintf(datei, "*"); }
                    else if (liz->l_ME[x][y] < 0 && liz->monster[-liz->l_ME[x][y] - 1].l_typ == 6) { fprintf(datei, "M"); }
                    else if (liz->l_ME[x][y] < 0 && liz->monster[-liz->l_ME[x][y] - 1].l_typ == 5) { fprintf(datei, "T"); }
                    else if (liz->l_ME[x][y] < 0) { fprintf(datei, "@"); }
                    else if (liz->l_map[x][y] != 1) { fprintf(datei, " "); }
                }
            }

            fprintf(datei, "\n\n\n\n");
            fprintf(datei, "\t\t\t\t=========================================================\n");
            fprintf(datei, "\t\t\t\t                   >  Labyrinth debug  <                 \n");
            fprintf(datei, "\t\t\t\t=========================================================\n");
            fprintf(datei, "\t\t\t\tDie Karte wurde um 90 grad Geneigt um es lesbar zu machen\n\n");

            for (int x = 0; x < l_breite; x++) {
                fprintf(datei, "\n");
                for (int y = 0; y < l_hohe; y++) {
                    if (liz->l_map[x][y] == 1) { fprintf(datei, "[ #### ]"); }
                    else if (liz->l_map[x][y] == 2) { fprintf(datei, "[ XXXX ]"); }
                    else if (liz->l_map[x][y] == 3) { fprintf(datei, "[ ++++ ]"); }
                    else if (liz->l_ME[x][y] > 0) { fprintf(datei, "[ **** ]"); }
                    else if (liz->l_ME[x][y] < 0 && liz->monster[-liz->l_ME[x][y] - 1].l_typ == 6) { fprintf(datei, "[ Mimi ]"); }
                    else if (liz->l_ME[x][y] < 0 && liz->monster[-liz->l_ME[x][y] - 1].l_typ == 5) { fprintf(datei, "[ TRUH ]"); }
                    else if (liz->l_ME[x][y] < 0) { fprintf(datei, "[ @@@@ ]"); }
                    else { fprintf(datei, "[ %4d ]", liz->l_MW[x][y]); }
                }
            }

            fclose(datei);
        }
        // -------------------------------------------------------
    }

}
void l_map_erstellen(l_infozentrum* liz, int type) {
    memset(liz->monster, 0, sizeof(liz->monster));
wiederholen:
    // Map Erstellen: Vorbereitung
    if (type != 3) {
        for (int x = 0; x < l_breite; x++) {
            for (int y = 0; y < l_hohe; y++) {
                liz->l_map[x][y] = 1;
                liz->l_ME[x][y] = 0;
            }
        }
    }
    else {
        for (int x = 0; x < l_breite; x++) {
            for (int y = 0; y < l_hohe; y++) {
                if (x == 0 || y == 0 || x == l_breite - 1 || y == l_hohe - 1) { liz->l_map[x][y] = 1; }
                else { liz->l_map[x][y] = 0; }
                liz->l_ME[x][y] = 0;
            }
        }
    }

    if (type == 1) {
        // Map Erstellen: Erstellen
        int max_cells = l_breite * l_hohe;
        int* stack_b = (int*)malloc(max_cells * sizeof(int));
        if (stack_b == NULL) return;
        int* stack_h = (int*)malloc(max_cells * sizeof(int));
        if (stack_h == NULL) { free(stack_b); return; }

        liz->player.l_pos_h = 1; liz->player.l_pos_b = 1;
        liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h] = 0;
        stack_b[0] = liz->player.l_pos_b; stack_h[0] = liz->player.l_pos_h;
        int top = 1;

        while (top > 0) {

            int curr_b = stack_b[top - 1]; int curr_h = stack_h[top - 1];

            int kn_o = (curr_h - 2 > 0) && (liz->l_map[curr_b][curr_h - 2] == 1);
            int kn_u = (curr_h + 2 < l_hohe - 1) && (liz->l_map[curr_b][curr_h + 2] == 1);
            int kn_l = (curr_b - 2 > 0) && (liz->l_map[curr_b - 2][curr_h] == 1);
            int kn_r = (curr_b + 2 < l_breite - 1) && (liz->l_map[curr_b + 2][curr_h] == 1);

            if (kn_o || kn_u || kn_l || kn_r) {

                int NOSW[4] = { 1, 2, 3, 4 };
                for (int i = 0; i < 4; i++) {
                    int r = rand() % 4;
                    int temp = NOSW[i];
                    NOSW[i] = NOSW[r];
                    NOSW[r] = temp;
                }

                for (int i = 0; i < 4; i++) {
                    int rt = NOSW[i];
                    int erfolg = 0;

                    if (rt == 1 && kn_o) {
                        liz->l_map[curr_b][curr_h - 1] = 0;
                        liz->l_map[curr_b][curr_h - 2] = 0;
                        liz->player.l_pos_h = curr_h - 2;
                        erfolg = 1;
                    }
                    else if (rt == 2 && kn_r) {
                        liz->l_map[curr_b + 1][curr_h] = 0;
                        liz->l_map[curr_b + 2][curr_h] = 0;
                        liz->player.l_pos_b = curr_b + 2;
                        erfolg = 1;
                    }
                    else if (rt == 3 && kn_u) {
                        liz->l_map[curr_b][curr_h + 1] = 0;
                        liz->l_map[curr_b][curr_h + 2] = 0;
                        liz->player.l_pos_h = curr_h + 2;
                        erfolg = 1;
                    }
                    else if (rt == 4 && kn_l) {
                        liz->l_map[curr_b - 1][curr_h] = 0;
                        liz->l_map[curr_b - 2][curr_h] = 0;
                        liz->player.l_pos_b = curr_b - 2;
                        erfolg = 1;
                    }

                    if (erfolg) {
                        stack_b[top] = liz->player.l_pos_b;
                        stack_h[top] = liz->player.l_pos_h;
                        top++;
                        break;
                    }
                }
            }
            else {
                top--;
                if (top > 0) {
                    liz->player.l_pos_b = stack_b[top - 1];
                    liz->player.l_pos_h = stack_h[top - 1];
                }
            }
        }
        free(stack_b);
        free(stack_h);

        // Map Erstellen: Die rest
        for (int x = 1; x < l_breite - 1; x++) {
            for (int y = 1; y < l_hohe - 1; y++) {
                if (liz->l_map[x][y] == 1) {
                    int weg_horizontal = (liz->l_map[x - 1][y] == 0 && liz->l_map[x + 1][y] == 0);
                    int weg_vertikal = (liz->l_map[x][y - 1] == 0 && liz->l_map[x][y + 1] == 0);
                    if (weg_horizontal || weg_vertikal) {
                        if (rand() % 15 == 0) {
                            liz->l_map[x][y] = 0;
                        }
                    }
                }
            }
        }
    }
    if (type == 2) {
        // Map Erstellen: Erstellen
        int ende = 1;
        liz->player.l_pos_h = 1;
        liz->player.l_pos_b = 1;
        liz->player.l_buffer_b = 0;
        liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h] = 0;
        while ((l_breite * l_breite * l_breite) > ende) {
            ende = ende + 1;
            int NOSW[4] = { 1, 2, 3, 4 };
            for (int i = 0; i < 4; i++) {
                int r = rand() % 4;
                int temp = NOSW[i];
                NOSW[i] = NOSW[r];
                NOSW[r] = temp;
            }

            for (int i = 0; i < 4; i++) {
                int rt = NOSW[i];
                if ((rt == 1) && (liz->player.l_pos_h - 2) > 0) {
                    if (liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h - 2] == 1) {
                        liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h - 1] = 0;
                        liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h - 2] = 0;
                        liz->player.l_pos_h = (liz->player.l_pos_h - 2);
                    }
                }
                else if ((rt == 2) && (liz->player.l_pos_b + 2) < (l_breite - 1)) {
                    if (liz->l_map[liz->player.l_pos_b + 2][liz->player.l_pos_h] == 1) {
                        liz->l_map[liz->player.l_pos_b + 1][liz->player.l_pos_h] = 0;
                        liz->l_map[liz->player.l_pos_b + 2][liz->player.l_pos_h] = 0;
                        liz->player.l_pos_b = (liz->player.l_pos_b + 2);
                    }
                }
                else if ((rt == 3) && (liz->player.l_pos_h + 2) < (l_hohe - 1)) {
                    if ((rt == 3) && liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h + 2] == 1) {
                        liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h + 1] = 0;
                        liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h + 2] = 0;
                        liz->player.l_pos_h = (liz->player.l_pos_h + 2);
                    }
                }
                else if ((rt == 4) && (liz->player.l_pos_b - 2) > 0) {
                    if ((rt == 4) && liz->l_map[liz->player.l_pos_b - 2][liz->player.l_pos_h] == 1) {
                        liz->l_map[liz->player.l_pos_b - 1][liz->player.l_pos_h] = 0;
                        liz->l_map[liz->player.l_pos_b - 2][liz->player.l_pos_h] = 0;
                        liz->player.l_pos_b = (liz->player.l_pos_b - 2);
                    }
                }
            }
            int kn_o = (liz->player.l_pos_h - 2 > 0) && (liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h - 2] == 1);
            int kn_u = (liz->player.l_pos_h + 2 < l_hohe - 1) && (liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h + 2] == 1);
            int kn_l = (liz->player.l_pos_b - 2 > 0) && (liz->l_map[liz->player.l_pos_b - 2][liz->player.l_pos_h] == 1);
            int kn_r = (liz->player.l_pos_b + 2 < l_breite - 1) && (liz->l_map[liz->player.l_pos_b + 2][liz->player.l_pos_h] == 1);
            if (!kn_o && !kn_u && !kn_l && !kn_r) {
                liz->player.l_buffer_b = 1;
            }
            if (liz->player.l_buffer_b == 1) {
                if ((liz->player.l_pos_h - 2 > 0) && (liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h - 2] == 0)) {
                    liz->player.l_pos_h = (liz->player.l_pos_h - 2);
                }
                else if ((liz->player.l_pos_b + 2 < l_breite - 1) && (liz->l_map[liz->player.l_pos_b + 2][liz->player.l_pos_h] == 0)) {
                    liz->player.l_pos_b = (liz->player.l_pos_b + 2);
                }
                else if ((liz->player.l_pos_h + 2 < l_hohe - 1) && (liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h + 2] == 0)) {
                    liz->player.l_pos_h = (liz->player.l_pos_h + 2);
                }
                else if ((liz->player.l_pos_b - 2 > 0) && (liz->l_map[liz->player.l_pos_b - 2][liz->player.l_pos_h] == 0)) {
                    liz->player.l_pos_b = (liz->player.l_pos_b - 2);
                }

                liz->player.l_buffer_b = 0;
            }
        }

        // Map Erstellen: Die rest
        for (int x = 1; x < l_breite - 1; x++) {
            for (int y = 1; y < l_hohe - 1; y++) {
                if (liz->l_map[x][y] == 1) {
                    int weg_horizontal = (liz->l_map[x - 1][y] == 0 && liz->l_map[x + 1][y] == 0);
                    int weg_vertikal = (liz->l_map[x][y - 1] == 0 && liz->l_map[x][y + 1] == 0);
                    if (weg_horizontal || weg_vertikal) {
                        if (rand() % 15 == 0) {
                            liz->l_map[x][y] = 0;
                        }
                    }
                }
            }
        }
    }
    else if (type == 3) {
        // Hindernis Erstellen: Erstellen
        int hindernis_anzahl = (l_breite * l_hohe) / 50;
        for (int i = 0; i < hindernis_anzahl; i++) {
            int groesse = rand() % 9 + 4;
            int curr_x, curr_y;

            while (1) {
                curr_x = rand() % (l_breite - 2) + 1;
                curr_y = rand() % (l_hohe - 2) + 1;
                if (liz->l_map[curr_x][curr_y] == 0) {
                    liz->l_map[curr_x][curr_y] = 4;
                    break;
                }
            }

            for (int j = 1; j < groesse; j++) {
                int platziert = 0;

                for (int versuch = 0; versuch < 10; versuch++) {
                    int dir = rand() % 4;
                    int nx = curr_x;
                    int ny = curr_y;

                    if (dir == 0) ny--;
                    else if (dir == 1) nx++;
                    else if (dir == 2) ny++;
                    else if (dir == 3) nx--;

                    if (nx > 0 && nx < l_breite - 1 && ny > 0 && ny < l_hohe - 1) {
                        if (liz->l_map[nx][ny] == 0) {
                            liz->l_map[nx][ny] = 1;
                            curr_x = nx;
                            curr_y = ny;
                            platziert = 1;
                            break;
                        }
                    }
                }
                if (!platziert) break;
            }
        }
    }
    // MW Erstellen: Vorbereitung
    liz->player.l_pos_h = 1; liz->player.l_pos_b = 1;
    liz->l_map[liz->player.l_pos_b][liz->player.l_pos_h] = 2;
    for (int x = 0; x < l_breite; x++) {
        for (int y = 0; y < l_hohe; y++) {
            if (liz->l_map[x][y] == 2) {
                liz->l_MW[x][y] = 0;
            }
            else {
                liz->l_MW[x][y] = -1;
            }
        }
    }

    // MW Erstellen: Erstellen
    int changed = 1;
    int aktuelle_zahl = 0;
    while (changed) {
        changed = 0;

        for (int x = 0; x < l_breite; x++) {
            for (int y = 0; y < l_hohe; y++) {
                if (liz->l_MW[x][y] == aktuelle_zahl) {
                    if (y - 1 >= 0 && liz->l_map[x][y - 1] == 0 && liz->l_MW[x][y - 1] == -1) {
                        liz->l_MW[x][y - 1] = aktuelle_zahl + 1;
                        changed = 1;
                    }
                    if (y + 1 < l_hohe && liz->l_map[x][y + 1] == 0 && liz->l_MW[x][y + 1] == -1) {
                        liz->l_MW[x][y + 1] = aktuelle_zahl + 1;
                        changed = 1;
                    }
                    if (x - 1 >= 0 && liz->l_map[x - 1][y] == 0 && liz->l_MW[x - 1][y] == -1) {
                        liz->l_MW[x - 1][y] = aktuelle_zahl + 1;
                        changed = 1;
                    }
                    if (x + 1 < l_breite && liz->l_map[x + 1][y] == 0 && liz->l_MW[x + 1][y] == -1) {
                        liz->l_MW[x + 1][y] = aktuelle_zahl + 1;
                        changed = 1;
                    }
                }
            }
        }
        aktuelle_zahl++;
    }

    // MW Erstellen: Die rest
    int l_MW_end = -1;
    liz->player.l_buffer_b = 1; liz->player.l_buffer_h = 1;
    for (int x = 0; x < l_breite - 1; x++) {
        for (int y = 0; y < l_hohe - 1; y++) {
            if (liz->l_MW[x][y] > l_MW_end) {
                l_MW_end = liz->l_MW[x][y];
                liz->player.l_buffer_h = y;
                liz->player.l_buffer_b = x;
            }
        }
    }
    if (l_MW_end < 10) { goto wiederholen; }
    liz->l_map[liz->player.l_buffer_b][liz->player.l_buffer_h] = 3;
    liz->l_mw_max = l_MW_end;
    liz->l_exit_b = liz->player.l_buffer_b;
    liz->l_exit_h = liz->player.l_buffer_h;
    if (l_debug) liz->l_map[1][3] = 3;
}
void l_berechne_mw_max(l_infozentrum* liz) {
    int mw_max = 0;
    int exit_b = 1, exit_h = 1;
    for (int x = 0; x < l_breite; x++) {
        for (int y = 0; y < l_hohe; y++) {
            if (liz->l_MW[x][y] > mw_max) {
                mw_max = liz->l_MW[x][y];
                exit_b = x;
                exit_h = y;
            }
        }
    }
    liz->l_mw_max = mw_max;
    liz->l_exit_b = exit_b;
    liz->l_exit_h = exit_h;
}
int  l_death_scren() {
    int taste;
    int wahl = 0;
    const char* menu[] = {
        " Load Game",
        " Restart",
        " Exit"
    };
    int size = sizeof(menu) / sizeof(menu[0]);
    while (1) {
        cls();
        printf("\n\n\n\t\t\t\t ###   ###  #   # #####     ###  #   # ##### #### \n");
        printf("\t\t\t\t#     #   # ## ## #        #   # #   # #     #   #\n");
        printf("\t\t\t\t#  ## ##### # # # ####     #   # #   # ####  #### \n");
        printf("\t\t\t\t#   # #   # #   # #        #   #  # #  #     #  # \n");
        printf("\t\t\t\t ###  #   # #   # #####     ###    #   ##### #   #\n");
        printf("\t\t\t\t--------------------------------------------------\n\n");
        for (int i = 0; i < size; i++) {
            if (i == wahl) printf("\t\t\t\t\t\t\033[1;37m%s\033[0m\n", menu[i]);
            else            printf("\t\t\t\t\t\t \033[90;40m%s\033[0m\n", menu[i]);
        }
        taste = _getch();
        if (taste == 224) {
            taste = _getch();
            if (taste == 72 && wahl > 0)        wahl--;
            if (taste == 80 && wahl < size - 1) wahl++;
        }
        else if (taste == 13 || taste == ' ') { return wahl; }
        else {
            if ((taste == 'w' || taste == 'W') && wahl > 0)        wahl--;
            if ((taste == 's' || taste == 'S') && wahl < size - 1) wahl++;
            int e_gz = taste - '1';
            if (e_gz >= 0 && e_gz < size) { wahl = e_gz; }
        }
    }
}
void l_freigeben(l_infozentrum* liz) {

    if (liz->l_map != NULL) {
        for (int i = 0; i < l_breite; i++) free(liz->l_map[i]);
        free(liz->l_map); liz->l_map = NULL;
    }
    if (liz->l_ME != NULL) {
        for (int i = 0; i < l_breite; i++) free(liz->l_ME[i]);
        free(liz->l_ME); liz->l_ME = NULL;
    }
    if (liz->l_MW != NULL) {
        for (int i = 0; i < l_breite; i++) free(liz->l_MW[i]);
        free(liz->l_MW); liz->l_MW = NULL;
    }

}

void l_p_AT(l_infozentrum* liz) {
    int wahl = 0;
    const int anzahl = 4;
    int taste;
    const char* namen[] = { "Vigor", "Endurance", "Dexterity", "Strength" };

    while (1) {
        cls();

        l_player* p = &liz->player;
        l_player vor = *p;

        int* attrs_cur[4] = { &p->l_vig,   &p->l_end,   &p->l_dex,   &p->l_str };
        int* attrs_vor[4] = { &vor.l_vig,  &vor.l_end,  &vor.l_dex,  &vor.l_str };

        (*attrs_vor[wahl])++;
        vor.l_level = p->l_level + 1;
        l_berechne_spieler_werte(&vor);

        int kosten = l_berechne_runen_kosten(p->l_level);
        int kann = (p->l_runen >= kosten);

        int hp_a = l_berechne_hp(p->l_vig), hp_v = l_berechne_hp(vor.l_vig);
        int st_a = l_berechne_stamina(p->l_end), st_v = l_berechne_stamina(vor.l_end);
        int tr_a = l_berechne_treffer(p->l_dex), tr_v = l_berechne_treffer(vor.l_dex);
        int sc_a = l_berechne_schaden(p->l_str), sc_v = l_berechne_schaden(vor.l_str);

        const char* bw_n[4] = { "HP:", "Ausdauer:", "Treffer:", "Schaden:" };
        int bw_a[4] = { hp_a, st_a, tr_a, sc_a };
        int bw_v[4] = { hp_v, st_v, tr_v, sc_v };
        int bw_pct[4] = { 0, 0, 1, 0 };

        const char* kt_n[10] = { "Kopf", "Koerp.", "Arm L", "Arm R", "Hand L", "Hand R", "Bein L", "Bein R", "Fuss L", "Fuss R" };
        int kt_a[10], kt_v[10];
        for (int z = 0; z < ANZAHL_ZONEN; z++) {
            kt_a[z] = p->l_max_vig[z];
            kt_v[z] = vor.l_max_vig[z];
        }

        printf("\n");
        printf(" +----------------------------------+ +------------------------------+ +------------------------------+\n");
        printf(" | %-32s | | %-28s | | %-28s |\n", "Level Up", "Basis Werte", "Koerperteile");
        printf(" +----------------------------------+ +------------------------------+ +------------------------------+\n");

        for (int r = 1; r <= 15; r++) {

            // ================= SPALTE 1 (Level Up - 32 Zeichen breit) =================
            printf(" | ");
            if (r == 1) {
                printf("Level         %4d => ", p->l_level);
                printf("\033[1;33m%4d\033[0m      ", vor.l_level);
            }
            else if (r == 2) {
                int rest = kann ? (p->l_runen - kosten) : p->l_runen;
                printf("Runen      %7d => ", p->l_runen);
                if (kann) printf("\033[1;33m%7d\033[0m   ", rest);
                else      printf("%7d   ", rest);
            }
            else if (r == 4) { printf("Attribute                       "); }
            else if (r == 5) { printf("--------------------------------"); }
            else if (r >= 6 && r <= 9) {
                int idx = r - 6;
                int ac = *attrs_cur[idx];
                int av = *attrs_vor[idx];
                if (idx == wahl) printf("\033[1;37m->\033[0m %-10s %4d => ", namen[idx], ac);
                else             printf("   %-10s %4d => ", namen[idx], ac);

                if (av != ac) printf("\033[1;33m%4d\033[0m      ", av);
                else          printf("%4d      ", av);
            }
            else if (r == 11) {
                if (kann) printf("Kosten:   \033[1;33m%7d\033[0m Runen         ", kosten);
                else      printf("Kosten:   \033[1;31m%7d\033[0m (Zu wenig)    ", kosten);
            }
            else if (r == 13) { printf("\033[90m[W/S / Pfeile]\033[0m Attribut         "); }
            else if (r == 14) { printf("\033[90m[ENTER / SPACE]\033[0m Kaufen          "); }
            else if (r == 15) { printf("\033[90m[ESC]\033[0m Zurueck                   "); }
            else { printf("                                "); }
            printf(" | | ");


            // ================= SPALTE 2 (Basis Werte - 28 Zeichen breit) =================
            if (r >= 1 && r <= 4) {
                int idx = r - 1;
                if (bw_pct[idx]) {
                    printf("%-10s  %3d%% => ", bw_n[idx], bw_a[idx]);
                    if (bw_v[idx] != bw_a[idx]) printf("\033[1;33m%3d%%\033[0m    ", bw_v[idx]);
                    else                        printf("%3d%%    ", bw_v[idx]);
                }
                else {
                    printf("%-10s  %4d => ", bw_n[idx], bw_a[idx]);
                    if (bw_v[idx] != bw_a[idx]) printf("\033[1;33m%4d\033[0m    ", bw_v[idx]);
                    else                        printf("%4d    ", bw_v[idx]);
                }
            }
            else { printf("                            "); }
            printf(" | | ");


            // ================= SPALTE 3 (Koerperteile - 28 Zeichen) =================
            if (r >= 1 && r <= 10) {
                int idx = r - 1;
                printf("%-8s    %4d => ", kt_n[idx], kt_a[idx]);
                if (kt_v[idx] != kt_a[idx]) printf("\033[1;33m%4d\033[0m    ", kt_v[idx]);
                else                        printf("%4d    ", kt_v[idx]);
            }
            else { printf("                            "); }
            printf(" |\n");
        }

        printf(" +----------------------------------+ +------------------------------+ +------------------------------+\n\n");

        taste = _getch();
        if (taste == 27) break;

        if (taste == 224) {
            taste = _getch();
            if (taste == 72 && wahl > 0)        wahl--;
            if (taste == 80 && wahl < anzahl - 1) wahl++;
        }
        else if (taste == 13 || taste == ' ') {
            if (kann) {
                p->l_runen -= kosten;
                (*attrs_cur[wahl])++;
                p->l_level++;
                l_berechne_spieler_werte(p);

                for (int z = 0; z < ANZAHL_ZONEN; z++) p->l_cur_vig[z] = p->l_max_vig[z];
            }
        }
        else {
            if ((taste == 'w' || taste == 'W') && wahl > 0)        wahl--;
            if ((taste == 's' || taste == 'S') && wahl < anzahl - 1) wahl++;
        }
    }
}
int  l_berechne_hp(int vig) {
    if (vig < 1) vig = 1;

    switch (l_schwierigkeit) {
    case 1:
        if (vig <= 20) return 400 + (vig - 1) * 26;
        if (vig <= 40) return 894 + (vig - 20) * 22;
        if (vig <= 60) return 1334 + (vig - 40) * 10;
        return               1534 + (vig - 60) * 6;

    case 2:
        if (vig <= 20) return 300 + (vig - 1) * 22;
        if (vig <= 40) return 718 + (vig - 20) * 19;
        if (vig <= 60) return 1098 + (vig - 40) * 7;
        return               1238 + (vig - 60) * 4;

    case 3:
        if (vig <= 20) return 250 + (vig - 1) * 18;
        if (vig <= 40) return 592 + (vig - 20) * 14;
        if (vig <= 60) return 872 + (vig - 40) * 5;
        return               972 + (vig - 60) * 2;

    case 4:
        if (vig <= 20) return 200 + (vig - 1) * 14;
        if (vig <= 40) return 466 + (vig - 20) * 9;
        if (vig <= 60) return 646 + (vig - 40) * 3;
        return               706 + (vig - 60) * 1;

    default: return 0;
    }
}
int  l_berechne_stamina(int end) {
    if (end < 1) end = 1;

    switch (l_schwierigkeit) {
    case 1:
        if (end <= 40) return 100 + (end - 1) * 3;
        return                217 + (end - 40) / 3;

    case 2:
        if (end <= 40) return 80 + (end - 1) * 2;
        return                158 + (end - 40) / 5;

    case 3:
        if (end <= 40) return 70 + (end - 1) * 1;
        return                109 + (end - 40) / 8;

    case 4:
        if (end <= 30) return 60 + (end - 1) * 1;
        if (end <= 50) return 89 + (end - 30) / 3;
        return                95 + (end - 50) / 15;

    default: return 0;
    }
}
int  l_berechne_treffer(int dex) {
    if (dex < 1) dex = 1;

    switch (l_schwierigkeit) {
    case 1:
        if (dex <= 40) { int t = 65 + (dex - 1); return t > 95 ? 95 : t; }
        return 95;

    case 2: {
        if (dex <= 40) return 55 + (dex - 1);
        int t = 94 + (dex - 40) / 5;
        return t > 95 ? 95 : t;
    }
    case 3:
        if (dex <= 40) return 45 + (dex - 1) * 1;
        return                84 + (dex - 40) / 8;

    case 4:
        if (dex <= 30) return 35 + (dex - 1) * 1;
        return                64 + (dex - 30) / 12;

    default: return 0;
    }
}
int  l_berechne_schaden(int str) {
    if (str < 1) str = 1;

    switch (l_schwierigkeit) {
    case 1:
        if (str <= 40) return 15 + (str - 1) * 4;
        if (str <= 60) return 171 + (str - 40) * 2;
        return                211 + (str - 60) / 1;

    case 2:
        if (str <= 40) return 10 + (str - 1) * 3;
        if (str <= 60) return 127 + (str - 40) * 1;
        return                147 + (str - 60) / 3;

    case 3:
        if (str <= 40) return 8 + (str - 1) * 2;
        if (str <= 60) return 86 + (str - 40) * 1;
        return               106 + (str - 60) / 6;

    case 4:
        if (str <= 40) return 5 + (str - 1) * 1;
        if (str <= 60) return 44 + (str - 40) / 2;
        return                54 + (str - 60) / 12;

    default: return 0;
    }
}
int  l_berechne_runen_kosten(int level) {
    if (level < 1) level = 1;
    double l = (double)level;

    int kosten = (int)(0.02 * l * l * l + 3.06 * l * l + 105.6 * l - 895.0);
    if (kosten < 673) kosten = 673;

    switch (l_schwierigkeit) {
    case 1: return (int)(kosten * 0.75);
    case 2: return kosten;
    case 3: return (int)(kosten * 1.40);
    case 4: return (int)(kosten * 2.00);
    default: return kosten;
    }
}
void l_berechne_spieler_werte(l_player* p) {
    int hp = l_berechne_hp(p->l_vig);
    p->l_max_vig[KF] = (hp * 10) / 100;
    p->l_max_vig[KR] = (hp * 30) / 100;
    p->l_max_vig[AL] = (hp * 12) / 100;
    p->l_max_vig[AR] = (hp * 12) / 100;
    p->l_max_vig[HL] = (hp * 5) / 100;
    p->l_max_vig[HR] = (hp * 5) / 100;
    p->l_max_vig[BL] = (hp * 10) / 100;
    p->l_max_vig[BR] = (hp * 10) / 100;
    p->l_max_vig[FL] = (hp * 3) / 100;
    p->l_max_vig[FR] = (hp * 3) / 100;
}
int  l_eff_stat(l_player* p, int stat_typ) {
    int basis = 0;
    switch (stat_typ) {
    case 0: basis = p->l_vig; break;
    case 1: basis = p->l_end; break;
    case 2: basis = p->l_dex; break;
    case 3: basis = p->l_str; break;
    }

    // Alle ausger steten Ringe addieren
    for (int i = 0; i < MAX_SLOT; i++) {
        if (p->inventar.buff[i].l_id == 0) continue;
        l_effekte* e = &p->inventar.buff[i].effekte;
        switch (stat_typ) {
        case 0: basis += e->l_bonus_vig; break;
        case 1: basis += e->l_bonus_end; break;
        case 2: basis += e->l_bonus_dex; break;
        case 3: basis += e->l_bonus_str; break;
        }
    }
    return basis;
}

int  l_ESP_menu(l_infozentrum* liz) {
    int wahl = 0;
    while (1) {
        const char* menu[6] = {
        " Weiterspielen",
        " Level Up",
        " Equipment",
        " Inventory",
        " Verlassen"
        };
        int menu_groesse = 5;
        if (l_schwierigkeit != 4) {
            menu[5] = menu[4];
            menu[4] = " Spielstand Speichern";
            menu_groesse = 6;
        }
        const char* menu_name[] = { "Menu" };
        wahl = menu_c(wahl, menu_groesse, menu, menu_name);
        if (l_schwierigkeit == 4) {
            if (wahl == 0) { return 0; }
            else if (wahl == 1) { l_p_AT(liz); }
            else if (wahl == 2) { l_equipment(liz); }
            else if (wahl == 3) { l_p_inventory(liz); }
            else if (wahl == 4) { return 27; }
        }
        else {
            if (wahl == 0) { return 0; }
            else if (wahl == 1) { l_p_AT(liz); }
            else if (wahl == 2) { l_equipment(liz); }
            else if (wahl == 3) { l_p_inventory(liz); }
            else if (wahl == 4) { l_spielstand_speichern(liz); return 0; }
            else if (wahl == 5) { return 27; }
        }
    }
}
void l_p_inventory(l_infozentrum* liz) {
    const char* kat_namen[] = { "Alle", "Verbrauch", "Buff", "Waffe", "Ruestung", "Sonstiges" };
    int kat_typen[6][5] = {
        { -1,              0,              0,          0,           0 }, // Alle
        { TYP_VERBRAUCH,   0,              0,          0,           0 }, // Verbrauch
        { TYP_BUFF,        0,              0,          0,           0 }, // Buff
        { TYP_SCHWERT,     TYP_GROSSSCHWERT, TYP_MESSER, TYP_BOGEN, 0 }, // Waffe
        { TYP_HELM,        TYP_RUESTUNG,   TYP_HOSE,   TYP_SCHUHE,  0 }, // Ruestung
        { TYP_GEGENSTAND,  0,              0,          0,           0 }, // Sonstiges
    };
    const int KAT_ANZ = 6;
    const int LISTE_MAX = 11;

    int kat = 0;
    int kat_alt = -1;
    int wahl = 0;
    int von = 0;
    int taste;

    l_player* p = &liz->player;
    l_inventar* inv = &p->inventar;

    l_item* gefiltert[INV_GROESSE];
    int     gef_anz = 0;

    const char* typ_namen[] = {
        "Leer", "Buff", "Verbrauch", "Gegenstand",
        "Grossschwert", "Schwert", "Messer", "Bogen",
        "Helm", "Ruestung", "Hose", "Schuhe"
    };

    while (1) {
        // --- 1. FILTER ITEMS BY CATEGORY ---
        if (kat != kat_alt) {
            gef_anz = 0; wahl = 0; von = 0; kat_alt = kat;
            for (int i = 0; i < INV_GROESSE; i++) {
                if (inv->slots[i].l_id == 0) continue;

                int typ = inv->slots[i].l_typ;

                if (kat_typen[kat][0] == -1) {
                    gefiltert[gef_anz++] = &inv->slots[i];
                }
                else {
                    for (int j = 0; j < 5 && kat_typen[kat][j] != 0; j++) {
                        if (kat_typen[kat][j] == typ) {
                            gefiltert[gef_anz++] = &inv->slots[i];
                            break;
                        }
                    }
                }
            }
        }

        // --- 2. SCROLLING LOGIC ---
        if (gef_anz > 0) {
            if (wahl < von)                  von = wahl;
            if (wahl >= von + LISTE_MAX)     von = wahl - LISTE_MAX + 1;
        }

        l_item* sel = (gef_anz > 0 && wahl < gef_anz) ? gefiltert[wahl] : NULL;

        // --- 3. PREPARE PLAYER STATS ---
        int hp_cur = 0, hp_max = 0;
        for (int z = 0; z < ANZAHL_ZONEN; z++) {
            hp_cur += p->l_cur_vig[z];
            hp_max += p->l_max_vig[z];
        }

        char desc[4][27];
        for (int i = 0; i < 4; i++) {
            memset(desc[i], 0, 27);
            if (!sel) continue;
            int off = i * 26;
            int dlen = (int)strlen(sel->l_beschreibung);
            if (off >= dlen) continue;
            strncpy_s(desc[i], 27, sel->l_beschreibung + off, 26);
        }

        cls();

        printf("\n");
        printf(" +----------------------+ +----------------------------+ +----------------------+\n");
        printf(" | \033[1;37m%-20s\033[0m | | %-26.26s | | %-20s |\n", "Inventar", sel ? sel->l_name : "", "Spieler Status");
        printf(" +----------------------+ +----------------------------+ +----------------------+\n");

        printf(" | ");
        if (kat > 0) printf("\033[90m<\033[0m ");
        else         printf("  ");

        printf("\033[1;33m%-17.17s\033[0m", kat_namen[kat]);

        if (kat < KAT_ANZ - 1) printf("\033[90m>\033[0m ");
        else                   printf("  ");

        printf("| | %-26s | | %-20s |\n", "", "");

        printf(" | %-20s | | %-26s | | %-20s |\n", "--------------------", "--------------------------", "--------------------");

        // Main List Body
        for (int r = 0; r < LISTE_MAX; r++) {
            printf(" | ");

            // Left Column: Item List
            int idx = von + r;
            if (idx < gef_anz) {
                l_item* it = gefiltert[idx];
                char entry[21];
                if (it->l_menge > 1) sprintf_s(entry, 21, "%-13.13s x%-4d", it->l_name, it->l_menge);
                else sprintf_s(entry, 21, "%-19.19s", it->l_name);

                if (idx == wahl) printf("\033[1;37m->%-18.18s\033[0m", entry);
                else printf("  %-18.18s", entry);
            }
            else { printf("%-20s", ""); }

            printf(" | | ");

            // Middle Column: Item Details
            if (sel) {
                switch (r) {
                case 0: {
                    const char* tn = (sel->l_typ >= 0 && sel->l_typ <= 11) ? typ_namen[sel->l_typ] : "?";
                    printf("Typ:      %-16.16s", tn);
                    break;
                }
                case 1: printf("Staerke:  %-16d", sel->l_staerke);    break;
                case 2: printf("Menge:    %-16d", sel->l_menge);      break;
                case 3: printf("Haltbar.: %-16d", sel->l_haltbarkeit); break;
                case 4: printf("%-26s", ""); break;
                case 5: printf("\033[90m%-26s\033[0m", "--- Elementar ---"); break;
                case 6: printf("  Feuer:%-5d  Frost:%-5d", sel->elemente.feuer, sel->elemente.frost); break;
                case 7: printf("  Blitz:%-5d  Gift: %-5d", sel->elemente.blitz, sel->elemente.gift); break;
                case 8: printf("%-26s", ""); break;
                case 9:  printf("\033[90m%-26s\033[0m", "--- Beschreibung ---"); break;
                case 10: printf("%-26.26s", desc[0]); break;
                default: printf("%-26s", ""); break;
                }
            }
            else {
                if (r == 5) printf("%-26s", "(Inventar leer)");
                else        printf("%-26s", "");
            }

            printf(" | | ");

            // Right Column: Player Stats
            switch (r) {
            case 0:  printf("Level:    %10d", p->l_level);                    break;
            case 1:  printf("Runen:    %10d", p->l_runen);                    break;
            case 2:  printf("%-20s", "");                                     break;
            case 3:  printf("Vigor:      %8d", p->l_vig);                     break;
            case 4:  printf("Endurance:  %8d", p->l_end);                     break;
            case 5:  printf("Dexterity:  %8d", p->l_dex);                     break;
            case 6:  printf("Strength:   %8d", p->l_str);                     break;
            case 7:  printf("%-20s", "");                                     break;
            case 8:  printf("HP: %4d / %4d     ", hp_cur, hp_max);            break;
            case 9:  printf("Ausdauer:   %8d", l_berechne_stamina(p->l_end)); break;
            case 10: printf("Schaden:    %8d", l_berechne_schaden(p->l_str)); break;
            default: printf("%-20s", "");                                     break;
            }

            printf(" |\n");
        }

        printf(" +----------------------+ +----------------------------+ +----------------------+\n");
        printf(" \033[90m[W/S]\033[0m Item   "
            "\033[90m[A/D]\033[0m Kategorie   "
            "\033[90m[ENTER]\033[0m Benutzen   "
            "\033[90m[ESC]\033[0m Zurueck");
        if (gef_anz > 0) printf("   \033[90m(%d/%d)\033[0m", wahl + 1, gef_anz);
        printf("\n");

        // --- 5. INPUT HANDLING ---
        taste = _getch();
        if (taste == 27) return;

        if (taste == 224) {
            taste = _getch();
            if (taste == 72 && wahl > 0)              wahl--;
            if (taste == 80 && wahl < gef_anz - 1)    wahl++;
            if (taste == 75 && kat > 0) { kat--; kat_alt = -1; }
            if (taste == 77 && kat < KAT_ANZ - 1) { kat++; kat_alt = -1; }
        }
        else if (taste == 13 || taste == ' ') {
            // ITEM USAGE LOGIC
            if (!sel || sel->l_id == 0) continue;

            if (sel->l_typ == TYP_VERBRAUCH) {
                int heal = sel->l_staerke;
                for (int z = 0; z < ANZAHL_ZONEN && heal > 0; z++) {
                    int diff = p->l_max_vig[z] - p->l_cur_vig[z];
                    int heilt = (heal < diff) ? heal : diff;
                    p->l_cur_vig[z] += heilt;
                    heal -= heilt;
                }

                sel->l_menge--;
                if (sel->l_menge <= 0) {
                    memset(sel, 0, sizeof(l_item));
                    kat_alt = -1;
                }
            }
            else if (sel->l_typ == TYP_BUFF) {
                sel->l_menge--;
                if (sel->l_menge <= 0) {
                    memset(sel, 0, sizeof(l_item));
                    kat_alt = -1;
                }
            }
        }
        else {
            if ((taste == 'w' || taste == 'W') && wahl > 0)              wahl--;
            if ((taste == 's' || taste == 'S') && wahl < gef_anz - 1)    wahl++;
            if ((taste == 'a' || taste == 'A') && kat > 0) { kat--; kat_alt = -1; }
            if ((taste == 'd' || taste == 'D') && kat < KAT_ANZ - 1) { kat++; kat_alt = -1; }
        }
    }
}
void l_equipment(l_infozentrum* liz) {
    // Slot-Indizes:
    // 0       = Waffe (aktiv)
    // 1-4     = Ruestung (Helm, Koerper, Hose, Schuhe)
    // 5-8     = Ringe   (TYP_BUFF)
    // 9-12    = Waffen-Wechsel-Slots
    // 13-18   = Quick-Slots (TYP_VERBRAUCH)
    // 19-22   = Pfeil-Slots

    const int ANZ_SLOTS = 23;
    const int ZEIGE_MAX = 12;

    const char* slot_namen[] = {
        "Waffe  ",                                          // 0
        "Helm   ", "Koerper", "Hose   ", "Schuhe ",        // 1-4
        "Ring  1", "Ring  2", "Ring  3", "Ring  4",        // 5-8
        "W-Slot1", "W-Slot2", "W-Slot3", "W-Slot4",        // 9-12
        "Quick 1", "Quick 2", "Quick 3",                   // 13-15
        "Quick 4", "Quick 5", "Quick 6",                   // 16-18
        "Pfeil 1", "Pfeil 2", "Pfeil 3", "Pfeil 4",        // 19-22
    };

    int slot_typen[23][4] = {
        { TYP_SCHWERT, TYP_GROSSSCHWERT, TYP_MESSER, TYP_BOGEN }, // Waffe
        { TYP_HELM,     0, 0, 0 },  // Helm
        { TYP_RUESTUNG, 0, 0, 0 },  // Koerper
        { TYP_HOSE,     0, 0, 0 },  // Hose
        { TYP_SCHUHE,   0, 0, 0 },  // Schuhe
        { TYP_BUFF, 0, 0, 0 },      // Ring 1
        { TYP_BUFF, 0, 0, 0 },      // Ring 2
        { TYP_BUFF, 0, 0, 0 },      // Ring 3
        { TYP_BUFF, 0, 0, 0 },      // Ring 4
        { TYP_SCHWERT, TYP_GROSSSCHWERT, TYP_MESSER, TYP_BOGEN }, // W-Slot 1
        { TYP_SCHWERT, TYP_GROSSSCHWERT, TYP_MESSER, TYP_BOGEN }, // W-Slot 2
        { TYP_SCHWERT, TYP_GROSSSCHWERT, TYP_MESSER, TYP_BOGEN }, // W-Slot 3
        { TYP_SCHWERT, TYP_GROSSSCHWERT, TYP_MESSER, TYP_BOGEN }, // W-Slot 4
        { TYP_VERBRAUCH, 0, 0, 0 }, // Quick 1
        { TYP_VERBRAUCH, 0, 0, 0 }, // Quick 2
        { TYP_VERBRAUCH, 0, 0, 0 }, // Quick 3
        { TYP_VERBRAUCH, 0, 0, 0 }, // Quick 4
        { TYP_VERBRAUCH, 0, 0, 0 }, // Quick 5
        { TYP_VERBRAUCH, 0, 0, 0 }, // Quick 6
        { TYP_BOGEN, 0, 0, 0 },     // Pfeil 1
        { TYP_BOGEN, 0, 0, 0 },     // Pfeil 2
        { TYP_BOGEN, 0, 0, 0 },     // Pfeil 3
        { TYP_BOGEN, 0, 0, 0 },     // Pfeil 4
    };

    int wahl = 0;
    int von = 0;
    int taste;

    while (1) {
        cls();

        l_player* p = &liz->player;
        l_inventar* inv = &p->inventar;

        l_item* slot_ptrs[ANZ_SLOTS];
        slot_ptrs[0] = &inv->l_waffe;
        slot_ptrs[1] = &inv->l_helm;
        slot_ptrs[2] = &inv->l_koerper;
        slot_ptrs[3] = &inv->l_hose;
        slot_ptrs[4] = &inv->l_schuhe;
        for (int i = 5; i <= 8; i++) slot_ptrs[i] = &inv->buff[i - 5];
        for (int i = 9; i <= 12; i++) slot_ptrs[i] = &inv->waffen_slots[i - 9];
        for (int i = 13; i <= 18; i++) slot_ptrs[i] = &inv->quick[i - 13];
        for (int i = 19; i <= 22; i++) slot_ptrs[i] = &inv->arrow[i - 19];

        l_item* sel = slot_ptrs[wahl];

        if (wahl < von)               von = wahl;
        if (wahl >= von + ZEIGE_MAX)  von = wahl - ZEIGE_MAX + 1;

        int hp_cur = 0, hp_max = 0;
        for (int z = 0; z < ANZAHL_ZONEN; z++) {
            hp_cur += p->l_cur_vig[z];
            hp_max += p->l_max_vig[z];
        }
        int stamina = l_berechne_stamina(EFF_END(p));
        int schaden = l_berechne_schaden(EFF_STR(p));
        int treffer = l_berechne_treffer(EFF_DEX(p));
        int bonus_str = EFF_STR(p) - p->l_str;
        int bonus_dex = EFF_DEX(p) - p->l_dex;
        int bonus_end = EFF_END(p) - p->l_end;
        int bonus_vig = EFF_VIG(p) - p->l_vig;

        const char* typ_namen[] = {
            "Leer","Buff","Verbrauch","Gegenstand",
            "Grossschwert","Schwert","Messer","Bogen",
            "Helm","Ruestung","Hose","Schuhe"
        };
        const char* typ_name = (sel->l_id > 0 && sel->l_typ >= 0 && sel->l_typ <= 11) ? typ_namen[sel->l_typ] : "Leer";

        printf("\n");
        printf(" +----------------------+ +----------------------------+ +----------------------+\n");
        printf(" | %-20s | | %-26.26s | | %-20s |\n", "Equipment", sel->l_id > 0 ? sel->l_name : "(Leer)", "Spieler Status");
        printf(" +----------------------+ +----------------------------+ +----------------------+\n");

        for (int r = 0; r < ZEIGE_MAX; r++) {
            printf(" | ");

            int idx = von + r;
            if (idx < ANZ_SLOTS) {
                const char* iname = (slot_ptrs[idx]->l_id > 0) ? slot_ptrs[idx]->l_name : "--";
                if (idx == wahl) printf("\033[1;37m->\033[0m %-7s: %-8.8s", slot_namen[idx], iname);
                else printf("   %-7s: %-8.8s", slot_namen[idx], iname);
            }
            else { printf("%-20s", ""); }

            printf(" | | ");

            if (sel->l_id > 0) {
                switch (r) {
                case 0: printf("Typ:      %-16.16s", typ_name);          break;
                case 1: printf("Staerke:  %-16d", sel->l_staerke);     break;
                case 2: printf("Haltbar.: %-16d", sel->l_haltbarkeit); break;
                case 3: printf("%-26s", "--- Elementar ---");            break;
                case 4: printf("  Feuer:   %-15d", sel->elemente.feuer);  break;
                case 5: printf("  Frost:   %-15d", sel->elemente.frost);  break;
                case 6: printf("  Blitz:   %-15d", sel->elemente.blitz);  break;
                case 7: printf("  Gift:    %-15d", sel->elemente.gift);   break;
                case 8: {
                    if (sel->l_typ == TYP_BUFF) printf("%-26s", "--- Ring Boni ---");
                    else printf("%-26s", "--- Beschreibung ---");
                    break;
                }
                case 9: {
                    if (sel->l_typ == TYP_BUFF) {
                        l_effekte* e = &sel->effekte;
                        char boni[27] = { 0 };
                        int off = 0;
                        if (e->l_bonus_str > 0) off += sprintf_s(boni + off, 27 - off, "+%d STR  ", e->l_bonus_str);
                        if (e->l_bonus_dex > 0) off += sprintf_s(boni + off, 27 - off, "+%d DEX  ", e->l_bonus_dex);
                        if (e->l_bonus_end > 0) off += sprintf_s(boni + off, 27 - off, "+%d END  ", e->l_bonus_end);
                        if (e->l_bonus_vig > 0)        sprintf_s(boni + off, 27 - off, "+%d VIG  ", e->l_bonus_vig);
                        printf("  \033[1;32m%-24s\033[0m", boni);
                    }
                    else {
                        char z1[27] = { 0 };
                        if (strlen(sel->l_beschreibung) > 0) strncpy_s(z1, 27, sel->l_beschreibung, 26);
                        printf("%-26.26s", z1);
                    }
                    break;
                }
                case 10: {
                    char z2[27] = { 0 };
                    if (strlen(sel->l_beschreibung) > 26) strncpy_s(z2, 27, sel->l_beschreibung + 26, 26);
                    printf("%-26.26s", z2);
                    break;
                }
                case 11: {
                    char z3[27] = { 0 };
                    if (strlen(sel->l_beschreibung) > 52) strncpy_s(z3, 27, sel->l_beschreibung + 52, 26);
                    printf("%-26.26s", z3);
                    break;
                }
                default: printf("%-26s", ""); break;
                }
            }
            else {
                if (r == 0) printf("%-26s", "(Kein Item ausgestattet)");
                else        printf("%-26s", "");
            }

            printf(" | | ");

            switch (r) {
            case 0:  printf("Level:  %12d", p->l_level); break;
            case 1:  printf("Runen:  %12d", p->l_runen); break;
            case 2:  printf("%-20s", "");                break;
            case 3: {
                if (bonus_vig > 0) printf("Vigor:   %4d \033[1;32m+%d\033[0m%*s", p->l_vig, bonus_vig, (int)(5 - (bonus_vig > 99 ? 3 : (bonus_vig > 9 ? 2 : 1))), "");
                else printf("Vigor:      %8d", p->l_vig);
                break;
            }
            case 4: {
                if (bonus_end > 0) printf("Endur.:  %4d \033[1;32m+%d\033[0m%*s", p->l_end, bonus_end, (int)(5 - (bonus_end > 99 ? 3 : (bonus_end > 9 ? 2 : 1))), "");
                else printf("Endurance:  %8d", p->l_end);
                break;
            }
            case 5: {
                if (bonus_dex > 0) printf("Dex.:    %4d \033[1;32m+%d\033[0m%*s", p->l_dex, bonus_dex, (int)(5 - (bonus_dex > 99 ? 3 : (bonus_dex > 9 ? 2 : 1))), "");
                else printf("Dexterity:  %8d", p->l_dex);
                break;
            }
            case 6: {
                if (bonus_str > 0) printf("Str.:    %4d \033[1;32m+%d\033[0m%*s", p->l_str, bonus_str, (int)(5 - (bonus_str > 99 ? 3 : (bonus_str > 9 ? 2 : 1))), "");
                else printf("Strength:   %8d", p->l_str);
                break;
            }
            case 7:  printf("%-20s", "");                          break;
            case 8:  printf("HP: %4d/%4d       ", hp_cur, hp_max); break;
            case 9:  printf("Ausdauer:   %8d", stamina);          break;
            case 10: printf("Schaden:    %8d", schaden);          break;
            case 11: printf("Treffer:    %7d%%", treffer);          break;
            default: printf("%-20s", "");                          break;
            }

            printf(" |\n");
        }

        printf(" +----------------------+ +----------------------------+ +----------------------+\n");
        printf(" \033[90m(%d/%d)\033[0m", wahl + 1, ANZ_SLOTS);
        printf("  \033[90m[W/S]\033[0m Slot"
            "  \033[90m[ENTER]\033[0m Ausruesten"
            "  \033[90m[TAB]\033[0m Waffe wechseln"
            "  \033[90m[R]\033[0m Ablegen"
            "  \033[90m[ESC]\033[0m Zurueck\n\n");

        taste = _getch();
        if (taste == 27) return;

        if (taste == '\t') {
            if (wahl >= 9 && wahl <= 12 && slot_ptrs[wahl]->l_id != 0) {
                l_item temp = inv->l_waffe;
                inv->l_waffe = *slot_ptrs[wahl];
                *slot_ptrs[wahl] = temp;
            }
        }
        else if (taste == 224) {
            taste = _getch();
            if (taste == 72 && wahl > 0)              wahl--;
            if (taste == 80 && wahl < ANZ_SLOTS - 1)  wahl++;
        }
        else if (taste == 'r' || taste == 'R') {
            if (sel->l_id != 0) {
                int freier_slot = -1;
                for (int i = 0; i < INV_GROESSE; i++) {
                    if (inv->slots[i].l_id == 0) {
                        freier_slot = i;
                        break;
                    }
                }
                if (freier_slot != -1) {
                    inv->slots[freier_slot] = *sel;
                    memset(sel, 0, sizeof(l_item));
                }
                else {
                    cls();
                    printf("\n\n  Dein Inventar ist voll! Du kannst das Item nicht ablegen.\n\n");
                    Sleep(1200);
                }
            }
        }
        else if (taste == 13 || taste == ' ') {
            l_item* passend[INV_GROESSE];
            int p_anz = 0;

            for (int i = 0; i < INV_GROESSE; i++) {
                if (inv->slots[i].l_id == 0) continue;
                int typ = inv->slots[i].l_typ;
                for (int j = 0; j < 4; j++) {
                    if (slot_typen[wahl][j] != 0 && slot_typen[wahl][j] == typ) {
                        passend[p_anz++] = &inv->slots[i];
                        break;
                    }
                }
            }

            if (p_anz == 0) {
                cls();
                printf("\n\n  Kein passendes Item im Inventar!\n\n");
                Sleep(800);
                continue;
            }

            int sub = 0;
            while (1) {
                cls();
                printf("\n  +------------------------------------------+\n");
                printf("  | %-40s |\n", "Item auswaehlen");
                printf("  +------------------------------------------+\n");

                for (int i = 0; i < p_anz; i++) {
                    if (i == sub) printf("  | \033[1;37m-> %-22.22s Str.:%6d\033[0m    |\n", passend[i]->l_name, passend[i]->l_staerke);
                    else printf("  |    %-22.22s Str.:%6d    |\n", passend[i]->l_name, passend[i]->l_staerke);
                }

                printf("  +------------------------------------------+\n");
                printf("  \033[90m[W/S]\033[0m Auswahl"
                    "  \033[90m[ENTER]\033[0m Ausruesten"
                    "  \033[90m[ESC]\033[0m Abbrechen\n\n");

                int st = _getch();
                if (st == 27) break;
                if (st == 224) {
                    st = _getch();
                    if (st == 72 && sub > 0)          sub--;
                    if (st == 80 && sub < p_anz - 1)  sub++;
                }
                else if (st == 13 || st == ' ') {
                    l_equip_item(sel, passend[sub]);
                    break;
                }
                else {
                    if ((st == 'w' || st == 'W') && sub > 0)          sub--;
                    if ((st == 's' || st == 'S') && sub < p_anz - 1)  sub++;
                }
            }
        }
        else {
            if ((taste == 'w' || taste == 'W') && wahl > 0)              wahl--;
            if ((taste == 's' || taste == 'S') && wahl < ANZ_SLOTS - 1)  wahl++;
        }
    }
}
void l_equip_item(l_item* ziel, l_item* quelle) {
    if (ziel == NULL || quelle == NULL) return;
    l_item temp = *ziel;
    *ziel = *quelle;
    *quelle = temp;
}
void l_iteam_samlung(l_infozentrum* liz) {
    memset(liz->item_c, 0, sizeof(liz->item_c));

    // ==========================================================
    // VERBRAUCH-ITEMS (ID 1 - 9)
    // ==========================================================

    liz->item_c[1].l_id = 1; liz->item_c[1].l_typ = TYP_VERBRAUCH;
    liz->item_c[1].l_staerke = 30; liz->item_c[1].l_menge = 1;
    strcpy_s(liz->item_c[1].l_name, 32, "Kleiner Heiltrank");
    strcpy_s(liz->item_c[1].l_beschreibung, 256, "Heilt leichte Wunden.");

    liz->item_c[2].l_id = 2; liz->item_c[2].l_typ = TYP_VERBRAUCH;
    liz->item_c[2].l_staerke = 80; liz->item_c[2].l_menge = 1;
    strcpy_s(liz->item_c[2].l_name, 32, "Mittlerer Heiltrank");
    strcpy_s(liz->item_c[2].l_beschreibung, 256, "Heilt mittelschwere Verletzungen.");

    liz->item_c[3].l_id = 3; liz->item_c[3].l_typ = TYP_VERBRAUCH;
    liz->item_c[3].l_staerke = 180; liz->item_c[3].l_menge = 1;
    strcpy_s(liz->item_c[3].l_name, 32, "Grosser Heiltrank");
    strcpy_s(liz->item_c[3].l_beschreibung, 256, "Heilt schwere Verletzungen vollstaendig.");

    liz->item_c[4].l_id = 4; liz->item_c[4].l_typ = TYP_VERBRAUCH;
    liz->item_c[4].l_staerke = 0; liz->item_c[4].l_menge = 1;
    liz->item_c[4].effekte.l_regen = 2;
    strcpy_s(liz->item_c[4].l_name, 32, "Antidot");
    strcpy_s(liz->item_c[4].l_beschreibung, 256, "Neutralisiert Gift. Heilt 2 Runden lang.");

    liz->item_c[5].l_id = 5; liz->item_c[5].l_typ = TYP_VERBRAUCH;
    liz->item_c[5].l_staerke = 0; liz->item_c[5].l_menge = 1;
    liz->item_c[5].effekte.l_regen = 1;
    strcpy_s(liz->item_c[5].l_name, 32, "Ausdauer-Trank");
    strcpy_s(liz->item_c[5].l_beschreibung, 256, "Stellt Ausdauer vollstaendig wieder her.");

    // ==========================================================
    // WAFFEN   Spieler (ID 10 - 18)
    // ==========================================================

    liz->item_c[10].l_id = 10; liz->item_c[10].l_typ = TYP_SCHWERT;
    liz->item_c[10].l_staerke = 8; liz->item_c[10].l_menge = 1; liz->item_c[10].l_haltbarkeit = 30;
    strcpy_s(liz->item_c[10].l_name, 32, "Rostiges Schwert");
    strcpy_s(liz->item_c[10].l_beschreibung, 256, "Ein altes Schwert. Besser als nichts.");

    liz->item_c[11].l_id = 11; liz->item_c[11].l_typ = TYP_SCHWERT;
    liz->item_c[11].l_staerke = 18; liz->item_c[11].l_menge = 1; liz->item_c[11].l_haltbarkeit = 60;
    strcpy_s(liz->item_c[11].l_name, 32, "Eisenschwert");
    strcpy_s(liz->item_c[11].l_beschreibung, 256, "Solides Schwert aus gehaertetem Eisen.");

    liz->item_c[12].l_id = 12; liz->item_c[12].l_typ = TYP_SCHWERT;
    liz->item_c[12].l_staerke = 22; liz->item_c[12].l_menge = 1; liz->item_c[12].l_haltbarkeit = 55;
    liz->item_c[12].elemente.feuer = 15;
    strcpy_s(liz->item_c[12].l_name, 32, "Feuerklinge");
    strcpy_s(liz->item_c[12].l_beschreibung, 256, "In Drachenfeuer getraenkte Klinge.");

    liz->item_c[13].l_id = 13; liz->item_c[13].l_typ = TYP_MESSER;
    liz->item_c[13].l_staerke = 14; liz->item_c[13].l_menge = 1; liz->item_c[13].l_haltbarkeit = 45;
    liz->item_c[13].elemente.frost = 20;
    strcpy_s(liz->item_c[13].l_name, 32, "Frostdolch");
    strcpy_s(liz->item_c[13].l_beschreibung, 256, "Hinterlaesst eiskalte Wunden.");

    liz->item_c[14].l_id = 14; liz->item_c[14].l_typ = TYP_MESSER;
    liz->item_c[14].l_staerke = 16; liz->item_c[14].l_menge = 1; liz->item_c[14].l_haltbarkeit = 40;
    liz->item_c[14].effekte.l_blutung = 2;
    strcpy_s(liz->item_c[14].l_name, 32, "Blutklinge");
    strcpy_s(liz->item_c[14].l_beschreibung, 256, "Verursacht starke Blutungen.");

    liz->item_c[15].l_id = 15; liz->item_c[15].l_typ = TYP_BOGEN;
    liz->item_c[15].l_staerke = 20; liz->item_c[15].l_menge = 1; liz->item_c[15].l_haltbarkeit = 50;
    strcpy_s(liz->item_c[15].l_name, 32, "Langgrimmbogen");
    strcpy_s(liz->item_c[15].l_beschreibung, 256, "Trifft auch Luft-Gegner. Skaliert mit DEX.");

    liz->item_c[16].l_id = 16; liz->item_c[16].l_typ = TYP_BOGEN;
    liz->item_c[16].l_staerke = 18; liz->item_c[16].l_menge = 1; liz->item_c[16].l_haltbarkeit = 45;
    liz->item_c[16].elemente.blitz = 25;
    strcpy_s(liz->item_c[16].l_name, 32, "Blitzbogen");
    strcpy_s(liz->item_c[16].l_beschreibung, 256, "Pfeile die wie Blitze treffen.");

    liz->item_c[17].l_id = 17; liz->item_c[17].l_typ = TYP_GROSSSCHWERT;
    liz->item_c[17].l_staerke = 35; liz->item_c[17].l_menge = 1; liz->item_c[17].l_haltbarkeit = 70;
    strcpy_s(liz->item_c[17].l_name, 32, "Grossschwert d.Waechters");
    strcpy_s(liz->item_c[17].l_beschreibung, 256, "Trifft auch fliegende Feinde.");

    liz->item_c[18].l_id = 18; liz->item_c[18].l_typ = TYP_GROSSSCHWERT;
    liz->item_c[18].l_staerke = 38; liz->item_c[18].l_menge = 1; liz->item_c[18].l_haltbarkeit = 65;
    liz->item_c[18].elemente.feuer = 20; liz->item_c[18].elemente.blitz = 10;
    strcpy_s(liz->item_c[18].l_name, 32, "Flammenklinge");
    strcpy_s(liz->item_c[18].l_beschreibung, 256, "Brennt und blitzt zugleich.");

    // ==========================================================
    // RUESTUNG   Spieler (ID 20 - 27)
    // ==========================================================

    liz->item_c[20].l_id = 20; liz->item_c[20].l_typ = TYP_HELM;
    liz->item_c[20].l_staerke = 3; liz->item_c[20].l_menge = 1;
    strcpy_s(liz->item_c[20].l_name, 32, "Lederhelm");
    strcpy_s(liz->item_c[20].l_beschreibung, 256, "Leichter Schutz aus Leder.");

    liz->item_c[21].l_id = 21; liz->item_c[21].l_typ = TYP_RUESTUNG;
    liz->item_c[21].l_staerke = 6; liz->item_c[21].l_menge = 1;
    strcpy_s(liz->item_c[21].l_name, 32, "Lederruestung");
    strcpy_s(liz->item_c[21].l_beschreibung, 256, "Einfache Ruestung aus gegerbtem Leder.");

    liz->item_c[22].l_id = 22; liz->item_c[22].l_typ = TYP_HOSE;
    liz->item_c[22].l_staerke = 4; liz->item_c[22].l_menge = 1;
    strcpy_s(liz->item_c[22].l_name, 32, "Lederhose");
    strcpy_s(liz->item_c[22].l_beschreibung, 256, "Leichter Beinschutz.");

    liz->item_c[23].l_id = 23; liz->item_c[23].l_typ = TYP_SCHUHE;
    liz->item_c[23].l_staerke = 2; liz->item_c[23].l_menge = 1;
    strcpy_s(liz->item_c[23].l_name, 32, "Lederstiefel");
    strcpy_s(liz->item_c[23].l_beschreibung, 256, "Einfache Stiefel aus Leder.");

    liz->item_c[24].l_id = 24; liz->item_c[24].l_typ = TYP_HELM;
    liz->item_c[24].l_staerke = 10; liz->item_c[24].l_menge = 1;
    strcpy_s(liz->item_c[24].l_name, 32, "Eisenhelm");
    strcpy_s(liz->item_c[24].l_beschreibung, 256, "Solider Schutz aus Eisen.");

    liz->item_c[25].l_id = 25; liz->item_c[25].l_typ = TYP_RUESTUNG;
    liz->item_c[25].l_staerke = 18; liz->item_c[25].l_menge = 1;
    strcpy_s(liz->item_c[25].l_name, 32, "Eisenpanzer");
    strcpy_s(liz->item_c[25].l_beschreibung, 256, "Schwer aber widerstandsfaehig.");

    liz->item_c[26].l_id = 26; liz->item_c[26].l_typ = TYP_HOSE;
    liz->item_c[26].l_staerke = 12; liz->item_c[26].l_menge = 1;
    strcpy_s(liz->item_c[26].l_name, 32, "Eisenbeinlinge");
    strcpy_s(liz->item_c[26].l_beschreibung, 256, "Stabiler Beinschutz aus Eisen.");

    liz->item_c[27].l_id = 27; liz->item_c[27].l_typ = TYP_SCHUHE;
    liz->item_c[27].l_staerke = 8; liz->item_c[27].l_menge = 1;
    strcpy_s(liz->item_c[27].l_name, 32, "Eisenstiefel");
    strcpy_s(liz->item_c[27].l_beschreibung, 256, "Schwere Stiefel mit gutem Halt.");

    // ==========================================================
    // RINGE (ID 40 - 46)
    // ==========================================================

    liz->item_c[40].l_id = 40; liz->item_c[40].l_typ = TYP_BUFF;
    liz->item_c[40].l_menge = 1; liz->item_c[40].effekte.l_bonus_str = 5;
    strcpy_s(liz->item_c[40].l_name, 32, "Ring der Staerke");
    strcpy_s(liz->item_c[40].l_beschreibung, 256, "+5 Strength solange ausgeruestet.");

    liz->item_c[41].l_id = 41; liz->item_c[41].l_typ = TYP_BUFF;
    liz->item_c[41].l_menge = 1; liz->item_c[41].effekte.l_bonus_dex = 5;
    strcpy_s(liz->item_c[41].l_name, 32, "Ring der Geschick.");
    strcpy_s(liz->item_c[41].l_beschreibung, 256, "+5 Dexterity solange ausgeruestet.");

    liz->item_c[42].l_id = 42; liz->item_c[42].l_typ = TYP_BUFF;
    liz->item_c[42].l_menge = 1; liz->item_c[42].effekte.l_bonus_vig = 4;
    strcpy_s(liz->item_c[42].l_name, 32, "Ring des Lebens");
    strcpy_s(liz->item_c[42].l_beschreibung, 256, "+4 Vigor (mehr max. HP).");

    liz->item_c[43].l_id = 43; liz->item_c[43].l_typ = TYP_BUFF;
    liz->item_c[43].l_menge = 1; liz->item_c[43].effekte.l_bonus_end = 4;
    strcpy_s(liz->item_c[43].l_name, 32, "Ring der Ausdauer");
    strcpy_s(liz->item_c[43].l_beschreibung, 256, "+4 Endurance (mehr Stamina).");

    liz->item_c[44].l_id = 44; liz->item_c[44].l_typ = TYP_BUFF;
    liz->item_c[44].l_menge = 1;
    liz->item_c[44].effekte.l_bonus_str = 3; liz->item_c[44].effekte.l_bonus_dex = 3;
    strcpy_s(liz->item_c[44].l_name, 32, "Feuerring");
    strcpy_s(liz->item_c[44].l_beschreibung, 256, "+3 STR und +3 DEX zugleich.");

    liz->item_c[45].l_id = 45; liz->item_c[45].l_typ = TYP_BUFF;
    liz->item_c[45].l_menge = 1; liz->item_c[45].effekte.l_bonus_str = 8;
    strcpy_s(liz->item_c[45].l_name, 32, "Ring des Kriegers");
    strcpy_s(liz->item_c[45].l_beschreibung, 256, "+8 Strength. Nur fuer die Staerksten.");

    liz->item_c[46].l_id = 46; liz->item_c[46].l_typ = TYP_BUFF;
    liz->item_c[46].l_menge = 1; liz->item_c[46].effekte.l_bonus_dex = 8;
    strcpy_s(liz->item_c[46].l_name, 32, "Ring des Schattens");
    strcpy_s(liz->item_c[46].l_beschreibung, 256, "+8 Dexterity. Ideal fuer Bogenschuetzen.");

    // ==========================================================
    // RINGE (ID 65, 81 - n)
    // ==========================================================

    liz->item_c[65].l_id = 65; liz->item_c[65].l_typ = TYP_MESSER;
    liz->item_c[65].l_staerke = 5; liz->item_c[65].l_menge = 1;
    liz->item_c[65].elemente.gift = 15;
    liz->item_c[65].effekte.l_blutung = 1;
    strcpy_s(liz->item_c[65].l_name, 32, "Giftzahn");
    strcpy_s(liz->item_c[65].l_beschreibung, 256, "Zahn einer Riesenspinne. Vergiftet.");

    liz->item_c[81].l_id = 81; liz->item_c[81].l_typ = TYP_BOGEN;
    liz->item_c[81].l_staerke = 7; liz->item_c[81].l_menge = 1;
    strcpy_s(liz->item_c[81].l_name, 32, "Bogen");
    strcpy_s(liz->item_c[81].l_beschreibung, 256, "Ein Einfacher Bogen.");

    // ==========================================================
    // MONSTER-EQUIPMENT (ID 50 - 89)
    // ==========================================================

    // -- Soul of Cinder (50 54) -------------------------------
    liz->item_c[50].l_id = 50; liz->item_c[50].l_typ = TYP_GROSSSCHWERT;
    liz->item_c[50].l_staerke = 20; liz->item_c[50].l_menge = 1; liz->item_c[50].l_haltbarkeit = 90;
    liz->item_c[50].elemente.feuer = 15;
    strcpy_s(liz->item_c[50].l_name, 32, "Klinge des Cinders");
    strcpy_s(liz->item_c[50].l_beschreibung, 256, "Waffe des letzten Flammenwaechters.");

    liz->item_c[51].l_id = 51; liz->item_c[51].l_typ = TYP_HELM;
    liz->item_c[51].l_staerke = 20; liz->item_c[51].l_menge = 1;
    strcpy_s(liz->item_c[51].l_name, 32, "Cinder-Helm");
    strcpy_s(liz->item_c[51].l_beschreibung, 256, "Helm des letzten Flammenwaechters.");

    liz->item_c[52].l_id = 52; liz->item_c[52].l_typ = TYP_RUESTUNG;
    liz->item_c[52].l_staerke = 35; liz->item_c[52].l_menge = 1;
    strcpy_s(liz->item_c[52].l_name, 32, "Cinder-Panzer");
    strcpy_s(liz->item_c[52].l_beschreibung, 256, "Panzer des letzten Flammenwaechters.");

    liz->item_c[53].l_id = 53; liz->item_c[53].l_typ = TYP_HOSE;
    liz->item_c[53].l_staerke = 18; liz->item_c[53].l_menge = 1;
    strcpy_s(liz->item_c[53].l_name, 32, "Cinder-Hose");
    strcpy_s(liz->item_c[53].l_beschreibung, 256, "Beinschutz des letzten Flammenwaechters.");

    liz->item_c[54].l_id = 54; liz->item_c[54].l_typ = TYP_SCHUHE;
    liz->item_c[54].l_staerke = 12; liz->item_c[54].l_menge = 1;
    strcpy_s(liz->item_c[54].l_name, 32, "Cinder-Stiefel");
    strcpy_s(liz->item_c[54].l_beschreibung, 256, "Stiefel des letzten Flammenwaechters.");

    // -- Der Waechter (55 - 59) ---------------------------------
    liz->item_c[55].l_id = 55; liz->item_c[55].l_typ = TYP_SCHWERT;
    liz->item_c[55].l_staerke = 10; liz->item_c[55].l_menge = 1;
    strcpy_s(liz->item_c[55].l_name, 32, "Waechter-Klinge");
    strcpy_s(liz->item_c[55].l_beschreibung, 256, "Alte Klinge eines Labyrintwaechters.");

    liz->item_c[56].l_id = 56; liz->item_c[56].l_typ = TYP_HELM;
    liz->item_c[56].l_staerke = 8; liz->item_c[56].l_menge = 1;
    strcpy_s(liz->item_c[56].l_name, 32, "Waechter-Helm");
    strcpy_s(liz->item_c[56].l_beschreibung, 256, "Helm eines alten Waechters.");

    liz->item_c[57].l_id = 57; liz->item_c[57].l_typ = TYP_RUESTUNG;
    liz->item_c[57].l_staerke = 14; liz->item_c[57].l_menge = 1;
    strcpy_s(liz->item_c[57].l_name, 32, "Waechter-Ruestung");
    strcpy_s(liz->item_c[57].l_beschreibung, 256, "Ruestung eines alten Waechters.");

    liz->item_c[58].l_id = 58; liz->item_c[58].l_typ = TYP_HOSE;
    liz->item_c[58].l_staerke = 8; liz->item_c[58].l_menge = 1;
    strcpy_s(liz->item_c[58].l_name, 32, "Waechter-Hose");
    strcpy_s(liz->item_c[58].l_beschreibung, 256, "Beinschutz eines alten Waechters.");

    liz->item_c[59].l_id = 59; liz->item_c[59].l_typ = TYP_SCHUHE;
    liz->item_c[59].l_staerke = 5; liz->item_c[59].l_menge = 1;
    strcpy_s(liz->item_c[59].l_name, 32, "Waechter-Stiefel");
    strcpy_s(liz->item_c[59].l_beschreibung, 256, "Stiefel eines alten Waechters.");

    // -- Gefangener (60) -------------------------------------
    liz->item_c[60].l_id = 60; liz->item_c[60].l_typ = TYP_GROSSSCHWERT;
    liz->item_c[60].l_staerke = 12; liz->item_c[60].l_menge = 1;
    liz->item_c[60].effekte.l_blutung = 5;
    strcpy_s(liz->item_c[60].l_name, 32, "Splitter GrossSchwert");
    strcpy_s(liz->item_c[60].l_beschreibung, 256, "Bruechig. Verursacht Splitter und Blutungen.");

    // -- Verfluchter Ritter (61 - 64) --------------------------
    liz->item_c[61].l_id = 61; liz->item_c[61].l_typ = TYP_SCHWERT;
    liz->item_c[61].l_staerke = 12; liz->item_c[61].l_menge = 1;
    liz->item_c[61].elemente.blitz = 8;
    strcpy_s(liz->item_c[61].l_name, 32, "Verfluchte Klinge");
    strcpy_s(liz->item_c[61].l_beschreibung, 256, "Vom Fluch des Labyrinths durchdrungen.");

    liz->item_c[62].l_id = 62; liz->item_c[62].l_typ = TYP_HELM;
    liz->item_c[62].l_staerke = 7; liz->item_c[62].l_menge = 1;
    strcpy_s(liz->item_c[62].l_name, 32, "Fluch-Helm");
    strcpy_s(liz->item_c[62].l_beschreibung, 256, "Vom Fluch gezeichneter Helm.");

    liz->item_c[63].l_id = 63; liz->item_c[63].l_typ = TYP_RUESTUNG;
    liz->item_c[63].l_staerke = 12; liz->item_c[63].l_menge = 1;
    strcpy_s(liz->item_c[63].l_name, 32, "Fluch-Ruestung");
    strcpy_s(liz->item_c[63].l_beschreibung, 256, "Vom Fluch gezeichnete Ruestung.");

    liz->item_c[64].l_id = 64; liz->item_c[64].l_typ = TYP_HOSE;
    liz->item_c[64].l_staerke = 6; liz->item_c[64].l_menge = 1;
    strcpy_s(liz->item_c[64].l_name, 32, "Fluch-Hose");
    strcpy_s(liz->item_c[64].l_beschreibung, 256, "Vom Fluch gezeichneter Beinschutz.");

    // -- Magier (66) ----------------------------------------
    liz->item_c[66].l_id = 66; liz->item_c[66].l_typ = TYP_GEGENSTAND;
    liz->item_c[66].l_staerke = 8; liz->item_c[66].l_menge = 1;
    liz->item_c[66].elemente.feuer = 20;
    strcpy_s(liz->item_c[66].l_name, 32, "Feuerklumpen");
    strcpy_s(liz->item_c[66].l_beschreibung, 256, "Sehr Hell beim benutzten");

    // -- Siegmeyer (67 - 71) --------------------------------
    liz->item_c[67].l_id = 67; liz->item_c[67].l_typ = TYP_GROSSSCHWERT;
    liz->item_c[67].l_staerke = 18; liz->item_c[67].l_menge = 1; liz->item_c[67].l_haltbarkeit = 80;
    strcpy_s(liz->item_c[67].l_name, 32, "Siegmeyers Klinge");
    strcpy_s(liz->item_c[67].l_beschreibung, 256, "Schwere Klinge eines alten Ritters.");

    liz->item_c[68].l_id = 68; liz->item_c[68].l_typ = TYP_HELM;
    liz->item_c[68].l_staerke = 16; liz->item_c[68].l_menge = 1;
    strcpy_s(liz->item_c[68].l_name, 32, "Ritter-Helm");
    strcpy_s(liz->item_c[68].l_beschreibung, 256, "Helm eines ehrenwerten Ritters.");

    liz->item_c[69].l_id = 69; liz->item_c[69].l_typ = TYP_RUESTUNG;
    liz->item_c[69].l_staerke = 28; liz->item_c[69].l_menge = 1;
    strcpy_s(liz->item_c[69].l_name, 32, "Ritter-Plattenruestung");
    strcpy_s(liz->item_c[69].l_beschreibung, 256, "Solide Plattenruestung eines Ritters.");

    liz->item_c[70].l_id = 70; liz->item_c[70].l_typ = TYP_HOSE;
    liz->item_c[70].l_staerke = 14; liz->item_c[70].l_menge = 1;
    strcpy_s(liz->item_c[70].l_name, 32, "Ritter-Beinling");
    strcpy_s(liz->item_c[70].l_beschreibung, 256, "Stabiler Beinschutz eines Ritters.");

    liz->item_c[71].l_id = 71; liz->item_c[71].l_typ = TYP_SCHUHE;
    liz->item_c[71].l_staerke = 10; liz->item_c[71].l_menge = 1;
    strcpy_s(liz->item_c[71].l_name, 32, "Ritter-Stiefel");
    strcpy_s(liz->item_c[71].l_beschreibung, 256, "Stiefel eines ehrenwerten Ritters.");

    // -- Verlorenes Ritter (72 - 76) ------------------------
    liz->item_c[72].l_id = 72; liz->item_c[72].l_typ = TYP_MESSER;
    liz->item_c[72].l_staerke = 20; liz->item_c[72].l_menge = 1;
    strcpy_s(liz->item_c[72].l_name, 32, "Steinfaust");
    strcpy_s(liz->item_c[72].l_beschreibung, 256, "Faust aus lebendigem Stein.");

    liz->item_c[73].l_id = 73; liz->item_c[73].l_typ = TYP_HELM;
    liz->item_c[73].l_staerke = 16; liz->item_c[73].l_menge = 1;
    strcpy_s(liz->item_c[73].l_name, 32, "Steinschicht");
    strcpy_s(liz->item_c[73].l_beschreibung, 256, "Harte Steinschicht als Helm.");

    liz->item_c[74].l_id = 74; liz->item_c[74].l_typ = TYP_RUESTUNG;
    liz->item_c[74].l_staerke = 29; liz->item_c[74].l_menge = 1;
    strcpy_s(liz->item_c[74].l_name, 32, "Steinkoerper");
    strcpy_s(liz->item_c[74].l_beschreibung, 256, "Koerper aus lebendigem Stein.");

    liz->item_c[75].l_id = 75; liz->item_c[75].l_typ = TYP_HOSE;
    liz->item_c[75].l_staerke = 15; liz->item_c[75].l_menge = 1;
    strcpy_s(liz->item_c[75].l_name, 32, "Steinbeine");
    strcpy_s(liz->item_c[75].l_beschreibung, 256, "Massive Beine aus lebendigem Stein.");

    liz->item_c[76].l_id = 76; liz->item_c[76].l_typ = TYP_SCHUHE;
    liz->item_c[76].l_staerke = 11; liz->item_c[76].l_menge = 1;
    strcpy_s(liz->item_c[76].l_name, 32, "Steinfuesse");
    strcpy_s(liz->item_c[76].l_beschreibung, 256, "Fuesse aus lebendigem Stein.");

    // -- Goblin (77) ----------------------------------------
    liz->item_c[77].l_id = 77; liz->item_c[77].l_typ = TYP_MESSER;
    liz->item_c[77].l_staerke = 8; liz->item_c[77].l_menge = 1;
    strcpy_s(liz->item_c[77].l_name, 32, "Dolch");
    strcpy_s(liz->item_c[77].l_beschreibung, 256, "Ein einfacher Dolch");

    // -- Mimic (78 - 80) --------------------------------------
    liz->item_c[78].l_id = 78; liz->item_c[78].l_typ = TYP_MESSER;
    liz->item_c[78].l_staerke = 14; liz->item_c[78].l_menge = 1;
    liz->item_c[78].effekte.l_blutung = 2;
    strcpy_s(liz->item_c[78].l_name, 32, "Mimic-Biss");
    strcpy_s(liz->item_c[78].l_beschreibung, 256, "Zaehne einer Mimic-Kreatur.");

    liz->item_c[79].l_id = 79; liz->item_c[79].l_typ = TYP_HELM;
    liz->item_c[79].l_staerke = 15; liz->item_c[79].l_menge = 1;
    strcpy_s(liz->item_c[79].l_name, 32, "Verzauberter Helm");
    strcpy_s(liz->item_c[79].l_beschreibung, 256, "Aus dem Inneren einer Mimic.");

    liz->item_c[80].l_id = 80; liz->item_c[80].l_typ = TYP_RUESTUNG;
    liz->item_c[80].l_staerke = 25; liz->item_c[80].l_menge = 1;
    strcpy_s(liz->item_c[80].l_name, 32, "Verzauberte Ruestung");
    strcpy_s(liz->item_c[80].l_beschreibung, 256, "Aus dem Inneren einer Mimic.");
}
void l_addon_iteam_laden(l_infozentrum* liz) {

}

void l_battle(l_infozentrum* liz, int m_index) {
    l_player* p = &liz->player;
    l_monster* m = &liz->monster[m_index];

    while (liz->monster[m_index].l_cur_vig > 0) {
        if (liz->player.l_cur_vig[KF] < 1 || liz->player.l_cur_vig[KR] < 1) {
            cls();
            char text[128];
            if (liz->player.l_cur_vig[KF] < 1) sprintf_s(text, sizeof(text), "\033[1;31m*** Das Monster hat deinen Kopf abgeschlagen ***\033[0m");
            if (liz->player.l_cur_vig[KR] < 1) sprintf_s(text, sizeof(text), "\033[1;31m*** Das Monster hat deinen Koerper getoetet ***\033[0m");
            printf("\n\n\n\t\t\t\t=========================================================\n");
            l_PrCe("\033[1;31m        Du wurdest besiegt       \033[0m");
            l_PrCe(text);
            printf("\t\t\t\t=========================================================\n\n");
            _getch();
            return;
        }
        int ergebnis = l_battle_p(liz, m_index);
        if (ergebnis == 1) return;
        if (liz->player.l_cur_vig[KF] < 1 || liz->player.l_cur_vig[KR] < 1) {
            cls();
            char text[128];
            if (liz->player.l_cur_vig[KF] < 1) sprintf_s(text, sizeof(text), "\033[1;31m*** Der Monster hat dein Kopf abgeschlagen ***\033[0m");
            if (liz->player.l_cur_vig[KR] < 1) sprintf_s(text, sizeof(text), "\033[1;31m*** Der Monster hat dein Koerper getoeted ***\033[0m");
            printf("\n\n\n\t\t\t\t=========================================================\n");
            l_PrCe("\033[1;31m        Du wurdest besiegt       \033[0m");
            l_PrCe(text);
            printf("\t\t\t\t=========================================================\n\n");
            _getch();
            return;
        }
        if (liz->monster[m_index].l_cur_vig < 1) {
            cls();
            char text[128];
            printf("\t\t\033[1;33m");
            sprintf_s(text, sizeof(text), "*** %s wurde besiegt! ***", liz->monster[m_index].l_name);
            printf("\033[0m");
            printf("\n\n\n\t\t\t\t=========================================================\n");
            l_PrCe(text);
            printf("\t\t\t\t=========================================================\n\n");
            _getch();
        }
        else if (ergebnis == 2) {
            // Ausgewichen
            l_status_effekte_auswerten(&liz->player);
        }
        else {
            l_battle_logic_m(liz, m_index);
            l_status_effekte_auswerten(&liz->player);
        }
    }
}
int  l_battle_p(l_infozentrum* liz, int m_index) {
    l_player* p = &liz->player;
    l_monster* m = &liz->monster[m_index];

    const char* menu[] = { "Aktion", "Equipment", "Inventory", "Beschreibung", "Flucht" };
    const char* aktion_menu[] = { "Angriff", "Quick Slot", "Ausweichen", "Ruhen" };

    char art[35][76] = { 0 };
    FILE* file;
    if (fopen_s(&file, m->l_ascii_art_datei, "r") == 0 && file != NULL) {
        int line = 0;
        while (line < 35 && fgets(art[line], sizeof(art[line]), file) != NULL) {
            art[line][strcspn(art[line], "\r\n")] = 0;
            line++;
        }
        fclose(file);
    }
    else { strcpy_s(art[3], sizeof(art[3]), "  (ASCII-Art nicht gefunden)"); }

    int wahl = 0;  const int menu_size = 5;
    int aktion_wahl = 0;  const int aktion_size = 4;
    int in_aktion = 0;

    while (1) {
        int p_hp_max = 0, p_hp_cur = 0;
        for (int z = 0; z < ANZAHL_ZONEN; z++) {
            p_hp_max += p->l_max_vig[z];
            p_hp_cur += p->l_cur_vig[z];
        }
        int p_stam_max = l_berechne_stamina(EFF_END(p));
        if (p->l_cur_end > p_stam_max) p->l_cur_end = p_stam_max;
        int p_stam_cur = p->l_cur_end;
        int m_hp_max = m->l_vig;
        int m_hp_cur = m->l_cur_vig;

        cls();
        printf(" \033[90m+---------------------------------------------------------------------------+\033[0m\n");
        for (int i = 0; i < 25; i++) {
            printf(" \033[90m|\033[0m %-73.73s \033[90m|\033[0m ", art[i]);

            switch (i) {
            case 0:  printf("\033[1;31m--- Gegner ---\033[0m"); break;
            case 1:  printf("Name: \033[1;37m%s\033[0m", m->l_name); break;
            case 2: {
                printf("HP:   ");
                l_draw_bar(m_hp_cur, m_hp_max, 14);
                printf(" %4d/%4d", m_hp_cur, m_hp_max);
                break;
            }
            case 3:  break;
            case 4:  printf("\033[1;37m--- Spieler ---\033[0m"); break;
            case 5: {
                printf("HP:   ");
                l_draw_bar(p_hp_cur, p_hp_max, 14);
                printf(" %4d/%4d", p_hp_cur, p_hp_max);
                break;
            }
            case 6: {
                printf("Aus.: ");
                l_draw_bar(p_stam_cur, p_stam_max, 14);
                printf(" %4d/%4d", p_stam_cur, p_stam_max);
                break;
            }
            case 7: printf("Lv.%d  Str.%d  Dex.%d", p->l_level, p->l_str, p->l_dex); break;
            case 8: printf("\033[90m---------------------------------\033[0m"); break;
            case 9: {
                if (!in_aktion) printf("\033[1;37m--- Optionen ---\033[0m");
                else            printf("\033[1;33m--- Aktionen ---\033[0m");
                break;
            }
            case 10: printf("\033[90m---------------------------------\033[0m"); break;
            default: {
                int row = i - 11;
                int cur_size = in_aktion ? aktion_size : menu_size;
                int cur_wahl = in_aktion ? aktion_wahl : wahl;
                const char** cur_menu = in_aktion ? aktion_menu : menu;

                if (row >= 0 && row < cur_size) {
                    if (row == cur_wahl) printf("\033[1;37m -> %-20.20s\033[0m", cur_menu[row]);
                    else printf("\033[90m    %-20.20s\033[0m", cur_menu[row]);
                }
                break;
            }
            }
            printf("\n");
        }

        printf(" \033[90m+---------------------------------------------------------------------------+\033[0m\n");
        printf(" \033[90m[W/S / Pfeile]\033[0m Navigieren   "
            "\033[90m[ENTER / SPACE]\033[0m Bestaetigen");
        if (in_aktion) printf("   \033[90m[ESC]\033[0m Zurueck");

        int taste = _getch();
        if (taste == 27 && in_aktion) { in_aktion = 0; continue; }
        if (taste == 224) {
            taste = _getch();
            if (!in_aktion) {
                if (taste == 72 && wahl > 0) wahl--;
                if (taste == 80 && wahl < menu_size - 1) wahl++;
            }
            else {
                if (taste == 72 && aktion_wahl > 0) aktion_wahl--;
                if (taste == 80 && aktion_wahl < aktion_size - 1) aktion_wahl++;
            }
        }
        else if (taste == 'w' || taste == 'W') {
            if (!in_aktion && wahl > 0) wahl--;
            if (in_aktion && aktion_wahl > 0) aktion_wahl--;
        }
        else if (taste == 's' || taste == 'S') {
            if (!in_aktion && wahl < menu_size - 1) wahl++;
            if (in_aktion && aktion_wahl < aktion_size - 1) aktion_wahl++;
        }
        else if (taste == 13 || taste == ' ') {
            if (!in_aktion) {
                if (wahl == 0) { in_aktion = 1; aktion_wahl = 0; }
                else if (wahl == 1) { l_equipment(liz); }
                else if (wahl == 2) { l_p_inventory(liz); }
                else if (wahl == 3) {
                    cls();
                    printf("\t\t\t  \033[1;37m=========================================================\033[0m\n");
                    printf("\t\t\t        > \033[1;31m%-20.20s\033[0m <\n", m->l_name);
                    printf("\t\t\t  \033[1;37m=========================================================\033[0m\n\n");
                    printf("\t\t\t\t%s\n\n", m->l_beschreibung);
                    printf("\t\t\t  \033[90mTyp:         %d\033[0m\n", m->l_typ);
                    printf("\t\t\t  \033[90mIntelligenz: %d\033[0m\n", m->l_intelligent);
                    printf("\t\t\t  \033[90mRunen-Drop:  %d\033[0m\n\n", m->runen);
                    system("pause");
                }
                else if (wahl == 4) {
                    int p_b = liz->player.l_pos_b;
                    int p_h = liz->player.l_pos_h;
                    int wege[4];
                    int anz_wege = 0;

                    if (p_h > 0 && liz->l_map[p_b][p_h - 1] == 0 && liz->l_ME[p_b][p_h - 1] == 0) wege[anz_wege++] = 1;
                    if (p_h < l_hohe - 1 && liz->l_map[p_b][p_h + 1] == 0 && liz->l_ME[p_b][p_h + 1] == 0) wege[anz_wege++] = 2;
                    if (p_b > 0 && liz->l_map[p_b - 1][p_h] == 0 && liz->l_ME[p_b - 1][p_h] == 0) wege[anz_wege++] = 3;
                    if (p_b < l_breite - 1 && liz->l_map[p_b + 1][p_h] == 0 && liz->l_ME[p_b + 1][p_h] == 0) wege[anz_wege++] = 4;

                    if (anz_wege == 0) {
                        cls();
                        printf("\n\n\n\n\n\n");
                        l_PrCe("\033[1;31m>> Du bist in die Enge getrieben! Keine Flucht moeglich! <<\033[0m");
                        Sleep(1500);
                        continue;
                    }
                    int flucht_erfolgreich = 0;

                    if (l_schwierigkeit == 1) { flucht_erfolgreich = 1; }
                    else {
                        int fliehen_debuf = 0;
                        if (p->l_cur_vig[BL] <= 0) fliehen_debuf += 20;
                        if (p->l_cur_vig[BR] <= 0) fliehen_debuf += 20;
                        if (p->l_cur_vig[FL] <= 0) fliehen_debuf += 5;
                        if (p->l_cur_vig[FR] <= 0) fliehen_debuf += 5;

                        int basis_schwelle = (l_schwierigkeit == 2) ? 25 : (l_schwierigkeit == 3) ? 45 : 65;
                        int flucht_schwelle = basis_schwelle + fliehen_debuf;

                        if (m->l_typ == 2 || m->l_typ == 4) {
                            int treffer_zuschlag = (l_schwierigkeit == 2) ? 30 : (l_schwierigkeit == 3) ? 50 : (l_schwierigkeit == 4) ? 75 : 0;
                            treffer_zuschlag += fliehen_debuf;

                            if (rand() % 100 < treffer_zuschlag) {
                                int zone = rand() % ANZAHL_ZONEN;
                                p->l_cur_vig[zone] -= m->l_str;
                                if (p->l_cur_vig[zone] < 0) p->l_cur_vig[zone] = 0;
                                cls();
                                printf("\n\n\n\n\n\n");
                                printf("\t\t\t  \033[1;31m>> Flucht fehlgeschlagen! <<\033[0m\n\n");
                                printf("\t\t\t  \033[1;37m%s\033[0m trifft dich von hinten!\n", m->l_name);
                                printf("\t\t\t  Du erleidest \033[1;31m%d\033[0m Schaden.\n\n", m->l_str);
                                if (fliehen_debuf > 0) printf("\t\t\t  \033[1;33m(Beinverlust macht Flucht schwerer!)\033[0m\n\n");
                                Sleep(2500);
                                int total = 0;
                                for (int z = 0; z < ANZAHL_ZONEN; z++) total += p->l_cur_vig[z];
                                if (total <= 0 || p->l_cur_vig[KF] == 0 || p->l_cur_vig[KR] == 0) return 0;
                            }
                            else { flucht_erfolgreich = 1; }
                        }
                        else {
                            if (rand() % 100 < flucht_schwelle) {
                                cls();
                                printf("\n\n\n\n\n\n");
                                printf("\t\t\t  \033[1;31m>> Flucht fehlgeschlagen! <<\033[0m\n\n");
                                if (fliehen_debuf > 0) printf("\t\t\t  \033[1;33mDeine verletzten Beine machen Flucht schwerer!\033[0m\n\n");
                                Sleep(1800);
                                return 0;
                            }
                            else { flucht_erfolgreich = 1; }
                        }
                    }

                    if (flucht_erfolgreich) {
                        int richtung = wege[rand() % anz_wege];

                        liz->l_map[p_b][p_h] = 0;
                        liz->l_map[liz->player.l_buffer_b][liz->player.l_buffer_h] = 0;
                        if (richtung == 1) p_h--;
                        else if (richtung == 2) p_h++;
                        else if (richtung == 3) p_b--;
                        else if (richtung == 4) p_b++;

                        liz->player.l_pos_b = p_b;
                        liz->player.l_pos_h = p_h;
                        liz->player.l_buffer_b = p_b;
                        liz->player.l_buffer_h = p_h;
                        liz->l_map[p_b][p_h] = 2;

                        liz->monster[m_index].flucht_cooldown = 2;

                        cls();
                        printf("\n\n\n\n\n\n");
                        l_PrCe("\033[1;32m>> Du bist erfolgreich entkommen! <<\033[0m");
                        Sleep(1200);
                        return 1;
                    }
                }
            }
            else {
                if (aktion_wahl == 0) {
                    l_battle_logic_p(liz, m_index);
                    return 0;
                }
                else if (aktion_wahl == 1) {
                    l_inventar* inv = &liz->player.inventar;
                    int anz = 0;
                    for (int i = 0; i < QUICK_GROESSE; i++) if (inv->quick[i].l_id != 0) anz++;

                    if (anz == 0) {
                        cls();
                        printf("\n\n\t\t\033[1;33mKeine Items in den Quick Slots!\033[0m\n\n");
                        Sleep(1200);
                    }
                    else {
                        int sub = 0;
                        while (sub < QUICK_GROESSE && inv->quick[sub].l_id == 0) sub++;

                        while (1) {
                            cls();
                            printf("\n  +------------------------------------------+\n");
                            printf("  | %-40s |\n", "Quick Slot benutzen");
                            printf("  +------------------------------------------+\n");

                            for (int i = 0; i < QUICK_GROESSE; i++) {
                                if (inv->quick[i].l_id == 0) continue;
                                if (i == sub) printf("  | \033[1;37m-> %-18.18s x%-3d Str:%5d\033[0m     |\n", inv->quick[i].l_name, inv->quick[i].l_menge, inv->quick[i].l_staerke);
                                else printf("  |    %-18.18s x%-3d Str:%5d     |\n", inv->quick[i].l_name, inv->quick[i].l_menge, inv->quick[i].l_staerke);
                            }
                            printf("  +------------------------------------------+\n");
                            printf("  \033[90m[W/S]\033[0m Auswahl  "
                                "\033[90m[ENTER]\033[0m Benutzen  "
                                "\033[90m[ESC]\033[0m Zurueck\n\n");

                            int qt = _getch();
                            if (qt == 27) break;

                            if (qt == 224) {
                                qt = _getch();
                                if (qt == 72) { int prev = sub - 1; while (prev >= 0 && inv->quick[prev].l_id == 0) prev--; if (prev >= 0) sub = prev; }
                                if (qt == 80) { int next = sub + 1; while (next < QUICK_GROESSE && inv->quick[next].l_id == 0) next++; if (next < QUICK_GROESSE) sub = next; }
                            }
                            else if (qt == 'w' || qt == 'W') { int prev = sub - 1; while (prev >= 0 && inv->quick[prev].l_id == 0) prev--; if (prev >= 0) sub = prev; }
                            else if (qt == 's' || qt == 'S') { int next = sub + 1; while (next < QUICK_GROESSE && inv->quick[next].l_id == 0) next++; if (next < QUICK_GROESSE) sub = next; }
                            else if (qt == 13 || qt == ' ') {
                                l_item* sel = &inv->quick[sub];
                                if (sel->l_id == 0) break;

                                if (sel->l_typ == TYP_VERBRAUCH) {
                                    int heal = sel->l_staerke;
                                    int geheilt = 0;
                                    for (int z = 0; z < ANZAHL_ZONEN && heal > 0; z++) {
                                        int diff = p->l_max_vig[z] - p->l_cur_vig[z];
                                        int h = (heal < diff) ? heal : diff;
                                        p->l_cur_vig[z] += h;
                                        heal -= h; geheilt += h;
                                    }
                                    cls();
                                    printf("\n\n\t\t\033[1;32m>>> %s benutzt! <<<\033[0m\n\n", sel->l_name);
                                    if (geheilt > 0) printf("\t\tDu heilst \033[1;32m%d\033[0m Leben.\n\n", geheilt);
                                    else printf("\t\tDu bist bereits voll geheilt.\n\n");
                                    sel->l_menge--;
                                    if (sel->l_menge <= 0) memset(sel, 0, sizeof(l_item));
                                    Sleep(1200);
                                    return 0;
                                }
                                else if (sel->l_typ == TYP_BUFF) {
                                    l_effekte* e = &sel->effekte;
                                    if (e->l_regen > 0)     p->status.l_regen += e->l_regen;
                                    if (e->l_steinhaut > 0) p->status.l_steinhaut += e->l_steinhaut;
                                    if (e->l_raserei > 0)   p->status.l_raserei += e->l_raserei;
                                    cls();
                                    printf("\n\n\t\t\033[1;33m>>> %s aktiviert! <<<\033[0m\n\n", sel->l_name);
                                    Sleep(1200);
                                    sel->l_menge--;
                                    if (sel->l_menge <= 0) memset(sel, 0, sizeof(l_item));
                                    return 0;
                                }
                            }
                        }
                    }
                }
                else if (aktion_wahl == 2) {
                    int ausdauer_max = l_berechne_stamina(EFF_END(p));
                    int ausdauer_kosten = ausdauer_max / 5;
                    if (ausdauer_kosten < 5) ausdauer_kosten = 5;

                    int bein_debuf = 0;
                    if (p->l_cur_vig[BL] <= 0) bein_debuf += 10;
                    if (p->l_cur_vig[BR] <= 0) bein_debuf += 10;
                    ausdauer_kosten += bein_debuf;

                    if (p->l_cur_end >= ausdauer_kosten) {
                        p->l_cur_end -= ausdauer_kosten;
                        cls();
                        if (bein_debuf > 0) printf("\n\n\t\t\033[1;33m>>> Du weichst m hsam aus! (Beinverlust) <<<\033[0m\n\n");
                        else printf("\n\n\t\t\033[1;32m>>> Du weichst dem Angriff aus! <<<\033[0m\n\n");
                        printf("\t\t\033[90m-%d Ausdauer  (verbleibend: %d / %d)\033[0m\n\n", ausdauer_kosten, p->l_cur_end, ausdauer_max);
                        Sleep(1200);
                        return 2;
                    }
                    else {
                        cls();
                        printf("\n\n\t\t\033[1;31m>>> Zu wenig Ausdauer zum Ausweichen! <<<\033[0m\n\n");
                        printf("\t\tBen tigt: \033[1;31m%d\033[0m  Verf gbar: \033[1;31m%d\033[0m\n\n", ausdauer_kosten, p->l_cur_end);
                        Sleep(1500);
                    }
                }
                else if (aktion_wahl == 3) {
                    p->l_cur_end += (p_stam_max / 3);
                    if (p->l_cur_end > p_stam_max) p->l_cur_end = p_stam_max;
                    return 0;
                }
            }
        }
    }
}
void l_battle_logic_p(l_infozentrum* liz, int m_index) {
    int kann_treffen = 0;
    l_player* p = &liz->player;

    if (liz->monster[m_index].l_typ == 1 || liz->monster[m_index].l_typ == 2 || liz->monster[m_index].l_typ == 6) { kann_treffen = 1; }
    else if (liz->monster[m_index].l_typ == 3 && (p->inventar.l_waffe.l_typ == TYP_BOGEN || p->inventar.l_waffe.l_typ == TYP_GROSSSCHWERT)) { kann_treffen = 1; }
    else if (liz->monster[m_index].l_typ == 4 && liz->player.inventar.l_waffe.l_typ == TYP_BOGEN) { kann_treffen = 1; }

    if (!kann_treffen && l_tutorial) {
        cls();
        printf("\n\n\t\t\033[1;31mDeine Waffe kann diesen Gegner nicht treffen!\033[0m\n\n");
        if (liz->monster[m_index].l_typ == 3) printf("\t\t\033[90mTipp: Bogen oder Grossschwert verwenden.\033[0m\n\n");
        else if (liz->monster[m_index].l_typ == 4) printf("\t\t\033[90mTipp: Nur Bogen trifft diesen Gegner.\033[0m\n\n");
        system("pause");
        return;
    }
    else if (!kann_treffen) {
        cls();
        printf("\n\n\t\t\033[1;31mDeine Waffe kann diesen Gegner nicht treffen!\033[0m\n\n");
        if (liz->monster[m_index].l_typ == 3) printf("\t\t\033[90mBogen oder Grossschwert benoetigt.\033[0m\n\n");
        else if (liz->monster[m_index].l_typ == 4) printf("\t\t\033[90mNur ein Bogen trifft diesen Gegner.\033[0m\n\n");
        system("pause");
        return;
    }

    int ausdauer_kosten = (p->inventar.l_waffe.l_id > 0) ? (p->inventar.l_waffe.l_staerke / 4 + 5) : 10;
    int cur_end = p->l_cur_end - ausdauer_kosten;
    if (cur_end < 0) {
        p->l_cur_end = 0;
        cls();
        printf("\n\n\t\t\033[1;31mZu wenig Ausdauer zum Angreifen!\033[0m\n\n");
        printf("\t\tAusdauer: \033[1;31m%d\033[0m  Benoetigt: \033[1;31m%d\033[0m\n\n", p->l_cur_end, ausdauer_kosten);
        system("pause");
        return;
    }
    p->l_cur_end = cur_end;

    if ((rand() % 100) < l_berechne_treffer(p->l_dex)) {
        int schaden = l_berechne_gesamtschaden(
            EFF_STR(&liz->player),
            EFF_DEX(&liz->player),
            &liz->player.inventar.l_waffe,
            &liz->monster[m_index].resistenzen,
            &liz->monster[m_index].elementar_aktuell,
            &liz->monster[m_index].status,
            &liz->monster[m_index].l_helm,
            &liz->monster[m_index].l_koerper,
            &liz->monster[m_index].l_hose,
            &liz->monster[m_index].l_schuhe
        );

        float arm_mod = 1.0f;
        if (p->l_cur_vig[AL] <= 0 && p->l_cur_vig[AR] <= 0) arm_mod *= 0.50f;
        else if (p->l_cur_vig[AL] <= 0 || p->l_cur_vig[AR] <= 0) arm_mod *= 0.75f;

        if (p->l_cur_vig[HL] <= 0 && p->l_cur_vig[HR] <= 0) arm_mod *= 0.70f;
        else if (p->l_cur_vig[HL] <= 0 || p->l_cur_vig[HR] <= 0) arm_mod *= 0.85f;

        schaden = (int)(schaden * arm_mod);
        if (schaden < 1) schaden = 1;

        liz->monster[m_index].l_cur_vig -= schaden;

        cls();
        printf("\n\n\t\t\033[1;32mTREFFER!\033[0m\n");
        printf("\t\tDu hast %d Gesamtschaden verursacht.\n", schaden);

        // Elementar-Anzeige
        if (liz->monster[m_index].status.l_blutung >= BLTUNG_Ladung) printf("\t\t  \033[1;31m> Monster faengt an zu bluten!\033[0m\n");

        // K rperteil-Nachteil / Arm Verleztzt
        if (arm_mod < 1.0f) {
            printf("\n");
            if (arm_mod <= 0.50f) printf("\t\t  \033[1;31m[Beide Arme verletzt: nur %d%% Schaden]\033[0m\n", (int)(arm_mod * 100));
            else if (arm_mod <= 0.70f) printf("\t\t  \033[1;33m[Beide H nde verletzt: nur %d%% Schaden]\033[0m\n", (int)(arm_mod * 100));
            else printf("\t\t  \033[1;33m[Arm verletzt: Schaden reduziert auf %d%%]\033[0m\n", (int)(arm_mod * 100));
        }
        system("pause");
    }
    else {
        cls();
        printf("\n\n\n");
        l_PrCe("Du hast den Gegner nicht getroffen.");
        system("pause");
    }
}
int  l_berechne_gesamtschaden(int a_str, int a_dex, l_item* waffe, l_elementar* ziel_res, l_elementar* ziel_elem_akt, l_effekte* ziel_status, l_item* helm, l_item* koerper, l_item* hose, l_item* schuhe) {
    int physisch = 0;
    int w_str = (waffe != NULL && waffe->l_id > 0) ? waffe->l_staerke : 0;
    int w_typ = (waffe != NULL && waffe->l_id > 0) ? waffe->l_typ : 0;

    if (w_typ == 7) {
        int str_anteil = l_berechne_schaden(a_str) / 2;
        int dex_anteil = (a_dex * w_str) / 15;
        physisch = str_anteil + dex_anteil + w_str;
    }
    else { physisch = l_berechne_schaden(a_str) + w_str; }

    int verteidigung = 0;
    if (helm != NULL && helm->l_id > 0)       verteidigung += helm->l_staerke;
    if (koerper != NULL && koerper->l_id > 0) verteidigung += koerper->l_staerke;
    if (hose != NULL && hose->l_id > 0)       verteidigung += hose->l_staerke;
    if (schuhe != NULL && schuhe->l_id > 0)   verteidigung += schuhe->l_staerke;

    physisch -= verteidigung;
    if (physisch < 0) physisch = 0;

    int feuer = 0, frost = 0, blitz = 0;

    if (waffe != NULL && waffe->l_id > 0) {
        feuer = waffe->elemente.feuer - ziel_res->feuer;
        frost = waffe->elemente.frost - ziel_res->frost;
        blitz = waffe->elemente.blitz - ziel_res->blitz;

        if (feuer < 0) feuer = 0;
        if (frost < 0) frost = 0;
        if (blitz < 0) blitz = 0;

        int gift = waffe->elemente.gift - ziel_res->gift;
        if (gift > 0) {
            ziel_elem_akt->gift += gift;
        }
        if (waffe->effekte.l_blutung > 0) {
            ziel_status->l_blutung += waffe->effekte.l_blutung;
        }
    }

    int gesamt = physisch + feuer + frost + blitz;
    if (gesamt < 1) gesamt = 1;

    return gesamt;
}
void l_battle_logic_m(l_infozentrum* liz, int m_index) {
    l_monster* m = &liz->monster[m_index];
    l_player* p = &liz->player;
    int ki_typ = m->l_intelligent;

    int stam_kosten = (m->l_waffe.l_id > 0) ? (m->l_waffe.l_staerke / 5 + 5) : 10;
    m->l_cur_dex -= stam_kosten;
    if (m->l_cur_dex < 0) m->l_cur_dex = 0;

    int m_treffer = l_berechne_treffer(m->l_dex);
    int p_ausweich = (EFF_DEX(p) - 1) / 3;
    int stam_debuf = (m->l_cur_dex <= 0) ? 40 : (m->l_cur_dex < m->l_dex / 3) ? 20 : 0;
    int finale_treffer = m_treffer - p_ausweich - stam_debuf;
    if (finale_treffer < 5) finale_treffer = 5;

    if ((rand() % 100) >= finale_treffer) {
        cls();
        if (stam_debuf > 0) printf("\n\n\t\t\033[1;33m%s ist erschoepft und trifft nicht!\033[0m\n\n", m->l_name);
        else printf("\n\n\t\t\033[1;32m%s hat verfehlt!\033[0m\n\n", m->l_name);
        system("pause");
        return;
    }

    int schaden = l_berechne_monster_gesamtschaden(
        m->l_str, &m->l_waffe,
        &p->resistenzen, &p->elementar_aktuell, &p->status,
        &p->inventar.l_helm, &p->inventar.l_koerper,
        &p->inventar.l_hose, &p->inventar.l_schuhe
    );

    const char* zonen_namen[] = {
        "Kopf","Koerper","Arm L","Arm R",
        "Hand L","Hand R","Bein L","Bein R","Fuss L","Fuss R"
    };

    if (ki_typ == 1) {
        int ziel_zone = l_ziel_zone_waehlen(p, m->l_dex);
        p->l_cur_vig[ziel_zone] -= schaden;
        if (p->l_cur_vig[ziel_zone] < 0) p->l_cur_vig[ziel_zone] = 0;

        cls();
        printf("\n\n\t\t\033[1;31m%s greift an!\033[0m\n\n", m->l_name);
        printf("\t\t%d Schaden am \033[1;31m%s\033[0m.\n", schaden, zonen_namen[ziel_zone]);
        system("pause");
    }
    else if (ki_typ == 2) {
        int ziel_zone = l_ziel_zone_waehlen(p, m->l_dex);
        p->l_cur_vig[ziel_zone] -= schaden;
        if (p->l_cur_vig[ziel_zone] < 0) p->l_cur_vig[ziel_zone] = 0;

        int blutung = 0;
        if (rand() % 10 < 3) { p->status.l_blutung += 1; blutung = 1; }

        cls();
        printf("\n\n\t\t\033[1;31m%s greift an!\033[0m\n\n", m->l_name);
        printf("\t\t%d Schaden am \033[1;31m%s\033[0m.\n", schaden, zonen_namen[ziel_zone]);
        if (blutung) printf("\t\t\033[1;31m> Du f ngst an zu bluten!\033[0m\n");
        system("pause");
    }
    else if (ki_typ == 3) {
        int gezielt = (rand() % 10 < 6);
        int ziel_zone;
        if (gezielt && p->l_cur_vig[KR] > 0) {
            int verletzte = 0;
            for (int z = 0; z < ANZAHL_ZONEN; z++) {
                if (z != KF && p->l_max_vig[z] > 0 && p->l_cur_vig[z] < p->l_max_vig[z] / 2) verletzte++;
            }
            ziel_zone = (verletzte >= 4 && p->l_cur_vig[KF] > 0) ? KF : KR;
        }
        else { ziel_zone = l_ziel_zone_waehlen(p, m->l_dex); }

        int finaler_schaden = gezielt ? (int)(schaden * 1.3f) : schaden;
        p->l_cur_vig[ziel_zone] -= finaler_schaden;
        if (p->l_cur_vig[ziel_zone] < 0) p->l_cur_vig[ziel_zone] = 0;

        cls();
        printf("\n\n\t\t\033[1;31m%s zielt gezielt!\033[0m\n\n", m->l_name);
        if (gezielt) printf("\t\t\033[1;31m> Gezielter Treffer!\033[0m\n");
        printf("\t\t%d Schaden am \033[1;31m%s\033[0m.\n", finaler_schaden, zonen_namen[ziel_zone]);
        system("pause");
    }
    else if (ki_typ == 4) {
        int schaden_pro = schaden / 2;
        if (schaden_pro < 1) schaden_pro = 1;

        cls();
        printf("\n\n\t\t\033[1;31m%s schl gt mit voller Wucht!\033[0m\n\n", m->l_name);

        int treffer = 0;
        for (int t = 0; t < 2; t++) {
            int ziel_zone = l_ziel_zone_waehlen(p, m->l_dex);
            if (p->l_cur_vig[ziel_zone] > 0) {
                p->l_cur_vig[ziel_zone] -= schaden_pro;
                if (p->l_cur_vig[ziel_zone] < 0) p->l_cur_vig[ziel_zone] = 0;
                printf("\t\t\033[1;31m> %d Schaden am %s!\033[0m\n", schaden_pro, zonen_namen[ziel_zone]);
                treffer++;
            }
        }
        printf("\t\tGesamt: \033[1;31m%d\033[0m Schaden.\n", schaden_pro * treffer);
        system("pause");
    }
}
int  l_berechne_monster_gesamtschaden(int m_str, l_item* waffe, l_elementar* ziel_res, l_elementar* ziel_elem_akt, l_effekte* ziel_status, l_item* helm, l_item* koerper, l_item* hose, l_item* schuhe) {
    int w_str = (waffe != NULL && waffe->l_id > 0) ? waffe->l_staerke : 0;
    int physisch = m_str + w_str;

    int verteidigung = 0;
    if (helm != NULL && helm->l_id > 0) verteidigung += helm->l_staerke;
    if (koerper != NULL && koerper->l_id > 0) verteidigung += koerper->l_staerke;
    if (hose != NULL && hose->l_id > 0) verteidigung += hose->l_staerke;
    if (schuhe != NULL && schuhe->l_id > 0) verteidigung += schuhe->l_staerke;
    verteidigung = verteidigung / 2;

    physisch -= verteidigung;
    if (physisch < 1) physisch = 1;

    switch (l_schwierigkeit) {
    case 1: physisch = (int)(physisch * 0.60f); break;
    case 2: physisch = (int)(physisch * 0.85f); break;
    case 3: physisch = (int)(physisch * 1.20f); break;
    case 4: physisch = (int)(physisch * 1.65f); break;
    }
    if (physisch < 1) physisch = 1;

    int feuer = 0, frost = 0, blitz = 0;
    if (waffe != NULL && waffe->l_id > 0) {
        feuer = waffe->elemente.feuer - ziel_res->feuer;
        frost = waffe->elemente.frost - ziel_res->frost;
        blitz = waffe->elemente.blitz - ziel_res->blitz;
        if (feuer < 0) feuer = 0;
        if (frost < 0) frost = 0;
        if (blitz < 0) blitz = 0;

        int gift = waffe->elemente.gift - ziel_res->gift;
        if (gift > 0) ziel_elem_akt->gift += gift;
        if (waffe->effekte.l_blutung > 0) ziel_status->l_blutung += waffe->effekte.l_blutung;
    }

    int gesamt = physisch + feuer + frost + blitz;
    if (gesamt < 1) gesamt = 1;
    return gesamt;
}
int  l_ziel_zone_waehlen(l_player* p, int m_dex) {
    int verletzte = 0;
    for (int z = 0; z < ANZAHL_ZONEN; z++) {
        if (z != KF && p->l_max_vig[z] > 0 &&
            p->l_cur_vig[z] < p->l_max_vig[z] / 2)
            verletzte++;
    }

    int kopf_chance = 5 + (m_dex / 20);
    int Koerper_chance = 25;
    int glied_chance = 100 - kopf_chance - Koerper_chance;

    int r = rand() % 100;
    int ziel_zone;

    if (r < glied_chance) {
        int limbs[] = { AL, AR, HL, HR, BL, BR, FL, FR };
        int versuche = 0;
        do {
            ziel_zone = limbs[rand() % 8];
            versuche++;
        } while (p->l_cur_vig[ziel_zone] <= 0 && versuche < 16);
    }
    else if (r < glied_chance + Koerper_chance) { ziel_zone = KR; }
    else {
        if (verletzte >= 3 && p->l_cur_vig[KF] > 0) ziel_zone = KF;
        else ziel_zone = KR;
    }

    if (p->l_cur_vig[ziel_zone] <= 0) {
        for (int z = 1; z < ANZAHL_ZONEN; z++) {
            if (p->l_cur_vig[z] > 0) { ziel_zone = z; break; }
        }
        if (p->l_cur_vig[ziel_zone] <= 0 && p->l_cur_vig[KF] > 0)
            ziel_zone = KF;
    }
    return ziel_zone;
}
void l_status_effekte_auswerten(l_player* p) {

    // Blutung: baut sich auf, ab Schwelle massiver Schaden
    if (p->status.l_blutung >= BLTUNG_Ladung) {
        int blut_schaden = 0;
        for (int z = 0; z < ANZAHL_ZONEN; z++) blut_schaden += p->l_max_vig[z];
        blut_schaden = blut_schaden / 4; // 25% max HP

        int zone = rand() % ANZAHL_ZONEN;
        p->l_cur_vig[zone] -= blut_schaden;
        if (p->l_cur_vig[zone] < 0) p->l_cur_vig[zone] = 0;
        p->status.l_blutung = 0;

        cls();
        printf("\n\n\t\t\033[1;31m>>> BLUTUNG! %d Schaden! <<<\033[0m\n\n", blut_schaden);
        system("pause");
    }

    // Gift: Schaden pro Runde
    if (p->elementar_aktuell.gift > 0) {
        int gift_schaden = p->elementar_aktuell.gift * 3;
        int zone = rand() % ANZAHL_ZONEN;
        p->l_cur_vig[zone] -= gift_schaden;
        if (p->l_cur_vig[zone] < 0) p->l_cur_vig[zone] = 0;
        p->elementar_aktuell.gift--;

        cls();
        printf("\n\n\t\t\033[1;32m>>> GIFT: %d Schaden! (noch %d Runden) <<<\033[0m\n\n", gift_schaden, p->elementar_aktuell.gift);
        system("pause");
    }
}
void l_bosskampf_artorias(l_infozentrum* liz, int m_index) {
    cls();
    printf("\n\n\n");
    l_PrCe("\"Warum lebst du noch, du muetest tot sein?\"");
    Sleep(3000);
    l_PrCe("Artorias, Der Abgrundschreiter Greift an!");
    Sleep(2000);
    char art[31][100] = { 0 };
    FILE* file;
    if (fopen_s(&file, liz->monster[m_index].l_ascii_art_datei, "r") == 0 && file != NULL) {
        int line = 0;
        while (line < 31 && fgets(art[line], sizeof(art[line]), file) != NULL) {
            art[line][strcspn(art[line], "\r\n")] = 0;
            line++;
        }
        fclose(file);
    }
    else { strcpy_s(art[3], sizeof(art[3]), "  (ASCII-Art nicht gefunden)"); }
    cls();
    for (int i = 0; i < 31; i++) { printf("%s\n", art[i]); }
    Sleep(2000);
    cls();

    int boss_hp = liz->monster_c[30].l_cur_vig;
    int boss_max_hp = liz->monster_c[30].l_vig;
    int p_b = liz->player.l_pos_b;
    int p_h = liz->player.l_pos_h;

    int b_b = liz->monster[m_index].pos_b;
    int b_h = liz->monster[m_index].pos_h;
    if (b_b == 0 || b_h == 0) { b_b = p_b + 2; b_h = p_h; }

    int roll_frames = 0;
    int roll_cooldown = 0;
    int p_move_cooldown = 0;
    int boss_attack_timer = 0;
    int boss_attack_phase = 0;
    int p_attack_cooldown = 0;
    int boss_attack_type = 0;
    int boss_telegraph_b = 0;
    int boss_telegraph_h = 0;
    int boss_stagger = 0;
    int boss_dir_b = 1;
    int boss_dir_h = 0;

    int boss_angriff_geschwindkeit = 30;
    if (l_schwierigkeit == 3) boss_angriff_geschwindkeit = 25;
    if (l_schwierigkeit == 4) boss_angriff_geschwindkeit = 20;

    int spieler_geschwindigkeit = 4;
    if (l_schwierigkeit != 1) {
        if (liz->player.l_cur_vig[BL] == 0) spieler_geschwindigkeit += 2;
        if (liz->player.l_cur_vig[BR] == 0) spieler_geschwindigkeit += 2;
        else if (liz->player.l_cur_vig[BL] < (liz->player.l_max_vig[BL] / 2) && liz->player.l_cur_vig[BR] < (liz->player.l_max_vig[BR] / 2)) spieler_geschwindigkeit += 2;
        if (liz->player.l_cur_vig[FL] == 0) spieler_geschwindigkeit += 1;
        if (liz->player.l_cur_vig[FR] == 0) spieler_geschwindigkeit += 1;
        else if (liz->player.l_cur_vig[FL] < (liz->player.l_max_vig[FL] / 2) && liz->player.l_cur_vig[FR] < (liz->player.l_max_vig[FR] / 2)) spieler_geschwindigkeit += 1;
    }

    int max_size = l_breite * l_breite;
    char* buffer = (char*)calloc(max_size, sizeof(char));

    int boss_anvisiert = 40;
    ULONGLONG last_frame = GetTickCount64();
    while (boss_hp > 0 && liz->player.l_cur_vig[KF] > 0 && liz->player.l_cur_vig[KR] > 0) {
        ULONGLONG now = GetTickCount64();
        if (now - last_frame < boss_angriff_geschwindkeit) continue;
        last_frame = now;

        if (roll_frames > 0) roll_frames--;
        if (roll_cooldown > 0) roll_cooldown--;
        if (p_move_cooldown > 0) p_move_cooldown--;
        if (p_attack_cooldown > 0) p_attack_cooldown--;
        if (boss_stagger > 0) boss_stagger--;

        if (_kbhit()) {
            int taste = _getch();
            int next_b = p_b;
            int next_h = p_h;

            if (taste == 224) {
                taste = _getch();
                if (taste == 72) next_h--;
                if (taste == 80) next_h++;
                if (taste == 75) next_b--;
                if (taste == 77) next_b++;
            }
            else if (taste == 73 || taste == 105) {
                l_p_inventory(liz);
                last_frame = GetTickCount64();
                cls();
                continue;
            }
            else if (taste == 85 || taste == 117) {
                l_p_AT(liz);
                last_frame = GetTickCount64();
                cls();
                continue;
            }
            else {
                if (taste == 'w' || taste == 'W') next_h--;
                if (taste == 's' || taste == 'S') next_h++;
                if (taste == 'a' || taste == 'A') next_b--;
                if (taste == 'd' || taste == 'D') next_b++;

                if (taste == ' ' && roll_cooldown == 0) {
                    roll_frames = 10;
                    roll_cooldown = 25;
                }

                if ((taste == 'e' || taste == 'E' || taste == 13) && p_attack_cooldown == 0) {
                    if (abs(p_b - b_b) <= 1 && abs(p_h - b_h) <= 1) {
                        boss_hp -= (15 + (liz->player.l_str / 2));
                    }
                    p_attack_cooldown = 12;
                }
            }

            if (p_move_cooldown == 0 && liz->l_map[next_b][next_h] != 1) {
                if (next_b != p_b || next_h != p_h) { p_move_cooldown = spieler_geschwindigkeit; }
                p_b = next_b;
                p_h = next_h;
            }
        }

        boss_attack_timer++;

        if (boss_attack_phase == 0) {
            if (boss_attack_timer % 10 == 0) {
                static int history_b[5] = { 0 };
                static int history_h[5] = { 0 };

                int best_dist = 999999;
                int next_b = b_b, next_h = b_h;

                int nb[4] = { b_b, b_b, b_b - 1, b_b + 1 };
                int nh[4] = { b_h - 1, b_h + 1, b_h, b_h };

                for (int i = 0; i < 4; i++) {
                    int xb = nb[i];
                    int xh = nh[i];

                    if (xb >= 0 && xb < l_breite && xh >= 0 && xh < l_hohe && liz->l_map[xb][xh] != 1) {
                        int dist = abs(p_b - xb) + abs(p_h - xh);

                        for (int h = 0; h < 5; h++) {
                            if (history_b[h] == xb && history_h[h] == xh) {
                                dist += 50;
                                break;
                            }
                        }

                        if (dist < best_dist) {
                            best_dist = dist;
                            next_b = xb;
                            next_h = xh;
                        }
                    }
                }

                for (int h = 4; h > 0; h--) {
                    history_b[h] = history_b[h - 1];
                    history_h[h] = history_h[h - 1];
                }
                history_b[0] = b_b;
                history_h[0] = b_h;

                b_b = next_b;
                b_h = next_h;
            }
            if (boss_stagger == 0 && boss_attack_timer > boss_anvisiert && abs(p_b - b_b) <= 4 && abs(p_h - b_h) <= 4) {
                int dist = abs(p_b - b_b) + abs(p_h - b_h);

                int diff_b = p_b - b_b;
                int diff_h = p_h - b_h;
                if (abs(diff_b) >= abs(diff_h) && diff_b != 0) { boss_dir_b = (diff_b > 0) ? 1 : -1; boss_dir_h = 0; }
                else if (diff_h != 0) { boss_dir_b = 0; boss_dir_h = (diff_h > 0) ? 1 : -1; }

                int boss_low_life = (boss_hp <= boss_max_hp / 4);

                if (boss_low_life && dist > 2 && rand() % 2 == 0) { boss_attack_type = 6; }
                else if (dist > 2) { boss_attack_type = 1; }
                else {
                    int wahl = rand() % 5;
                    if (wahl == 0) boss_attack_type = 0;      // Nahkampf
                    else if (wahl == 1) boss_attack_type = 2; // grosser Flaechenangriff
                    else if (wahl == 2) boss_attack_type = 3; // Hieb
                    else if (wahl == 3) boss_attack_type = 4; // Stiche 
                    else boss_attack_type = 5;                // kleiner schneller Wirbel
                }
                boss_telegraph_b = p_b;
                boss_telegraph_h = p_h;
                boss_attack_phase = 1;
                boss_attack_timer = 0;
            }
        }
        else if (boss_attack_phase == 1) {
            int telegraph_dauer = (boss_attack_type == 2) ? 25 :
                (boss_attack_type == 1) ? 20 :
                (boss_attack_type == 6) ? 12 :
                (boss_attack_type == 4) ? 12 :
                (boss_attack_type == 3) ? 10 :
                (boss_attack_type == 5) ? 10 : 15; // Angriffs geschwindkeit
            if (boss_attack_timer > telegraph_dauer) {
                boss_attack_phase = (boss_attack_type == 6) ? 4 : 2;
                boss_attack_timer = 0;
            }
        }
        else if (boss_attack_phase == 2) {
            int treffer = 0;
            int schaden = 10;

            if (boss_attack_type == 0) {
                if (abs(p_b - b_b) <= 1 && abs(p_h - b_h) <= 1) treffer = 1;
                schaden = 10;
            }
            else if (boss_attack_type == 1) {
                if (liz->l_map[boss_telegraph_b][boss_telegraph_h] != 1) {
                    b_b = boss_telegraph_b;
                    b_h = boss_telegraph_h;
                }
                if (abs(p_b - b_b) <= 1 && abs(p_h - b_h) <= 1) treffer = 1;
                schaden = 50;
            }
            else if (boss_attack_type == 2) {
                if (abs(p_b - b_b) <= 2 && abs(p_h - b_h) <= 2) treffer = 1;
                schaden = 20;
            }
            else if (boss_attack_type == 3) {
                // Hieb: 3 Felder breit, genau 1 Feld vor dem Boss in Blickrichtung
                if (boss_dir_h == 0) {
                    int frontb = b_b + boss_dir_b;
                    if (p_b == frontb && p_h >= b_h - 1 && p_h <= b_h + 1) treffer = 1;
                }
                else {
                    int fronth = b_h + boss_dir_h;
                    if (p_h == fronth && p_b >= b_b - 1 && p_b <= b_b + 1) treffer = 1;
                }
                schaden = 15;
            }
            else if (boss_attack_type == 4) {
                // Stiche: 3 Felder tief, in einer Linie vor dem Boss
                if (boss_dir_h == 0) {
                    if (p_h == b_h && (p_b == b_b + boss_dir_b || p_b == b_b + boss_dir_b * 2 || p_b == b_b + boss_dir_b * 3)) treffer = 1;
                }
                else {
                    if (p_b == b_b && (p_h == b_h + boss_dir_h || p_h == b_h + boss_dir_h * 2 || p_h == b_h + boss_dir_h * 3)) treffer = 1;
                }
                schaden = 18;
            }
            else { // Typ 5: kleiner schneller Wirbel
                if (abs(p_b - b_b) <= 1 && abs(p_h - b_h) <= 1) treffer = 1;
                schaden = 8;
            }

            if (treffer && roll_frames <= 0) {
                liz->player.l_cur_vig[KF] -= schaden;
                liz->player.l_cur_vig[KR] -= schaden;
            }
            boss_attack_phase = 0;
            boss_stagger = 15; // Pause nach Angriff
        }
        else if (boss_attack_phase == 4) {
            // Renn und Stossangriff
            if (boss_attack_timer % 2 == 0) {
                if (b_b < boss_telegraph_b && liz->l_map[b_b + 1][b_h] != 1) b_b++;
                else if (b_b > boss_telegraph_b && liz->l_map[b_b - 1][b_h] != 1) b_b--;
                else if (b_h < boss_telegraph_h && liz->l_map[b_b][b_h + 1] != 1) b_h++;
                else if (b_h > boss_telegraph_h && liz->l_map[b_b][b_h - 1] != 1) b_h--;
                else { boss_attack_timer = 9999; }
            }
            int am_ziel = (b_b == boss_telegraph_b && b_h == boss_telegraph_h);
            int spieler_im_weg = (abs(p_b - b_b) <= 1 && abs(p_h - b_h) <= 1);
            if (am_ziel || spieler_im_weg || boss_attack_timer > 60) {
                // Lauf vorbei (Ziel erreicht, Spieler getroffen, oder Sicherheits-Timeout) -> Stoss pruefen
                int treffer = (abs(p_b - b_b) <= 1 && abs(p_h - b_h) <= 1) ? 1 : 0;
                if (treffer && roll_frames <= 0) {
                    liz->player.l_cur_vig[KF] -= 30;
                    liz->player.l_cur_vig[KR] -= 30;
                }
                boss_attack_phase = 0;
                boss_attack_timer = 0;
                boss_stagger = 15;
            }
        }

        COORD home = { 0, 0 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), home);

        int b_idx = 0;

        b_idx += sprintf_s(&buffer[b_idx], max_size - b_idx, "\n  [ BOSS HP: %4d / %4d ]    [ DEINE HP: %4d ]\n", boss_hp, boss_max_hp, liz->player.l_cur_vig[KF] + liz->player.l_cur_vig[KR]);
        b_idx += sprintf_s(&buffer[b_idx], max_size - b_idx, "  [LEERTASTE] Ausweichen | [E] Angreifen | [WASD] Laufen | [I] Inventar | [U] Ausruestung\n");

        int dz_radius = (boss_attack_type == 2) ? 2 : (boss_attack_type == 3 || boss_attack_type == 4) ? 0 : 1;
        int dz_b = (boss_attack_phase == 1 && (boss_attack_type == 1 || boss_attack_type == 6)) ? boss_telegraph_b : b_b;
        int dz_h = (boss_attack_phase == 1 && (boss_attack_type == 1 || boss_attack_type == 6)) ? boss_telegraph_h : b_h;

        int line_b[3] = { -1, -1, -1 };
        int line_h[3] = { -1, -1, -1 };
        if (boss_attack_phase > 0 && boss_attack_type == 3) {
            if (boss_dir_h == 0) {
                int frontb = b_b + boss_dir_b;
                line_b[0] = frontb; line_h[0] = b_h - 1;
                line_b[1] = frontb; line_h[1] = b_h;
                line_b[2] = frontb; line_h[2] = b_h + 1;
            }
            else {
                int fronth = b_h + boss_dir_h;
                line_b[0] = b_b - 1; line_h[0] = fronth;
                line_b[1] = b_b;     line_h[1] = fronth;
                line_b[2] = b_b + 1; line_h[2] = fronth;
            }
        }
        else if (boss_attack_phase > 0 && boss_attack_type == 4) {
            if (boss_dir_h == 0) {
                line_b[0] = b_b + boss_dir_b;     line_h[0] = b_h;
                line_b[1] = b_b + boss_dir_b * 2; line_h[1] = b_h;
                line_b[2] = b_b + boss_dir_b * 3; line_h[2] = b_h;
            }
            else {
                line_b[0] = b_b; line_h[0] = b_h + boss_dir_h;
                line_b[1] = b_b; line_h[1] = b_h + boss_dir_h * 2;
                line_b[2] = b_b; line_h[2] = b_h + boss_dir_h * 3;
            }
        }

        for (int y = 0; y < l_hohe; y++) {
            for (int x = 0; x < l_breite; x++) {
                int is_danger_zone = (boss_attack_phase > 0 && abs(x - dz_b) <= dz_radius && abs(y - dz_h) <= dz_radius);
                if (!is_danger_zone) {
                    for (int li = 0; li < 3; li++) {
                        if (x == line_b[li] && y == line_h[li]) { is_danger_zone = 1; break; }
                    }
                }

                if (x == p_b && y == p_h) {
                    if (roll_frames > 0) { b_idx += sprintf_s(&buffer[b_idx], max_size - b_idx, "\033[1;36mX\033[0m"); }
                    else { b_idx += sprintf_s(&buffer[b_idx], max_size - b_idx, "\033[1;34mX\033[0m"); }
                }
                else if (x == b_b && y == b_h) { b_idx += sprintf_s(&buffer[b_idx], max_size - b_idx, "\033[1;31mA\033[0m"); }
                else if (is_danger_zone) {
                    if (boss_attack_phase == 1) b_idx += sprintf_s(&buffer[b_idx], max_size - b_idx, "\033[33m*\033[0m");
                    else b_idx += sprintf_s(&buffer[b_idx], max_size - b_idx, "\033[31m*\033[0m");
                }
                else if (liz->l_map[x][y] == 1) { buffer[b_idx++] = (char)l_mauer_design; }
                else { buffer[b_idx++] = ' '; }
            }
            buffer[b_idx++] = '\n';
        }
        buffer[--b_idx] = '\0';
        printf("%s", buffer);
    }
    free(buffer);
    liz->player.l_pos_b = p_b;
    liz->player.l_pos_h = p_h;
}

void l_monster_samlung(l_infozentrum* liz) {
    memset(liz->monster_c, 0, sizeof(liz->monster_c));

    // -- ID 1: Soul of Cinder ---------------------------------
    liz->monster_c[1].l_id = 1;
    liz->monster_c[1].l_typ = 1;
    liz->monster_c[1].drop_chance = 25;
    liz->monster_c[1].runen = 3000;
    liz->monster_c[1].pos_min = 85;
    liz->monster_c[1].pos_max = 100;
    liz->monster_c[1].l_intelligent = 3;
    liz->monster_c[1].l_vig = 600;
    liz->monster_c[1].l_cur_vig = liz->monster_c[1].l_vig;
    liz->monster_c[1].l_str = 40;
    liz->monster_c[1].l_dex = 30;
    liz->monster_c[1].l_cur_dex = 30;
    liz->monster_c[1].resistenzen.feuer = 30;
    liz->monster_c[1].resistenzen.blitz = 20;
    liz->monster_c[1].l_waffe = liz->item_c[50];
    liz->monster_c[1].l_helm = liz->item_c[51];
    liz->monster_c[1].l_koerper = liz->item_c[52];
    liz->monster_c[1].l_hose = liz->item_c[53];
    liz->monster_c[1].l_schuhe = liz->item_c[54];
    strcpy_s(liz->monster_c[1].l_name, 32, "Soul of Cinder");
    strcpy_s(liz->monster_c[1].l_beschreibung, 256, "Der letzte Waechter der Ersten Flamme.");
    strcpy_s(liz->monster_c[1].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Soul_of_cinder.dat");

    // -- ID 2: Der Waechter -----------------------------------
    liz->monster_c[2].l_id = 2;
    liz->monster_c[2].l_typ = 1;
    liz->monster_c[2].drop_chance = 10;
    liz->monster_c[2].runen = 1000;
    liz->monster_c[2].pos_min = 30;
    liz->monster_c[2].pos_max = 70;
    liz->monster_c[2].l_intelligent = 1;
    liz->monster_c[2].l_vig = 200;
    liz->monster_c[2].l_cur_vig = liz->monster_c[2].l_vig;
    liz->monster_c[2].l_str = 20;
    liz->monster_c[2].l_dex = 20;
    liz->monster_c[2].l_cur_dex = 20;
    liz->monster_c[2].l_waffe = liz->item_c[55];
    liz->monster_c[2].l_helm = liz->item_c[56];
    liz->monster_c[2].l_koerper = liz->item_c[57];
    liz->monster_c[2].l_hose = liz->item_c[58];
    liz->monster_c[2].l_schuhe = liz->item_c[59];
    strcpy_s(liz->monster_c[2].l_name, 32, "Der Waechter");
    strcpy_s(liz->monster_c[2].l_beschreibung, 256, "Ein alter Waechter des Labyrinths.");
    strcpy_s(liz->monster_c[2].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Der_Waechter.dat");

    // -- ID 3: Gefangener ------------------------------------
    liz->monster_c[3].l_id = 3;
    liz->monster_c[3].l_typ = 1;
    liz->monster_c[3].drop_chance = 12;
    liz->monster_c[3].runen = 500;
    liz->monster_c[3].pos_min = 40;
    liz->monster_c[3].pos_max = 75;
    liz->monster_c[3].l_intelligent = 2;
    liz->monster_c[3].l_vig = 280;
    liz->monster_c[3].l_cur_vig = liz->monster_c[3].l_vig;
    liz->monster_c[3].l_str = 22;
    liz->monster_c[3].l_dex = 18;
    liz->monster_c[3].l_cur_dex = 18;
    liz->monster_c[3].resistenzen.gift = 100;
    liz->monster_c[3].resistenzen.frost = 20;
    liz->monster_c[3].l_waffe = liz->item_c[60];
    strcpy_s(liz->monster_c[3].l_name, 32, "Gefangener");
    strcpy_s(liz->monster_c[3].l_beschreibung, 256, "Ein Gefangener der geflohen ist");
    strcpy_s(liz->monster_c[3].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Monster_3.dat");

    // -- ID 4: Verfluchter Ritter -----------------------------
    liz->monster_c[4].l_id = 4;
    liz->monster_c[4].l_typ = 1;
    liz->monster_c[4].drop_chance = 15;
    liz->monster_c[4].runen = 700;
    liz->monster_c[4].pos_min = 25;
    liz->monster_c[4].pos_max = 60;
    liz->monster_c[4].l_intelligent = 2;
    liz->monster_c[4].l_vig = 180;
    liz->monster_c[4].l_cur_vig = liz->monster_c[4].l_vig;
    liz->monster_c[4].l_str = 18;
    liz->monster_c[4].l_dex = 25;
    liz->monster_c[4].l_cur_dex = 25;
    liz->monster_c[4].resistenzen.blitz = 15;
    liz->monster_c[4].l_waffe = liz->item_c[61];
    liz->monster_c[4].l_helm = liz->item_c[62];
    liz->monster_c[4].l_koerper = liz->item_c[63];
    liz->monster_c[4].l_hose = liz->item_c[64];
    strcpy_s(liz->monster_c[4].l_name, 32, "Verfluchter Ritter");
    strcpy_s(liz->monster_c[4].l_beschreibung, 256, "Ein Ritter dem der Fluch des Labyrinths den Verstand raubte.");
    strcpy_s(liz->monster_c[4].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Monster_4.dat");

    // -- ID 5: Goblin -------------------------------------
    liz->monster_c[5].l_id = 5;
    liz->monster_c[5].l_typ = 1;
    liz->monster_c[5].drop_chance = 14;
    liz->monster_c[5].runen = 250;
    liz->monster_c[5].pos_min = 15;
    liz->monster_c[5].pos_max = 60;
    liz->monster_c[5].l_intelligent = 2;
    liz->monster_c[5].l_vig = 90;
    liz->monster_c[5].l_cur_vig = liz->monster_c[5].l_vig;
    liz->monster_c[5].l_str = 16;
    liz->monster_c[5].l_dex = 45;
    liz->monster_c[5].l_cur_dex = 45;
    liz->monster_c[5].resistenzen.gift = 10;
    liz->monster_c[5].l_waffe = liz->item_c[77];
    strcpy_s(liz->monster_c[5].l_name, 32, "Goblin");
    strcpy_s(liz->monster_c[5].l_beschreibung, 256, "Ein Haesslicher Goblin");
    strcpy_s(liz->monster_c[5].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Monster_5.dat");

    // -- ID 6: Magier ----------------------------------
    liz->monster_c[6].l_id = 6;
    liz->monster_c[6].l_typ = 1;
    liz->monster_c[6].drop_chance = 6;
    liz->monster_c[6].runen = 400;
    liz->monster_c[6].pos_min = 20;
    liz->monster_c[6].pos_max = 55;
    liz->monster_c[6].l_intelligent = 1;
    liz->monster_c[6].l_vig = 350;
    liz->monster_c[6].l_cur_vig = liz->monster_c[6].l_vig;
    liz->monster_c[6].l_str = 15;
    liz->monster_c[6].l_dex = 8;
    liz->monster_c[6].l_cur_dex = 8;
    liz->monster_c[6].resistenzen.gift = 40;
    liz->monster_c[6].resistenzen.frost = 25;
    liz->monster_c[6].resistenzen.feuer = 50;
    liz->monster_c[6].resistenzen.blitz = 35;
    liz->monster_c[6].resistenzen.wasser = 20;
    liz->monster_c[6].l_waffe = liz->item_c[66];
    strcpy_s(liz->monster_c[6].l_name, 32, "Magier");
    strcpy_s(liz->monster_c[6].l_beschreibung, 256, "Sehr wenig bekannt ueber ihn.");
    strcpy_s(liz->monster_c[6].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Monster_6.dat");

    // -- ID 7: Siegmeyer --------------------------------------
    liz->monster_c[7].l_id = 7;
    liz->monster_c[7].l_typ = 1;
    liz->monster_c[7].drop_chance = 18;
    liz->monster_c[7].runen = 700;
    liz->monster_c[7].pos_min = 70;
    liz->monster_c[7].pos_max = 90;
    liz->monster_c[7].l_intelligent = 1;
    liz->monster_c[7].l_vig = 450;
    liz->monster_c[7].l_cur_vig = liz->monster_c[7].l_vig;
    liz->monster_c[7].l_str = 32;
    liz->monster_c[7].l_dex = 25;
    liz->monster_c[7].l_cur_dex = 25;
    liz->monster_c[7].resistenzen.feuer = 12;
    liz->monster_c[7].resistenzen.frost = 12;
    liz->monster_c[7].l_waffe = liz->item_c[67];
    liz->monster_c[7].l_helm = liz->item_c[68];
    liz->monster_c[7].l_koerper = liz->item_c[69];
    liz->monster_c[7].l_hose = liz->item_c[70];
    liz->monster_c[7].l_schuhe = liz->item_c[71];
    strcpy_s(liz->monster_c[7].l_name, 32, "Siegmeyer");
    strcpy_s(liz->monster_c[7].l_beschreibung, 256, "Alter Ritter, der im Labyrinth umherirrt.");
    strcpy_s(liz->monster_c[7].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Siegmeyer.dat");

    // -- ID 8: Arkanor -------------------------------------
    liz->monster_c[8].l_id = 8;
    liz->monster_c[8].l_typ = 1;
    liz->monster_c[8].drop_chance = 10;
    liz->monster_c[8].runen = 5000;
    liz->monster_c[8].pos_min = 55;
    liz->monster_c[8].pos_max = 85;
    liz->monster_c[8].l_intelligent = 4;
    liz->monster_c[8].l_vig = 800;
    liz->monster_c[8].l_cur_vig = liz->monster_c[8].l_vig;
    liz->monster_c[8].l_str = 35;
    liz->monster_c[8].l_dex = 10;
    liz->monster_c[8].l_cur_dex = 10;
    liz->monster_c[8].resistenzen.feuer = 10;
    liz->monster_c[8].resistenzen.frost = 10;
    liz->monster_c[8].resistenzen.blitz = 10;
    liz->monster_c[8].l_waffe = liz->item_c[72];
    liz->monster_c[8].l_helm = liz->item_c[73];
    liz->monster_c[8].l_koerper = liz->item_c[74];
    liz->monster_c[8].l_hose = liz->item_c[75];
    liz->monster_c[8].l_schuhe = liz->item_c[76];
    strcpy_s(liz->monster_c[8].l_name, 32, "Arkanor");
    strcpy_s(liz->monster_c[8].l_beschreibung, 256, "Ritter aus stein,der seit lange vermisst ist.");
    strcpy_s(liz->monster_c[8].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Arkanor.dat");

    // -- ID 9: Mimic -----------------------------------------
    liz->monster_c[9].l_id = 9;
    liz->monster_c[9].l_typ = 6;
    liz->monster_c[9].drop_chance = 70;
    liz->monster_c[9].runen = 250;
    liz->monster_c[9].pos_min = 10;
    liz->monster_c[9].pos_max = 85;
    liz->monster_c[9].l_intelligent = 0;
    liz->monster_c[9].l_vig = 380;
    liz->monster_c[9].l_cur_vig = liz->monster_c[9].l_vig;
    liz->monster_c[9].l_str = 28;
    liz->monster_c[9].l_dex = 20;
    liz->monster_c[9].l_cur_dex = 20;
    liz->monster_c[9].resistenzen.gift = 20;
    liz->monster_c[9].l_waffe = liz->item_c[78];
    liz->monster_c[9].l_helm = liz->item_c[79];
    liz->monster_c[9].l_koerper = liz->item_c[80];
    strcpy_s(liz->monster_c[9].l_name, 32, "Mimic");
    strcpy_s(liz->monster_c[9].l_beschreibung, 256, "Sieht aus wie eine Truhe. Ist es aber nicht.");
    strcpy_s(liz->monster_c[9].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Mimic.dat");

    // -- ID 10: Truhe ----------------------------------------
    liz->monster_c[10].l_id = 10;
    liz->monster_c[10].l_typ = 5;
    liz->monster_c[10].runen = 300;
    liz->monster_c[10].pos_min = 0;
    liz->monster_c[10].pos_max = 100;
    liz->monster_c[10].l_intelligent = 0;
    strcpy_s(liz->monster_c[10].l_name, 32, "Truhe");
    strcpy_s(liz->monster_c[10].l_beschreibung, 256, "Eine alte Holztruhe. Hoffentlich nur eine Truhe.");
    strcpy_s(liz->monster_c[10].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Truhe.dat");

    // -- ID 30: Artorias -------------------------------------
    liz->monster_c[30].l_id = 30;
    liz->monster_c[30].l_typ = 1;
    liz->monster_c[30].runen = 4000;
    liz->monster_c[30].pos_min = 89;
    liz->monster_c[30].pos_max = 99;
    liz->monster_c[30].l_intelligent = 3;
    liz->monster_c[30].l_vig = 500;
    liz->monster_c[30].l_cur_vig = liz->monster_c[30].l_vig;
    strcpy_s(liz->monster_c[30].l_name, 32, "Artorias");
    strcpy_s(liz->monster_c[30].l_beschreibung, 256, "Ritter Artorias der Abgrundschreiter.");
    strcpy_s(liz->monster_c[30].l_ascii_art_datei, 64, "Game\\Standard\\Monster\\Artorias.dat");
}
void l_monster_spawnen(l_infozentrum* liz) {
    memset(liz->monster, 0, sizeof(liz->monster));

    int mw_max = 1;
    for (int x = 0; x < l_breite; x++) for (int y = 0; y < l_hohe; y++) if (liz->l_MW[x][y] > mw_max) mw_max = liz->l_MW[x][y];

    int anzahl = (l_breite * l_hohe) / 40;
    if (anzahl > MAX_MONSTER) anzahl = MAX_MONSTER - 1;

    int gespawnte_monster = 0;
    int spawn_count[MAX_MONSTER_DB] = { 0 };
    int max_spawns[MAX_MONSTER_DB] = { 0 };
    for (int i = 0; i < MAX_MONSTER_DB; i++) max_spawns[i] = 3 + (rand() % 3);

    for (int i = 0; i < anzahl; i++) {

        int rand_id = 0, ki = 0, id_versuche = 0, gueltige_id = 0;
        do {
            int verfuegbar[MAX_MONSTER_DB];
            int verf_anz = 0;
            for (int j = 1; j < MAX_MONSTER_DB; j++) {
                if (liz->monster_c[j].l_id != 0 && liz->monster_c[j].l_typ != 5 && liz->monster_c[j].l_typ != 6) {
                    if (j == 30 && liz->l_ebenen < 3) { continue; }
                    verfuegbar[verf_anz++] = j;
                }
            }
            if (verf_anz == 0) break;
            rand_id = verfuegbar[rand() % verf_anz];
            id_versuche++;
            if (liz->monster_c[rand_id].l_id == 0) continue;
            ki = liz->monster_c[rand_id].l_intelligent;
            if ((ki == 0 || ki == 3 || ki == 4) && spawn_count[rand_id] >= 1) continue;
            if ((ki != 0 && ki != 3 && ki != 4) && spawn_count[rand_id] >= max_spawns[rand_id]) continue;
            gueltige_id = 1;
            break;
        } while (id_versuche < 100);
        if (!gueltige_id) break;

        int pmin = liz->monster_c[rand_id].pos_min;
        int pmax = liz->monster_c[rand_id].pos_max;

        if (pmin == 0 && pmax == 0) { pmin = 0; pmax = 100; }

        int mw_min_val = (mw_max * pmin) / 100;
        int mw_max_val = (mw_max * pmax) / 100;

        int rx = 0, ry = 0, platz_versuche = 0, platz_gefunden = 0;
        do {
            rx = rand() % l_breite;
            ry = rand() % l_hohe;
            platz_versuche++;

            if (liz->l_map[rx][ry] != 0)           continue;
            if (liz->l_ME[rx][ry] != 0)           continue;
            if (liz->l_MW[rx][ry] < mw_min_val)   continue;
            if (liz->l_MW[rx][ry] > mw_max_val)   continue;

            platz_gefunden = 1;
            break;
        } while (platz_versuche < 300);

        if (!platz_gefunden) continue;

        liz->monster[gespawnte_monster] = liz->monster_c[rand_id];
        liz->monster[gespawnte_monster].pos_b = rx;
        liz->monster[gespawnte_monster].pos_h = ry;
        liz->monster[gespawnte_monster].pos_buffer_b = rx;
        liz->monster[gespawnte_monster].pos_buffer_h = ry;
        liz->l_ME[rx][ry] = -(gespawnte_monster + 1);

        spawn_count[rand_id]++;
        gespawnte_monster++;
    }
}
void l_truhe_spawnen(l_infozentrum* liz) {

    int mw_max = 1;
    for (int x = 0; x < l_breite; x++) for (int y = 0; y < l_hohe; y++) if (liz->l_MW[x][y] > mw_max) mw_max = liz->l_MW[x][y];

    int anzahl = (l_breite * l_hohe) / 150;
    if (anzahl < 2) anzahl = 2;

    int mimic_truhe_chance = 0;
    if (l_schwierigkeit == 2) {
        mimic_truhe_chance = 20;
    }
    else if (l_schwierigkeit == 3) {
        mimic_truhe_chance = 40;
    }
    else if (l_schwierigkeit == 4) {
        mimic_truhe_chance = 60;
    }

    for (int i = 0; i < anzahl; i++) {

        int slot = -1;
        for (int s = 0; s < MAX_MONSTER; s++) if (liz->monster[s].l_id == 0) { slot = s; break; }
        if (slot == -1) break;

        int ist_mimic = (rand() % 100) < mimic_truhe_chance;
        int db_id = ist_mimic ? 9 : 10;

        int pmin = liz->monster_c[db_id].pos_min;
        int pmax = liz->monster_c[db_id].pos_max;
        if (pmin == 0 && pmax == 0) { pmin = 0; pmax = 100; }

        int mw_min_val = (mw_max * pmin) / 100;
        int mw_max_val = (mw_max * pmax) / 100;

        int rx = 0, ry = 0, platz_versuche = 0, platz_gefunden = 0;
        do {
            rx = rand() % l_breite;
            ry = rand() % l_hohe;
            platz_versuche++;

            if (liz->l_map[rx][ry] != 0) continue;
            if (liz->l_ME[rx][ry] != 0) continue;
            if (liz->l_MW[rx][ry] < mw_min_val) continue;
            if (liz->l_MW[rx][ry] > mw_max_val) continue;

            platz_gefunden = 1;
            break;
        } while (platz_versuche < 300);

        if (!platz_gefunden) continue;

        liz->monster[slot] = liz->monster_c[db_id];
        liz->monster[slot].pos_b = rx;
        liz->monster[slot].pos_h = ry;
        liz->monster[slot].pos_buffer_b = rx;
        liz->monster[slot].pos_buffer_h = ry;
        liz->l_ME[rx][ry] = -(slot + 1);
    }
}
void l_addon_monster_laden(const char* dateipfad, l_monster* m) {

}
void l_monster_intelligent(l_infozentrum* liz, int m_index) {
    if (liz->monster[m_index].flucht_cooldown > 0) {
        liz->monster[m_index].flucht_cooldown--;
        return;
    }

    int ki_typ = liz->monster[m_index].l_intelligent;
    if (ki_typ == 0) return;

    int curr_b = liz->monster[m_index].pos_b;
    int curr_h = liz->monster[m_index].pos_h;
    int px = liz->player.l_pos_b;
    int py = liz->player.l_pos_h;

    int m_wert = -(m_index + 1);

    if (curr_b == px && curr_h == py) return;

    int dist_zu_spieler = abs(px - curr_b) + abs(py - curr_h);

    int ist_p_N = (curr_b == px && curr_h - 1 == py);
    int ist_p_S = (curr_b == px && curr_h + 1 == py);
    int ist_p_W = (curr_b - 1 == px && curr_h == py);
    int ist_p_O = (curr_b + 1 == px && curr_h == py);

    int kn_N = (curr_h > 0) && (ist_p_N || (liz->l_map[curr_b][curr_h - 1] == 0 && liz->l_ME[curr_b][curr_h - 1] == 0));
    int kn_S = (curr_h < l_hohe - 1) && (ist_p_S || (liz->l_map[curr_b][curr_h + 1] == 0 && liz->l_ME[curr_b][curr_h + 1] == 0));
    int kn_W = (curr_b > 0) && (ist_p_W || (liz->l_map[curr_b - 1][curr_h] == 0 && liz->l_ME[curr_b - 1][curr_h] == 0));
    int kn_O = (curr_b < l_breite - 1) && (ist_p_O || (liz->l_map[curr_b + 1][curr_h] == 0 && liz->l_ME[curr_b + 1][curr_h] == 0));

    int NOSW[4] = { 1, 2, 3, 4 };
    for (int i = 0; i < 4; i++) {
        int r = rand() % 4;
        int temp = NOSW[i];
        NOSW[i] = NOSW[r];
        NOSW[r] = temp;
    }

    int sees_player = 1;
    if (dist_zu_spieler <= 6) {
        int x0 = curr_b, y0 = curr_h;
        int x1 = px, y1 = py;
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (1) {
            if (liz->l_map[x0][y0] == 1) { sees_player = 0; break; }
            if (x0 == x1 && y0 == y1) break;

            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
    else { sees_player = 0; }

    int mw_diff = abs(liz->l_MW[curr_b][curr_h] - liz->l_MW[px][py]);
    int notices_player = (mw_diff <= 6) && sees_player;

    if (ki_typ == 1) {
        int pmin = liz->monster[m_index].pos_min;
        int pmax = liz->monster[m_index].pos_max;
        if (pmin == 0 && pmax == 0) { pmin = 0; pmax = 100; }
        int mw_min_val = (liz->l_mw_max * pmin) / 100;
        int mw_max_val = (liz->l_mw_max * pmax) / 100;

        int erlaubt_N = kn_N && liz->l_MW[curr_b][curr_h - 1] >= mw_min_val && liz->l_MW[curr_b][curr_h - 1] <= mw_max_val;
        int erlaubt_S = kn_S && liz->l_MW[curr_b][curr_h + 1] >= mw_min_val && liz->l_MW[curr_b][curr_h + 1] <= mw_max_val;
        int erlaubt_W = kn_W && liz->l_MW[curr_b - 1][curr_h] >= mw_min_val && liz->l_MW[curr_b - 1][curr_h] <= mw_max_val;
        int erlaubt_O = kn_O && liz->l_MW[curr_b + 1][curr_h] >= mw_min_val && liz->l_MW[curr_b + 1][curr_h] <= mw_max_val;

        int moved = 0;

        if (notices_player) {
            int best_dist = dist_zu_spieler;
            int best_b = curr_b, best_h = curr_h;

            if (erlaubt_N) { int d = abs(px - curr_b) + abs(py - (curr_h - 1)); if (d < best_dist) { best_dist = d; best_b = curr_b;     best_h = curr_h - 1; } }
            if (erlaubt_S) { int d = abs(px - curr_b) + abs(py - (curr_h + 1)); if (d < best_dist) { best_dist = d; best_b = curr_b;     best_h = curr_h + 1; } }
            if (erlaubt_W) { int d = abs(px - (curr_b - 1)) + abs(py - curr_h); if (d < best_dist) { best_dist = d; best_b = curr_b - 1; best_h = curr_h; } }
            if (erlaubt_O) { int d = abs(px - (curr_b + 1)) + abs(py - curr_h); if (d < best_dist) { best_dist = d; best_b = curr_b + 1; best_h = curr_h; } }

            if (best_b != curr_b || best_h != curr_h) {
                liz->l_ME[curr_b][curr_h] = 0;
                liz->monster[m_index].pos_b = best_b;
                liz->monster[m_index].pos_h = best_h;
                liz->l_ME[best_b][best_h] = m_wert;
                moved = 1;
            }
        }
        if (!moved && (erlaubt_N || erlaubt_S || erlaubt_W || erlaubt_O)) {
            for (int i = 0; i < 4; i++) {
                int rt = NOSW[i];
                if (rt == 1 && erlaubt_N) { liz->l_ME[curr_b][curr_h] = 0; liz->monster[m_index].pos_h -= 1; liz->l_ME[curr_b][curr_h - 1] = m_wert; break; }
                else if (rt == 2 && erlaubt_O) { liz->l_ME[curr_b][curr_h] = 0; liz->monster[m_index].pos_b += 1; liz->l_ME[curr_b + 1][curr_h] = m_wert; break; }
                else if (rt == 3 && erlaubt_S) { liz->l_ME[curr_b][curr_h] = 0; liz->monster[m_index].pos_h += 1; liz->l_ME[curr_b][curr_h + 1] = m_wert; break; }
                else if (rt == 4 && erlaubt_W) { liz->l_ME[curr_b][curr_h] = 0; liz->monster[m_index].pos_b -= 1; liz->l_ME[curr_b - 1][curr_h] = m_wert; break; }
            }
        }
    }
    else if (ki_typ == 2) {
        int pmin = liz->monster[m_index].pos_min;
        int pmax = liz->monster[m_index].pos_max;
        if (pmin == 0 && pmax == 0) { pmin = 0; pmax = 100; }
        int mw_min_val = (liz->l_mw_max * pmin) / 100;
        int mw_max_val = (liz->l_mw_max * pmax) / 100;

        int erlaubt_N = kn_N && liz->l_MW[curr_b][curr_h - 1] >= mw_min_val && liz->l_MW[curr_b][curr_h - 1] <= mw_max_val;
        int erlaubt_S = kn_S && liz->l_MW[curr_b][curr_h + 1] >= mw_min_val && liz->l_MW[curr_b][curr_h + 1] <= mw_max_val;
        int erlaubt_W = kn_W && liz->l_MW[curr_b - 1][curr_h] >= mw_min_val && liz->l_MW[curr_b - 1][curr_h] <= mw_max_val;
        int erlaubt_O = kn_O && liz->l_MW[curr_b + 1][curr_h] >= mw_min_val && liz->l_MW[curr_b + 1][curr_h] <= mw_max_val;

        int moved = 0;
        int spieler_in_zone = (liz->l_MW[px][py] >= mw_min_val && liz->l_MW[px][py] <= mw_max_val);

        if (notices_player && spieler_in_zone) {
            int best_dist = dist_zu_spieler;
            int best_b = curr_b, best_h = curr_h;

            if (kn_N) { int d = abs(px - curr_b) + abs(py - (curr_h - 1)); if (d < best_dist) { best_dist = d; best_b = curr_b;     best_h = curr_h - 1; } }
            if (kn_S) { int d = abs(px - curr_b) + abs(py - (curr_h + 1)); if (d < best_dist) { best_dist = d; best_b = curr_b;     best_h = curr_h + 1; } }
            if (kn_W) { int d = abs(px - (curr_b - 1)) + abs(py - curr_h); if (d < best_dist) { best_dist = d; best_b = curr_b - 1; best_h = curr_h; } }
            if (kn_O) { int d = abs(px - (curr_b + 1)) + abs(py - curr_h); if (d < best_dist) { best_dist = d; best_b = curr_b + 1; best_h = curr_h; } }

            if (best_b != curr_b || best_h != curr_h) {
                liz->l_ME[curr_b][curr_h] = 0;
                liz->monster[m_index].pos_b = best_b;
                liz->monster[m_index].pos_h = best_h;
                liz->l_ME[best_b][best_h] = m_wert;
                moved = 1;
            }
        }

        if (!moved) {
            int ziel_mw = (m_index % 2 == 0) ? mw_min_val : mw_max_val;
            int aktuelle_diff = abs(liz->l_MW[curr_b][curr_h] - ziel_mw);
            int best_b = curr_b, best_h = curr_h, best_diff = aktuelle_diff;

            if (erlaubt_N) { int d = abs(liz->l_MW[curr_b][curr_h - 1] - ziel_mw); if (d < best_diff) { best_diff = d; best_b = curr_b;     best_h = curr_h - 1; } }
            if (erlaubt_S) { int d = abs(liz->l_MW[curr_b][curr_h + 1] - ziel_mw); if (d < best_diff) { best_diff = d; best_b = curr_b;     best_h = curr_h + 1; } }
            if (erlaubt_W) { int d = abs(liz->l_MW[curr_b - 1][curr_h] - ziel_mw); if (d < best_diff) { best_diff = d; best_b = curr_b - 1; best_h = curr_h; } }
            if (erlaubt_O) { int d = abs(liz->l_MW[curr_b + 1][curr_h] - ziel_mw); if (d < best_diff) { best_diff = d; best_b = curr_b + 1; best_h = curr_h; } }

            if (best_b != curr_b || best_h != curr_h) {
                liz->l_ME[curr_b][curr_h] = 0;
                liz->monster[m_index].pos_b = best_b;
                liz->monster[m_index].pos_h = best_h;
                liz->l_ME[best_b][best_h] = m_wert;
            }
        }
    }
    else if (ki_typ == 3) {
        int pmin = liz->monster[m_index].pos_min;
        int pmax = liz->monster[m_index].pos_max;
        if (pmin == 0 && pmax == 0) { pmin = 0; pmax = 100; }
        int mw_min_val = (liz->l_mw_max * pmin) / 100;
        int mw_max_val = (liz->l_mw_max * pmax) / 100;

        int erlaubt_N = kn_N && liz->l_MW[curr_b][curr_h - 1] >= mw_min_val && liz->l_MW[curr_b][curr_h - 1] <= mw_max_val;
        int erlaubt_S = kn_S && liz->l_MW[curr_b][curr_h + 1] >= mw_min_val && liz->l_MW[curr_b][curr_h + 1] <= mw_max_val;
        int erlaubt_W = kn_W && liz->l_MW[curr_b - 1][curr_h] >= mw_min_val && liz->l_MW[curr_b - 1][curr_h] <= mw_max_val;
        int erlaubt_O = kn_O && liz->l_MW[curr_b + 1][curr_h] >= mw_min_val && liz->l_MW[curr_b + 1][curr_h] <= mw_max_val;

        int best_b = curr_b, best_h = curr_h;
        int dist_zu_ausgang = abs(liz->l_exit_b - curr_b) + abs(liz->l_exit_h - curr_h);
        int best_dist = dist_zu_ausgang;

        if (erlaubt_N) { int d = abs(liz->l_exit_b - curr_b) + abs(liz->l_exit_h - (curr_h - 1)); if (d < best_dist) { best_dist = d; best_b = curr_b;     best_h = curr_h - 1; } }
        if (erlaubt_S) { int d = abs(liz->l_exit_b - curr_b) + abs(liz->l_exit_h - (curr_h + 1)); if (d < best_dist) { best_dist = d; best_b = curr_b;     best_h = curr_h + 1; } }
        if (erlaubt_W) { int d = abs(liz->l_exit_b - (curr_b - 1)) + abs(liz->l_exit_h - curr_h); if (d < best_dist) { best_dist = d; best_b = curr_b - 1; best_h = curr_h; } }
        if (erlaubt_O) { int d = abs(liz->l_exit_b - (curr_b + 1)) + abs(liz->l_exit_h - curr_h); if (d < best_dist) { best_dist = d; best_b = curr_b + 1; best_h = curr_h; } }

        if (best_b != curr_b || best_h != curr_h) {
            liz->l_ME[curr_b][curr_h] = 0;
            liz->monster[m_index].pos_b = best_b;
            liz->monster[m_index].pos_h = best_h;
            liz->l_ME[best_b][best_h] = m_wert;
        }
    }
    else if (ki_typ == 4) {
        int pmin = liz->monster[m_index].pos_min;
        int pmax = liz->monster[m_index].pos_max;
        if (pmin == 0 && pmax == 0) { pmin = 0; pmax = 100; }
        int mw_min_val = (liz->l_mw_max * pmin) / 100;
        int mw_max_val = (liz->l_mw_max * pmax) / 100;

        int erlaubt_N = kn_N && liz->l_MW[curr_b][curr_h - 1] >= mw_min_val && liz->l_MW[curr_b][curr_h - 1] <= mw_max_val;
        int erlaubt_S = kn_S && liz->l_MW[curr_b][curr_h + 1] >= mw_min_val && liz->l_MW[curr_b][curr_h + 1] <= mw_max_val;
        int erlaubt_W = kn_W && liz->l_MW[curr_b - 1][curr_h] >= mw_min_val && liz->l_MW[curr_b - 1][curr_h] <= mw_max_val;
        int erlaubt_O = kn_O && liz->l_MW[curr_b + 1][curr_h] >= mw_min_val && liz->l_MW[curr_b + 1][curr_h] <= mw_max_val;

        if (notices_player && dist_zu_spieler > 1) {
            int visited[100][50];
            int from_b[100][50], from_h[100][50];
            int q_b[5000], q_h[5000];
            memset(visited, 0, sizeof(visited));
            int head = 0, tail = 0;

            visited[curr_b][curr_h] = 1;
            from_b[curr_b][curr_h] = -1; from_h[curr_b][curr_h] = -1;
            q_b[tail] = curr_b; q_h[tail] = curr_h; tail++;

            int found = 0;
            while (head < tail && !found) {
                int cb = q_b[head], ch = q_h[head]; head++;
                int nb[4] = { cb,     cb,     cb - 1, cb + 1 };
                int nh[4] = { ch - 1, ch + 1, ch,     ch };
                for (int i = 0; i < 4 && !found; i++) {
                    int nb_ = nb[i], nh_ = nh[i];
                    if (nb_ < 0 || nb_ >= l_breite || nh_ < 0 || nh_ >= l_hohe) continue;
                    if (visited[nb_][nh_]) continue;
                    if (nb_ == px && nh_ == py) {
                        from_b[px][py] = cb; from_h[px][py] = ch;
                        found = 1; break;
                    }
                    if (liz->l_map[nb_][nh_] != 0) continue;
                    if (liz->l_ME[nb_][nh_] != 0) continue;
                    if (liz->l_MW[nb_][nh_] < mw_min_val || liz->l_MW[nb_][nh_] > mw_max_val) continue;
                    visited[nb_][nh_] = 1;
                    from_b[nb_][nh_] = cb; from_h[nb_][nh_] = ch;
                    q_b[tail] = nb_; q_h[tail] = nh_; tail++;
                }
            }

            if (found) {
                int step_b = px, step_h = py;
                while (from_b[step_b][step_h] != curr_b || from_h[step_b][step_h] != curr_h) {
                    int pb = from_b[step_b][step_h];
                    int ph = from_h[step_b][step_h];
                    step_b = pb; step_h = ph;
                }
                liz->l_ME[curr_b][curr_h] = 0;
                liz->monster[m_index].pos_b = step_b;
                liz->monster[m_index].pos_h = step_h;
                liz->l_ME[step_b][step_h] = m_wert;
            }
        }
        else if (!notices_player && (erlaubt_N || erlaubt_S || erlaubt_W || erlaubt_O)) {
            for (int i = 0; i < 4; i++) {
                int rt = NOSW[i];
                if (rt == 1 && erlaubt_N) { liz->l_ME[curr_b][curr_h] = 0; liz->monster[m_index].pos_h -= 1; liz->l_ME[curr_b][curr_h - 1] = m_wert; break; }
                else if (rt == 2 && erlaubt_O) { liz->l_ME[curr_b][curr_h] = 0; liz->monster[m_index].pos_b += 1; liz->l_ME[curr_b + 1][curr_h] = m_wert; break; }
                else if (rt == 3 && erlaubt_S) { liz->l_ME[curr_b][curr_h] = 0; liz->monster[m_index].pos_h += 1; liz->l_ME[curr_b][curr_h + 1] = m_wert; break; }
                else if (rt == 4 && erlaubt_W) { liz->l_ME[curr_b][curr_h] = 0; liz->monster[m_index].pos_b -= 1; liz->l_ME[curr_b - 1][curr_h] = m_wert; break; }
            }
        }
    }
}
void l_monster_drop(l_infozentrum* liz, int m_index) {
    l_monster* m = &liz->monster[m_index];
    l_player* p = &liz->player;

    if (m->drop_chance <= 0) return;

    if ((rand() % 100) >= m->drop_chance) return;

    l_item* moegliche[5];
    int anz = 0;
    if (m->l_waffe.l_id > 0) moegliche[anz++] = &m->l_waffe;
    if (m->l_helm.l_id > 0) moegliche[anz++] = &m->l_helm;
    if (m->l_koerper.l_id > 0) moegliche[anz++] = &m->l_koerper;
    if (m->l_hose.l_id > 0) moegliche[anz++] = &m->l_hose;
    if (m->l_schuhe.l_id > 0) moegliche[anz++] = &m->l_schuhe;

    if (anz == 0) return;

    l_item* drop = moegliche[rand() % anz];

    int slot = l_freien_slot(&p->inventar);

    cls();
    printf("\n");
    printf("\t\t\033[1;33m  ___________________________________________\033[0m\n\n");
    printf("\t\t\033[1;33m           >>>  ITEM DROP!  <<<\033[0m\n\n");
    printf("\t\t  \033[1;37m%s\033[0m hat folgendes hinterlassen:\n\n", m->l_name);
    printf("\t\t  \033[1;33m[ %-28.28s ]\033[0m\n\n", drop->l_name);
    printf("\t\t    Staerke   : %d\n", drop->l_staerke);
    if (drop->elemente.feuer > 0) printf("\t\t    Feuer     : %d\n", drop->elemente.feuer);
    if (drop->elemente.frost > 0) printf("\t\t    Frost     : %d\n", drop->elemente.frost);
    if (drop->elemente.blitz > 0) printf("\t\t    Blitz     : %d\n", drop->elemente.blitz);
    if (drop->elemente.gift > 0) printf("\t\t    Gift      : %d\n", drop->elemente.gift);
    printf("\n");

    if (slot >= 0) {
        p->inventar.slots[slot] = *drop;
        printf("\t\t\033[1;32m> Item ins Inventar gelegt! (Slot %d)\033[0m\n", slot + 1);
    }
    else {
        printf("\t\t\033[1;31m> Inventar ist voll! Item geht verloren.\033[0m\n");
        printf("\t\t  \033[90mTipp: Ruste Items ab oder leg sie ab.\033[0m\n");
    }

    printf("\n\t\t\033[1;33m  ___________________________________________\033[0m\n\n");
    system("pause");
}
int  l_freien_slot(l_inventar* inv) {
    for (int i = 0; i < INV_GROESSE; i++) {
        if (inv->slots[i].l_id == 0) return i;
    }
    return -1;
}
void l_truhe_oeffnen(l_infozentrum* liz, int m_index) {
    l_player* p = &liz->player;
    int id = 82;
    int SVI = 0;
    int tsize = 0;
    while (1) {
        if (liz->item_c[id].l_id != id) { break; }
        SVI++;
        id++;
    }
    SVI += 47;
    int* loot_pool = (int*)calloc(SVI, sizeof(int));
    for (int i = 1; i <= SVI + 1; i++) {
        if (liz->item_c[i].l_id == i) {
            if (i <= 46 || i == 65 || i >= 81) {
                loot_pool[tsize++] = i;
            }
        }
    }
    l_item gefunden = liz->item_c[loot_pool[rand() % tsize]];
    int runen_gefunden = liz->monster[m_index].runen + (rand() % 50);
    free(loot_pool);
    int slot = l_freien_slot(&p->inventar);

    cls();
    printf("\n");
    printf("\t\t\033[1;33m  ___________________________________________\033[0m\n\n");
    printf("\t\t\033[1;33m           >>>  TRUHE GEOEFFNET!  <<<\033[0m\n\n");
    printf("\t\t  Du findest:\n\n");
    printf("\t\t  \033[1;33m[ %-28.28s ]\033[0m\n\n", gefunden.l_name);
    if (gefunden.l_staerke > 0)           printf("\t\t    Staerke    : %d\n", gefunden.l_staerke);
    if (gefunden.effekte.l_bonus_str > 0) printf("\t\t    Bonus STR  : +%d\n", gefunden.effekte.l_bonus_str);
    if (gefunden.effekte.l_bonus_dex > 0) printf("\t\t    Bonus DEX  : +%d\n", gefunden.effekte.l_bonus_dex);
    if (gefunden.effekte.l_bonus_vig > 0) printf("\t\t    Bonus VIG  : +%d\n", gefunden.effekte.l_bonus_vig);
    if (gefunden.effekte.l_bonus_end > 0) printf("\t\t    Bonus END  : +%d\n", gefunden.effekte.l_bonus_end);
    printf("\n");

    if (slot >= 0) {
        p->inventar.slots[slot] = gefunden;
        printf("\t\t\033[1;32m> Item ins Inventar gelegt! (Slot %d)\033[0m\n", slot + 1);
    }
    else {
        printf("\t\t\033[1;31m> Inventar ist voll! Item geht verloren.\033[0m\n");
        printf("\t\t  \033[90mTipp: Ruste Items ab oder leg sie ab.\033[0m\n");
    }

    p->l_runen += runen_gefunden;
    printf("\t\t\033[1;33m> %d Runen gefunden!\033[0m\n", runen_gefunden);

    printf("\n\t\t\033[1;33m  ___________________________________________\033[0m\n\n");
    system("pause");
}
// ---- Menu -----------------------------------------------------
void l_E_MG() {
    int wahl = 0;
    int size = 4;
    int taste;

    while (1) {
        cls();
        printf("\n\n\n\t\t\t\t=========================================================\n");
        l_PrCe("Map Groesse bestimmen");
        printf("\t\t\t\t=========================================================\n\n");

        if (l_tutorial) {
            printf("\t\t\t\t %s 1. Klein  (Hoehe: 15, Breite: 31)\n", (wahl == 0) ? "-->" : "   ");
            printf("\t\t\t\t %s 2. Mittel (Hoehe: 27, Breite: 55)\n", (wahl == 1) ? "-->" : "   ");
            printf("\t\t\t\t %s 3. Gross  (Hoehe: 35, Breite: 71)\n", (wahl == 2) ? "-->" : "   ");
            printf("\t\t\t\t %s 4. Custom (Eigene Groesse eintippen)\n\n", (wahl == 3) ? "-->" : "   ");
        }
        else {
            printf("\t\t\t\t %s 1. Klein \n", (wahl == 0) ? "-->" : "   ");
            printf("\t\t\t\t %s 2. Mittel \n", (wahl == 1) ? "-->" : "   ");
            printf("\t\t\t\t %s 3. Gross   \n", (wahl == 2) ? "-->" : "   ");
            printf("\t\t\t\t %s 4. Custom \n\n", (wahl == 3) ? "-->" : "   ");
        }
        printf("\t\t\t\t=========================================================\n");
        printf("\t\t\t\t Steuerung: Pfeiltasten / WS | Zahlen 1-4\n");
        printf("\t\t\t\t Bestaetigen mit: ENTER oder LEERTASTE\n");
        printf("\t\t\t\t Verlassen mit: Escape\n");

        taste = _getch();
        if (taste == 27) return;
        if (taste == 224) {
            taste = _getch();
            if (taste == 72 && wahl > 0)        wahl--;
            if (taste == 80 && wahl < size - 1) wahl++;
        }

        else if (taste == 13 || taste == ' ') {
            if (wahl == 0) {
                l_hohe = 15;
                l_breite = (l_hohe * 2) + 1;
                l_esmg = 0;
                return;
            }
            else if (wahl == 1) {
                l_hohe = 27;
                l_breite = (l_hohe * 2) + 1;
                l_esmg = 1;
                return;
            }
            else if (wahl == 2) {
                l_hohe = 35;
                l_breite = (l_hohe * 2) + 1;
                l_esmg = 2;
                return;
            }
            else if (wahl == 3) {
                cls();
                printf("\n\n\n\t\t\t\t=========================================================\n");
                printf("\t\t\t\t                >  Eigene Groesse eingeben <\n");
                printf("\t\t\t\t=========================================================\n\n");
                if (l_tutorial) printf("\t\t\t\t Es wird ausehen wie eine Quadrat \n");
                printf("\t\t\t\t Bitte gewuenschte Hoehe eingeben: ");

                if (scanf_s("%d", &l_hohe) != 1 || l_hohe < 10 || l_hohe > 49) {
                    l_hohe = 25;
                    printf("\t\t\t\t Ungueltige Eingabe! Setze auf Standard (27).\n");
                    printf("\t\t\t\t Minimum: 10, Maximum: 49\n");
                    _getch();
                }
                if ((l_hohe % 2) == 0) {
                    l_hohe = l_hohe + 1;
                }
                l_breite = (l_hohe * 2) + 1;
                while (getchar() != '\n');
                l_esmg = 3;
                return;
            }
        }

        else {
            if ((taste == 'w' || taste == 'W') && wahl > 0)        wahl--;
            if ((taste == 's' || taste == 'S') && wahl < size - 1) wahl++;

            if (taste >= '1' && taste <= '4') {
                wahl = taste - '1';
            }
        }
    }
}
void l_E_SW() {
    int wahl = 0;
    int size = 4;
    int taste;

    while (1) {
        cls();
        printf("\n\n\n\t\t\t\t=========================================================\n");
        printf("\t\t\t\t                >  Schwierigkeit Aendern <\n");
        printf("\t\t\t\t=========================================================\n\n");

        printf("\t\t\t\t %s 1. Einfach\n", (wahl == 0) ? "-->" : "   ");
        printf("\t\t\t\t %s 2. Mittel  \n", (wahl == 1) ? "-->" : "   ");
        printf("\t\t\t\t %s 3. Schwere  \n", (wahl == 2) ? "-->" : "   ");
        printf("\t\t\t\t %s 4. Hardcore\n\n", (wahl == 3) ? "-->" : "   ");

        printf("\t\t\t\t=========================================================\n");
        printf("\t\t\t\t Steuerung: Pfeiltasten / WS | Zahlen 1-4\n");
        printf("\t\t\t\t Bestaetigen mit: ENTER oder LEERTASTE\n");
        printf("\t\t\t\t Verlassen mit: Escape\n");

        taste = _getch();
        if (taste == 27) return;
        if (taste == 224) {
            taste = _getch();
            if (taste == 72 && wahl > 0)        wahl--;
            if (taste == 80 && wahl < size - 1) wahl++;
        }

        else if (taste == 13 || taste == ' ') {
            if (wahl == 0) {
                l_schwierigkeit = 1;
                return;
            }
            else if (wahl == 1) {
                l_schwierigkeit = 2;
                return;
            }
            else if (wahl == 2) {
                l_schwierigkeit = 3;
                return;
            }
            else if (wahl == 3) {
                l_schwierigkeit = 4;
                return;
            }
        }

        else {
            if ((taste == 'w' || taste == 'W') && wahl > 0)        wahl--;
            if ((taste == 's' || taste == 'S') && wahl < size - 1) wahl++;

            if (taste >= '1' && taste <= '4') {
                wahl = taste - '1';
            }
        }
    }
}
void l_E_DN() {
    int wahl = 0;
    int size = 4;
    int taste;

    while (1) {
        cls();
        printf("\n\n\n\t\t\t\t=========================================================\n");
        l_PrCe("Mauer Design Aendern");
        printf("\t\t\t\t=========================================================\n\n");

        printf("\t\t\t\t %s 1. %c  \n", (wahl == 0) ? "-->" : "   ", 35);
        printf("\t\t\t\t %s 2. %c  \n", (wahl == 1) ? "-->" : "   ", 177);
        printf("\t\t\t\t %s 3. %c  \n", (wahl == 2) ? "-->" : "   ", 176);
        printf("\t\t\t\t %s 4. %c\n\n", (wahl == 3) ? "-->" : "   ", 178);

        printf("\t\t\t\t=========================================================\n");
        printf("\t\t\t\t Steuerung: Pfeiltasten / WS | Zahlen 1-4\n");
        printf("\t\t\t\t Bestaetigen mit: ENTER oder LEERTASTE\n");
        printf("\t\t\t\t Verlassen mit: Escape\n");

        taste = _getch();
        if (taste == 27) return;
        if (taste == 224) {
            taste = _getch();
            if (taste == 72 && wahl > 0)        wahl--;
            if (taste == 80 && wahl < size - 1) wahl++;
        }
        else if (taste == 13 || taste == ' ') {
            if (wahl == 0) {
                l_mauer_design = 35;
                return;
            }
            else if (wahl == 1) {
                l_mauer_design = 177;
                return;
            }
            else if (wahl == 2) {
                l_mauer_design = 176;
                return;
            }
            else if (wahl == 3) {
                l_mauer_design = 178;
                return;
            }
        }
        else {
            if ((taste == 'w' || taste == 'W') && wahl > 0)        wahl--;
            if ((taste == 's' || taste == 'S') && wahl < size - 1) wahl++;
            if (taste >= '1' && taste <= '4') {
                wahl = taste - '1';
            }
        }
    }
}
void l_achievements() {
    cls();
    FILE* datei;
    errno_t err = fopen_s(&datei, "Game\\achievements.dat", "r");

    if (err != 0 || datei == NULL) {
        printf("Konnte achievements.txt nicht finden!\n");
        return;
    }
    char zeile[150];
    int anzahl = 0;

    while (fgets(zeile, sizeof(zeile), datei) != NULL) {
        if (strncmp(zeile, "Achievement:", 12) == 0) { anzahl++; }
    }

    if (anzahl == 0) {
        printf("Keine Achievements in der Datei gefunden.\n");
        fclose(datei);
        return;
    }
    l_achievement* achs = (l_achievement*)malloc(anzahl * sizeof(l_achievement));
    if (achs == NULL) {
        printf("Fehler: Nicht genug Arbeitsspeicher!\n");
        fclose(datei);
        return;
    }
    rewind(datei);

    int index = 0;
    while (fgets(zeile, sizeof(zeile), datei) != NULL) {
        zeile[strcspn(zeile, "\n")] = 0;
        if (sscanf_s(zeile, "Achievement: %[^:]: %d", achs[index].name, (unsigned)sizeof(achs[index].name), &achs[index].status) == 2) {
            index++;
        }
    }
    fclose(datei);
    printf("\n\t\t====================================== ACHIEVEMENTS ======================================\n\n\t\t");
    for (int i = 0; i < index; i++) {
        char haken = (achs[i].status == 1) ? 'X' : ' ';
        printf("%-35s: [%c]      ", achs[i].name, haken);
        if ((i + 1) % 2 == 0) {
            printf("\n\t\t");
        }
    }
    if (index % 2 != 0) { printf("\n"); }
    printf("\n\t\t==========================================================================================\n\n");
    free(achs);
    system("pause");
}
void l_spielstand_speichern(l_infozentrum* liz) {
    if (l_schwierigkeit == 4) {
        cls();
        printf("\n\n\t\t\033[1;31mKein Speichern im Hardcore-Modus!\033[0m\n\n");
        Sleep(1500);
        return;
    }

    if (!l_folder_exists("Game\\save")) l_create_folder("Game\\save");

    FILE* f = NULL;
    errno_t err = fopen_s(&f, SAVE_PFAD, "wb");
    if (err != 0 || f == NULL) {
        cls();
        printf("\n\n\t\t\033[1;31mFehler %d: Spielstand konnte nicht gespeichert werden!\033[0m\n", err);
        printf("\t\tPfad: %s\n\n", SAVE_PFAD);
        system("pause");
        return;
    }

    int magic = SAVE_MAGIC;
    int version = SAVE_VERSION;
    fwrite(&magic, sizeof(int), 1, f);
    fwrite(&version, sizeof(int), 1, f);
    fwrite(&l_breite, sizeof(int), 1, f);
    fwrite(&l_hohe, sizeof(int), 1, f);
    fwrite(&l_schwierigkeit, sizeof(int), 1, f);
    fwrite(&liz->player, sizeof(l_player), 1, f);

    for (int x = 0; x < l_breite; x++) fwrite(liz->l_map[x], sizeof(int), l_hohe, f);
    for (int x = 0; x < l_breite; x++) fwrite(liz->l_ME[x], sizeof(int), l_hohe, f);
    for (int x = 0; x < l_breite; x++) fwrite(liz->l_MW[x], sizeof(int), l_hohe, f);

    fwrite(liz->monster, sizeof(l_monster), MAX_MONSTER, f);
    fwrite(&liz->l_ebenen, sizeof(int), 1, f);

    int schreib_fehler = ferror(f);
    fclose(f);

    if (schreib_fehler) {
        cls();
        printf("\n\n\t\t\033[1;31mSchreibfehler! Spielstand eventuell beschaedigt.\033[0m\n\n");
        system("pause");
        return;
    }

    cls();
    printf("\n\n\t\t\033[1;32m  ___________________________________________\033[0m\n\n");
    printf("\t\t\033[1;32m       >>>  Spielstand gespeichert!  <<<\033[0m\n\n");
    printf("\t\t  Datei  : %s\n", SAVE_PFAD);
    printf("\t\t  Level  : \033[1;33m%d\033[0m\n", liz->player.l_level);
    printf("\t\t  Runen  : \033[1;33m%d\033[0m\n\n", liz->player.l_runen);
    printf("\t\t\033[1;32m  ___________________________________________\033[0m\n\n");
    system("pause");
    cls();
    while (_kbhit()) _getch();
}
int  l_spielstand_laden(l_infozentrum* liz) {

    if (!l_file_exists(SAVE_PFAD)) {
        cls();
        printf("\n\n\t\t\033[1;33m  Kein Spielstand gefunden!\033[0m\n\n");
        printf("\t\t  Erwartet unter:\n\t\t  %s\n\n", SAVE_PFAD);
        system("pause");
        return 0;
    }

    FILE* f = NULL;
    errno_t err = fopen_s(&f, SAVE_PFAD, "rb");
    if (err != 0 || f == NULL) {
        cls();
        printf("\n\n\t\t\033[1;31mFehler %d: Kann Datei nicht oeffnen!\033[0m\n\n", err);
        system("pause");
        return 0;
    }

    int magic = 0, version = 0, sb = 0, sh = 0, ssw = 0;
    fread(&magic, sizeof(int), 1, f);
    fread(&version, sizeof(int), 1, f);
    fread(&sb, sizeof(int), 1, f);
    fread(&sh, sizeof(int), 1, f);
    fread(&ssw, sizeof(int), 1, f);

    if (magic != SAVE_MAGIC) {
        fclose(f);
        cls();
        printf("\n\n\t\t\033[1;31mFehler: Datei ist keine Labyrinth-Speicherdatei!\033[0m\n\n");
        Sleep(2000);
        return 0;
    }
    if (version != SAVE_VERSION) {
        fclose(f);
        remove(SAVE_PFAD);
        cls();
        printf("\n\n\t\t\033[1;33m  Spielstand-Version veraltet!\033[0m\n\n");
        printf("\t\t  Gespeichert: v%d   Erwartet: v%d\n", version, SAVE_VERSION);
        printf("\t\t  Alter Spielstand wurde geloescht.\n");
        printf("\t\t  Bitte starte ein neues Spiel.\n\n");
        system("pause");
        return 0;
    }
    if (sb < 5 || sh < 5 || sb > 500 || sh > 500) {
        fclose(f);
        cls();
        printf("\n\n\t\t\033[1;31mFehler: Map-Groesse ungueltig (%d x %d)!\033[0m\n\n", sb, sh);
        system("pause");
        return 0;
    }

    l_freigeben(liz);

    l_breite = sb;
    l_hohe = sh;
    l_schwierigkeit = ssw;

    liz->l_map = (int**)calloc(l_breite, sizeof(int*));
    liz->l_ME = (int**)calloc(l_breite, sizeof(int*));
    liz->l_MW = (int**)calloc(l_breite, sizeof(int*));

    if (!liz->l_map || !liz->l_ME || !liz->l_MW) {
        fclose(f); l_freigeben(liz);
        cls();
        printf("\n\n\t\t\033[1;31mFehler: Nicht genug Arbeitsspeicher!\033[0m\n\n");
        system("pause");
        return 0;
    }

    int alloc_fehler = 0;
    for (int i = 0; i < l_breite; i++) {
        liz->l_map[i] = (int*)calloc(l_hohe, sizeof(int));
        liz->l_ME[i] = (int*)calloc(l_hohe, sizeof(int));
        liz->l_MW[i] = (int*)calloc(l_hohe, sizeof(int));
        if (!liz->l_map[i] || !liz->l_ME[i] || !liz->l_MW[i]) {
            alloc_fehler = 1; break;
        }
    }

    if (alloc_fehler) {
        fclose(f); l_freigeben(liz); cls();
        printf("\n\n\t\t\033[1;31mFehler: Nicht genug Arbeitsspeicher!\033[0m\n\n");
        system("pause");
        return 0;
    }

    fread(&liz->player, sizeof(l_player), 1, f);
    for (int x = 0; x < l_breite; x++) fread(liz->l_map[x], sizeof(int), l_hohe, f);
    for (int x = 0; x < l_breite; x++) fread(liz->l_ME[x], sizeof(int), l_hohe, f);
    for (int x = 0; x < l_breite; x++) fread(liz->l_MW[x], sizeof(int), l_hohe, f);
    fread(liz->monster, sizeof(l_monster), MAX_MONSTER, f);
    fread(&liz->l_ebenen, sizeof(int), 1, f);

    int les_fehler = ferror(f);
    fclose(f);

    if (les_fehler) {
        l_freigeben(liz); cls();
        printf("\n\n\t\t\033[1;31mFehler beim Lesen! Spielstand beschaedigt.\033[0m\n\n");
        system("pause");
        return 0;
    }

    l_iteam_samlung(liz);
    l_monster_samlung(liz);
    l_berechne_mw_max(liz);

    cls();
    printf("\n\n\t\t\033[1;32m  ___________________________________________\033[0m\n\n");
    printf("\t\t\033[1;32m        >>>  Spielstand geladen!  <<<\033[0m\n\n");
    printf("\t\t  Level       : \033[1;33m%d\033[0m\n", liz->player.l_level);
    printf("\t\t  Runen       : \033[1;33m%d\033[0m\n", liz->player.l_runen);
    printf("\t\t  Schwierigk. : \033[1;33m%d\033[0m\n\n", l_schwierigkeit);
    printf("\t\t\033[1;32m  ___________________________________________\033[0m\n\n");
    system("pause");
    cls();
    while (_kbhit()) _getch();
    return 1;
}
void l_addons() {
    const char* kat_namen[] = {
        "Monster", "Waffe", "Buff", "Verbrauch",
        "Helm", "Rustung", "Hose", "Schuhe"
    };
    const char* kat_ordner[] = {
        "Game\\Addon\\Monster",  "Game\\Addon\\Waffen",
        "Game\\Addon\\Buff",     "Game\\Addon\\Verbrauch",
        "Game\\Addon\\Helm",     "Game\\Addon\\Rustung",
        "Game\\Addon\\Hose",     "Game\\Addon\\Schuhe"
    };
    const int KAT_ANZ = 8;
    const int MAX_ITEMS = 50;
    const int ZEIGE_MAX = 10;

    int kat = 0;
    int kat_alt = -1;
    int item = 0;
    int taste;

    char items[50][MAX_PATH];
    int  item_anz = 0;
    int  von = 0;

    while (1) {
        if (kat != kat_alt) {
            item_anz = 0;
            item = 0;
            von = 0;
            kat_alt = kat;

            char such_pfad[MAX_PATH];
            sprintf_s(such_pfad, sizeof(such_pfad), "%s\\*.txt", kat_ordner[kat]);

            WIN32_FIND_DATAA fd;
            HANDLE h = FindFirstFileA(such_pfad, &fd);
            if (h != INVALID_HANDLE_VALUE) {
                do {
                    if (item_anz >= MAX_ITEMS) break;
                    strncpy_s(items[item_anz], MAX_PATH, fd.cFileName, MAX_PATH - 1);
                    char* punkt = strrchr(items[item_anz], '.');
                    if (punkt) *punkt = '\0';
                    item_anz++;
                } while (FindNextFileA(h, &fd));
                FindClose(h);
            }
        }

        if (item < von) { von = item; }
        else if (item >= von + ZEIGE_MAX) { von = item - ZEIGE_MAX + 1; }

        int bis = von + ZEIGE_MAX;
        if (bis > item_anz) bis = item_anz;

        cls();

        printf("\n");
        printf("  +========================================================================+\n");
        printf("  |                               > Addons <                               |\n");
        printf("  +========================================================================+\n\n");

        printf("  ");
        if (kat > 0) printf("\033[1;37m<\033[0m ");
        else         printf("  ");

        for (int i = 0; i < KAT_ANZ; i++) {
            if (i > 0) printf(" \033[90m|\033[0m ");
            if (i == kat) printf("\033[1;33m%s\033[0m", kat_namen[i]);
            else          printf("\033[90m%s\033[0m", kat_namen[i]);
        }

        if (kat < KAT_ANZ - 1) printf(" \033[1;37m>\033[0m");
        printf("\n");
        printf("  +------------------------------------------------------------------------+\n");

        if (item_anz == 0) {
            printf("  |                                                                        |\n");
            printf("  |  \033[90m%-69s\033[0m |\n", "(Keine Addons .txt Datei in Ordner legen)");
            printf("  |                                                                        |\n");

            char bsp_text[128];
            sprintf_s(bsp_text, sizeof(bsp_text), "Beispiel: %s\\mein_addon.txt", kat_ordner[kat]);
            printf("  |  \033[90m%-69s\033[0m |\n", bsp_text);

            for (int r = 4; r < ZEIGE_MAX; r++)
                printf("  |                                                                        |\n");
        }
        else {
            for (int i = von; i < bis; i++) {
                if (i == item) {
                    printf("  |  \033[1;33m-> %-65s\033[0m  |\n", items[i]);
                }
                else {
                    printf("  |     %-65s  |\n", items[i]);
                }
            }
            for (int r = bis - von; r < ZEIGE_MAX; r++)
                printf("  |                                                                        |\n");
        }

        printf("  +------------------------------------------------------------------------+\n\n");

        printf("  Ordner : %s\n", kat_ordner[kat]);
        printf("  Addons : %d", item_anz);
        if (item_anz > 0) printf("   |   Ausgewaehlt: \033[1;37m%s\033[0m", items[item]);
        printf("\n\n");

        printf("  \033[90m[A/D / Links/Rechts]\033[0m Kategorie   "
            "\033[90m[W/S / Hoch/Runter]\033[0m Item   "
            "\033[90m[ESC]\033[0m Zurueck\n");

        if (item_anz > ZEIGE_MAX) {
            printf("  \033[90m(%d/%d)\033[0m\n", item + 1, item_anz);
        }

        taste = _getch();
        if (taste == 27) return;

        if (taste == 224) {
            taste = _getch();
            if (taste == 72 && item > 0)              item--;
            if (taste == 80 && item < item_anz - 1)   item++;
            if (taste == 75 && kat > 0)                kat--;
            if (taste == 77 && kat < KAT_ANZ - 1)     kat++;
        }
        else {
            if ((taste == 'w' || taste == 'W') && item > 0)              item--;
            if ((taste == 's' || taste == 'S') && item < item_anz - 1)  item++;
            if ((taste == 'a' || taste == 'A') && kat > 0)              kat--;
            if ((taste == 'd' || taste == 'D') && kat < KAT_ANZ - 1)    kat++;
        }
    }
}
// ---------------------------------------------------------------

/*

Muss gemacht werden:
- Stats von Monster anpassen (Die mit hohen schutzt)
- Story (Man spiel nach eine ereignise und steht die spuren)
- Enierungen fuer Monster (Wenn spieler um eine ecke lauft das der Monster auch ihn nach der ecke verfolgt, ausser spieler ist schon weg von monster ecke erreicht hat)
- Addons (Optional, aber man soll seine eigene code in spiel laden koennen)
- NPC (Optional)
- am ende alles auf bugs und fehler und rechtschreibung korigieren,
  so wie öäü in dialog nicht benutzt werder sondern (ue,oe,ae,ss),
  und auch die sachen nochmal ueberprueffen die eigentlich als fertig markiert sind.

--------------------------------------------------------------------------------
Idee fuers verbesserung:
- Boss-KI finalisieren: Nutze das Kurzzeitgedaechtnis-Prinzip für den Boss,
  um fluessige Bewegungen zu erreichen, ohne riesige Pfadfindungs-Arrays zu benoetigen.
  Stelle sicher, dass die "Ausweichen"-Logik (kurzes Aufleuchten/Invulnerability-Frames) sauber getriggert wird.
  ----------
- Balancing der Spawn-Logik: Ueberpruefe, ob die prozentuale Platzierung (pos_min bis pos_max)
  der Monster korrekt auf der generierten Map funktioniert, damit sie nicht in Wänden spawnen oder das Spiel unspielbar machen.
  ----------
- Logic verbesserung: Ueberprueffe ob die Logic hinter der KI alles richtig macht:
  (KI1: Zufaellige -> spieler verfolgen, wenn in der naehe
  |KI2: Zone -> angriff auf spieler, wenn in Zone
  |KI3: Ausgang bewachen -> Spieler in weg stehen
  |KI4: Beste weg zu spieler -> Spiler jagen)
  das die KI auch eine auf Teamplay macht:
  - wenn spiler in Zone ist das die monster die zu zone gehoeren dann versuchen die spieler zu umziegelnd damit kein flucht moeglichkeit da ist,
    und wenn ein Monster ein eingang schon bewacht das man dann zu denn naechsten geht.
  - Wenn eine ausgang schon jemand ist das man dann zu denn andere geht.
  - Wenn jaeger zu spieler will das die monster dann aus denn weg gehen um platzt fuer jaeger zu machen.
  ----------
- Feinschliff & Debugging: Nutze den im Code bereits vorbereiteten Debug-Modus,
  um die Spawns und das Verhalten der KI auf verschiedenen Map-Größen (l_breite, l_hohe) zu testen.
  ----------
- Content-Erweiterung: Da die Strukturen für l_monster_c (Monster-Datenbank) und l_item_c (Item-Datenbank) vorhanden sind,
  besteht die finale Arbeit primär darin, diese Datenbanken mit Inhalten zu füllen.
--------------------------------------------------------------------------------

Alles was mit eine -> markiert ist, ist gefixt oder muss ge ndert werden an denn Funktion selbst (Logic):
Bug 17:

*/

/*

To do:
Game: Schwierigkeit, Mehrere Ebene, iteams (Wie buff(buff sind wie ringe usw) iteam oder uses iteam), Tutorial,

Player:
Inventar: Attributes Tabelle, inventory, iteams die man equibt hat, Quick slot fuer uses iteam,
Leben: Koerper teile (beide bein, Koerper, beide arme, Kopf), sonderheiten,
Level: Levelpoints, skills,
Attributes: Dex (Geschicklichkeit), Vigor (Leben), Strength (Staerke), Endurance (Ausdauer),
Vorteile: (wie h he Treff chance wenn man Dex (Geschicklichkeit) skilt),

Map:
Monster: Wer&Wo&Was, Faehigkeiten, besonderheiten (wie inteligent, Waffen, ruestung usw.),
Sachen: Truhe auf Map,

Extra:
Spiel Stand speicher/laden, Achivment, Menu wenn man ESC dr ckt, NPC mit dioalog oder Minigames, zwischen speicher,


Waehrung: Runen
Level Up mitteln: Runen -> Kaufen, (Attributes kaeufe -> automatisch level aufsstieg)
Addons: wie extra waffen oder Monster die man selber hinzu fuegen kann.

Mit diesesn code kann ich die Tasten heraus finden:
int taste;
printf("Druecke eine Taste zum Testen: ");
taste = _getch();
printf("\nDer ASCII-Code der Taste ist: %d\n", taste);
_getch();

*/

/*

Erlidigt:
Alles was mit eine -> markiert ist, ist gefixt oder muss ge ndert werden an denn Funktion selbst (Logic):
Bug 1: Wenn ich und gegner aufeinmal (gleichzeitig) auf eine feld komme verschwindet der Monster. -> sollte gefixt sein
Bug 2: cls(); funktionieren  in Tutorial nicht gut. -> Gefixt
Bug 3: Wenn man rechts klick drueck in game dann bug spiel so Komisch. -> Es ist wegen windows, weil man mit rechts klick das was kopiert ist in denn Console verscuht einzuf gen. Loesung einfach kein rechtsklick dr cken.
Bug 4: Wenn man einmal schon gestorben aber dann Flucht geht dann kommt man in Death scren. -> Sollte Gefixt sein.
Bug 5: l_AF("Tutorial Abgeschlossen"); funktionieren in Tutorial nicht -> l_AF ist gefixt
Bug 6: Ausdauer Bug. -> Gefixt
Bug 7: Auf denn Zweite und dritte Map gehen manche monster nicht und viele bug herum. -> Gefixt
Bug 8: Auf denn Dritte Map gibt es monster, obwohl da kein sein sollte. -> Gefixt
Bug 9: Man kann nicht fliehen wenn man in eine Kampf ist, weil man direkt wieder in eine kampf geworfen wird, weil man immer noch auf der gleiche stelle ist. -> Gefixt
Bug 10: In kampf gegen denn Boss (3 Ebene) Bugt manchmal die anzeige. -> Gefixt
Bug 11: (Ebene 3 & Boss) Wenn der spieler links oder rechts hinter ein wand ist geht der boss nicht nach unten oder oben -> Gefixt
Bug 12: (Ebene 3 & Boss) Der Boss one hitte manchmal denn spieler obwohl der Spieler ueber 100 Hp hat. (Meist bei der zweite angriff) -> Gefixt
Bug 12: Warnungen Groessten teil beheben. -> Gefixt
Bug 13: FLucht bug, wenn man erfolgreich flieht dann dazu kommne das es mehre spieler auf Map gibt. -> Gefixt
Bug 14: (Ebene 3 & Boss) Der Boss wen links oder recht hinternisse sind, geht der boss nach oben und unten einmal dann wieder zurück zu kommt nicht weiter. -> gefixt
Bug 15: Truhe Bug von truhe bekomme ich nichts manchmal. -> Gefixt
Bug 16: Artorias spawnt in ebene 1 und 2 aus irgendein grund. -> sollte gefixt sein

Wurde gemacht:
- Message bug -> Sollte Fertig sein (Schwere alles zu Finden, braucht beta tester)
- Tutorial_story in tutorial -> Fertig
- Mehr Monster und itemas -> Fertig
- Schaden anpassung von Monster (Machen einfach zu viel schaden) -> Fertig
- Mehrere Ebene und die sollen nicht alle gleich ausehen -> Fertig
- _kbhit() verwende damit das spiel ganze zeit laeuft -> Fertig
- Artorias soll hinzugef gt werden, als spezail boss in der dritte ebene. -> Fertig
- Bewegung systeme von Monster -> fertig
- Truhe und Mimic -> Fertig
- Iteam 65 soll geaendert werden -> Fertig
- Artorias mehr angriffe (Mehr Aktion) -> Fertig
- Geschwindkeit von Spieler eine grenze setzten. -> Fertig
- Monster Stats anpassung (Spieler macht bei manche monster nur 1 schaden) -> Fertig
- Ausdauer Regennaríeren waehrend man in Map ist. -> Fertig
- Wenn man ein Bein oder Fuss verliert ist man dann langsamer. -> Fertig
- Wenn man gegner nicht trift wegen zu wenig ausdauer, dann soll ausauer auf 0 gesetzt werden. -> Fertig
- Beim geschalterte Fluch greift der Monster an. -> Fertig

Wurde Gefixt: Die Battle systeme, weil es funktioniert aber nicht so wie man es will,
1. Monster bekommt kein ausdauer abgezogen und macht ausweichen sinnlos.
2. Monster Treffen bisher immer und muss ge ndert werden.
3. Monster trifft sehr oft kopf und kopft sollte man nur selten treffen.
(Kopf kann man nur treffen wenn zwei koerper teile ab sind aber man sollte dann eher denn koerper treffen und nicht denn kopf und
wenn man viel in Geschickt gekillt hat sollte der Monster denn spiler unwahrscheinlicher treffen)
4. Koerper teile m ssen mehr funktionen haben wie das man schlechter fliehen kann wenn man ein Bein verloren hat und dass man noch schwere hat zu fliehen wenn beide beine verloren hat.

Wurde gemacht fuer Artorias:
Wie mache ich das man in Map k mpfen kann, nur Artorias und in der dritte map.
Kampf: der kampf seht so aus es spielt in der normale map statt und der monster ist auch da und wenn man ihn n ht kommt eine dialog dann geht kampf loss,
der monster macht angriffe und man seht es auch auf der map und man muss es ausweichen (rollen und beim rollen wird  der charakter vielleicht kurz hell blau,
damit man steht das er ausweicht) und man muss denn boss versuchen anzugreifen, der Boss braucht mehr angriffe und der Boss muss verschwinden wenn der tot ist und
wenn der Boss lebt muss gibt es keine ausgang, der boss soll am in der n he von ausgang spawnen.
Man soll kein E durchspamen koennen und mehrere angriff m glichkeit von Bosss

Wurde Verbessert: Bewegung systeme es funktioniert aber nicht so wie man es will.
Fuer alle: pos_min und pos_max soll sagen prozentual (Zum beispiel min 80 und max 100 steht fuer 80-100% der map) wo der Monster auf denn Map sein duerfen und auch spawnen.
1. Zuf llig bewegen, wenn spiler in der naehe dann soll der Monster Verfolgen.
2. Bewacht eine gebiete der Map
3. Spawn in der nahe der ausgang und verscuht denn spieler in weg zu stehen damit der spiele nicht zu ausgang kommt
4. Spawn zuf llig auf denn Map und jagt denn spieler.

*/

/*

Die syntax unten um die Funktion in eine andere datei zu verschieben:
#ifndef TESTW
#define TESTW
(Funktionen)
#endif

Labyrinth_Funktionen.h
Standart_Funktion.h

Optionen wie man Addon/Mods laden kann(M glichkeit darbieten eigene code zum laufen zu bringen):
----------------------------- Geht nicht weil es muss vor programm start da sein.
test3();
    FILE* datei;
    errno_t err = fopen_s(&datei, "..\\Test\\TEST2.h", "w");

    if (err != 0 || datei == NULL) {
        printf("Fehler beim Oeffnen der Datei\n");
        system("pause");
        return 1;
    }

    fprintf(datei, "#ifndef TESTW\n");
    fprintf(datei, "#define TESTW\n\n");
    fprintf(datei, "void test3() {\n");
    fprintf(datei, "    printf(\"Der Trick funktioniert!\\n\");\n");
    fprintf(datei, "    system(\"pause\");\n");
    fprintf(datei, "    printf(\"=========================================================\\n\\n\");\n");
    fprintf(datei, "}\n\n");
    fprintf(datei, "#endif\n");

    fclose(datei);

    printf("Die Header-Datei wurde erfolgreich generiert!\n");
    system("pause");
    test3();
    system("pause");


    ----------------------------- Geht nicht, weil mein_pulgin.dll ist entweder nicht f r die Ausf hrunger unter windows vorgesehen oder enth hlt fehler (Kurz gesagt keine ahnung):

    typedef void (*test3_func)();
    HMODULE hDll = LoadLibraryA("mein_plugin.dll");
    test3_func geladene_funktion = (test3_func)GetProcAddress(hDll, "test3");
    if (hDll == NULL) {
        DWORD fehler = GetLastError();
        printf("Fehler: Die DLL konnte nicht geladen werden!\n");
        printf("Windows-Fehlercode: %lu\n", fehler);

        if (fehler == 126) {
            printf("-> Code 126 bedeutet: Datei nicht gefunden. Pruefe den Ordner!\n");
        }
        else if (fehler == 193) {
            printf("-> Code 193 bedeutet: Architektur-Konflikt! (Z. B. Hauptprogramm ist 64-Bit, aber die DLL wurde als 32-Bit kompiliert).\n");
        }

        system("pause");
        return 1;
    }
    if (geladene_funktion == NULL) {
        DWORD fehlerCode = GetLastError();
        printf("GetProcAddress ist fehlgeschlagen! Windows-Fehlercode: %lu\n", fehlerCode);

        if (fehlerCode == 127) {
            printf("-> Fehler 127 bedeutet: 'Prozedur nicht gefunden'. Der Name 'test3' existiert so nicht in der DLL.\n");
        }

        FreeLibrary(hDll);
        system("pause");
        return 1;
    }
    geladene_funktion();
    FreeLibrary(hDll);

*/