/*
 * SDL Poker Game - Five-Card Draw Poker
 * Language: C (C99/C11)
 * Graphics: SDL2, SDL2_image
 * Platform: Windows 10/11 with MinGW-w64
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define DECK_SIZE 52
#define HAND_SIZE 5
#define CARD_WIDTH 80
#define CARD_HEIGHT 120
#define BUTTON_WIDTH 120
#define BUTTON_HEIGHT 50

/* Card Suits and Ranks */
typedef enum {
    HEARTS, DIAMONDS, CLUBS, SPADES
} Suit;

typedef enum {
    TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE
} Rank;

/* Game States for State Machine */
typedef enum {
    START_ROUND,
    PLAYER_TURN,
    AI_TURN,
    SHOWDOWN,
    ROUND_END
} GameState;

/* Hand Rankings */
typedef enum {
    HIGH_CARD,
    PAIR,
    TWO_PAIR,
    THREE_OF_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_KIND,
    STRAIGHT_FLUSH
} HandRank;

/* Card Structure */
typedef struct {
    Suit suit;
    Rank rank;
    int value;
} Card;

/* Deck Structure - Contains array of 52 cards */
typedef struct {
    Card cards[DECK_SIZE];
    int top;
} Deck;

/* Hand Evaluation Result */
typedef struct {
    HandRank rank;
    int high_value;
} HandEval;

/* Button Structure */
typedef struct {
    SDL_Rect rect;
    const char* label;
    int active;
} Button;

/* Global Game Variables */
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* card_textures[DECK_SIZE];
SDL_Texture* card_back_texture = NULL;
SDL_Texture* table_texture = NULL;

Deck deck;
Card player_hand[HAND_SIZE];
Card ai_hand[HAND_SIZE];
int player_chips = 1000;
int ai_chips = 1000;
int pot = 0;
GameState game_state = START_ROUND;
int ai_revealed = 0;

Button bet_button = {{50, 600, BUTTON_WIDTH, BUTTON_HEIGHT}, "BET 50", 1};
Button check_button = {{200, 600, BUTTON_WIDTH, BUTTON_HEIGHT}, "CHECK", 1};
Button fold_button = {{350, 600, BUTTON_WIDTH, BUTTON_HEIGHT}, "FOLD", 1};

/* Function Prototypes */
void init_deck(Deck* d);
void fisher_yates_shuffle(Deck* d);
void deal_card(Deck* d, Card* hand, int index);
int compare_cards(const void* a, const void* b);
HandEval evaluate_hand(Card* hand);
int compare_hands(HandEval h1, HandEval h2);
SDL_Texture* load_texture(const char* path);
void load_all_textures();
void render_card(Card* c, int x, int y, int face_up);
void render_hand(Card* hand, int y, int face_up);
void render_button(Button* btn);
int is_point_in_button(int x, int y, Button* btn);
void handle_player_action(const char* action);
void ai_decision();
void showdown();
void reset_round();
void cleanup();

/* Card suit/rank to string helpers */
const char* suit_to_string(Suit s) {
    switch(s) {
        case HEARTS: return "H";
        case DIAMONDS: return "D";
        case CLUBS: return "C";
        case SPADES: return "S";
        default: return "?";
    }
}

const char* rank_to_string(Rank r) {
    switch(r) {
        case TWO: return "2";
        case THREE: return "3";
        case FOUR: return "4";
        case FIVE: return "5";
        case SIX: return "6";
        case SEVEN: return "7";
        case EIGHT: return "8";
        case NINE: return "9";
        case TEN: return "10";
        case JACK: return "J";
        case QUEEN: return "Q";
        case KING: return "K";
        case ACE: return "A";
        default: return "?";
    }
}

/* Initialize deck with 52 cards using array */
void init_deck(Deck* d) {
    int index = 0;
    for (int suit = HEARTS; suit <= SPADES; suit++) {
        for (int rank = TWO; rank <= ACE; rank++) {
            d->cards[index].suit = suit;
            d->cards[index].rank = rank;
            d->cards[index].value = rank;
            index++;
        }
    }
    d->top = 0;
}

/* Fisher-Yates Shuffle Algorithm */
void fisher_yates_shuffle(Deck* d) {
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = d->cards[i];
        d->cards[i] = d->cards[j];
        d->cards[j] = temp;
    }
    d->top = 0;
}

/* Deal a card from deck to hand */
void deal_card(Deck* d, Card* hand, int index) {
    if (d->top < DECK_SIZE) {
        hand[index] = d->cards[d->top++];
    }
}

/* Comparison function for qsort - sorts by rank */
int compare_cards(const void* a, const void* b) {
    Card* card_a = (Card*)a;
    Card* card_b = (Card*)b;
    return card_a->value - card_b->value;
}

