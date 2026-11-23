/* user.h - registration, login, logout, password hashing */
#ifndef USER_H
#define USER_H

#include "common.h"

int  user_register(AppState *app);   /* returns 1 on success */
int  user_login(AppState *app);      /* returns 1 on success, sets current_user_id */
void user_logout(AppState *app);

/* Prints a message and returns 0 if nobody is logged in. */
int  user_require_login(const AppState *app);

/* Username of the logged-in user ("" if nobody). */
const char *user_current_name(const AppState *app);

/* Salted hash of password, written into out_hash (HASH_LEN bytes). */
void user_hash_password(const char *password, const char *salt, char *out_hash);

#endif
