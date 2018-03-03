/* COMP10002 Assignment 2, this programs find cheapest path from a point to
 * other points
 *
 * Novan Allanadi, October 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define STREETS_PER_NODE 4
#define IMPOSSIBLE_ST 999
#define INITIAL_SIZE 10
#define ASCII_VAL 97
#define MULTIPLIER 2
#define COL_MODIFIER 1
#define ROW_MODIFIER 0

/****************************************************************/

typedef struct {
    int row, col, gridloc_num;
} city_info;

typedef struct{
    int x;
    char y;
} coordinate;

typedef struct{
    coordinate *route;
    int index, max_size;
} paths;

typedef struct grids grids;

struct grids{
    coordinate name;
    grids *visited_from;
    int costs[STREETS_PER_NODE], dist;
};

/****************************************************************/

city_info city_process(grids ***loc, coordinate **grid_loc);
grids **allocate_location(int col, int row);
void free_grid(grids ***loc, coordinate **grid_loc, paths *routes, int rows);
void pathfinder(grids **loc, coordinate *grid_loc, city_info *city);
void print_shortest_route(paths *routes, grids **loc);
paths trace_route(grids *node);
void backtrace(grids *node, paths *routes);
int order_checker(coordinate *prev_visitor, coordinate *curr_visitor);
void initialize_max_value(grids **loc, city_info *city);
void find_route(grids **loc, grids **source, int source_num, city_info *city,
                int max_node);
void multiple_source_pathfinder(grids** loc, coordinate *grid_loc,
                                city_info *city);
void print_map(grids** loc, city_info *city);
int max_nodes(int possibilities);

/****************************************************************/

/* main function initializes all the function
 */

int main(int argc, char **argv) {
    /* declaring all the necessary variables */
    grids **loc = NULL;
    coordinate *grid_loc = NULL;
    paths routes;
    int index;

    /* getting the details of the city */
    city_info city = city_process(&loc, &grid_loc);

    /* getting the cheapest route to every nodes */
    pathfinder(loc, grid_loc, &city);

    /* tracing and printing the route to every destinations */
    for(index = 1; index < city.gridloc_num; index++){
        routes = trace_route(
                &loc[grid_loc[index].y - ASCII_VAL][grid_loc[index].x]);
        print_shortest_route(&routes, loc);
    }

    /* getting the cheapest routes to every nodes from multiple sources */
    multiple_source_pathfinder(loc, grid_loc, &city);

    /* printing the map */
    print_map(loc, &city);

    /* freeing all the allocated variables */
    free_grid(&loc, &grid_loc, &routes, city.row);
    return 0;
}

/****************************************************************/

/* city_process take in the input text and returning some details regarding the
 * city
 */

