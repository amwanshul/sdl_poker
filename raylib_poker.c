#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>


typedef enum { HEARTS, DIAMONDS, CLUBS, SPADES } Suit;
typedef enum {
    RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7,
    RANK_8, RANK_9, RANK_10, RANK_J, RANK_Q, RANK_K, RANK_A
} Rank;

typedef struct {
    Rank rank;
    Suit suit;
} Card;

typedef struct {
    int top;
    Card cards[52];
} DeckStack;

typedef enum {
    HAND_HIGH_CARD,
    HAND_PAIR,
    HAND_TWO_PAIR,
    HAND_TRIPS,
    HAND_STRAIGHT,
    HAND_FLUSH,
    HAND_FULL_HOUSE,
    HAND_QUADS,
    HAND_STRAIGHT_FLUSH
} HandRank;

typedef struct {
    HandRank rank;
    Rank high_card_rank;
} HandEvaluation;

typedef struct {
    Rectangle rect;
    const char *text;
    bool enabled;
    bool visible;
} Button;

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_INIT_ROUND,
    GAME_STATE_BETTING_1,
    GAME_STATE_AI_TURN_1,
    GAME_STATE_PLAYER_DRAW,
    GAME_STATE_AI_DRAW,
    GAME_STATE_BETTING_2,
    GAME_STATE_AI_TURN_2,
    GAME_STATE_SHOWDOWN,
    GAME_STATE_ROUND_END
} GameState;

typedef struct {
    bool discard_mask[5];
    int discard_count;
    int score;
} DrawStrategy;


Font main_font;
Card deck[52];
DeckStack deck_stack;
Card player_hand[5];
Card ai_hand[5];
GameState current_state = GAME_STATE_MENU;
Button btn_bet20, btn_check, btn_fold;
float pot = 0;
float current_bet = 0;
float player_chips = 1000;
float ai_chips = 1000;
bool round_initialized = false;
char game_log[20][128];
int log_count = 0;


void add_to_log(const char *msg) {
    if (log_count < 20) strncpy(game_log[log_count++], msg, 127);
    else {
        for (int i = 1; i < 20; i++) strcpy(game_log[i - 1], game_log[i]);
        strncpy(game_log[19], msg, 127);
    }
}

const char *rank_to_string(Rank r) {
    static const char *names[] = {"2","3","4","5","6","7","8","9","10","J","Q","K","A"};
    return names[r];
}

const char *suit_to_string(Suit s) {
    static const char *names[] = {"♥","♦","♣","♠"};
    return names[s];
}

int rank_to_int(Rank rank) { return (int)rank; }

int compare_cards(const void *a, const void *b) {
    Card *ca = (Card *)a;
    Card *cb = (Card *)b;
    return rank_to_int(cb->rank) - rank_to_int(ca->rank);
}

// deck management
void init_deck(DeckStack *stack) {
    stack->top = 0;
    int index = 0;
    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 13; r++) {
            stack->cards[index++] = (Card){r, s};
        }
    }
}

void shuffle_deck(DeckStack *stack) {
    for (int i = 0; i < 52; i++) {
        int j = rand() % 52;
        Card temp = stack->cards[i];
        stack->cards[i] = stack->cards[j];
        stack->cards[j] = temp;
    }
    stack->top = 0;
}

Card deal_card(DeckStack *stack) {
    if (stack->top >= 52) stack->top = 0;
    return stack->cards[stack->top++];
}

// hand evaluation 
HandEvaluation evaluate_hand(Card *hand, int hand_size) {
    HandEvaluation eval = { HAND_HIGH_CARD, RANK_2 };
    for (int i = 0; i < hand_size; i++) {
        if (hand[i].rank > eval.high_card_rank)
            eval.high_card_rank = hand[i].rank;
    }
    return eval;
}

int compare_hands(HandEvaluation player_eval, HandEvaluation ai_eval) {
    if (player_eval.rank > ai_eval.rank) return 1;
    if (player_eval.rank < ai_eval.rank) return -1;
    if (player_eval.high_card_rank > ai_eval.high_card_rank) return 1;
    if (player_eval.high_card_rank < ai_eval.high_card_rank) return -1;
    return 0;
}

