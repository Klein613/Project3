#include "lcd/lcd.h"
#include "utils.h"
#include "assembly/example.h"

#define SCREEN_WIDTH   80
#define SCREEN_HEIGHT  160

#define PLAYER_WIDTH   2
#define PLAYER_HEIGHT  2
#define PLAYER_SPEED   5
#define PLAYER_COLOR   YELLOW
#define PLAYER_INIT_X  20
#define PLAYER_INIT_Y  50
#define GRAVITY 1


#define WALL_WIDTH     5
#define WALL_COLOR     RED
#define WALL_SPEED     2

#define MAX_LIFE       9
#define INVINCIBLE_TIME 45

#define TARGET_FPS 40
#define FRAME_TIME_MS (1000 / TARGET_FPS)

#define MAX_TAIL_LENGTH 20

#define DEBOUNCE_TIME 300 // 去抖动时间 (0.3 秒)

int tail_x[MAX_TAIL_LENGTH];
int tail_y[MAX_TAIL_LENGTH];
int tail_length = 0;


int player_x, player_y;
int player_life;
int player_invincible_time = 0;
int player_score = 0;
int player_velocity = 0;

int wall_x, wall_y;
int wall1_x, wall1_y;

uint64_t start_time, end_time, elapsed_time_ms;

int space = 0;
int start = 0;
int difficulty_level = 0;
int debounce_timer = 0;

int prev_player_x;
int prev_player_y;
int prev_wall_x;
int prev_wall_y;
int prev_wall1_x;
int prev_wall1_y;

void Inp_init(void) {
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOC);

  gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ,
            GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
  gpio_init(GPIOC, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ,
            GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}

void IO_init(void) {
  Inp_init(); // inport init
  Lcd_Init(); // LCD init
}

void draw_player(void) {
  u16 color = (player_invincible_time > 0) ? GREEN : PLAYER_COLOR;
  LCD_Fill(player_x, player_y, player_x + PLAYER_WIDTH, player_y + PLAYER_HEIGHT, color);
}
void update_player(void){

  if (Get_Button(JOY_LEFT)) {
    player_velocity = -PLAYER_SPEED;
  } 
  else {
    player_velocity += GRAVITY;
  }

  player_y += player_velocity;

  player_y += PLAYER_SPEED/2;
 if(player_x >= wall_x + WALL_WIDTH - WALL_SPEED && player_x < wall_x + WALL_WIDTH)
 {
    if(player_y + PLAYER_WIDTH <= wall_y - PLAYER_SPEED  && player_y - PLAYER_SPEED >= wall_y - space)
    {

      player_score++;
      
    }
 }
 if(player_x >= wall1_x + WALL_WIDTH - WALL_SPEED && player_x < wall1_x + WALL_WIDTH)
 {
    if(player_y + PLAYER_WIDTH <= wall1_y - PLAYER_SPEED  && player_y - PLAYER_SPEED >= wall1_y - space)
    {

      player_score++;
      
    }
 }
 // 记录玩家的位置
  tail_x[tail_length] = player_x;
  tail_y[tail_length] = player_y;
  tail_length++;

  // 如果尾巴长度超过最大值,移除最早的位置
  if (tail_length >= MAX_TAIL_LENGTH) {
    for (int i = 0; i < tail_length - 1; i++) {
      tail_x[i] = tail_x[i + 1];
      tail_y[i] = tail_y[i + 1];
    }
    tail_length--;
  }
}
void update_player_tail(void) {
  // 将尾巴的位置向左移动
  for (int i = 0; i < tail_length; i++) {
    tail_x[i] -= WALL_SPEED;
  }
}

void draw_wall(void) {
  LCD_Fill(wall_x, wall_y, wall_x + WALL_WIDTH, 160, WALL_COLOR);
  LCD_Fill(wall_x, 40, wall_x + WALL_WIDTH, wall_y-space , WALL_COLOR);
  LCD_Fill(wall1_x, wall1_y, wall1_x + WALL_WIDTH, 160, WALL_COLOR);
  LCD_Fill(wall1_x, 40, wall1_x + WALL_WIDTH, wall1_y-space , WALL_COLOR);
  LCD_DrawRectangle(0,41,79,159,WHITE);
}

