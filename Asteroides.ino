#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define PIN_UP 10
#define PIN_LEFT 12
#define PIN_RIGHT 13
#define PIN_FIRE 1
#define PIN_BUZZER 4
#define VERDE 0x07E0
#define PRETO 0x0000

Adafruit_ST7735 tft = Adafruit_ST7735(5, 7, 6);

struct Objeto {
  float x, y, vx, vy, angulo;
  bool ativo;
  int tamanho;
};

Objeto nave;
Objeto tiros[12]; // Aumentado para o tiro duplo
Objeto asteroides[15];

int score = 0, vidas = 3, gameState = 0, rodada = 1;
bool temNaveSombra = false;

// --- SONS ---
void somLaser() { tone(PIN_BUZZER, 800, 5); delay(5); tone(PIN_BUZZER, 600, 5); }
void somExplosao() { tone(PIN_BUZZER, 150, 40); }

void musicaFunebre() {
  int notas[] = {196, 196, 196, 155, 196, 155, 130}; 
  int dura[] = {400, 400, 400, 600, 200, 200, 800};
  for (int i = 0; i < 7; i++) {
    tone(PIN_BUZZER, notas[i], dura[i]);
    delay(dura[i] + 50);
  }
  noTone(PIN_BUZZER);
}

// --- DESENHOS ---
void desenharNaveESombra(uint16_t cor) {
  // Nave Principal
  int x1 = nave.x + cos(nave.angulo)*7;    int y1 = nave.y + sin(nave.angulo)*7;
  int x2 = nave.x + cos(nave.angulo+2.4)*6; int y2 = nave.y + sin(nave.angulo+2.4)*6;
  int x3 = nave.x + cos(nave.angulo-2.4)*6; int y3 = nave.y + sin(nave.angulo-2.4)*6;
  tft.drawTriangle(x1, y1, x2, y2, x3, y3, cor);

  if(temNaveSombra) {
    // Nave Sombra posicionada AO LADO (90 graus em relação ao bico)
    float ox = nave.x + cos(nave.angulo + 1.57) * 12; 
    float oy = nave.y + sin(nave.angulo + 1.57) * 12;
    int sx1 = ox + cos(nave.angulo)*5;    int sy1 = oy + sin(nave.angulo)*5;
    int sx2 = ox + cos(nave.angulo+2.4)*4; int sy2 = oy + sin(nave.angulo+2.4)*4;
    int sx3 = ox + cos(nave.angulo-2.4)*4; int sy3 = oy + sin(nave.angulo-2.4)*4;
    tft.drawTriangle(sx1, sy1, sx2, sy2, sx3, sy3, cor);
  }
}

void desenharAsteroide(Objeto &ast, uint16_t cor) {
  int r = ast.tamanho;
  for(int i=0; i<6; i++) {
    float a1 = i*(PI*2/6); float a2 = (i+1)*(PI*2/6);
    tft.drawLine(ast.x+cos(a1)*r, ast.y+sin(a1)*r, ast.x+cos(a2)*r, ast.y+sin(a2)*r, cor);
  }
}

void iniciarFase() {
  tft.fillScreen(PRETO);
  nave.x = 80; nave.y = 64; nave.vx = 0; nave.vy = 0;
  int qtd = 2 + rodada;
  if(qtd > 8) qtd = 8;
  for(int i=0; i<15; i++) asteroides[i].ativo = false;
  for(int i=0; i<qtd; i++) {
    asteroides[i] = {(float)random(160), (float)random(128), (float)random(-10,10)/10.0, (float)random(-10,10)/10.0, 0, true, 12};
  }
}

void telaInicial() {
  tft.fillScreen(PRETO);
  tft.setTextColor(VERDE);
  tft.setTextSize(2);
  tft.setCursor(25, 40); tft.print("ASTEROIDS");
  tft.setTextSize(1);
  tft.setCursor(45, 70); tft.print("S3-MINI PRO");
  while(digitalRead(PIN_FIRE) == HIGH) {
    tft.setCursor(25, 100);
    if((millis()/500)%2 == 0) tft.print("PRESS FIRE TO START");
    else tft.fillRect(25, 100, 115, 8, PRETO);
    delay(50);
  }
  score = 0; vidas = 3; rodada = 1; temNaveSombra = false;
  iniciarFase();
  gameState = 1;
}

void setup() {
  pinMode(PIN_UP, INPUT_PULLUP); pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP); pinMode(PIN_FIRE, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  SPI.begin(2, -1, 3, 5); tft.initR(INITR_BLACKTAB); tft.setRotation(3);
  telaInicial();
}

