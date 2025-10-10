#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include <raylib.h>
#include <raymath.h>

#ifndef _WIN32
#include <err.h>
#else
#define errx(n, ...) do { fprintf(stderr, __VA_ARGS__); exit(n); } while (0);
#define err(...) errx(__VA_ARGS__)
#endif


#define BU  ((uint8_t)(1))
#define BR  ((uint8_t)(1<<1))
#define BD  ((uint8_t)(1<<2))
#define BL  ((uint8_t)(1<<3))
#define BUR ((uint8_t)(1<<4))
#define BDR ((uint8_t)(1<<5))
#define BUL ((uint8_t)(1<<6))
#define BDL ((uint8_t)(1<<7))

#define W 9   /* actually 8  */
#define H 13  /* actually 12 */
#define PWIDTH  64
#define PHEIGHT 64

#define AT(b, x, y) (b->pt[((y)*W)+(x)])

typedef enum {
  PL_VS_BOT        = 0,
  BOT_VS_PL        = 1,
  BOT_VS_BOT       = 2,
  BOT_VS_BOT_SLOW  = 3
} Mode;

typedef enum {
  WIN_P1 = 0, /* same as b->plr */
  WIN_P2 = 1, /* there's no such thing as a draw */
  NONE   = 2,
} Result;

typedef struct {
  uint8_t mask, borderp;
} Point;

typedef struct {
  Point pt[W*H];
  Result res;
  uint8_t x, y; /* player (x y) */
  uint8_t plr; /* 0 | 1 */
} Board;

static char   *argv0    = NULL;
static uint8_t iflag    = 0;
static int     ABSDEPTH = 64;
static int     DEPTH    =  2;

static Color color_bg     = (Color){0x22, 0x22, 0x22, 0xff};
static Color color_fg     = (Color){0xde, 0xde, 0xde, 0xff};
static Color color_fg_dim = (Color){0x66, 0x66, 0x66, 0xbb};
static Color color_p1     = (Color){0xee, 0x88, 0x88, 0xff};
static Color color_p2     = (Color){0x88, 0xee, 0x88, 0xff};

static uint8_t wins[8][2] = {
  {3,  0}, {4,  0}, {5,  0},
  {3, 12}, {4, 12}, {5, 12},
};