// ai bot 
void ai_action() {
    int choice = rand() % 3;
    switch (choice) {
        case 0:
            add_to_log("AI checks.");
            current_state = (current_state == GAME_STATE_AI_TURN_1)
                            ? GAME_STATE_PLAYER_DRAW
                            : GAME_STATE_SHOWDOWN;
            break;
        case 1:
            add_to_log("AI bets 20.");
            ai_chips -= 20; pot += 20; current_bet = 20;
            current_state = (current_state == GAME_STATE_AI_TURN_1)
                            ? GAME_STATE_BETTING_1
                            : GAME_STATE_BETTING_2;
            break;
        case 2:
            add_to_log("AI folds. You win the pot!");
            player_chips += pot;
            current_state = GAME_STATE_ROUND_END;
            break;
    }
}

void ai_draw_cards() {
    add_to_log("AI draws 1 card (placeholder).");
    ai_hand[rand() % 5] = deal_card(&deck_stack);
}

DrawStrategy get_ai_draw_potential(Card* hand) {
    DrawStrategy ds = { .score = 1, .discard_count = 0 };
    for (int i = 0; i < 5; i++) ds.discard_mask[i] = false;
    return ds;
}

// button 
void draw_button(Button button) {
    if (!button.visible) return;

    Vector2 mpos = GetMousePosition();
    bool hover = CheckCollisionPointRec(mpos, button.rect);
    float scale = 1.0f;

    if (hover && button.enabled) scale = 1.06f;
    if (!button.enabled) scale = 0.97f;

    Rectangle r = button.rect;
    float w = r.width * scale;
    float h = r.height * scale;
    float x = r.x + (r.width - w) / 2.0f;
    float y = r.y + (r.height - h) / 2.0f;

    Color base = button.enabled ? (hover ? (Color){70, 130, 180, 255} : (Color){60, 110, 160, 255})
                                : (Color){80, 80, 80, 180};
    Color outline = button.enabled ? RAYWHITE : GRAY;

    DrawRectangleRounded((Rectangle){x, y, w, h}, 0.2f, 8, base);
    DrawRectangleRoundedLines((Rectangle){x, y, w, h}, 0.2f, 8, 2.0f, outline);

    Vector2 text_size = MeasureTextEx(main_font, button.text, 20, 1);
    DrawTextEx(main_font,
               button.text,
               (Vector2){x + (w - text_size.x) / 2.0f, y + (h - text_size.y) / 2.0f + 2},
               20,
               1,
               RAYWHITE);
}


void init_buttons() {
    btn_bet20 = (Button){ .rect = {380, 500, 100, 40}, .text = "Bet 20", .enabled = true, .visible = false };
    btn_check = (Button){ .rect = {500, 500, 100, 40}, .text = "Check", .enabled = true, .visible = false };
    btn_fold  = (Button){ .rect = {620, 500, 100, 40}, .text = "Fold",  .enabled = true, .visible = false };
}

void init_round() {
    init_deck(&deck_stack);
    shuffle_deck(&deck_stack);

    for (int i = 0; i < 5; i++) {
        player_hand[i] = deal_card(&deck_stack);
        ai_hand[i] = deal_card(&deck_stack);
    }

    pot = 20;
    player_chips -= 10;
    ai_chips -= 10;

    add_to_log("New round started. Ante 10 from each player.");
    round_initialized = true;
    current_state = GAME_STATE_BETTING_1;
}

// drawing functions
void draw_hand(Card *hand, int x, int y, bool hidden) {
    for (int i = 0; i < 5; i++) {
        Rectangle card_rect = { x + i * 70, y, 60, 90 };
        DrawRectangleRounded(card_rect, 0.1f, 6, WHITE);
        DrawRectangleRoundedLines(card_rect, 0.1f, 6, 2, DARKGRAY);
        if (!hidden) {
            DrawText(rank_to_string(hand[i].rank), x + i * 70 + 8, y + 10, 20, BLACK);
            DrawText(suit_to_string(hand[i].suit), x + i * 70 + 8, y + 40, 20, (hand[i].suit < 2) ? RED : BLACK);
        } else {
            DrawRectangle(x + i * 70, y, 60, 90, BLUE);
            DrawRectangleLines(x + i * 70, y, 60, 90, GOLD);
        }
    }
}

