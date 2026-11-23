/* storage.h - the ONLY module that reads/writes files (implemented in Phase 4) */
#ifndef STORAGE_H
#define STORAGE_H

#include "common.h"

#define FILE_USERS         "data/users.csv"
#define FILE_CRYPTOS       "data/cryptocurrencies.csv"
#define FILE_TRANSACTIONS  "data/transactions.csv"
#define FILE_PORTFOLIO     "data/portfolio.csv"

/* Both return 1 on success, 0 on failure. Missing files are handled safely. */
int storage_load_all(AppState *app);
int storage_save_all(const AppState *app);

#endif