void
init_board(Board *b)
{
  int i;
  b->x = 4, b->y = 6;
  b->res = NONE;
  memset(b->pt, 0, sizeof(*b->pt)*W*H);
  AT(b, 0, 1).borderp |= BR|BD;
  AT(b, 1, 1).borderp |= BR|BL;
  AT(b, 2, 1).borderp |= BR|BL;
  AT(b, 3, 1).borderp |= BU|BL;
  AT(b, 3, 0).borderp |= BD|BR;
  AT(b, 4, 0).borderp |= BL|BR;
  AT(b, 5, 0).borderp |= BL|BD;
  AT(b, 5, 1).borderp |= BU|BR;
  AT(b, 6, 1).borderp |= BL|BR;
  AT(b, 7, 1).borderp |= BL|BR;
  AT(b, 8, 1).borderp |= BL|BD;

  AT(b, 0, 11).borderp |= BR|BU;
  AT(b, 1, 11).borderp |= BR|BL;
  AT(b, 2, 11).borderp |= BR|BL;
  AT(b, 3, 11).borderp |= BD|BL;
  AT(b, 3, 12).borderp |= BU|BR;
  AT(b, 4, 12).borderp |= BL|BR;
  AT(b, 5, 12).borderp |= BL|BU;
  AT(b, 5, 11).borderp |= BD|BR;
  AT(b, 6, 11).borderp |= BL|BR;
  AT(b, 7, 11).borderp |= BL|BR;
  AT(b, 8, 11).borderp |= BL|BU;

  AT(b, 0, 1).mask = (uint8_t)~(BDR);
  AT(b, 1, 1).mask = (uint8_t)~(BDL|BD|BDR);
  AT(b, 2, 1).mask = (uint8_t)~(BDL|BD|BDR);
  AT(b, 3, 1).mask = (uint8_t)~(BDL|BD|BDR|BR|BUR);
  AT(b, 3, 0).mask = (uint8_t)~(BDR);
  AT(b, 4, 0).mask = (uint8_t)~(BDL|BD|BDR);
  AT(b, 5, 0).mask = (uint8_t)~(BDL);
  AT(b, 5, 1).mask = (uint8_t)~(BDL|BD|BDR|BL|BUL);
  AT(b, 6, 1).mask = (uint8_t)~(BDL|BD|BDR);
  AT(b, 7, 1).mask = (uint8_t)~(BDL|BD|BDR);
  AT(b, 8, 1).mask = (uint8_t)~(BDL);

  AT(b, 0, 11).mask = (uint8_t)~(BUR);
  AT(b, 1, 11).mask = (uint8_t)~(BUL|BU|BUR);
  AT(b, 2, 11).mask = (uint8_t)~(BUL|BU|BUR);
  AT(b, 3, 11).mask = (uint8_t)~(BUL|BU|BUR|BR|BDR);
  AT(b, 3, 12).mask = (uint8_t)~(BUR);
  AT(b, 4, 12).mask = (uint8_t)~(BUL|BU|BUR);
  AT(b, 5, 12).mask = (uint8_t)~(BUL);
  AT(b, 5, 11).mask = (uint8_t)~(BUL|BU|BUR|BL|BDL);
  AT(b, 6, 11).mask = (uint8_t)~(BUL|BU|BUR);
  AT(b, 7, 11).mask = (uint8_t)~(BUL|BU|BUR);
  AT(b, 8, 11).mask = (uint8_t)~(BUL);

  for (i = 2; i <= 10; ++i) {
    AT(b, 0, i).borderp |= BU|BD;
    AT(b, 0, i).mask = (uint8_t)~(BUR|BR|BDR);
    AT(b, 8, i).borderp |= BU|BD;
    AT(b, 8, i).mask = (uint8_t)~(BUL|BL|BDL);
  }
}

#define BORDER(fromx, fromy, tox, toy)                          \
  DrawLineEx((Vector2){(fromx)*PWIDTH, (fromy)*PHEIGHT},        \
             (Vector2){(tox)*PWIDTH,   (toy)*PHEIGHT}, 5.f, VIOLET)

#define HIST(fromx, fromy, tox, toy)                            \
  DrawLineEx((Vector2){(fromx)*PWIDTH, (fromy)*PHEIGHT},        \
             (Vector2){(tox)*PWIDTH,   (toy)*PHEIGHT}, 3.f, color_fg)

#define PROPOSE(fromx, fromy, tox, toy)                         \
  DrawLineEx((Vector2){(fromx)*PWIDTH, (fromy)*PHEIGHT},        \
             (Vector2){(tox)*PWIDTH,   (toy)*PHEIGHT}, 3.f, color_fg_dim)

__attribute__((unused)) static void
draw_pt_border(int a, int b, int c, int d)
{
  BORDER(a, b, c, d);
}

__attribute__((unused)) static void
draw_pt_hist(int a, int b, int c, int d)
{
  HIST(a, b, c, d);
}

static void
draw_pt_propose(int a, int b, int c, int d)
{
  PROPOSE(a, b, c, d);
}

static void
mask_to_point(uint8_t mask, uint8_t *x, uint8_t *y)
{
  if (mask & BR)  ++*x;
  if (mask & BL)  --*x;
  if (mask & BD)  ++*y;
  if (mask & BU)  --*y;
  if (mask & BUR) ++*x, --*y;
  if (mask & BUL) --*x, --*y;
  if (mask & BDR) ++*x, ++*y;
  if (mask & BDL) --*x, ++*y;
}