city_info city_process(grids ***loc, coordinate **grid_loc){
    city_info city;

    /* Taking the city dimensions */
    scanf("%d %d", &city.col, &city.row);

    int possibles = city.row * city.col;
    int impossible_routes = 0, possible_costs = 0;
    int i, j, col_num = 0, row_num = 0;

    /* malloc for the needed structs */
    grids **temp = allocate_location(city.col, city.row);
    *loc = temp;

    /* taking all the input to the struct array */
    for(i = 0; i < possibles; i++){

        scanf("%d%c", &temp[row_num][col_num].name.x,
              &temp[row_num][col_num].name.y);

        /* taking the costs to other nodes */
        for(j = 0; j < STREETS_PER_NODE; j++){
            scanf("%d", &temp[row_num][col_num].costs[j]);

            /* adding to the total of impossible routes */
            if (temp[row_num][col_num].costs[j] == IMPOSSIBLE_ST){
                impossible_routes++;
            }

                /* adding to the total of costs of possible routes */
            else{
                possible_costs += temp[row_num][col_num].costs[j];
            }
        }

        /* changing the index of the multidimensional array for inputting*/
        col_num++;
        if (col_num == city.col){
            row_num++;
            col_num = 0;
        }
    }

    int index = 0, curr_size = INITIAL_SIZE;
    coordinate *temp_gridloc =
            (coordinate *) malloc(sizeof(coordinate) * INITIAL_SIZE);

    *grid_loc = temp_gridloc;

    /* taking the provided grid locations */
    while(scanf("%d%c", &temp_gridloc[index].x, &temp_gridloc[index].y) == 2){
        /* should the array of structs is not enough, it will realloc the array
         * to a bigger size
         */
        if (index + 1 == curr_size) {
            curr_size *= MULTIPLIER;
            temp_gridloc = (coordinate *)
                    realloc(grid_loc, sizeof(coordinate) * curr_size);
        }
        index++;
    }

    /* printing the information of the city */
    printf("S1: grid is %d x %d, and has %d intersections\n", city.col,
           city.row, possibles);
    printf("S1: of %d possibilities, %d of them cannot be used\n",
           possibles * STREETS_PER_NODE, impossible_routes);
    printf("S1: total cost of remaining possibilities is %d seconds\n",
           possible_costs);
    printf("S1: %d grid locations supplied, first one is %d%c, last one "
                   "is %d%c\n\n", index, temp_gridloc[0].x, temp_gridloc[0].y,
           temp_gridloc[index-1].x, temp_gridloc[index-1].y);

    /* adding how many grid locations to the city info variable */
    city.gridloc_num = index;

    /* returning the city information */
    return city;
}

/****************************************************************/

/* allocating memory for the multidimensional array
 */

grids **allocate_location(int col, int row){
    grids **locations;
    int i;
    locations = (grids **) malloc(sizeof(grids *) * row);
    for (i = 0; i < row; i++) {
        locations[i] = (grids *) malloc(sizeof(grids) * col);
    }
    return locations;
}

/****************************************************************/

/* freeing all the allocated variables
 */

void free_grid(grids ***loc, coordinate **grid_loc, paths *routes, int rows){
    int i;
    free(*grid_loc);
    free(routes->route);
    for (i = 0; i < rows; i++){
        free((*loc)[i]);
    }
    free(*loc);
}

/****************************************************************/

/* initializing the values needed before calling the pathfinding algorithm
 */

void pathfinder(grids **loc, coordinate *grid_loc, city_info *city) {

    /* setting the source and the destination */
    grids *source = &loc[((int) grid_loc[0].y) - ASCII_VAL][grid_loc[0].x];

    /* initialize all distance value to maximum value */
    initialize_max_value(loc, city);

    /* setting the starting point and route initial values */
    source->dist = 0;
    source->visited_from = NULL;

    /* calling the recursive function to calculate all routes possible to
     * the destination
     */
    find_route(loc, &source, 1, city,
               max_nodes(city->row * city->col * STREETS_PER_NODE));
}

/****************************************************************/

/* printing the cheapest route
 */

void print_shortest_route(paths *routes, grids **loc){
    /* printing the starting point */
    printf("S2: start at grid %d%c, cost of %d\n",
           routes->route[routes->index].x,
           routes->route[routes->index].y,
           loc[(int) routes->route[routes->index].y - ASCII_VAL]
           [routes->route[routes->index].x].dist);
    int i;

    /* printing the cheapest route */
    for (i = routes->index - 1; i >= 0; i--){
        printf("S2:       then to %d%c, cost of %d\n", routes->route[i].x,
               routes->route[i].y,
               loc[(int) routes->route[i].y - ASCII_VAL]
               [routes->route[i].x].dist);
    }
}

/****************************************************************/

/* initalizing the variables needed before calling the recursive backtracing
 * function
 */

paths trace_route(grids *node) {
    paths routes;
    /* setting initial conditions for the route */
    routes.route = (coordinate *) malloc(sizeof(coordinate) * INITIAL_SIZE);
    routes.index = 0;
    routes.max_size = INITIAL_SIZE;

    /* calling the recursive function */
    backtrace(node, &routes);

    /* returning the route */
    return routes;
}

/****************************************************************/

/* finding the cheapest route by tracing back from the destination
 */