/* Count occurrences of each rank using counting algorithm */
void count_ranks(Card* hand, int counts[13]) {
    memset(counts, 0, 13 * sizeof(int));
    for (int i = 0; i < HAND_SIZE; i++) {
        counts[hand[i].rank - 2]++;
    }
}

/* Check if hand is a flush */
int is_flush(Card* hand) {
    Suit first = hand[0].suit;
    for (int i = 1; i < HAND_SIZE; i++) {
        if (hand[i].suit != first) return 0;
    }
    return 1;
}

/* Check if hand is a straight (hand must be sorted) */
int is_straight(Card* hand) {
    for (int i = 0; i < HAND_SIZE - 1; i++) {
        if (hand[i+1].value != hand[i].value + 1) {
            return 0;
        }
    }
    return 1;
}

/* Evaluate hand using sorting and counting algorithms */
HandEval evaluate_hand(Card* hand) {
    HandEval result = {HIGH_CARD, 0};
    Card sorted[HAND_SIZE];
    memcpy(sorted, hand, HAND_SIZE * sizeof(Card));
    
    /* Sort cards using qsort */
    qsort(sorted, HAND_SIZE, sizeof(Card), compare_cards);
    
    /* Count ranks using counting algorithm */
    int counts[13];
    count_ranks(sorted, counts);
    
    int pairs = 0, threes = 0, fours = 0;
    int high = 0;
    
    for (int i = 0; i < 13; i++) {
        if (counts[i] == 4) {
            fours = 1;
            high = i + 2;
        } else if (counts[i] == 3) {
            threes = 1;
            high = i + 2;
        } else if (counts[i] == 2) {
            pairs++;
            high = i + 2;
        } else if (counts[i] == 1 && high == 0) {
            high = i + 2;
        }
    }
    
    int flush = is_flush(sorted);
    int straight = is_straight(sorted);
    
    /* Determine hand rank */
    if (straight && flush) {
        result.rank = STRAIGHT_FLUSH;
        result.high_value = sorted[4].value;
    } else if (fours) {
        result.rank = FOUR_OF_KIND;
        result.high_value = high;
    } else if (threes && pairs == 1) {
        result.rank = FULL_HOUSE;
        result.high_value = high;
    } else if (flush) {
        result.rank = FLUSH;
        result.high_value = sorted[4].value;
    } else if (straight) {
        result.rank = STRAIGHT;
        result.high_value = sorted[4].value;
    } else if (threes) {
        result.rank = THREE_OF_KIND;
        result.high_value = high;
    } else if (pairs == 2) {
        result.rank = TWO_PAIR;
        result.high_value = high;
    } else if (pairs == 1) {
        result.rank = PAIR;
        result.high_value = high;
    } else {
        result.rank = HIGH_CARD;
        result.high_value = sorted[4].value;
    }
    
    return result;
}

/* Compare two hands - returns 1 if h1 wins, -1 if h2 wins, 0 for tie */
int compare_hands(HandEval h1, HandEval h2) {
    if (h1.rank > h2.rank) return 1;
    if (h1.rank < h2.rank) return -1;
    if (h1.high_value > h2.high_value) return 1;
    if (h1.high_value < h2.high_value) return -1;
    return 0;
}

/* Load texture from file path */
SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Failed to load image %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

/* Load all card textures and table background */
void load_all_textures() {
    /* Load card back */
    card_back_texture = load_texture("res/card_back.png");
    
    /* Load table background */
    table_texture = load_texture("res/table.png");
    
    /* Load all 52 card face textures */
    int index = 0;
    const char* suits[] = {"H", "D", "C", "S"};
    const char* ranks[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
    
    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 13; r++) {
            char path[256];
            snprintf(path, sizeof(path), "res/%s%s.png", suits[s], ranks[r]);
            card_textures[index++] = load_texture(path);
        }
    }
}

/* Get texture index for a card */
int get_card_texture_index(Card* c) {
    return (c->suit * 13) + (c->rank - 2);
}

/* Render a single card */
void render_card(Card* c, int x, int y, int face_up) {
    SDL_Rect dest = {x, y, CARD_WIDTH, CARD_HEIGHT};
    
    if (face_up) {
        int index = get_card_texture_index(c);
        if (card_textures[index]) {
            SDL_RenderCopy(renderer, card_textures[index], NULL, &dest);
        }
    } else {
        if (card_back_texture) {
            SDL_RenderCopy(renderer, card_back_texture, NULL, &dest);
        }
    }
}

/* Render a hand of 5 cards */
void render_hand(Card* hand, int y, int face_up) {
    int start_x = (WINDOW_WIDTH - (HAND_SIZE * CARD_WIDTH + 4 * 20)) / 2;
    for (int i = 0; i < HAND_SIZE; i++) {
        render_card(&hand[i], start_x + i * (CARD_WIDTH + 20), y, face_up);
    }
}

