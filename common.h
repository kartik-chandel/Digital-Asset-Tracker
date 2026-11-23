/* common.h - shared constants and data structures for the whole project */
#ifndef COMMON_H
#define COMMON_H

/* ---------- Capacity limits (fixed-size arrays keep memory handling simple) ---------- */
#define MAX_USERS         100
#define MAX_CRYPTOS       50
#define MAX_HOLDINGS      500
#define MAX_TRANSACTIONS  1000

/* ---------- String sizes (include space for the '\0') ---------- */
#define USERNAME_LEN  32
#define HASH_LEN      65
#define SALT_LEN      17
#define NAME_LEN      32
#define SYMBOL_LEN    10
#define API_ID_LEN    32
#define DATE_LEN      20   /* "YYYY-MM-DD HH:MM:SS" */

/* ---------- Transaction types ---------- */
#define TXN_BUY  'B'
#define TXN_SELL 'S'

/* A registered user. Password is never stored in plaintext. */
typedef struct {
    int  user_id;
    char username[USERNAME_LEN];
    char password_hash[HASH_LEN];
    char salt[SALT_LEN];
} User;

/* One cryptocurrency in the market database. Prices are in INR. */
typedef struct {
    int    asset_id;
    char   name[NAME_LEN];
    char   symbol[SYMBOL_LEN];
    char   api_id[API_ID_LEN];   /* CoinGecko id, used in Phase 5 */
    double price_inr;
    double change_24h;           /* percent */
} Crypto;

/* One coin owned by one user (weighted-average cost accounting). */
typedef struct {
    int    user_id;
    char   symbol[SYMBOL_LEN];
    double quantity;
    double avg_price;            /* weighted average cost per coin */
    double cost_basis;           /* remaining cost = quantity * avg_price */
    double realized_pl;          /* profit/loss locked in by sells */
} Holding;

/* One BUY or SELL record. */
typedef struct {
    int    transaction_id;
    int    user_id;
    char   symbol[SYMBOL_LEN];
    char   type;                 /* TXN_BUY or TXN_SELL */
    double quantity;
    double price;                /* price per coin */
    double total_value;          /* quantity * price */
    char   date[DATE_LEN];
} Transaction;

/* Everything the program keeps in memory. Passed to modules by pointer
   so we do not need global variables. */
typedef struct {
    User        users[MAX_USERS];
    int         user_count;

    Crypto      cryptos[MAX_CRYPTOS];
    int         crypto_count;

    Holding     holdings[MAX_HOLDINGS];
    int         holding_count;

    Transaction transactions[MAX_TRANSACTIONS];
    int         transaction_count;

    int         current_user_id;      /* 0 = nobody logged in */
    int         next_user_id;
    int         next_transaction_id;
} AppState;

#endif