void backtrace(grids *node, paths *routes){
    /* if there are more coordinates that the car need to go through, the
     * function will reallocate for more spaces
     */
    if(routes->index == routes->max_size){
        routes->max_size *= MULTIPLIER;
        routes->route = (coordinate *)
                realloc(routes->route, sizeof(coordinate) * routes->max_size);
    }

    /* adding the coordinate name to the route array */
    routes->route[routes->index].x = node->name.x;
    routes->route[routes->index].y = node->name.y;

    /* a node that is not visited by other nodes suggests that it is the
     * starting point and the function will stop
     */
    if(node->visited_from == NULL){
        return;
    }

    /* adding the index to show how many coordinates the function go through */
    routes->index++;

    /* call the function recursively */
    backtrace(node->visited_from, routes);

}

/****************************************************************/

/* initialize all the necessary values before doing multiple source pathfinding
 */

void multiple_source_pathfinder(grids **loc, coordinate *grid_loc,
                                city_info *city){
    int i;
    /* initializing all the values to maximum value */
    initialize_max_value(loc, city);

    /* adding all the grid locations to the starting point array and initialize
     * all the necessary variables to the necessary values
     */
    grids *source[city->gridloc_num];
    for(i = 0; i < city->gridloc_num; i++){
        source[i] = &loc[(int)grid_loc[i].y - ASCII_VAL][grid_loc[i].x];
        source[i]->dist = 0;
        source[i]->visited_from = NULL;
    }

    /* calculating the maximum neighbours an array will store */
    int max_node = max_nodes(city->col * city->row * STREETS_PER_NODE);

    /* calling the recursive fucntion */
    find_route(loc, source, city->gridloc_num, city, max_node);
}

/****************************************************************/

/* to initialize all the distance value to maximum
 */

void initialize_max_value(grids **loc, city_info *city){
    int i, j;
    for(i = 0; i < city->row; i++){
        for(j = 0; j < city->col; j++){
            loc[i][j].dist = INT_MAX;
        }
    }
}

/****************************************************************/

/* printing the map for the cheapest routes to multiple locations
 */

void print_map(grids **loc, city_info *city){
    int i, j, k;

    /* printing the column numbers and border */
    printf("\nS3:");
    for(i = 0; i < city->col; i++){
        printf("        %d", i);
    }
    printf("\n");
    printf("S3:   +----+");
    for(i = 1; i < city->col; i++){
        printf("--------+");
    }
    printf("\n");


    for(i = 0; i < city->row; i++){

        /* printing the row letters */
        printf("S3: %c |   ", i + ASCII_VAL);

        /* this loop prints the distance of each node and its left and right
         * arrows
         */
        for (j = 0; j < city->col; j++){

            /* if it is the last node of a row, just prints the distance */
            if(j + 1 == city->col){
                printf("%2d\n", loc[i][j].dist);
                break;
            }

            /* print the distance */
            printf("%2d", loc[i][j].dist);

            /* if a node is visited from the right, it prints a left arrow */
            if(loc[i][j].visited_from != NULL &&
                loc[i][j].visited_from->name.x == loc[i][j+1].name.x &&
                loc[i][j].visited_from->name.y == loc[i][j+1].name.y){
                printf(" <<<<  ");
            }

            /* if a node is visited from the left, it prints a right arrow */
            else if (loc[i][j + 1].visited_from != NULL &&
                    loc[i][j + 1].visited_from->name.x == loc[i][j].name.x &&
                    loc[i][j + 1].visited_from->name.y == loc[i][j].name.y){
                printf(" >>>>  ");
            }

            /* if it is not visited by any node, it prints whitespaces */
            else{
                printf("       ");
            }
        }

        /* printing the up and down arrows twice */
        for (k = 0; k < 2; k++){

            /* if it is the last row, the printing stops */
            if(i + 1 == city->row){
                printf("\n");
                break;
            }

            printf("S3:   |    ");

            for(j = 0; j < city->col; j++){
                /* if a node is visited from below, an upper arrow is printed */
                if(loc[i][j].visited_from != NULL &&
                   loc[i][j].visited_from->name.x == loc[i+1][j].name.x &&
                    loc[i][j].visited_from->name.y == loc[i+1][j].name.y){
                    printf("^");
                }

                /* if a node is visited from above, a down arrow is printed */
                else if (loc[i+1][j].visited_from != NULL &&
                        loc[i+1][j].visited_from->name.x == loc[i][j].name.x &&
                        loc[i+1][j].visited_from->name.y == loc[i][j].name.y){
                    printf("v");
                }

                /* if a node is not visited, it prints a whitespace */
                else{
                    printf(" ");
                }

                /* printing new line if it is the last node */
                if(j+1 == city->col){
                    printf("\n");
                    break;
                }

                /* printing the whitespaces between each node */
                printf("        ");
            }
        }
    }
}