/* This does not work if a mask covers more than 1 point */
static void
draw_point_apply(void (*f)(int, int, int, int), uint8_t mask, uint8_t x, uint8_t y)
{
  uint8_t newx = x, newy = y;
  mask_to_point(mask, &newx, &newy);
  if (newx != x || newy != y)
    f(x, y, newx, newy);
}

static void
draw_point(Board *b, int x, int y)
{
  if (AT(b, x, y).mask & BR)  HIST(x, y, x+1, y);
  if (AT(b, x, y).mask & BL)  HIST(x, y, x-1, y);
  if (AT(b, x, y).mask & BD)  HIST(x, y, x, y+1);
  if (AT(b, x, y).mask & BU)  HIST(x, y, x, y-1);
  if (AT(b, x, y).mask & BUR) HIST(x, y, x+1, y-1);
  if (AT(b, x, y).mask & BUL) HIST(x, y, x-1, y-1);
  if (AT(b, x, y).mask & BDR) HIST(x, y, x+1, y+1);
  if (AT(b, x, y).mask & BDL) HIST(x, y, x-1, y+1);

  if (AT(b, x, y).borderp & BR) BORDER(x, y, x+1, y);
  if (AT(b, x, y).borderp & BL) BORDER(x, y, x-1, y);
  if (AT(b, x, y).borderp & BD) BORDER(x, y, x, y+1);
  if (AT(b, x, y).borderp & BU) BORDER(x, y, x, y-1);

}

static void
draw_points(Board *b)
{
  int i, j;
  for (i = 0; i < H; ++i)
    for (j = 0; j < W; ++j)
      draw_point(b, j, i);
}

static uint8_t
rmask(uint8_t mask)
{
  uint8_t res = 0;
  if (mask&BL)  res |= BR;
  if (mask&BR)  res |= BL;
  if (mask&BU)  res |= BD;
  if (mask&BD)  res |= BU;
  if (mask&BDL) res |= BUR;
  if (mask&BDR) res |= BUL;
  if (mask&BUL) res |= BDR;
  if (mask&BUR) res |= BDL;

  return res;
}

static uint8_t
legalp(Point from, Point to, uint8_t mask)
{
  if (mask&from.mask) return 0;
  if (rmask(mask)&to.mask) return 0;
  return 1;
}

static uint8_t
legalp2(Board *b, uint8_t mask)
{
  uint8_t x = b->x, y = b->y;
  mask_to_point(mask, &x, &y);
  /* if (x == 0  && (mask&BL || mask&BUL || mask&BDL)) return false; */
  if (b->x >= 8  && (mask&BR || mask&BUR || mask&BDR)) return false;
  if (b->y == 0  && ((mask&BU) || (mask&BUL) || (mask&BUR))) return false;
  if (b->y >= 12 && (mask&BD || mask&BDL || mask&BDR)) return false;
  return legalp(AT(b, b->x, b->y), AT(b, x, y), mask);
}


static void check_result(Board *b, uint8_t silentp);

/* returns 1 if move was legal & made */
static uint8_t
board_do_move(Board *b, uint8_t mask, uint8_t silentp)
{
  uint8_t nx = b->x, ny = b->y, chplr = 0;
  mask_to_point(mask, &nx, &ny);

  if (b->res != NONE)
    return 0;

  if (legalp(AT(b, b->x, b->y), AT(b, nx, ny), mask)) {
    AT(b, b->x, b->y).mask |= mask;
    b->x = nx, b->y = ny;
    if (!AT(b, b->x, b->y).mask) chplr = 1;
    AT(b, b->x, b->y).mask |= rmask(mask);
    check_result(b, silentp);
    if (chplr) b->plr = !b->plr;
    return 1;
  }
  return 0;
}

