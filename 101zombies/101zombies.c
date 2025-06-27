#include <stdio.h>
#include <furi.h>
#include <gui/gui.h>

typedef enum {
    StateTitle,
    StateRules,
//    StateMainGame,
//    StateBackMenu,
//    StateGameOver,
    StateConfirmExit,
} GameState;

typedef struct {
    GameState screen;
    uint8_t selection;
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
        case StateConfirmExit:
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 10, 10, "Give up?");

            if(state->selection == 0) {
                canvas_draw_str(canvas, 10, 30, "> No");
                canvas_draw_str(canvas, 10, 40, "  Yes, I'm scared.");
            } else {
                canvas_draw_str(canvas, 10, 30, "  No");
                canvas_draw_str(canvas, 10, 40, "> Yes, I'm scared.");
            }
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
            app_state.screen == StateConfirmExit; //
            app_state.selection = 0;              //
            view_port_update(view_port);          //
            continue;                             //
        }

        if(event.key == InputKeyOk) {
            switch(app_state.screen) {
                case StateTitle:
                    app_state.screen = StateRules;
                    break;
                case StateRules:
                    app_state.screen = StateTitle;
                    break;
                case StateConfirmExit:
                    if(event.key == InputKeyUp || event.key == InputKeyDown) {
                        app_state.selection ^= 1;
                    } else {
                        if(app_state.selection == 0) {
                            app_state.screen = StateTitle;
                        } else {
                            break;
                        }
                    }
                    view_port_update(view_port);
                    break;
                default:
                    break;
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