/****************************************************************/

/* a pathfinding algorithm that finds the cheapest cost to every node
 */

void find_route(grids **loc, grids **source, int source_num,
                city_info *city, int max_node){
    /* declaring all the necessary variables */
    grids *neighbours[max_node];
    int node_num = 0, i, j;
    int directions[STREETS_PER_NODE][2] = {{0, 1}, {-1, 0}, {0, -1}, {1, 0}},
    blocked = 1;

    /* checking for every source node */
    for(i = 0; i < source_num; i++){

        /* checking for the neighbours for every source node */
        for(j = 0; j < STREETS_PER_NODE; j++){

            /* if source does not exist, the loop breaks */
            if(source[i] == NULL){
                break;
            }
            /* if the node is unreachable, it is ignored */
            else if (source[i]->costs[j] == IMPOSSIBLE_ST){
                continue;
            }

            /* calculating the index and adding the neighbour */
            int col_num = source[i]->name.x + directions[j][COL_MODIFIER];
            int row_num = ((int) source[i]->name.y) -
                          ASCII_VAL + directions[j][ROW_MODIFIER];
            neighbours[node_num] = &loc[row_num][col_num];

            int distance = source[i]->dist + source[i]->costs[j];

            /* if the cumulative cost is equal or cheaper it is processed else
             * the neighbour is ignored and not processed for the next recursion
             * as a source node
             */
            if (distance <= neighbours[node_num]->dist){
                blocked = 0;

                /* if the cost is the same, it will be check for its
                 * lexicographic order
                 */
                if (distance == neighbours[node_num]->dist &&
                        order_checker(&neighbours[node_num]->visited_from->name,
                                      &source[i]->name)){
                    neighbours[node_num]->visited_from = source[i];
                }
                /* if it is cheaper, it updates the cost and change its previous
                 * node info
                 */
                else if (distance < neighbours[node_num]->dist){
                    neighbours[node_num]->dist = distance;
                    neighbours[node_num]->visited_from = source[i];
                }
            }
            else{
                neighbours[node_num] = NULL;
            }
            node_num++;
        }
    }

    /* if a node has no neighbours to reach, the recursion stops */
    if(blocked){
        return;
    }

    /* calling the recursion function */
    find_route(loc, neighbours, node_num, city, max_node);
}

/****************************************************************/

/* calculating the maximum possible neighbouring nodes
 */

int max_nodes(int possibilities) {
    int i, reduction = 1;

    /* keep reducing the possibles with the numbers of the power of how many
     * streets per node
     */
    for (i = 0; possibilities > reduction; i++){
        possibilities -= reduction;
        reduction = (int) pow(STREETS_PER_NODE, i+1);
    }
    return possibilities;
}

/****************************************************************/

/* checking the lexicographical order
 */

int order_checker(coordinate *prev_visitor, coordinate *curr_visitor){

    /* if the number is lower, then it is lower */
    if(curr_visitor->x < prev_visitor->x){
        return 1;
    }
    /* if the number is equal and its letter is lower, then it is lower */
    else if(curr_visitor->x == prev_visitor->x &&
            curr_visitor->y < prev_visitor->y){
        return 1;
    }
    return 0;
}

/*Algorithms are fun*/