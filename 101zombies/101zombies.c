  GNU nano 7.2                                                          101zombies.c                                                                    
#include <stdio.h>
#include <furi.h>
#include <gui/gui.h>

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    //canvas_set_font(canvas, FontPrimary);
    //canvas_draw_str(canvas, 32, 15, "101 Zombies!");

    canvas_set_font(canvas, FontSecondary);
    //int y = 30;
    int y = 10;

    canvas_draw_str(canvas, 1, y += 1, "Choose to FIGHT or RUN away");
    canvas_draw_str(canvas, 1, y += 8, "from the zombie horde! Chance");
    canvas_draw_str(canvas, 1, y += 8, "for new weapons when fighting.");
    canvas_draw_str(canvas, 1, y += 8, "However, consecutive fighting");
    canvas_draw_str(canvas, 1, y += 8, "increases zombie damage. Chance");
    canvas_draw_str(canvas, 1, y += 8, "for medikits when running. How-");
    canvas_draw_str(canvas, 1, y += 8, "ever, constant running is for");
    canvas_draw_str(canvas, 1, y += 8, "weak little girls! Good luck.");
}
static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}
int32_t zombies_main(void* p) {
    UNUSED(p);
    InputEvent event;
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, event_queue);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    while(true) {
        furi_check(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk);
        if(event.key == InputKeyBack) {
            break;
        }
    }
    furi_message_queue_free(event_queue);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    return 0;
}
