#define TB_IMPL

#include "termbox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct ball {
  int x;
  int y;
  int dx;
  int dy;
} Ball;

static short color = TB_DEFAULT;
static int nballs = 10;
static short speedMult = 1;
static short rim = 0;
static float radius = 100;

int parse_options(int argc, char *argv[]);
void print_help();

int main(int argc, char *argv[]) {

  if (!parse_options(argc, argv))
    return 0;

  if (color == TB_DEFAULT)
    rim = 0;

  time_t t;
  Ball *balls = (Ball *)malloc(sizeof(Ball) * nballs);

  struct tb_event event = {0};

  srand((unsigned)time(&t));

  tb_init();

  tb_hide_cursor();

  int maxX = tb_width();
  int maxY = tb_height() * 2;
  int speed = (((1 / (float)(maxX + maxY)) * 5000000) + 50000) / speedMult;
  radius = radius / (float)(maxX * maxY); //(float)(maxX > maxY ? maxX : maxY);

  float innerRadius = radius * (1 + (float)(0.25 * rim));

  for (int i = 0; i < nballs; i++) {
    balls[i].x = rand() % maxX;
    balls[i].y = rand() % maxY;
    balls[i].dx = (rand() % 2 == 0) ? -1 : 1;
    balls[i].dy = (rand() % 2 == 0) ? -1 : 1;
  }

  while (1) {

    // move balls
    for (int i = 0; i < nballs; i++) {

      if (balls[i].x + balls[i].dx >= maxX || balls[i].x + balls[i].dx < 0)
        balls[i].dx *= -1;

      if (balls[i].y + balls[i].dy >= maxY || balls[i].y + balls[i].dy < 0)
        balls[i].dy *= -1;

      balls[i].x += balls[i].dx;
      balls[i].y += balls[i].dy;
    }

    // render
    for (int i = 0; i < maxX; i++) {
      for (int j = 0; j < maxY / 2; j++) {
        // calculate the two halfs of the block at the same time
        float sum[2] = {0};

        for (int j2 = 0; j2 < 2; j2++) {

          for (int k = 0; k < nballs; k++) {
            int y = j * 2 + j2;
            sum[j2] += 1 / ((float)((i - balls[k].x) * (i - balls[k].x) +
                                    (y - balls[k].y) * (y - balls[k].y)));
          }
        }

        if (sum[0] > radius) {
          if (sum[1] > radius) {
            tb_printf(i, j, color | TB_BOLD, 0, "█");
          } else {
            tb_printf(i, j, color | TB_BOLD, 0, "▀");
          }
        } else if (sum[1] > radius) {
          tb_printf(i, j, color | TB_BOLD, 0, "▄");
        }

        if (rim) {
          if (sum[0] > innerRadius) {
            if (sum[1] > innerRadius) {
              tb_printf(i, j, color, 0, "█");
            } else {
              tb_printf(i, j, color | TB_BOLD, color, "▄");
            }
          } else if (sum[1] > innerRadius) {
            tb_printf(i, j, color | TB_BOLD, color, "▀");
          }
        }
      }
    }
    tb_present();
    usleep(speed);
    tb_clear();

    tb_peek_event(&event, 10);
    if (event.key == TB_KEY_CTRL_C || event.key == TB_KEY_ESC ||
        event.ch == 'q' || event.ch == 'Q')
      break;
  }

  tb_shutdown();

  free(balls);
}

int parse_options(int argc, char *argv[]) {
  if (argc == 1)
    return 1;
  int c;
  while ((c = getopt(argc, argv, ":c:s:r:R:b:h")) != -1) {
    switch (c) {
    case 'c':
      if (strcmp(optarg, "red") == 0) {
        color = TB_RED;
      } else if (strcmp(optarg, "yellow") == 0) {
        color = TB_YELLOW;
      } else if (strcmp(optarg, "blue") == 0) {
        color = TB_BLUE;
      } else if (strcmp(optarg, "green") == 0) {
        color = TB_GREEN;
      } else if (strcmp(optarg, "magenta") == 0) {
        color = TB_MAGENTA;
      } else if (strcmp(optarg, "cyan") == 0) {
        color = TB_CYAN;
      } else {
        printf("Unknown color\n");
        return 0;
      }
      break;
    case 's':
      speedMult = atoi(optarg);
      if (speedMult > 5 || speedMult <= 0) {
        printf("Invalid speed, only values between 1 and 5 are allowed\n");
        return 0;
      }
      break;
    case 'R':
      rim = atoi(optarg);
      if (rim > 5 || rim < 1) {
        printf("Invalid rim, only values between 1 and 5 are allowed\n");
        return 0;
      }
      break;
    case 'r':
      radius = 50 + (11 - atoi(optarg)) * 10;
      if (radius > 150 || radius < 50) {
        printf("Invalid radius, only values between 1 and 10 are allowed\n");
        return 0;
      }
      break;

    case 'b':
      nballs = atoi(optarg);
      if (nballs > 20 || nballs < 2) {

        printf("Invalid number of metaballs, only values between 2 and 20 are"
               "allowed\n");
        return 0;
      }
      break;
    case 'h':
      print_help();
      return 0;
      break;
    case ':': /* -f or -o without operand */
      fprintf(stderr, "Option -%c requires an operand\n", optopt);
      return 0;
      break;
    case '?':
      fprintf(stderr, "Unrecognized option: -%c\n", optopt);
      return 0;
    }
  }
  return 1;
}

void print_help() {
  printf(
      "Usage: lavaterm [OPTIONS]\n"
      "OPTIONS:\n"
      "  -c COLOR          Set color. Available colours: red, blue, yellow, "
      "green, cyan and magenta. \n"
      "                    Besides those colors the default one is the normal "
      "foreground of your terminal.\n"
      "  -s SPEED          Set the speed, from 1 to 5. (default 1)\n"
      "  -r RADIUS         Set the radius of the metaballs, from 1 to 10. "
      "(default: 5)\n"
      "  -R RIM            Set a \"rim\" for each metaball, from 1 to 5. "
      "(default: none)\n"
      "                    This option does not work with the default color\n"
      "  -b NBALLS         Set the number of metaballs in the simulation, from "
      "2 to 20. (default: 10)\n"
      "  -h                Print help.\n"
      "From a tty the rim will not work well.\n");
}