static void
maybe_get_proposed_point(Board *b)
{
  float ang = Vector2LineAngle((Vector2){b->x*PWIDTH, b->y*PHEIGHT}, GetMousePosition())+PI;
  uint8_t prop = BL;
  if (ang >= PI/8.f && ang <= (PI/4.f+PI/8.f))                               prop = BDL;
  else if (ang >= (PI/4.f+PI/8.f) && ang <= (PI/2.f+PI/8.f))                 prop = BD;
  else if (ang >= (PI/2.f+PI/8.f) && ang <= (PI-PI/8.f))                     prop = BDR;
  else if (ang >= (PI-PI/8.f) && ang <= (PI+PI/8.f))                         prop = BR;
  else if (ang >= (PI+PI/8.f) && ang <= PI+PI/4.f+PI/8.f)                    prop = BUR;
  else if (ang >= PI+PI/4.f+PI/8.f && ang <= (3*(PI/2.f))+PI/8.f)            prop = BU;
  else if (ang >= (3*(PI/2.f))+PI/8.f && ang <= (3*(PI/2.f))+PI/8.f+PI/4.f)  prop = BUL;

  draw_point_apply(draw_pt_propose, prop, b->x, b->y);

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    board_do_move(b, prop, 0);
}

uint8_t
get_legal_moves(Board *b, uint8_t moves[8])
{
  int i, j;
  memcpy(moves, (uint8_t[8]){ BU, BR, BD, BL, BUR, BDR, BUL, BDL }, 8);
  for (i = j = 0; i < 8; ++i)
    if (!legalp2(b, moves[i]))
      moves[i] = 0;
    else
      moves[j++] = moves[i];

  return j;
}

static float
eval(Board *b)
{
  float f = (b->plr ? b->y : 12-b->y)*100;
  /* TODO: add option for "defensive" bot. for f=0 it will just "not lose" */
  if (b->res != NONE) {
    if (b->plr == b->res) f = 1.f/0.f;
    if (b->plr != b->res) f = -1.f/0.f;
  }

  return f;
}

/* requires *best to be 0 in the beginning */
static float
negamax(Board *b, int absdepth, int depth, float alpha, float beta, uint8_t *best_)
{
  float max = -1.f/0.f, score = 0;
  uint8_t moves[8], nm, i, best = 0, pwas = b->plr;
  Board *bc;

  /* printf("absdepth=%d\n", absdepth); */

  nm = get_legal_moves(b, moves);
  *best_ = best = moves[0];
  if (b->res != NONE || depth == 0 || absdepth == 0) return eval(b);

  bc = malloc(sizeof(Board));

  for (i = 0; i < nm; ++i) {
    memcpy(bc, b, sizeof(Board));
    board_do_move(bc, moves[i], 1);
    if (pwas == bc->plr)
      score = 1.f * negamax(bc, absdepth-1, depth, alpha, beta, best_);
    else
      score = -1.f * negamax(bc, absdepth-1, depth-1, -beta, -alpha, best_);
    if (score > max) {
      max = score;
      if (score > alpha) {
        best = moves[i];
        alpha = score;
      }
    }
    if (score >= beta)
      break;
  }
  free(bc);
  *best_ = best;
  /* printf("returning move: %d with score %f\n", *best_, max); */
  return max;
}


static float
incremental_negamax(Board *b, int max_absdepth, int maxdepth, uint8_t *m)
{
  int i;
  float ev = -1.f/0.f;
  for (i = 1; i <= max_absdepth; ++i) {
    ev = negamax(b, i, DEPTH, -1.f/0.f, 1.f/0.f, m);
    if (ev == 1.f/0.f)
      return ev;
  }

  return ev;
}

static void
move_bot(Board *b)
{
  uint8_t m = 0;
  float ev;
  if (iflag)
    ev = incremental_negamax(b, ABSDEPTH, DEPTH, &m);
  else
    ev = negamax(b, ABSDEPTH, DEPTH, -1.f/0.f, 1.f/0.f, &m);
  uint8_t x = b->x, y = b->y;
  mask_to_point(m, &x, &y);
  board_do_move(b, m, 0);
  /* uint8_t moves[8], nm; */
  /* nm = get_legal_moves(b, moves); */
  /* if (nm == 0) */
  /*   puts("No legal moves."); */
  /* else { */
  /*   board_do_move(b, moves[rand()%nm]); */
  /* } */
}

