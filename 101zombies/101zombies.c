#include <stdio.h>
#include <furi.h>
#include <gui/gui.h>
#include <stdlib.h>

typedef enum {
    StateTitle,
    StateRules,
    StateGame,
    StateResult,
    StateQuitMenu,
    StateWin,
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

    char line1[32];
    char line2[32];
    char line3[32];
    char line4[32];
    char line5[32];
    char line6[32];

    char result1[48];
    char result2[32];
    char result3[32];
} AppState;

static const char* health_emo[] = {
    "dead","very fucked up","fucked up","like total shit","like shit",
    "wounded","banged up","alright","great","fuckin' great","godlike"
};	

static const char* weapon_names[] = {
    "a machete","a chainsaw","a shotgun","an AK-47",
    "a minigun","a flamethrower","double flamethrowers","the death ray"
};

static const char* luck_text[] = {
    "you got no chance in hell.","odds are slim.","you'd better run.",
    "it's dicey at best.","it'll be a tough fight.","it's a toss up.",
    "you've seen worse.","the odds are with you.",
    "this shouldn't be a problem.","easy.","you can't lose!"
};

static float fight_probability(AppState* app_state) {
    // Same as Python prob() function
    float player = (app_state->health + (app_state->weapon * 10)) / 20.0f;
    float enemy  = (app_state->zombies * 5) / 10.0f;

    if((player + enemy) <= 0.0f) {
        return 0.0f;
    }

    return 100.0f * (player / (player + enemy)); // win chance in %
}

static void update_wave_text(AppState* app_state) {
    char dots[16];
    memset(dots, '.', app_state->zombies);
    dots[app_state->zombies] = '\0';

    snprintf(app_state->line1, sizeof(app_state->line1),
        "%d %s", app_state->remaining, dots);

    if(app_state->zombies == 1) {
        snprintf(app_state->line2, sizeof(app_state->line2),
	    "A lone zombie approaches.");
    } else {
	snprintf(app_state->line2, sizeof(app_state->line2),
	    "%d zombies approach.", app_state->zombies);	    	
    }

    int idx = app_state->health / 10;
    if(idx > 10) idx = 10;
    if(idx < 0) idx = 0;

    snprintf(app_state->line3, sizeof(app_state->line3),
        "You're feeling %s.", health_emo[idx]);

    snprintf(app_state->line4, sizeof(app_state->line4),
	"Armed with %s,", weapon_names[app_state->weapon]);

    float prob = fight_probability(app_state);
    int idx_prob = (int)(prob / 10.0f);
    if(idx_prob > 10) idx_prob = 10;

    snprintf(app_state->line5, sizeof(app_state->line5),
	"%s", luck_text[idx_prob]);	    
}

// Helper to reset game state and generate initial wave
static void reset_game_state(AppState* app_state) {
    app_state->remaining = 101;
    app_state->health = 100;
    app_state->weapon = 0;
    app_state->fatigue = 0;
    app_state->zombies = rand() % (app_state->weapon + 10) + 1;

    update_wave_text(app_state);
}

