#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAP_X 40
#define MAP_Y 20
#define OBJ_GOOD_LEVEL 15
#define OBJ_BAD_LEVEL 10

#define OBJ_GOOD_SYB                '$'
#define OBJ_BAD_SYB                 '#'
#define OBJ_SHOOT_SYB               '|'
#define OBJ_HIT_SYB                 'X'
#define OBJ_BACKGROUND_SYB          '*'
#define OBJ_PLAYER_A_SYB            'A'
#define OBJ_PLAYER_B_SYB            'B'
#define OBJ_BOTH_SYB                '@'

#define M_SLEEP(n) usleep(n*1000);
#define WINING_SCORE 100

#define DBG 0

enum player{
    PLAYER1,
    PLAYER2,
    PLAYER_MAX
};

static int cur_x[2] = {19, 21};
static int cur_y[2] = {19, 19};
static int action[2];
static int score[2];

static int good_obj_x[OBJ_GOOD_LEVEL];
static int good_obj_y[OBJ_GOOD_LEVEL];
static int bad_obj_x[OBJ_BAD_LEVEL];
static int bad_obj_y[OBJ_BAD_LEVEL];

pthread_mutex_t mutex;

enum player_status{
    PLAYER_NO_FOUND,
    PLAYER1_FOUND,
    PLAYER2_FOUND,
    PLAYER_ALL_FOUND
};

int player_find(int x, int y) {
    int flag[PLAYER_MAX] = {0};
    if ( (x == cur_x[0]) && (y == cur_y[0]) ) {
        flag[0] = 1;
    }
    if ( (x == cur_x[1]) && (y == cur_y[1]) ) {
        flag[1] = 1;
    }

    if (flag[0] && flag[1]){
        return PLAYER_ALL_FOUND;
    }
    else if (flag[0]) {
        return PLAYER1_FOUND;
    }
    else if (flag[1]) {
        return PLAYER2_FOUND;
    }

    return PLAYER_NO_FOUND;
}

enum object_detect{
    OBJECT_NONE,
    OBJECT_GOOD,
    OBJECT_BAD
};

int object_search(int x, int y) {

    for (int i=0; i<OBJ_GOOD_LEVEL; i++) {
        if (good_obj_x[i]==x && good_obj_y[i]==y)
            return OBJECT_GOOD;
    }
    for (int i=0; i<OBJ_BAD_LEVEL; i++) {
        if (bad_obj_x[i]==x && bad_obj_y[i]==y)
            return OBJECT_BAD;
    }

    return OBJECT_NONE;
}

enum object_status{
    OBJECT_NOT_FOUND,
    OBJECT_FOUND,
    OBJECT_MISS,
    OBJECT_HIT_BY_P1,
    OBJECT_HIT_BY_P2,
    OBJECT_HIT_BY_BOTH
};

int object_find(int x, int y, int *result) {
    int flag[PLAYER_MAX] = {0};
    if (x == cur_x[0])
        flag[0] = 1;
    if (x == cur_x[1])
        flag[1] = 1;

    if (!flag)
        return 0;

    int val;
    int status = object_search(x,y);
    if (status == OBJECT_NONE) {
        if (flag[0] || flag[1]) {
            return OBJECT_MISS;
        }
        return OBJECT_NOT_FOUND;
    } else if (status == OBJECT_GOOD){
        *result = OBJECT_GOOD;
        val=1;
    } else if (status == OBJECT_BAD) {
        *result = OBJECT_BAD;
        val=-1;
    }

    if (flag[0] && flag[1]) {
        score[0]+=val;
        score[1]+=val;
        return OBJECT_HIT_BY_BOTH;
    } else if (!flag[0] && !flag[1]) {
        return OBJECT_FOUND;
    } else if (flag[0]) {
        score[0]+=val;
        return OBJECT_HIT_BY_P1;
    } else if (flag[1]) {
        score[1]+=val;
        return OBJECT_HIT_BY_P2;
    }
}