void loop() {
  if (gameState == 1) {
    desenharNaveESombra(PRETO);
    for(int i=0; i<12; i++) if(tiros[i].ativo) tft.drawPixel(tiros[i].x, tiros[i].y, PRETO);
    for(int i=0; i<15; i++) if(asteroides[i].ativo) desenharAsteroide(asteroides[i], PRETO);

    if (digitalRead(PIN_LEFT) == LOW)  nave.angulo -= 0.15;
    if (digitalRead(PIN_RIGHT) == LOW) nave.angulo += 0.15;
    if (digitalRead(PIN_UP) == LOW) {
      nave.vx += cos(nave.angulo) * 0.15;
      nave.vy += sin(nave.angulo) * 0.15;
    }
    if (digitalRead(PIN_FIRE) == LOW) {
      int tirosCriados = 0;
      for(int i=0; i<12 && tirosCriados < (temNaveSombra ? 2 : 1); i++) {
        if(!tiros[i].ativo) {
          if(tirosCriados == 0) { // Tiro da nave principal
            tiros[i] = {nave.x, nave.y, cos(nave.angulo)*4, sin(nave.angulo)*4, 0, true, 0};
          } else { // Tiro da nave sombra lateral
            float ox = nave.x + cos(nave.angulo + 1.57) * 12;
            float oy = nave.y + sin(nave.angulo + 1.57) * 12;
            tiros[i] = {ox, oy, cos(nave.angulo)*4, sin(nave.angulo)*4, 0, true, 0};
          }
          tirosCriados++;
        }
      }
      somLaser(); delay(150);
    }

    nave.x += nave.vx; nave.y += nave.vy;
    nave.vx *= 0.98; nave.vy *= 0.98;
    if(nave.x > 160) nave.x=0; if(nave.x < 0) nave.x=160;
    if(nave.y > 128) nave.y=0; if(nave.y < 0) nave.y=128;

    bool algumAsteroide = false;
    for(int i=0; i<15; i++) {
      if(!asteroides[i].ativo) continue;
      algumAsteroide = true;
      asteroides[i].x += asteroides[i].vx; asteroides[i].y += asteroides[i].vy;
      if(asteroides[i].x > 160) asteroides[i].x=0; if(asteroides[i].x < 0) asteroides[i].x=160;
      if(asteroides[i].y > 128) asteroides[i].y=0; if(asteroides[i].y < 0) asteroides[i].y=128;
      
      float d = sqrt(pow(nave.x-asteroides[i].x,2)+pow(nave.y-asteroides[i].y,2));
      if(d < asteroides[i].tamanho + 3) {
        vidas--; somExplosao(); 
        temNaveSombra = false; // PERDE A NAVE SECUNDÁRIA AO MORRER
        delay(500);
        if(vidas <= 0) { gameState = 2; return; }
        iniciarFase();
      }

      for(int t=0; t<12; t++) {
        if(tiros[t].ativo) {
          float dTiro = sqrt(pow(tiros[t].x-asteroides[i].x,2)+pow(tiros[t].y-asteroides[i].y,2));
          if(dTiro < asteroides[i].tamanho + 2) {
             tiros[t].ativo = false;
             int tOld = asteroides[i].tamanho;
             asteroides[i].ativo = false;
             if(tOld == 12) {
               score += 50;
               int c=0; for(int k=0; k<15 && c<2; k++) if(!asteroides[k].ativo) { 
                 asteroides[k] = {asteroides[i].x, asteroides[i].y, (float)random(-15,15)/10.0, (float)random(-15,15)/10.0, 0, true, 6}; c++; 
               }
             } else { score += 150; }
          }
        }
      }
    }

    if(!algumAsteroide) {
      rodada++; temNaveSombra = true; // RECONQUISTA NA PRÓXIMA FASE
      tft.setCursor(40, 60); tft.print("LEVEL CLEAR!"); delay(1500);
      iniciarFase();
    }

    desenharNaveESombra(VERDE);
    for(int i=0; i<15; i++) if(asteroides[i].ativo) desenharAsteroide(asteroides[i], VERDE);
    for(int i=0; i<12; i++) {
      if(tiros[i].ativo) {
        tiros[i].x += tiros[i].vx; tiros[i].y += tiros[i].vy;
        if(tiros[i].x<0 || tiros[i].x>160 || tiros[i].y<0 || tiros[i].y>128) tiros[i].ativo = false;
        else tft.drawPixel(tiros[i].x, tiros[i].y, VERDE);
      }
    }
    tft.setCursor(2, 2); tft.print(score);
    for(int i=0; i<vidas; i++) {
       int vx = 140+(i*6); tft.drawTriangle(vx, 5, vx-3, 11, vx+3, 11, VERDE);
    }
    delay(10);
  } else if (gameState == 2) {
    tft.fillScreen(PRETO);
    tft.setCursor(35, 50); tft.setTextSize(2); tft.print("GAME OVER");
    musicaFunebre(); delay(1000); telaInicial();
  }
}