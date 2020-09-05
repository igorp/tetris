/* tetris.c written by Igor Podsechin during Sep 2017
 * A Tetris Jr. clone made for the purpose of prototyping
 * the program for an embedded chip with a LED matrix.
 */

#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

typedef enum { I, O, T, S, Z, J, L } block_type;
typedef enum { left, right } direction;

const int GAME_AREA_WIDTH  = 8;
const int GAME_AREA_HEIGHT = 16;

void update_score();
void set_current_block(bool b[][4]);
void rotate_current_block(direction d);
void print_current_block();
void drop_block_from_sky();
bool current_block_is_in_illegal_position();
void solidify_current_block();
void check_for_full_rows();
void reset_game();

// timers that update frames and game clocks
ALLEGRO_TIMER *fps_timer = NULL;
ALLEGRO_TIMER *game_timer = NULL;

const float FPS = 60;
float game_speed = 0.4;

int score = 0;
bool display_matrix[16][8];

bool block_I [4][4] = {{false, false, false, false},
                       {true,  true,  true,  true},
                       {false, false, false, false},
                       {false, false, false, false}};

bool block_O [4][4] = {{false, true,  true,  false},
                       {false, true,  true,  false},
                       {false, false, false, false},
                       {false, false, false, false}};

bool block_T [4][4] = {{false, true,  false, false},
                       {true,  true,  true,  false},
                       {false, false, false, false},
                       {false, false, false, false}};

bool block_S [4][4] = {{false, true,  true,  false},
                       {true,  true,  false, false},
                       {false, false, false, false},
                       {false, false, false, false}};

bool block_Z [4][4] = {{true,  true,  false, false},
                       {false, true,  true,  false},
                       {false, false, false, false},
                       {false, false, false, false}};

bool block_J [4][4] = {{true,  false, false, false},
                       {true,  true,  true,  false},
                       {false, false, false, false},
                       {false, false, false, false}};

bool block_L [4][4] = {{false, false, true,  false},
                       {true,  true,  true,  false},
                       {false, false, false, false},
                       {false, false, false, false}};

bool current_block[4][4];                
block_type current_block_type;

// these two variables give the location of 
// the current block in the game area
int block_location_x;
int block_location_y; 

int main() {
    int size = 40;
    // flags for timers to trigger updates
    bool redraw = true;
    bool update_logic = true;
    
    // seed the randomizer
    srand(time(NULL));
   
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;

    al_init_primitives_addon();
    al_init();
    al_install_keyboard();

    fps_timer = al_create_timer(1.0 / FPS);
    game_timer = al_create_timer(game_speed);
    display = al_create_display(8*size, 16*size);
    event_queue = al_create_event_queue();
        
    // register events to the event queue
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(fps_timer));
    al_register_event_source(event_queue, al_get_timer_event_source(game_timer));

    al_start_timer(fps_timer);
    al_start_timer(game_timer);

    drop_block_from_sky();
   
    while(true) {
        ALLEGRO_EVENT ev;

        al_wait_for_event(event_queue, &ev);
                                        
        if(ev.type == ALLEGRO_EVENT_TIMER) {
            if(ev.timer.source == fps_timer) {
                redraw = true;
            }
            if(ev.timer.source == game_timer) {
                update_logic = true;
            }
        }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
        // handle user input
        else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch(ev.keyboard.keycode) {
                case ALLEGRO_KEY_LEFT:
                    block_location_x--;
                    if(current_block_is_in_illegal_position()) {
                        block_location_x++;
                    }
                    break;
                case ALLEGRO_KEY_RIGHT:
                    block_location_x++;
                    if(current_block_is_in_illegal_position()) {
                        block_location_x--;
                    }
                    break;
                case ALLEGRO_KEY_UP:
                case ALLEGRO_KEY_SPACE:
                    rotate_current_block(right);
                    if(current_block_is_in_illegal_position()) {
                        rotate_current_block(left);
                    }
                    break;
            }
        }
        // if enough time has passed, update the game logic 
        if(update_logic) {
            update_logic = false;
            // move current block down
            block_location_y++;

            // check if block has hit bottom or another piece
            if(current_block_is_in_illegal_position()) {
                block_location_y--;
                solidify_current_block();
                check_for_full_rows();
                drop_block_from_sky();
                // check if the game has ended
                if(current_block_is_in_illegal_position()) {
                    reset_game();
                }
            }
        }

        // if enough time has passed, we redraw the screen
        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_clear_to_color(al_map_rgb(255, 255, 255));

            // draw game area
            for(int n = 0; n < 16; n++) {
                for(int m = 0; m < 8; m++) {
                    if(display_matrix[n][m] == true) {
                        al_draw_filled_rectangle(m * size, n * size, 
                            m * size + size, n * size + size, al_map_rgb(0, 0, 0));
                    }
                }
            }

            // draw current block
            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    if(current_block[i][j]) {
                        al_draw_filled_rectangle((block_location_x + j) * size, 
                        (block_location_y + i) * size, 
                        (block_location_x + j) * size + size, 
                        (block_location_y + i) * size + size, al_map_rgb(0, 0, 0));
                    }     
                }
            }
            al_flip_display();
        }
    }

    al_destroy_event_queue(event_queue);
    al_destroy_display(display);
    al_destroy_timer(fps_timer);
    al_destroy_timer(game_timer);
    
    return 0;
}

