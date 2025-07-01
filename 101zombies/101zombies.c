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
            canvas_draw_str(canvas, 20, 45, state->yesno_selected ? "> Yes" : "  Yes");
            canvas_draw_str(canvas, 70, 45, !state->yesno_selected ? "> No" : "  No");
            break;
        case StateGame:
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 10, 15, "Zombies incoming!");
            canvas_set_font(canvas, FontSecondary); //

            char buf[32];
            snprintf(buf, sizeof(buf), "Zombies: %d", state->zombies);
            canvas_draw_str(canvas, 10, 35, buf);

            canvas_draw_str(canvas, 10, 55, state->message);

            char health_buf[32];
            snprintf(health_buf, sizeof(health_buf), "Health: %d", state->health);
            canvas_draw_str(canvas, 10, 45, health_buf);
            break;
        case StateRetry:
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
//                app_state.horde = 101; // starting number of zombies
                app_state.weapon = 0; // starts off with machete
                app_state.fatigue = 0; // starts off godlike                 

                app_state.screen = StateGame;
                view_port_update(view_port);
                continue;
            }
        } else if(app_state.screen == StateGame && event.type == InputTypeShort) {
            app_state.zombies = rand() % 10 + 1;

            if(event.key == InputKeyRight) { // fight
                int damage = rand() % 15 + 1;
                app_state.health -= damage;
                snprintf(app_state.message, sizeof(app_state.message), "Fought! -%d HP", damage);
            } else if(event.key == InputKeyLeft) { // run
                int damage = rand() % 10;
                app_state.health -= damage;
                snprintf(app_state.message, sizeof(app_state.message), "Ran! -%d HP", damage);
            }
            if(app_state.health <= 0) {
                snprintf(app_state.message, sizeof(app_state.message), "You died!");
                app_state.screen = StateTitle; // or a new GameOver screen if you want
            }
            
            view_port_update(view_port);
        }
    }

    furi_message_queue_free(event_queue);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
