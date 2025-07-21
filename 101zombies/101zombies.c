#include <stdio.h>
#include <furi.h>
#include <gui/gui.h>
#include <stdlib.h>

typedef enum {
    StateTitle,
    StateRules,
    StateGame,
    StateQuitMenu,
    StateRetry,
} GameState;

typedef struct {
    GameState screen;
    GameState prev_screen;
    uint8_t yesno_selected;

    int zombies;
    int remaining;
    int health;
    int weapon;
    int fatigue;

    char message[64];
} AppState;


static void draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    AppState* state = ctx;

    canvas_clear(canvas);

    switch(state->screen) {
        case StateTitle:
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 32, 15, "101 Zombies!");
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 1, 30, "Choose to FIGHT or RUN away");
            canvas_draw_str(canvas, 1, 40, "from the zombie horde!");
            canvas_draw_str(canvas, 1, 50, "Chance for new weapons");
            canvas_draw_str(canvas, 1, 60, "when fighting. However,");
            break;
        case StateRules:
            canvas_draw_str(canvas, 1, 10, "consecutive fighting increases");
            canvas_draw_str(canvas, 1, 20, "zombie damage. Chance for");
            canvas_draw_str(canvas, 1, 30, "medikits when running.");
            canvas_draw_str(canvas, 1, 40, "However, constant running is");
            canvas_draw_str(canvas, 1, 50, "for weak little girls!");
            canvas_draw_str(canvas, 1, 60, "Good luck.");
            break;
        case StateQuitMenu:
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 20, 25, "Give up already?");
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 20, 45, state->yesno_selected ? "* Yes" : "  Yes");
            canvas_draw_str(canvas, 70, 45, !state->yesno_selected ? "* No" : "  No");
            break;
        case StateGame:
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 1, 10, state->message);
            char* second_line = strchr(state->message, '\n');
            second_line++; // move past '\n'
            canvas_draw_str(canvas, 1, 20, second_line);
            break;
        case StateRetry:
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 20, 20, "You died!");
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 20, 40, "Try again?");
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 20, 60, state->yesno_selected ? "* Yes" : "  Yes");
            canvas_draw_str(canvas, 70, 60, !state->yesno_selected ? "* No" : "  No");
            break;
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

static void generate_encounter(AppState* state) {
    state->zombies = rand() % 10 + 1;

    char dots[16];
    memset(dots, '.', state->zombies);
    dots[state->zombies] = '\0';

    snprintf(state->message, sizeof(state->message),
        "%d %s\n%d zombie%s approach%s.",
        state->remaining,   // countdown (101, 100, ...)
        dots,               // how many dots
        state->zombies,     // how many zombies
        state->zombies != 1 ? "s" : "",  // pluralize zombie
        state->zombies == 1 ? "es" : ""  // singular verb "approaches"
    );
}

int32_t zombies_main(void* p) {
    UNUSED(p);
    InputEvent event;

    AppState app_state = {.screen = StateTitle};

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, &app_state);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while(true) {
        furi_check(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk);

        if(event.key == InputKeyBack) {
            if(app_state.screen != StateQuitMenu) {
                app_state.prev_screen = app_state.screen;
                app_state.screen = StateQuitMenu;
                view_port_update(view_port);
            } else {
                app_state.screen = app_state.prev_screen;
                view_port_update(view_port);
            }
        } else if(app_state.screen == StateQuitMenu) {
            if(event.key == InputKeyLeft || event.key == InputKeyRight) {
                app_state.yesno_selected = !app_state.yesno_selected;
                view_port_update(view_port);
            } else if(event.type == InputTypeShort && event.key == InputKeyOk) {
                if(app_state.yesno_selected) {
                    break;
                } else {
                    app_state.screen = app_state.prev_screen;
                    view_port_update(view_port);
                    continue;
                }
            }
        } else if(event.type == InputTypeShort && event.key == InputKeyOk) {
            if(app_state.screen == StateTitle) {
                app_state.screen = StateRules;
                view_port_update(view_port);
                continue;
            } else if(app_state.screen == StateRules) {
                app_state.health = 100; // starting health
                app_state.remaining = 101; // starting number of zombies
                app_state.weapon = 0; // starts off with machete
                app_state.fatigue = 0; // starts off godlike

                generate_encounter(&app_state);

                app_state.screen = StateGame;
                view_port_update(view_port);
                continue;
            }
        } else if(app_state.screen == StateGame && event.type == InputTypeShort) {
            app_state.zombies = rand() % 10 + 1;

            if(app_state.remaining > 0) {
                app_state.remaining -= app_state.zombies;
                if(app_state.remaining < 0) app_state.remaining = 0; // prevent negative

            }

            char dots[16];// allow up to 16 dots
            memset(dots, '.', app_state.zombies);
            dots[app_state.zombies] = '\0'; // null terminate

            snprintf(app_state.message, sizeof(app_state.message),
                "%d %s\n%d zombie%s approach%s.",
                app_state.remaining,
                dots,
                app_state.zombies,
                app_state.zombies != 1 ? "s" : "",
                app_state.zombies == 1 ? "" : "es");

            if(event.key == InputKeyRight) { // fight
                int damage = rand() % 15 + 1;
                app_state.health -= damage;

                size_t len = strlen(app_state.message);
                snprintf(app_state.message + len, sizeof(app_state.message) - len, "\nFought! -%d HP", damage);

            } else if(event.key == InputKeyLeft) { // run
                int damage = rand() % 10;
                app_state.health -= damage;

                size_t len = strlen(app_state.message);
                snprintf(app_state.message + len, sizeof(app_state.message) - len, "\nRan! -%d HP", damage);
            }
            if(app_state.health <= 0) {
                app_state.yesno_selected = 1;  // default to "Yes"
                app_state.screen = StateRetry;
                view_port_update(view_port);
            }
        } else if(app_state.screen == StateRetry) {
            if(event.key == InputKeyLeft || event.key == InputKeyRight) {
                app_state.yesno_selected = !app_state.yesno_selected;
                view_port_update(view_port);

            } else if(event.key == InputKeyOk) {
                if(app_state.yesno_selected) {
                    app_state.health = 100;
                    app_state.remaining = 101;  // formerly horde
                    app_state.weapon = 0;
                    app_state.fatigue = 0;
                    app_state.zombies = 0;

                    generate_encounter(&app_state);

                    app_state.screen = StateGame;
                } else {
                    break;
                }
                view_port_update(view_port);
            }
        }
    }
    furi_message_queue_free(event_queue);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