void *playground(void *arg) {
    static round = 0;
    while(1) {
        if (object_update())
            break;

        for (int y=0; y<MAP_Y; y++) {
            for (int x=0; x<MAP_X; x++) {
                int p_status = player_find(x,y);
                if (p_status == PLAYER1_FOUND){
                    printf("%c", OBJ_PLAYER_A_SYB);
                    continue;
                } else if (p_status == PLAYER2_FOUND) {
                    printf("%c", OBJ_PLAYER_B_SYB);
                    continue;
                } else if (p_status == PLAYER_ALL_FOUND) {
                    printf("%c", OBJ_BOTH_SYB);
                    continue;
                }

                int obj = 0;
                int o_status = object_find(x,y,&obj);
                if (o_status == OBJECT_NOT_FOUND){
                    printf("%c", OBJ_BACKGROUND_SYB);
                    continue;
                } else if (o_status == OBJECT_FOUND) {
                    if (obj == OBJECT_GOOD)
                        printf("%c", OBJ_GOOD_SYB);
                    else if (obj == OBJECT_BAD)
                        printf("%c", OBJ_BAD_SYB);
                    continue;
                } else if (o_status == OBJECT_MISS) {
                    printf("%c", OBJ_SHOOT_SYB);
                    continue;
                } else if (o_status == OBJECT_HIT_BY_P1 || o_status == OBJECT_HIT_BY_P2) {
                    printf("%c", OBJ_HIT_SYB);
                    continue;
                }
            }
            printf("\n");
        }
        round++;
        printf("[Round] %d\n", round);
        printf("[Score] playerA: %d playerB: %d\n\n", score[0], score[1]);

        M_SLEEP(100);
    }
}
/*
void *player(void *arg) {
    while(1) {
        if ( cur_x[0] == (MAP_X-1) ) {
            cur_x[0] = 0;
            cur_y[0]++;
            if ( cur_y[0] == (MAP_Y-1) )
                cur_y[0] = 0;
        } else
            cur_x[0]++;

        M_SLEEP(100);
    }
}*/

void do_action(enum player p, int act) {

    switch(act){
    case 1:
        cur_x[p]++;
        break;
    case 2:
        cur_x[p]--;
        break;
    default:
        break;
    }

    if ( cur_x[p] == (MAP_X) )
        cur_x[p] = 0;
    else if ( cur_x[p] == -1 )
        cur_x[p] = MAP_X-1;
}

void *player(void *arg) {
    int action;
    while(1) {
        action = rand()%3;
        do_action(PLAYER1, action);
        action = rand()%3;
        do_action(PLAYER2, action);

        M_SLEEP(100);
    }
}


int object_update() {
    if (score[0] == WINING_SCORE && score[1] == WINING_SCORE) {
        printf("<system> Both player win the game!\n");
        return 1;
    }
    else if (score[0] == WINING_SCORE) {
        printf("<system> PlayerA win the game!\n");
        return 1;
    }
    else if (score[1] == WINING_SCORE) {
        printf("<system> PlayerB win the game!\n");
        return 1;
    }

    for (int i=0; i<OBJ_GOOD_LEVEL; i++) {
        good_obj_x[i] = rand() % MAP_X;
        good_obj_y[i] = rand() % (MAP_Y-1);
    }

    for (int i=0; i<OBJ_BAD_LEVEL; i++) {
        bad_obj_x[i] = rand() % MAP_X;
        bad_obj_y[i] = rand() % (MAP_Y-1);
    }

#if DBG == 1
    printf("good: ");
    for (int i=0; i<OBJ_GOOD_LEVEL; i++)
        printf("(%-2d,%-2d) ", good_obj_x[i], good_obj_y[i]);
    printf("\nbad:  ");
    for (int i=0; i<OBJ_BAD_LEVEL; i++)
        printf("(%-2d,%-2d) ", bad_obj_x[i], bad_obj_y[i]);
    printf("\n");
#endif // DBG

    return 0;
}

int main()
{
    printf("Welcome to playground!\n");
    srand(time(NULL));   // Initialization, should only be called once.

    if ( pthread_mutex_init(&mutex, 0) )
        printf("<error> Mutex init failed!\n");

    pthread_t playground_t, player_t;
    pthread_create(&player_t, NULL, player, "p_thread");
    pthread_create(&playground_t, NULL, playground, "pg_thread");


    pthread_join(playground_t, NULL);
    pthread_join(player_t, NULL);
    printf("hee\n");
    pthread_mutex_destroy(&mutex);

    return 0;
}