void draw_life(void) {
  LCD_ShowString(0 ,0,"Life", 65532);
  LCD_ShowNum(50 ,0 , player_life, 1, 65532);
}
void draw_score(void) {
  LCD_ShowString(0 ,20, "Score", 65532);
  LCD_ShowNum(50 ,20 , player_score, 2, 65532);
}

void draw_player_tail(void) {
  // 连接尾巴上的点
  for (int i = 1; i < tail_length; i++) {
    LCD_DrawLine(tail_x[i - 1], tail_y[i - 1], tail_x[i], tail_y[i], PLAYER_COLOR);
  }
}


void update_walls(void) {
  wall_x -= WALL_SPEED;
  wall1_x -= WALL_SPEED;
  if (wall_x <= 0) {             ///////这对墙过了，再生成一堵新的//////
    wall_x =  SCREEN_WIDTH;
    wall_y =  100 + rand() % (60);
  }
  if (wall1_x <= 0) {             ///////这对墙过了，再生成一堵新的//////
    wall1_x =  SCREEN_WIDTH;
    wall1_y = 100 + rand() % (60);
  }
}

int check_collision(void) {
  if (player_invincible_time > 0) {
    return 0;
  }
  
  if (player_x + PLAYER_WIDTH > wall_x && player_x <= wall_x + WALL_WIDTH) 
  {
    if(player_y + PLAYER_WIDTH >= wall_y || player_y <= wall_y - space)
    {
      return 1;
    }
    
  }
  if (player_x + PLAYER_WIDTH > wall1_x && player_x <= wall1_x + WALL_WIDTH) 
  {
    if(player_y + PLAYER_WIDTH >= wall1_y || player_y <= wall1_y - space)
    {
      return 1;
    }
    
  }
  if (player_y <= 40)
  {
    return 1;
  }
  if (player_y >= 160)
  {
    return 1;
  }
  return 0;
}

void game_loop(void) {
  while (1) {

    prev_player_x = player_x;
    prev_player_y = player_y;
    prev_wall_x = wall_x;
    prev_wall_y = wall_y;
    prev_wall1_x = wall1_x;
    prev_wall1_y = wall1_y;


    start_time = get_timer_value();
    
    update_player();
    update_walls();

    if (check_collision()) {
      if(player_life != 0)
      {
        player_life--;
      }

      player_x = PLAYER_INIT_X;
      player_y = PLAYER_INIT_Y;
      player_invincible_time = INVINCIBLE_TIME;
      player_velocity = 0;
    }
    
    // 清除上一帧的玩家位置
    LCD_Fill(prev_player_x, prev_player_y, prev_player_x + PLAYER_WIDTH, prev_player_y + PLAYER_HEIGHT, BLACK);

    // 绘制当前帧的玩家位置
    draw_player();

    // 清除上一帧的墙位置
    LCD_Fill(prev_wall_x, prev_wall_y, prev_wall_x + WALL_WIDTH, 160, BLACK);
    LCD_Fill(prev_wall_x, 40, prev_wall_x + WALL_WIDTH, prev_wall_y - space, BLACK);
    LCD_Fill(prev_wall1_x, prev_wall1_y, prev_wall1_x + WALL_WIDTH, 160, BLACK);
    LCD_Fill(prev_wall1_x, 40, prev_wall1_x + WALL_WIDTH, prev_wall1_y - space, BLACK);

    // 绘制当前帧的墙位置
    draw_wall();

    // 绘制生命值和得分
    draw_life();
    draw_score();


    // 清除上一帧的玩家尾巴
    for (int i = 1; i < tail_length; i++) {
      LCD_DrawLine(tail_x[i - 1], tail_y[i - 1], tail_x[i], tail_y[i], BLACK);
    }

    // 更新玩家尾巴的位置
    update_player_tail();

    // 绘制当前帧的玩家尾巴
    draw_player_tail();
    
    if (player_invincible_time > 0) {
      player_invincible_time--;
    }
    // 更新上一帧的位置
    prev_player_x = player_x;
    prev_player_y = player_y;
    prev_wall_x = wall_x;
    prev_wall_y = wall_y;
    prev_wall1_x = wall1_x;
    prev_wall1_y = wall1_y;

    
    end_time = get_timer_value();
    elapsed_time_ms = (end_time - start_time) * 1000 / SystemCoreClock;
    
    if (elapsed_time_ms < FRAME_TIME_MS) {
      delay_1ms(FRAME_TIME_MS - elapsed_time_ms);
    }
  }
}

