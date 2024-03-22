#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sleep.h"

#define grid_width 25
#define grid_height 25
#define grid_size 625
#define tile_set_size 5

typedef struct tile {
    char id;
    char str[20];
    int adj_mask[tile_set_size];
} tile;

typedef struct pos {
    int x;
    int y;
} pos;

pos pos_add(pos a, pos b){
    pos res = {a.x + b.x, a.y + b.y};
    return res;
}

typedef struct cell {
    int id;
    int visited;
} cell;

const pos neighbours[4] = {
    {0, -1}, 
    {1, 0}, 
    {0, 1}, 
    {-1, 0}
}; 

int random_int(int lower, int upper){
    return (rand() % (upper - lower + 1)) + lower; 
}

tile print_tile(tile *t){
    printf("%s ", t->str);
}

int check_bounds(int x, int y){
    if (x < 0 || y < 0 || x >= grid_width || y >= grid_height){
        return 0;
    } else {
        return 1;
    }
}

cell *pointer_grid(cell *grid, int x, int y){
    if (!check_bounds(x, y)) { 
        return NULL; 
    }
    return grid + (y * grid_height) + x;
}

cell *pointer_pos(cell *grid, pos possition){
    return pointer_grid(grid, possition.x, possition.y);
}

int get_id(cell *grid, pos possition){
    cell *point = pointer_pos(grid, possition);
    if (point == NULL) { return -1; }
    else { return point->id; }
}

int set_id(cell *grid, pos possition, int id){
    pointer_pos(grid, possition)->id = id;
}

void create_grid(cell *grid, cell def){
    for (int i = 0; i < grid_height; i++){
        for (int j = 0; j < grid_width; j++){
            grid[i * grid_height + j] = def;
        }
    }
}

void print_grid(cell *grid, tile *tiles){
    printf("\x1b[H\x1b[0J");
    for (int i = 0; i < grid_height; i++){
        for (int j = 0; j < grid_width; j++){
            int id = pointer_grid(grid, j, i)->id;
            if (id != -1) { print_tile(tiles + id); }
            else { printf(". "); }
        }
        printf("\n");
    }
}

int get_visited(cell *grid, pos possition){
    cell *point = pointer_pos(grid, possition);
    if (point == NULL){ return -1; }
    else { return point->visited; }
}

void set_visited(cell *grid, pos possition, int val){
    grid[possition.y * grid_height + possition.x].visited = val;
}

void get_possible_tiles(int *buf, int *count, pos prop_pos, cell *grid, tile *tiles){
    int adj_tiles[4];
    for (int i = 0; i < 4; i++){
        adj_tiles[i] = get_id(grid, pos_add(prop_pos, neighbours[i]));
    } // Builds a list of neighbours

    for (int i = 0; i < tile_set_size; i++){
        int valid = 1;

        if (adj_tiles[0] != -1 && !tiles[i].adj_mask[adj_tiles[0]]) { valid = 0; }
        else if (adj_tiles[1] != -1 && !tiles[i].adj_mask[adj_tiles[1]]) { valid = 0; }
        else if (adj_tiles[2] != -1 && !tiles[i].adj_mask[adj_tiles[2]]) { valid = 0; }
        else if (adj_tiles[3] != -1 && !tiles[i].adj_mask[adj_tiles[3]]) { valid = 0; }

        buf[i] = valid;
        if (valid) { *count += 1; }
    } // Finding possibilities
}

void propogate(cell *grid, tile *tiles, pos prop_pos, int hard){
    if (get_id(grid, prop_pos) != -1) { return; }
    if (!check_bounds(prop_pos.x, prop_pos.y)) { return; }
    if (get_visited(grid, prop_pos)) { return; }
    set_visited(grid, prop_pos, 1);
    // Collapse Cell
    int possibilities[tile_set_size];
    int count = 0;
    get_possible_tiles(possibilities, &count, prop_pos, grid, tiles);

    if (count == 0) {
        printf("Impossible instruction set.\n");
        exit(0);
    } // Impossible rule set

    int steps = random_int(1, count);
    int choice = 0;
    for (int i = 0; i < steps; i++){
        if (possibilities[choice] == 0){
            i -= 1;
        } 
        choice += 1;
    }
    choice -= 1; // Choose a tile

    if (hard || count == 1) { set_id(grid, prop_pos, choice); }
    int starting_index = random_int(0,3);
    for (int i = 0; i < 4; i++){
        propogate(grid, tiles, pos_add(prop_pos, neighbours[(starting_index + i) % 4]), 0);
    }
}

void generate(cell *grid, tile *tiles){
    pos starting_pos = {random_int(0, grid_width - 1), random_int(0, grid_height - 1)};
    set_id(grid, starting_pos, tile_set_size - 1);
    int starting_index = random_int(0,3);
    for (int i = 0; i < 4; i++){
        propogate(grid, tiles, pos_add(starting_pos, neighbours[(starting_index + i) % 4]), 0);
    }
    while (1){
        int min_count = tile_set_size + 1;
        pos min_pos = {0};
        int random_start_x = random_int(0, grid_width - 1);
        int random_start_y = random_int(0, grid_height - 1);
        for (int i = 0; i < grid_height; i++){
            for (int j = 0; j < grid_width; j++){
                pos tile_pos = {(random_start_x + j) % grid_width, (random_start_y + i) % grid_height};
                set_visited(grid, tile_pos, 0);
                int tile_id = get_id(grid, tile_pos);
                if (tile_id != -1) { continue; }
                int possibilites[tile_set_size]; // Not needed
                int count = 0; // Needed
                get_possible_tiles(possibilites, &count, tile_pos, grid, tiles);
                if (count < min_count) { 
                    min_count = count;
                    min_pos = tile_pos;
                }
            }
        }
        if (min_count == tile_set_size + 1){ 
            break; 
        }
        else { propogate(grid, tiles, min_pos, 1); }

        print_grid(grid, tiles);
        sleep_ms(7);
    }    
    print_grid(grid, tiles);
}

int main(){
    srand(time(0));

    tile tile_1 = {0, "\x1b[1;34m~\x1b[0m", {1, 1, 1, 0, 0}};
    tile tile_2 = {1, "\x1b[33m=\x1b[0m", {0, 0, 1, 0, 0}};
    tile tile_3 = {2, "\x1b[1;32m#\x1b[0m", {0, 1, 1, 1, 1}};
    tile tile_4 = {3, "\x1b[32m|\x1b[0m", {0, 0, 1, 1, 1}};
    tile tile_5 = {4, "\x1b[37m^\x1b[0m", {0, 0, 0, 1, 0}};

    tile *tiles = (tile *)malloc(sizeof(tile) * tile_set_size);
    *tiles = tile_1;
    *(tiles + 1) = tile_2;
    *(tiles + 2) = tile_3;
    *(tiles + 3) = tile_4;
    *(tiles + 4) = tile_5;

    cell *grid = (cell *)malloc(sizeof(cell) * grid_size);
    cell def = {-1, 0};

    create_grid(grid, def);
    generate(grid, tiles);

    free(grid);
    free(tiles);

    return 0;
}