/* Render a button */
void render_button(Button* btn) {
    if (!btn->active) return;
    
    /* Draw button background */
    SDL_SetRenderDrawColor(renderer, 100, 100, 200, 255);
    SDL_RenderFillRect(renderer, &btn->rect);
    
    /* Draw button border */
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &btn->rect);
}

/* Check if point is inside button */
int is_point_in_button(int x, int y, Button* btn) {
    return btn->active && 
           x >= btn->rect.x && x <= btn->rect.x + btn->rect.w &&
           y >= btn->rect.y && y <= btn->rect.y + btn->rect.h;
}

/* Handle player's action */
void handle_player_action(const char* action) {
    if (strcmp(action, "BET") == 0) {
        if (player_chips >= 50) {
            player_chips -= 50;
            pot += 50;
            game_state = AI_TURN;
        }
    } else if (strcmp(action, "CHECK") == 0) {
        game_state = AI_TURN;
    } else if (strcmp(action, "FOLD") == 0) {
        ai_chips += pot;
        pot = 0;
        game_state = ROUND_END;
    }
}

/* Simple AI decision logic */
void ai_decision() {
    /* AI simply calls or checks */
    if (pot > 20) {  /* If player bet */
        if (ai_chips >= 50) {
            ai_chips -= 50;
            pot += 50;
        }
    }
    game_state = SHOWDOWN;
}

/* Showdown - reveal hands and determine winner */
void showdown() {
    ai_revealed = 1;
    
    HandEval player_eval = evaluate_hand(player_hand);
    HandEval ai_eval = evaluate_hand(ai_hand);
    
    int result = compare_hands(player_eval, ai_eval);
    
    if (result > 0) {
        player_chips += pot;
        printf("Player wins with hand rank %d!\n", player_eval.rank);
    } else if (result < 0) {
        ai_chips += pot;
        printf("AI wins with hand rank %d!\n", ai_eval.rank);
    } else {
        player_chips += pot / 2;
        ai_chips += pot / 2;
        printf("Tie!\n");
    }
    
    pot = 0;
    game_state = ROUND_END;
}

/* Reset for new round */
void reset_round() {
    ai_revealed = 0;
    pot = 0;
    
    /* Ante */
    if (player_chips >= 10 && ai_chips >= 10) {
        player_chips -= 10;
        ai_chips -= 10;
        pot = 20;
    }
    
    /* Shuffle and deal */
    init_deck(&deck);
    fisher_yates_shuffle(&deck);
    
    for (int i = 0; i < HAND_SIZE; i++) {
        deal_card(&deck, player_hand, i);
        deal_card(&deck, ai_hand, i);
    }
    
    game_state = PLAYER_TURN;
}

/* Cleanup SDL resources */
void cleanup() {
    for (int i = 0; i < DECK_SIZE; i++) {
        if (card_textures[i]) SDL_DestroyTexture(card_textures[i]);
    }
    if (card_back_texture) SDL_DestroyTexture(card_back_texture);
    if (table_texture) SDL_DestroyTexture(table_texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

/* Main function */
int main(int argc, char* argv[]) {
    srand(time(NULL));
    
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    /* Initialize SDL_image */
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    
    /* Create window */
    window = SDL_CreateWindow("SDL Poker Game",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    /* Create renderer */
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    /* Load all textures */
    load_all_textures();
    
    /* Start first round */
    reset_round();
    
    /* Main game loop - State Machine */
    int running = 1;
    SDL_Event event;
    
    while (running) {
        /* Handle events */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x;
                int y = event.button.y;
                
                if (game_state == PLAYER_TURN) {
                    if (is_point_in_button(x, y, &bet_button)) {
                        handle_player_action("BET");
                    } else if (is_point_in_button(x, y, &check_button)) {
                        handle_player_action("CHECK");
                    } else if (is_point_in_button(x, y, &fold_button)) {
                        handle_player_action("FOLD");
                    }
                } else if (game_state == ROUND_END) {
                    /* Click anywhere to start new round */
                    reset_round();
                }
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                if (game_state == ROUND_END) {
                    reset_round();
                }
            }
        }
        
        /* State machine updates */
        if (game_state == AI_TURN) {
            SDL_Delay(1000);  /* Pause for AI "thinking" */
            ai_decision();
        } else if (game_state == SHOWDOWN) {
            SDL_Delay(500);
            showdown();
        }
        
        /* Render */
        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
        SDL_RenderClear(renderer);
        
        /* Render table background if loaded */
        if (table_texture) {
            SDL_RenderCopy(renderer, table_texture, NULL, NULL);
        }
        
        /* Render AI hand (top) */
        render_hand(ai_hand, 50, ai_revealed);
        
        /* Render player hand (bottom) */
        render_hand(player_hand, 550, 1);
        
        /* Render buttons during player turn */
        if (game_state == PLAYER_TURN) {
            render_button(&bet_button);
            render_button(&check_button);
            render_button(&fold_button);
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);  /* ~60 FPS */
    }
    
    cleanup();
    return 0;
}