static void
show_result(Board *b)
{
  if (b->res == WIN_P1)
    puts("P1 won.");
  if (b->res == WIN_P2)
    puts("P2 won.");
}

static void
check_result(Board *b, uint8_t silentp)
{
  uint8_t moves[8];
  uint8_t nm = get_legal_moves(b, moves);
  int i;

  if (nm == 0) {
    b->res = b->plr ? WIN_P1 : WIN_P2; /* other player won */
    goto fin;
  }
  for (i = 0; i < 6; ++i)
    if (b->x == wins[i][0] && b->y == wins[i][1]) {
      b->res = wins[i][1] == 12 ? WIN_P2 : WIN_P1;
      goto fin;
    }

  return;
 fin:
  if (!silentp)
    show_result(b);
}

/* FILE FORMAT:

   header part:
     BOARD%Wx%H\n
   where
     %W is the width
     %H is the height
   configuration part:
     %k=%v\n
     %k=%v\n
     ...
   where
     %k is a key, one of
       - res — game result
       - x   — current player x
       - y   — current player y
       - plr — current player
     %v is a value, an at most unsigned 8-bit integer
   points part:
     POINTS\n%b%m%b%m...
   where
     points are inserted row by row, every one consisting of:
       - %b — 8-bit integer specifying borders that are at given point
       - %m — 8-bit integer specifying mask of given point
     there are W*H points
*/

void
dump_board(Board *b, FILE *out)
{
  int i, j;
  fprintf(out, "BOARD%dx%d\n", W, H);
  fprintf(out, "res=%d\n", b->res);
  fprintf(out, "x=%d\n", b->x);
  fprintf(out, "y=%d\n", b->y);
  fprintf(out, "plr=%d\n", b->plr);
  fprintf(out, "POINTS=\n");
  for (i = 0; i < H; ++i)
    for (j = 0; j < W; ++j) {
      fputc(AT(b, j, i).borderp, out);
      fputc(AT(b, j, i).mask, out);
    }
}

void
dump_board_to(Board *b, char *filename)
{
  FILE *fp = fopen(filename, "w");
  if (!fp) err(errno, "Couldn't open %s", filename);
  dump_board(b, fp);
  fclose(fp);

  fprintf(stderr, "INFO: Dumped current board to %s\n", filename);
}

#define fscanf_expect(n, ...) do { if (fscanf(__VA_ARGS__) != (n)) errx(1, "Invalid format."); } while (0);

/* TODO: hack, unsafe. 64-byte sized buffer is A-OK with correct inputs. will overflow with incorrect inputs. */
void
load_board(Board *b, FILE *in)
{
  int i, j, _W, _H, value; /* value will be &0xff */
  char key[64] = {0};
  fscanf_expect(2, in, "BOARD%dx%d\n", &_W, &_H);
  if (_W != W || _H != H)
    errx(1, "Unsupported board sized %dx%d", _W, _H);
  while (fscanf(in, "%63[^=]=%d\n", key, &value) == 2) {
    if (strcmp(key, "res") == 0) b->res = value;
    else if (strcmp(key, "x") == 0)   b->x   = value;
    else if (strcmp(key, "y") == 0)   b->y   = value;
    else if (strcmp(key, "plr") == 0) b->plr = value;
    else errx(1, "Invalid format. Unknown key: %s", key);
  }
  if (strcmp(key, "POINTS") == 0) {
    for (i = 0; i < H; ++i)
      for (j = 0; j < W; ++j) {
        AT(b, j, i).borderp = fgetc(in);
        AT(b, j, i).mask = fgetc(in);
      }
  } else
    errx(1, "Invalid format. Expected POINTS.");
}