void game_init(void) {
  LCD_Clear(BLACK);
  
  player_x = PLAYER_INIT_X;
  player_y = PLAYER_INIT_Y;
  player_life = MAX_LIFE;

  wall_x =  SCREEN_WIDTH;
  wall_y =  100 + rand() % (60);
  wall1_x = wall_x - 40;
  wall1_y =  100 + rand() % (60);
}

void update_difficulty_display(void) {

  switch (difficulty_level) {
    case 1:
      
      LCD_ShowString(0, 70, "->", WHITE);
      LCD_ShowString(0, 90, "  ", WHITE);
      LCD_ShowString(0, 110, "  ", WHITE);
      break;
    case 2:
      LCD_ShowString(0, 70, "  ", WHITE);
      LCD_ShowString(0, 90, "->", WHITE);
      LCD_ShowString(0, 110, "  ", WHITE);
      break;
    case 3:
      LCD_ShowString(0, 70, "  ", WHITE);
      LCD_ShowString(0, 90, "  ", WHITE);
      LCD_ShowString(0, 110, "->", WHITE);
      break;
  }
}

int difficult_chosen(void) {
  static int debounce_timer_joy_up = 0;
  static int debounce_timer_joy_down = 0;
  static int debounce_timer_BUTTON_1 = 0;
  static int prev_difficulty_level = 0;

  if (Get_Button(JOY_LEFT)) {
    if (debounce_timer_joy_up == 0) {
      if (difficulty_level > 1) {
        difficulty_level--;
        debounce_timer_joy_up = DEBOUNCE_TIME;
      }
    }
  } 
  else {
    debounce_timer_joy_up = 0;
  }

  if (Get_Button(JOY_RIGHT)) {
    if (debounce_timer_joy_down == 0) {
      if (difficulty_level < 3) {
        difficulty_level++;
        debounce_timer_joy_down = DEBOUNCE_TIME;
      }
    }
  } else {
    debounce_timer_joy_down = 0;
  }

  if (Get_Button(BUTTON_1)) {
    if (debounce_timer_BUTTON_1 == 0) {
      debounce_timer_BUTTON_1 = DEBOUNCE_TIME;
      switch (difficulty_level) {
        case 1:
          return 50;
        case 2:
          return 30;
        case 3:
          return 5;
        default:
          return 0;
      }
    }
  } else {
    debounce_timer_BUTTON_1 = 0;
  }

  if (debounce_timer_joy_up > 0) {     ////冷却
    debounce_timer_joy_up--;
  }

  if (debounce_timer_joy_down > 0) {
    debounce_timer_joy_down--;
  }

  if (debounce_timer_BUTTON_1 > 0) {
    debounce_timer_BUTTON_1--;
  }

  if (difficulty_level != prev_difficulty_level) {
    update_difficulty_display();
    prev_difficulty_level = difficulty_level;
  }
  delay_1ms(1);
  return 0;
}






int main(void) {
  IO_init(); // 添加这一行以初始化输入端口和LCD
  draw();
  while (start==0)
  {
    int a = Get_Button(BUTTON_1);
    start = Press(a);
   
  }
  LCD_Clear(BLACK);
  LCD_ShowString(20, 70, "Easy", WHITE);
  LCD_ShowString(20, 90, "Medium", WHITE);
  LCD_ShowString(20, 110, "Hard", WHITE);
  while(space == 0)
  {
    space = difficult_chosen();
  }
  
  game_init();
  game_loop();

  return 0;
}