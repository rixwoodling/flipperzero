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
    int damage;

    char line1[32];
    char line2[32];
    char line3[32];
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
            canvas_draw_str(canvas, 1, 10, state->line1);
            canvas_draw_str(canvas, 1, 20, state->line2);
            canvas_draw_str(canvas, 1, 30, state->line3);
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


int32_t zombies_main(void* p) {
    UNUSED(p);
    InputEvent event;

    AppState app_state;
    memset(&app_state, 0, sizeof(AppState));
    app_state.screen = StateTitle,
    app_state.remaining = 101;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
//    furi_message_queue_reset(event_queue);
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, &app_state);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while(true) {
        furi_check(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk);

        // back key logic
        if(event.key == InputKeyBack) {
            if(app_state.screen != StateQuitMenu) {
                app_state.prev_screen = app_state.screen;
                app_state.screen = StateQuitMenu;
                view_port_update(view_port);
            } else {
                app_state.screen = app_state.prev_screen;
                view_port_update(view_port);
            }

        // quit menu
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

        // control OK key functionality
        } else if(event.type == InputTypeShort && event.key == InputKeyOk) {
            if(app_state.screen == StateTitle) {
                app_state.screen = StateRules;
                view_port_update(view_port);
                continue;
            } else if(app_state.screen == StateRules) {
                app_state.screen = StateGame;
                continue;
            }

        // inside game loop
        } else if(app_state.screen == StateGame) {

            if(app_state.remaining == 101) {
                app_state.health = 100;
                app_state.weapon = 0;
                app_state.fatigue = 0;
                app_state.zombies = rand() % 10 + 1;
            }

            char dots[16];// allow up to 16 dots
            memset(dots, '.', app_state.zombies);
            dots[app_state.zombies] = '\0'; // null terminate

            snprintf(app_state.line1, sizeof(app_state.line1),
                "%d %s", app_state.remaining, dots);

            snprintf(app_state.line2, sizeof(app_state.line2),
                "%d zombie%s approach%s.",
                app_state.zombies,
                app_state.zombies != 1 ? "s" : "",
                app_state.zombies == 1 ? "" : "es");

            size_t free_mem = memmgr_get_free_heap();
            snprintf(app_state.line3, sizeof(app_state.line3), "Mem: %uB", (unsigned int)free_mem);

            view_port_update(view_port);

            // combat
            if((event.key == InputKeyRight && event.type == InputTypeShort) ||
               (event.key == InputKeyLeft && event.type == InputTypeShort)) {

                if(event.key == InputKeyRight) { // fight logic
                    int damage = rand() % 15 + 1;
                    app_state.health -= damage;
                    // TODO: line3: Fought! -d% HP

                } else if(event.key == InputKeyLeft) { // run logic
                    int damage = rand() % 10;
                    app_state.health -= damage;
                    // TODO: line3 Ran! -d% HP
                }

                // subtract zombies from remaining
                app_state.remaining -= app_state.zombies;
                if(app_state.remaining < 0) app_state.remaining = 0;

                // death check
                if(app_state.health <= 0) {
                    app_state.yesno_selected = 1;  // default to "Yes"
                    app_state.screen = StateRetry;
                    view_port_update(view_port);
                }
                app_state.zombies = rand() % 10 + 1;
            }

//----->| // retry if dead
        } else if(app_state.screen == StateRetry) {
            if(event.key == InputKeyLeft || event.key == InputKeyRight) {
                app_state.yesno_selected = !app_state.yesno_selected;
                view_port_update(view_port);

            } else if(event.key == InputKeyOk) {
                if(app_state.yesno_selected) {
                    app_state.remaining = 101;
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
