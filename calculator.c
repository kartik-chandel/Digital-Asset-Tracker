/* calculator.c - pure profit/loss math (no input, no printing) */
#include "calculator.h"

double calc_current_value(double quantity, double current_price)
{
    return quantity * current_price;
}

double calc_avg_price(double cost_basis, double quantity)
{
    if (quantity <= 0.0) return 0.0;        /* avoid divide-by-zero */
    return cost_basis / quantity;
}

double calc_unrealized_pl(double current_value, double cost_basis)
{
    return current_value - cost_basis;
}

double calc_realized_pl_on_sell(double quantity, double sell_price, double avg_price)
{
    return quantity * (sell_price - avg_price);
}

double calc_return_pct(double profit_loss, double cost_basis)
{
    if (cost_basis <= 0.0) return 0.0;
    return (profit_loss / cost_basis) * 100.0;
}