static void draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    AppState* state = ctx;

    canvas_clear(canvas);

    switch(state->screen) {
        case StateTitle:
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 30, 15, "101 Zombies!");
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 1, 30, "Choose to FIGHT or RUN away");
            canvas_draw_str(canvas, 1, 40, "from the zombie horde!");
            canvas_draw_str(canvas, 1, 50, "Chance for new weapons");
            canvas_draw_str(canvas, 1, 60, "when fighting. However,");
            break;

        case StateRules:
            canvas_draw_str(canvas, 1, 10, "Consecutive fighting increases");
            canvas_draw_str(canvas, 1, 20, "zombie damage. Chance for");
            canvas_draw_str(canvas, 1, 30, "medikits when running.");
            canvas_draw_str(canvas, 1, 40, "However, constant running is");
            canvas_draw_str(canvas, 1, 50, "for weak little girls!");
            canvas_draw_str(canvas, 1, 60, "Good luck.");
            break;

        case StateQuitMenu:
	    canvas_set_font(canvas, FontSecondary);
	    canvas_draw_str(canvas, 1, 10, "github / rixwoodling"); 
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 20, 25, "Give up already?");
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 20, 45, state->yesno_selected ? "* Yes" : "  Yes");
            canvas_draw_str(canvas, 70, 45, !state->yesno_selected ? "* No" : "  No");
            break;

        case StateGame:
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 1, 10, state->line1);
            canvas_set_font(canvas, FontSecondary);
	    canvas_draw_str(canvas, 1, 20, state->line2);
            canvas_draw_str(canvas, 1, 30, state->line3);
            canvas_draw_str(canvas, 1, 40, state->line4);
            canvas_draw_str(canvas, 1, 50, state->line5);
            canvas_draw_str(canvas, 1, 60, state->line6);
	    break;

        case StateResult:
	    canvas_set_font(canvas, FontKeyboard);
	    canvas_draw_str(canvas, 1, 10, state->result1);
	    canvas_set_font(canvas, FontSecondary);
	    canvas_draw_str(canvas, 1, 20, state->result2);
            if(strlen(state->result3) > 0) {
	        canvas_draw_str(canvas, 1, 30, state->result3);
	    }
            break;

	case StateWin:
	    canvas_set_font(canvas, FontSecondary);
	    canvas_draw_str(canvas, 5, 25, "Few ever make it to the end.");
	    canvas_draw_str(canvas, 5, 35, "But you slaughtered them all.");
	    canvas_draw_str(canvas, 10, 45, "You are a fucking HERO!");
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 5, 60, state->yesno_selected ? "> Retry" : "  Retry");
            canvas_draw_str(canvas, 80, 60, !state->yesno_selected ? "> Exit" : "  Exit");
	    break;

        case StateRetry:
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 50, 15, "DEAD!");
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 1, 27, "Unfortunately, your wounds");
	    canvas_draw_str(canvas, 1, 37, "bring you down. And now you");
	    canvas_draw_str(canvas, 1, 47, "turn into a zombie!");
            canvas_set_font(canvas, FontKeyboard);
            canvas_draw_str(canvas, 5, 60, state->yesno_selected ? "> Retry" : "  Retry");
            canvas_draw_str(canvas, 80, 60, !state->yesno_selected ? "> Exit" : "  Exit");
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
    app_state.screen = StateTitle;
    app_state.remaining = 101;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, &app_state);
    view_port_input_callback_set(view_port, input_callback, event_queue);
    view_port_set_update_interval(view_port, 10000);

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
                    break; // exit app
                } else {
                    app_state.screen = app_state.prev_screen;
                    view_port_update(view_port);
                }
            }

	// win screen
	} else if(app_state.screen == StateWin) {
	    if(event.key == InputKeyLeft || event.key == InputKeyRight) {
	        app_state.yesno_selected = !app_state.yesno_selected;
		view_port_update(view_port);
	    } else if(event.key == InputKeyOk) {
		if(app_state.yesno_selected) {    
		    // retry selected
		    reset_game_state(&app_state);
	            app_state.screen = StateGame;
	        } else {	    
		    break;
		}    
                view_port_update(view_port);	    
            }

        // control OK key functionality
        } else if(event.key == InputKeyOk && event.type == InputTypeShort) {
            if(app_state.screen == StateTitle) {
                app_state.screen = StateRules;
                view_port_update(view_port);
                continue;
            } else if(app_state.screen == StateRules) {
                reset_game_state(&app_state);
                app_state.screen = StateGame;
                view_port_update(view_port);
                continue;
            }

        // GAME CODE
        } else if(app_state.screen == StateGame && event.type == InputTypeShort) {

            switch(event.key) {
                case InputKeyRight: {
		    // calculate win chance
		    float prob = fight_probability(&app_state);
                    int roll = rand() % 100 + 1; // random 1-100
		    
                    if(roll < prob) {
                        // win fight
		        app_state.remaining -= app_state.zombies;
		        if(app_state.remaining < 0) app_state.remaining = 0;

                        // fatigue increase on win
			app_state.fatigue += 1.5f;
			
			// result screen text
			snprintf(app_state.result1, sizeof(app_state.result1),
			    "%d ********", app_state.remaining);
	                snprintf(app_state.result2, sizeof(app_state.result2),
			    "You fight and win!");		

		        // weapon upgrade chance
		        if((rand() % 100) > 70 && app_state.weapon < 7) {
			    app_state.weapon += 1;
		            snprintf(app_state.result3, sizeof(app_state.result3),
				"You find %s! +%d",
		                weapon_names[app_state.weapon],
		                app_state.weapon);
			} else {
			    app_state.result3[0] = '\0';
		        }

                        // go to result screen
			app_state.screen = StateResult;
			view_port_update(view_port);
			break;

                    } else {
                        // lose fight
			app_state.fatigue += 2.5f; // losing is more tiring

			// damage = pain() + fatigue
                        int damage = (rand() % 15 + 1) + (int)app_state.fatigue;
                        app_state.health -= damage;

			app_state.remaining -= app_state.zombies;
			if(app_state.remaining < 0) app_state.remaining = 0;

                        // dead zombie stars
			char stars[32];
			int star_count = app_state.zombies;
			if(star_count > 30) star_count = 30;
			memset(stars, '*', star_count);
			stars[star_count] = '\0';

			// result screen for losing fight
			snprintf(app_state.result1, sizeof(app_state.result1),
			    "%d %s", app_state.remaining, stars);

			if(app_state.health > 0) {
			    snprintf(app_state.result2, sizeof(app_state.result2), "Zombies do some damage!");
			    snprintf(app_state.result3, sizeof(app_state.result3), "but you kill 'em all!");
			} else {
			    app_state.result2[0] = '\0';
		            app_state.result3[0] = '\0';	    
                        }
		        // go to result screen
			app_state.screen = StateResult;
	                view_port_update(view_port);
	                break;		
		    }
                } break;
		
		case InputKeyLeft: {
		    int damage = rand() % 10;
                    app_state.health -= damage;
                    app_state.remaining -= app_state.zombies;
		    if(app_state.remaining < 0) app_state.remaining = 0;
		    snprintf(app_state.line4, sizeof(app_state.line4), "You Ran!");

		    // Spawn next wave immediately
                    //if(app_state.health > 0) {
                    //    app_state.zombies = rand() % 10 + 1;
                    //}
                } break;		    

		default:
		    break;
            }

	    // death check
	    if(app_state.remaining <=0) {
                app_state.remaining = 0;
                app_state.screen = StateWin;		
	    } else if(app_state.health <=0) {
		app_state.health = 0;    
	        app_state.yesno_selected = 1;
	        app_state.screen = StateRetry;
	    }	

            // update wave text and redraw
	    update_wave_text(&app_state);
            view_port_update(view_port);

	} else if(app_state.screen == StateResult) {
            if(event.type == InputTypeShort && 
                (event.key == InputKeyOk || event.key == InputKeyLeft || event.key == InputKeyRight)) {

		if(app_state.health > 0 && app_state.remaining > 0) {
		    // spawn next wave
                    app_state.zombies = rand() % (app_state.weapon + 10) + 1;
	            if(app_state.zombies > app_state.remaining) {
	                app_state.zombies = app_state.remaining;
		    }
	            update_wave_text(&app_state);
       	            app_state.screen = StateGame;	
		
		} else if(app_state.remaining <= 0) {
		    app_state.screen = StateWin;

	        } else if(app_state.health <= 0) {
                    app_state.yesno_selected = 1;
		    app_state.screen = StateRetry;
	        }
                view_port_update(view_port);	
	    }	

        } else if(app_state.screen == StateRetry) {
            if(event.key == InputKeyLeft || event.key == InputKeyRight) {
                app_state.yesno_selected = !app_state.yesno_selected;
                view_port_update(view_port);

            } else if(event.key == InputKeyOk) {
                if(app_state.yesno_selected) {
                    reset_game_state(&app_state);
                    app_state.screen = StateGame;
                } else {
                    break; // quit game
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
