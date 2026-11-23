/* user.c - registration, login, logout, password hashing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include "user.h"
#include "utils.h"

#define MIN_USERNAME_LEN  3
#define MIN_PASSWORD_LEN  6
#define MAX_LOGIN_TRIES   3
#define HASH_ROUNDS       2000

/* ---------- password hashing ----------
   Hand-written salted hash (FNV-1a, 4 lanes x 2000 rounds -> 256 bits).
   It keeps plaintext passwords off disk, but it is NOT production-grade
   cryptography (real systems use bcrypt/argon2). Fine for this project. */

static uint64_t fnv_bytes(uint64_t h, const char *s)
{
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 0x100000001b3ULL;
    }
    return h;
}

static uint64_t fnv_u64(uint64_t h, uint64_t v)
{
    for (int i = 0; i < 8; i++) {
        h ^= (v >> (8 * i)) & 0xff;
        h *= 0x100000001b3ULL;
    }
    return h;
}

void user_hash_password(const char *password, const char *salt, char *out_hash)
{
    uint64_t lane[4] = { 0xcbf29ce484222325ULL, 0x9e3779b97f4a7c15ULL,
                         0xc2b2ae3d27d4eb4fULL, 0x165667b19e3779f9ULL };

    for (int round = 0; round < HASH_ROUNDS; round++) {
        for (int i = 0; i < 4; i++) {
            uint64_t h = lane[i] ^ (uint64_t)(round * 4 + i);
            h = fnv_bytes(h, salt);
            h = fnv_bytes(h, password);
            h = fnv_u64(h, lane[(i + 1) % 4]);   /* mix in neighbouring lane */
            lane[i] = h;
        }
    }

    snprintf(out_hash, HASH_LEN, "%016llx%016llx%016llx%016llx",
             (unsigned long long)lane[0], (unsigned long long)lane[1],
             (unsigned long long)lane[2], (unsigned long long)lane[3]);
}

/* Random 16-character salt so equal passwords get different hashes. */
static void generate_salt(char *salt, size_t size)
{
    static const char ALPHABET[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static int seeded = 0;

    if (!seeded) {
        srand((unsigned)time(NULL) ^ (unsigned)clock());
        seeded = 1;
    }
    for (size_t i = 0; i + 1 < size; i++) {
        salt[i] = ALPHABET[rand() % (int)(sizeof ALPHABET - 1)];
    }
    salt[size - 1] = '\0';
}

/* Wipes a password buffer (volatile stops the compiler removing the loop). */
static void secure_zero(char *buf, size_t n)
{
    volatile char *p = buf;
    while (n--) *p++ = 0;
}

/* ---------- helpers ---------- */

static int is_valid_username(const char *name)
{
    size_t len = strlen(name);

    if (len < MIN_USERNAME_LEN || len > USERNAME_LEN - 1) return 0;
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_') return 0;
    }
    return 1;
}

static int find_user_index(const AppState *app, const char *username)
{
    for (int i = 0; i < app->user_count; i++) {
        if (str_equals_ignore_case(app->users[i].username, username)) return i;
    }
    return -1;
}

/* ---------- public functions ---------- */

int user_register(AppState *app)
{
    char  username[64], password[64], confirm[64];
    User *u;

    if (app->user_count >= MAX_USERS) {
        printf("Maximum number of users reached.\n");
        return 0;
    }

    printf("\n--- Register ---\n");
    printf("Username (%d-%d letters, digits or _; blank to cancel): ",
           MIN_USERNAME_LEN, USERNAME_LEN - 1);
    read_line(username, sizeof username);
    str_trim(username);

    if (username[0] == '\0') {
        printf("Cancelled.\n");
        return 0;
    }
    if (!is_valid_username(username)) {
        printf("Invalid username. Use %d-%d letters, digits or underscore only.\n",
               MIN_USERNAME_LEN, USERNAME_LEN - 1);
        return 0;
    }
    if (find_user_index(app, username) >= 0) {
        printf("That username is already taken.\n");
        return 0;
    }

    read_password("Password (min 6 characters): ", password, sizeof password);
    if (strlen(password) < MIN_PASSWORD_LEN) {
        printf("Password too short (minimum %d characters).\n", MIN_PASSWORD_LEN);
        secure_zero(password, sizeof password);
        return 0;
    }

    read_password("Confirm password: ", confirm, sizeof confirm);
    if (strcmp(password, confirm) != 0) {
        printf("Passwords do not match.\n");
        secure_zero(password, sizeof password);
        secure_zero(confirm, sizeof confirm);
        return 0;
    }

    u = &app->users[app->user_count];
    u->user_id = app->next_user_id;
    snprintf(u->username, sizeof u->username, "%.*s", USERNAME_LEN - 1, username);  /* length already validated */
    generate_salt(u->salt, sizeof u->salt);
    user_hash_password(password, u->salt, u->password_hash);

    app->user_count++;
    app->next_user_id++;

    secure_zero(password, sizeof password);
    secure_zero(confirm, sizeof confirm);

    printf("Registration successful! Your user ID is %d. You can log in now.\n", u->user_id);
    return 1;
}

int user_login(AppState *app)
{
    char username[64], password[64], hash[HASH_LEN];
    int  idx;

    if (app->user_count == 0) {
        printf("\nNo users registered yet. Please register first.\n");
        return 0;
    }

    printf("\n--- Login ---\n");

    for (int attempt = 1; attempt <= MAX_LOGIN_TRIES; attempt++) {
        printf("Username (blank to cancel): ");
        read_line(username, sizeof username);
        str_trim(username);
        if (username[0] == '\0') {
            printf("Cancelled.\n");
            return 0;
        }

        read_password("Password: ", password, sizeof password);

        idx = find_user_index(app, username);
        if (idx >= 0) {
            user_hash_password(password, app->users[idx].salt, hash);
            if (strcmp(hash, app->users[idx].password_hash) == 0) {
                secure_zero(password, sizeof password);
                app->current_user_id = app->users[idx].user_id;
                printf("Welcome, %s!\n", app->users[idx].username);
                return 1;
            }
        }
        secure_zero(password, sizeof password);

        /* same message for unknown user and wrong password */
        printf("Invalid username or password.");
        if (attempt < MAX_LOGIN_TRIES) {
            printf(" (%d attempt(s) left)", MAX_LOGIN_TRIES - attempt);
        }
        printf("\n");
    }

    printf("Too many failed attempts. Returning to main menu.\n");
    return 0;
}

void user_logout(AppState *app)
{
    app->current_user_id = 0;
}

int user_require_login(const AppState *app)
{
    if (app->current_user_id == 0) {
        printf("You must be logged in to do that.\n");
        return 0;
    }
    return 1;
}

const char *user_current_name(const AppState *app)
{
    for (int i = 0; i < app->user_count; i++) {
        if (app->users[i].user_id == app->current_user_id) {
            return app->users[i].username;
        }
    }
    return "";
}
