/* calculator.h - pure profit/loss math, no input or printing (Phase 3) */
#ifndef CALCULATOR_H
#define CALCULATOR_H

double calc_current_value(double quantity, double current_price);
double calc_avg_price(double cost_basis, double quantity);
double calc_unrealized_pl(double current_value, double cost_basis);
double calc_realized_pl_on_sell(double quantity, double sell_price, double avg_price);
double calc_return_pct(double profit_loss, double cost_basis);

#endif