void
load_board_from(Board *b, char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp) err(errno, "Couldn't open %s", filename);
  load_board(b, fp);
  fclose(fp);

  fprintf(stderr, "INFO: Loaded %s\n", filename);
}

__attribute__((noreturn)) static void
usage(void)
{
  fprintf(stderr, "usage: %s [-hi] [-F targetfps] [-d depth] [-D absdepth] [file]\n", argv0);
  exit(1);
}

int
main(int argc, char **argv)
{
  int i, j;
  uint8_t xwas, ywas, tmp[8], nmoves = 0, Fflag = 60;
  char c;
  Board *b = malloc(sizeof(Board));
  Mode mflag = PL_VS_BOT;

  argv0 = *argv;
  while ((c = getopt(argc, argv, "hiF:d:D:m:")) != -1) {
    switch (c) {
    case 'F':
      Fflag = atoi(optarg);
      break;
    case 'd':
      DEPTH = atoi(optarg);
      break;
    case 'D':
      ABSDEPTH = atoi(optarg);
      break;
    case 'i':
      iflag = 1;
      break;
    case 'm':
      mflag = atoi(optarg);
      break;
    case 'h': /* fallthrough */
    default:
        usage();
    }
  }

  srand(time(0));

  InitWindow(PWIDTH*(W-1), PHEIGHT*(H-1), "pilka");
  SetTargetFPS(Fflag);

 beg:
  xwas = 255, ywas = 255; /* not even on the board */
  if (argv[optind])
    load_board_from(b, argv[optind]);
  else {
    memset(b, 0, sizeof(Board));
    init_board(b);
  }

  while (!WindowShouldClose()) {
    if (xwas != b->x || ywas != b->y)
      nmoves = get_legal_moves(b, tmp), xwas = b->x, ywas = b->y;

    BeginDrawing();

    ClearBackground(color_bg);
    for (i = 0; i < H; ++i)
      for (j = 0; j < W; ++j)
        DrawCircle(j*PWIDTH, i*PHEIGHT, 2.f, color_fg);

    draw_points(b);

    DrawRectangle(0, 0, 3*PWIDTH, PHEIGHT, color_bg);
    DrawRectangle(5*PWIDTH, 0, 3*PWIDTH, PHEIGHT, color_bg);
    DrawRectangle(0, 11*PHEIGHT, 3*PWIDTH, PHEIGHT, color_bg);
    DrawRectangle(5*PWIDTH, 11*PHEIGHT, 3*PWIDTH, PHEIGHT, color_bg);
    DrawRectangle(8*PWIDTH, 0, PWIDTH, 12*PHEIGHT, color_bg);

    DrawText(TextFormat("Possible moves: %d", nmoves), 0, 0, 8, color_fg);

    DrawCircle(b->x*PWIDTH, b->y*PHEIGHT, 5.f, b->plr ? color_p2 : color_p1);

    switch (mflag) {
    case PL_VS_BOT:
    case BOT_VS_PL:
      if (b->res == NONE) {
        if (b->plr == mflag)
          maybe_get_proposed_point(b);
        else
          move_bot(b);
      }
      break;
    case BOT_VS_BOT:
      move_bot(b);
      break;
    case BOT_VS_BOT_SLOW:
      if (IsKeyPressed(KEY_ENTER))
        move_bot(b);
      break;
    default:
      errx(1, "Unknown mode: %d.", mflag);
    }

    EndDrawing();

    if (IsKeyPressed(KEY_Q)) CloseWindow();
    if (IsKeyPressed(KEY_R)) goto beg;
    if (IsKeyPressed(KEY_D)) dump_board_to(b, "last.brd");
    if (IsKeyPressed(KEY_L)) load_board_from(b, "last.brd");
    if (IsKeyPressed(KEY_B)) move_bot(b);
  }

  return 0;
}
