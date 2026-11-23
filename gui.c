/* Digital Asset Tracker - native C GUI using raylib */
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define COINS 8
#define TXMAX 500
#define W 1100
#define H 720

typedef struct { const char *name,*symbol; double price; double qty, cost; } Coin;
typedef struct { char type[8]; int coin; double qty, price; char time[32]; } Tx;
static Coin coins[COINS]={
 {"Bitcoin","BTC",5700000,0,0},{"Ethereum","ETH",205000,0,0},
 {"Tether","USDT",83.2,0,0},{"BNB","BNB",51000,0,0},
 {"Solana","SOL",14500,0,0},{"XRP","XRP",205,0,0},
 {"Cardano","ADA",58,0,0},{"Dogecoin","DOGE",18.5,0,0}};
static Tx txs[TXMAX]; static int ntx=0, page=0, selected=0, mode=0;
static char qtyText[64]="1", status[160]="Welcome to Digital Asset Tracker";
static int qtyLen=1; static Color bg={12,18,32,255}, panel={22,31,50,255}, muted={151,166,190,255}, accent={67,139,255,255}, green={53,198,137,255}, red={245,100,110,255};
static double total_value(void){double v=0;for(int i=0;i<COINS;i++)v+=coins[i].qty*coins[i].price;return v;}
static double total_cost(void){double v=0;for(int i=0;i<COINS;i++)v+=coins[i].cost;return v;}
static void save_data(void){FILE*f=fopen("tracker_data.csv","w");if(!f)return;for(int i=0;i<COINS;i++)fprintf(f,"C,%d,%.12f,%.12f\n",i,coins[i].qty,coins[i].cost);for(int i=0;i<ntx;i++)fprintf(f,"T,%s,%d,%.12f,%.12f,%s\n",txs[i].type,txs[i].coin,txs[i].qty,txs[i].price,txs[i].time);fclose(f);}
static void load_data(void){FILE*f=fopen("tracker_data.csv","r");if(!f)return;char line[256];while(fgets(line,sizeof line,f)){if(line[0]=='C'){int i;double q,c;if(sscanf(line,"C,%d,%lf,%lf",&i,&q,&c)==3&&i>=0&&i<COINS){coins[i].qty=q;coins[i].cost=c;}}else if(line[0]=='T'&&ntx<TXMAX){Tx*t=&txs[ntx];if(sscanf(line,"T,%7[^,],%d,%lf,%lf,%31s",t->type,&t->coin,&t->qty,&t->price,t->time)==5)ntx++;}}fclose(f);}
static void label(const char*s,int x,int y,int size,Color c){DrawText(s,x,y,size,c);}
static void button(const char*s,Rectangle r,int active){Color c=active?accent:panel;DrawRectangleRounded(r,0.18f,8,c);DrawText(s,(int)r.x+16,(int)r.y+12,17,RAYWHITE);}
static int hit(Rectangle r){return CheckCollisionPointRec(GetMousePosition(),r)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);}
static void card(const char*t,const char*v,int x,int y,int w){DrawRectangleRounded((Rectangle){x,y,w,100},0.12f,8,panel);label(t,x+18,y+15,15,muted);label(v,x+18,y+47,25,RAYWHITE);}
static void add_trade(int sell){double q=strtod(qtyText,NULL);if(q<=0||!isfinite(q)){strcpy(status,"Enter a valid quantity greater than zero.");return;}Coin*c=&coins[selected];if(sell&&q>c->qty+1e-10){strcpy(status,"Not enough coins in your portfolio.");return;}if(sell){double avg=c->qty>0?c->cost/c->qty:0;c->qty-=q;c->cost-=avg*q;if(c->qty<1e-10){c->qty=0;c->cost=0;}}else{c->cost+=q*c->price;c->qty+=q;}if(ntx<TXMAX){Tx*t=&txs[ntx++];strcpy(t->type,sell?"SELL":"BUY");t->coin=selected;t->qty=q;t->price=c->price;time_t now=time(NULL);struct tm *tmv=localtime(&now);if(tmv)strftime(t->time,sizeof t->time,"%Y-%m-%d",tmv);else strcpy(t->time,"unknown");}save_data();strcpy(status,sell?"Sell transaction recorded.":"Buy transaction recorded.");qtyText[0]='1';qtyText[1]='\0';qtyLen=1;page=1;}
int main(void){SetConfigFlags(FLAG_MSAA_4X_HINT);InitWindow(W,H,"Digital Asset Tracker | C Edition");SetTargetFPS(60);load_data();while(!WindowShouldClose()){
 if(page==4||page==5){int ch=GetCharPressed();while(ch>0){if((ch>='0'&&ch<='9')||ch=='.'||ch=='-'){if(qtyLen<60){qtyText[qtyLen++]=(char)ch;qtyText[qtyLen]='\0';}}ch=GetCharPressed();}if(IsKeyPressed(KEY_BACKSPACE)&&qtyLen>0)qtyText[--qtyLen]='\0';if(IsKeyPressed(KEY_ESCAPE))page=1;}
 BeginDrawing();ClearBackground(bg);
 DrawRectangle(0,0,230,H,(Color){16,24,41,255});label("DAT / C",26,25,28,RAYWHITE);label("DIGITAL ASSET TRACKER",26,62,10,muted);
 const char*nav[]={"Overview","Portfolio","Market","Transactions"};for(int i=0;i<4;i++){Rectangle r={16,125+i*55,198,43};if(hit(r))page=i;button(nav[i],r,page==i);}
 label("LOCAL PORTFOLIO  •  INR",26,665,11,muted);
 label("Dashboard",265,28,27,RAYWHITE);label("Track your digital assets in one place",265,64,14,muted);
 if(page==0){char a[64],b[64],c[64];snprintf(a,sizeof a,"₹ %.2f",total_value());snprintf(b,sizeof b,"₹ %.2f",total_cost());snprintf(c,sizeof c,"₹ %.2f",total_value()-total_cost());card("PORTFOLIO VALUE",a,265,110,245);card("TOTAL INVESTED",b,530,110,245);card("UNREALIZED P/L",c,795,110,245);label("Your holdings",265,250,21,RAYWHITE);label("ASSET",280,294,12,muted);label("HOLDINGS",560,294,12,muted);label("PRICE (INR)",735,294,12,muted);label("VALUE",900,294,12,muted);for(int i=0;i<COINS;i++){int y=325+i*40;DrawLine(270,y+30,1035,y+30,(Color){42,53,73,255});label(coins[i].name,280,y,15,RAYWHITE);label(coins[i].symbol,420,y,13,muted);label(TextFormat("%.6f",coins[i].qty),560,y,14,RAYWHITE);label(TextFormat("₹ %.2f",coins[i].price),735,y,14,RAYWHITE);label(TextFormat("₹ %.2f",coins[i].qty*coins[i].price),900,y,14,RAYWHITE);}}
 else if(page==1){label("Portfolio holdings",265,112,21,RAYWHITE);label("ASSET",280,165,12,muted);label("QUANTITY",520,165,12,muted);label("AVG. COST",690,165,12,muted);label("CURRENT VALUE",870,165,12,muted);for(int i=0;i<COINS;i++){int y=195+i*52;label(TextFormat("%s (%s)",coins[i].name,coins[i].symbol),280,y,16,RAYWHITE);label(TextFormat("%.8f",coins[i].qty),520,y,15,RAYWHITE);label(TextFormat("₹ %.2f",coins[i].qty>0?coins[i].cost/coins[i].qty:0),690,y,15,muted);label(TextFormat("₹ %.2f",coins[i].qty*coins[i].price),870,y,15,RAYWHITE);}}
 else if(page==2){label("Cryptocurrency market",265,112,21,RAYWHITE);label("ASSET",280,165,12,muted);label("SYMBOL",550,165,12,muted);label("PRICE (INR)",740,165,12,muted);for(int i=0;i<COINS;i++){int y=200+i*48;label(coins[i].name,280,y,16,RAYWHITE);label(coins[i].symbol,550,y,15,muted);label(TextFormat("₹ %.2f",coins[i].price),740,y,15,RAYWHITE);}}
 else if(page==4||page==5){int buy=page==4;label(buy?"Buy cryptocurrency":"Sell cryptocurrency",265,115,22,RAYWHITE);label("Select asset",265,165,14,muted);for(int i=0;i<COINS;i++){Rectangle r={265+(i%4)*190,200+(i/4)*62,175,48};if(hit(r)){selected=i;}button(TextFormat("%s  (%s)",coins[i].name,coins[i].symbol),r,selected==i);}label(TextFormat("Selected: %s  |  Price: ₹ %.2f",coins[selected].name,coins[selected].price),265,345,17,RAYWHITE);label("Quantity",265,390,14,muted);DrawRectangleRounded((Rectangle){265,420,280,48},0.12f,6,panel);label(qtyText,282,433,20,RAYWHITE);label("Click the field and type; Backspace edits",265,477,12,muted);button(buy?"Confirm buy":"Confirm sell",(Rectangle){265,525,180,48},1);if(hit((Rectangle){265,525,180,48}))add_trade(!buy);button("Cancel",(Rectangle){460,525,120,48},0);if(hit((Rectangle){460,525,120,48}))page=1;}
 else if(page==3){label("Transaction history",265,112,21,RAYWHITE);label("TYPE",280,165,12,muted);label("ASSET",410,165,12,muted);label("QUANTITY",610,165,12,muted);label("PRICE",770,165,12,muted);label("DATE",930,165,12,muted);int start=ntx>9?ntx-9:0;for(int j=start;j<ntx;j++){int y=200+(j-start)*45;Tx*t=&txs[j];label(t->type,280,y,14,!strcmp(t->type,"BUY")?green:red);label(coins[t->coin].symbol,410,y,14,RAYWHITE);label(TextFormat("%.6f",t->qty),610,y,14,RAYWHITE);label(TextFormat("₹ %.2f",t->price),770,y,14,RAYWHITE);label(t->time,930,y,14,muted);}}
 label(status,265,675,13,muted);
 if(page==0||page==1||page==2){button("Buy",(Rectangle){820,28,90,40},1);if(hit((Rectangle){820,28,90,40}))page=4;button("Sell",(Rectangle){920,28,90,40},0);if(hit((Rectangle){920,28,90,40}))page=5;}
 EndDrawing();}
 save_data();CloseWindow();return 0;}