void draw_ui() {
    DrawText(TextFormat("Player: $%.0f", player_chips), 50, 450, 20, WHITE);
    DrawText(TextFormat("AI: $%.0f", ai_chips), 50, 50, 20, WHITE);
    DrawText(TextFormat("Pot: $%.0f", pot), 400, 300, 25, YELLOW);
    DrawText("Your Hand:", 50, 400, 20, LIGHTGRAY);
    draw_hand(player_hand, 180, 390, false);
    DrawText("AI Hand:", 50, 150, 20, LIGHTGRAY);
    draw_hand(ai_hand, 180, 140, current_state != GAME_STATE_SHOWDOWN);
    draw_button(btn_bet20);
    draw_button(btn_check);
    draw_button(btn_fold);
}

// main function
int main(void) {
    InitWindow(800, 600, "Raylib Poker Game");
    SetTargetFPS(60);
    srand(time(NULL));

    main_font = GetFontDefault();
    init_buttons();

    while (!WindowShouldClose()) {
        // Input
        if (current_state == GAME_STATE_MENU && IsKeyPressed(KEY_ENTER)) {
            current_state = GAME_STATE_INIT_ROUND;
        }

        if (current_state == GAME_STATE_INIT_ROUND && !round_initialized) {
            init_round();
        }

        // Button interactions
        if (current_state == GAME_STATE_BETTING_1) {
            btn_bet20.visible = btn_check.visible = btn_fold.visible = true;

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mp = GetMousePosition();
                if (CheckCollisionPointRec(mp, btn_bet20.rect)) {
                    add_to_log("You bet 20.");
                    player_chips -= 20; pot += 20;
                    current_bet = 20;
                    current_state = GAME_STATE_AI_TURN_1;
                } else if (CheckCollisionPointRec(mp, btn_check.rect)) {
                    add_to_log("You check.");
                    current_state = GAME_STATE_AI_TURN_1;
                } else if (CheckCollisionPointRec(mp, btn_fold.rect)) {
                    add_to_log("You folded. AI wins the pot.");
                    ai_chips += pot;
                    current_state = GAME_STATE_ROUND_END;
                }
            }
        }

        if (current_state == GAME_STATE_AI_TURN_1) {
            ai_action();
        }

        if (current_state == GAME_STATE_PLAYER_DRAW) {
            add_to_log("You draw 1 card (placeholder).");
            player_hand[rand() % 5] = deal_card(&deck_stack);
            current_state = GAME_STATE_AI_DRAW;
        }

        if (current_state == GAME_STATE_AI_DRAW) {
            ai_draw_cards();
            current_state = GAME_STATE_BETTING_2;
        }

        if (current_state == GAME_STATE_AI_TURN_2) {
            ai_action();
        }

        if (current_state == GAME_STATE_SHOWDOWN) {
            HandEvaluation p_eval = evaluate_hand(player_hand, 5);
            HandEvaluation a_eval = evaluate_hand(ai_hand, 5);
            int cmp = compare_hands(p_eval, a_eval);
            if (cmp > 0) { add_to_log("You win the showdown!"); player_chips += pot; }
            else if (cmp < 0) { add_to_log("AI wins the showdown!"); ai_chips += pot; }
            else { add_to_log("It's a tie! Pot split."); player_chips += pot/2; ai_chips += pot/2; }
            current_state = GAME_STATE_ROUND_END;
        }

        // DRAW
        BeginDrawing();
        ClearBackground((Color){0, 100, 0, 255});

        if (current_state == GAME_STATE_MENU) {
            DrawText("POKER GAME", 300, 200, 40, GOLD);
            DrawText("Press ENTER to start", 290, 300, 20, WHITE);
        } else {
            draw_ui();
        }

        // Game log
        for (int i = 0; i < log_count; i++)
            DrawText(game_log[i], 50, 520 + i * 15, 14, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