// Creates a new block and puts up to the top.
void drop_block_from_sky() {
    int r = rand() % 7;
    switch(r) {
        case I:
            set_current_block(block_I);
            current_block_type = I;
            break;
        case O:
            set_current_block(block_O);
            current_block_type = O;
            break;
        case T:
            set_current_block(block_T);
            current_block_type = T;
            break;
        case S:
            set_current_block(block_S);
            current_block_type = S;
            break;
        case Z:
            set_current_block(block_Z);
            current_block_type = Z;
            break;
        case J:
            set_current_block(block_J);
            current_block_type = J;
            break;
        case L:
            set_current_block(block_L);
            current_block_type = L;
            break;
    }
    block_location_x = 2;
    block_location_y = 2;
}

// Sets the topmost row of "pixels" equal to the binary
// representation of the current score.
void update_score() {
    int x = score;
    int n = 0;
    for (unsigned i = 1 << 7; i > 0; i = i / 2) {
        if(x >= i) {
            x -= i;
            display_matrix[0][n] = true;
        }
        else {
            display_matrix[0][n] = false;
        }
        n++;
    }
}

// Sets the block that is given as a parameter as the current block.
void set_current_block(bool b[][4]) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
           current_block[i][j] = b[i][j];
        }
    }
}

// Rotates current block in direction d, unless it is a square.
void rotate_current_block(direction d) {
    if(current_block_type != O) {
        // transpose matrix
        for(int i = 0; i < 3; i++) {
            for(int j = i+1; j < 4; j++) {
                bool temp = current_block[i][j];
                current_block[i][j] = current_block[j][i];
                current_block[j][i] = temp;
            }
        }
        // reverse row or column depending on rotation type
        for(int i = 0; i < 4; i++) {
            int j = 0;
            // set side length of submatrix to reverse
            int k = 2;
            if(current_block_type == I) {
                k = 3;
            }
            while(j < k) {
                if(d == right) {
                    bool temp = current_block[i][j];
                    current_block[i][j] = current_block[i][k];
                    current_block[i][k] = temp;
                }
                else {
                    bool temp = current_block[j][i];
                    current_block[j][i] = current_block[k][i];
                    current_block[k][i] = temp;
                }
                j++;
                k--;
            }
        }
    }
}

// Returns true if any part of the current block is out
// of the play area bounds or hits another block.
bool current_block_is_in_illegal_position() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            // part of the block..
            if(current_block[i][j]) {
                // ..is out of game-area
                if(block_location_x + j < 0 ||
                    block_location_x + j >= GAME_AREA_WIDTH ||
                    block_location_y + i < 2 ||
                    block_location_y + i >= GAME_AREA_HEIGHT) {
                    return true;
                }
                // ..hits an old block
                if(display_matrix[block_location_y + i][block_location_x + j]) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Sets the current block into the play-area making it part of it.
void solidify_current_block() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(current_block[i][j]) {
                display_matrix[block_location_y + i][block_location_x + j] = true;
            }
        }
    }
}
    
// Handles full rows by first locating and then deleting them.
void check_for_full_rows() {
    for(int i = 2; i < GAME_AREA_HEIGHT; i++) {
        bool row_is_full = true;
        for(int j = 0; j < GAME_AREA_WIDTH; j++) {
            if(!display_matrix[i][j]) {
                row_is_full = false;
                break;
            }
        }
        // delete the full row by shifting all rows above by one down
        if(row_is_full) {
            int n = i;
            while(n - 1 >= 2) {
                for(int j = 0; j < GAME_AREA_WIDTH; j++) {
                    display_matrix[n][j] = display_matrix[n - 1][j];
                }
                n--;
            }
            score++;
            update_score();
            // increase game speed at regular intervals
            if(score % 12 == 0) {
                if(game_speed > 0.1) {
                    game_speed -= 0.05;
                    al_set_timer_speed(game_timer, game_speed);
                }
            }
        }
    }
}

// Resets score, game speed and clear the game area.
void reset_game() {
    score = 0;
    update_score(); 

    game_speed = 0.4;
    al_set_timer_speed(game_timer, game_speed);

    for(int i = 0; i < GAME_AREA_HEIGHT; i++) {
        for(int j = 0; j < GAME_AREA_WIDTH; j++) {
            display_matrix[i][j] = 0;
        }
    }
}
