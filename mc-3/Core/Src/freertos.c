/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : FreeRTOS application code (Snake game)
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "kb.h"
#include "oled.h"
#include "fonts.h"
#include "music.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define CELL_SIZE      4U
#define BOARD_WIDTH    (OLED_WIDTH / CELL_SIZE)
#define BOARD_HEIGHT   (OLED_HEIGHT / CELL_SIZE)
#define MAX_SNAKE_LEN  (BOARD_WIDTH * BOARD_HEIGHT)

#define INPUT_QUEUE_LENGTH 8U
#define CMD_RESTART        0xFF

#define DIFF_EASY   0
#define DIFF_MEDIUM 1
#define DIFF_HARD   2
#define DIFF_EXTREME 3
#define DIFF_COUNT  4

#define CMD_MENU_UP    0xF0
#define CMD_MENU_DOWN  0xF1

typedef enum {
  DIR_UP = 0,
  DIR_RIGHT,
  DIR_DOWN,
  DIR_LEFT
} Direction;

typedef struct {
  uint8_t x;
  uint8_t y;
} Cell;

typedef struct {
  Cell body[MAX_SNAKE_LEN];
  uint16_t length;
  Direction direction;
  Direction next_direction;
  Cell food;
  bool alive;
  uint32_t score;
} GameState;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

static GameState g_state;
static uint32_t rng_state = 1U;
static volatile uint8_t g_dirty = 0U;
static volatile uint8_t g_menu_active = 1U;
static uint8_t g_selected_diff = DIFF_EASY;
static const uint16_t g_diff_step_ms[DIFF_COUNT] = { 150U, 125U, 100U, 75U };

osMessageQueueId_t inputQueueHandle;
const osMessageQueueAttr_t inputQueue_attributes = {
  .name = "inputQueue"
};

osMutexId_t gameStateMutexHandle;
const osMutexAttr_t gameStateMutex_attributes = {
  .name = "gameStateMutex"
};
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for InputTask */
osThreadId_t InputTaskHandle;
const osThreadAttr_t InputTask_attributes = {
  .name = "InputTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for GameTask */
osThreadId_t GameTaskHandle;
const osThreadAttr_t GameTask_attributes = {
  .name = "GameTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for RenderTask */
osThreadId_t RenderTaskHandle;
const osThreadAttr_t RenderTask_attributes = {
  .name = "RenderTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

static uint8_t is_opposite(Direction a, Direction b);
static void reset_game(GameState *state);
static void handle_input(GameState *state, uint8_t cmd);
static void game_step(GameState *state, uint8_t *ate_food, uint8_t *died);
static void render_game_full(const GameState *state);
static void render_menu(uint8_t selected_diff);
static uint32_t random32(void);
static uint8_t key_to_command(char key, uint8_t *cmd_out);
static void draw_cell(uint8_t x, uint8_t y, OLED_COLOR color, uint8_t is_food);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartInputTask(void *argument);
void StartGameTask(void *argument);
void StartRenderTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
  /* Called if pvPortMalloc() fails. */
}
/* USER CODE END 5 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  reset_game(&g_state);
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  inputQueueHandle = osMessageQueueNew(INPUT_QUEUE_LENGTH, sizeof(uint8_t), &inputQueue_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of InputTask */
  InputTaskHandle = osThreadNew(StartInputTask, NULL, &InputTask_attributes);

  /* creation of GameTask */
  GameTaskHandle = osThreadNew(StartGameTask, NULL, &GameTask_attributes);

  /* creation of RenderTask */
  RenderTaskHandle = osThreadNew(StartRenderTask, NULL, &RenderTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  gameStateMutexHandle = osMutexNew(&gameStateMutex_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartInputTask */
/**
* @brief Function implementing the InputTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartInputTask */
void StartInputTask(void *argument)
{
  /* USER CODE BEGIN StartInputTask */
  kb_init();
  char prev_key = 0;

  /* Infinite loop */
  for(;;)
  {
    char key = kb_get_symb();
    if (key == 0) {
      prev_key = 0;
      osDelay(30);
      continue;
    }

    if (key != prev_key) {
      uint8_t cmd;
      if (key_to_command(key, &cmd) == 1) {
        (void)osMessageQueuePut(inputQueueHandle, &cmd, 0, 0);
      }
      prev_key = key;
    }
    osDelay(30);
  }
  /* USER CODE END StartInputTask */
}

/* USER CODE BEGIN Header_StartGameTask */
/**
* @brief Function implementing the GameTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGameTask */
void StartGameTask(void *argument)
{
  /* USER CODE BEGIN StartGameTask */
  TickType_t last_wake_time = xTaskGetTickCount();
  TickType_t step_delay = pdMS_TO_TICKS(g_diff_step_ms[g_selected_diff]);
  uint8_t ate_food = 0;
  uint8_t died = 0;

  osMutexAcquire(gameStateMutexHandle, osWaitForever);
  reset_game(&g_state);
  osMutexRelease(gameStateMutexHandle);

  /* Infinite loop */
  for(;;)
  {
    ate_food = 0;
    died = 0;

    osMutexAcquire(gameStateMutexHandle, osWaitForever);
    uint8_t cmd;
    while (osMessageQueueGet(inputQueueHandle, &cmd, NULL, 0) == osOK) {
      if (g_menu_active) {
        if (cmd == CMD_MENU_UP && g_selected_diff > 0) {
          g_selected_diff--;
          g_dirty = 1U;
        } else if (cmd == CMD_MENU_DOWN && g_selected_diff + 1 < DIFF_COUNT) {
          g_selected_diff++;
          g_dirty = 1U;
        } else if (cmd == CMD_RESTART) {
          g_menu_active = 0U;
          step_delay = pdMS_TO_TICKS(g_diff_step_ms[g_selected_diff]);
          last_wake_time = xTaskGetTickCount();
          reset_game(&g_state);
          g_dirty = 1U;
        }
      } else {
        handle_input(&g_state, cmd);
      }
    }

    if (!g_menu_active) {
      game_step(&g_state, &ate_food, &died);
      g_dirty = 1U;
    }
    osMutexRelease(gameStateMutexHandle);

    if (ate_food) {
      play_note(1200);
      osDelay(30);
      stop_note();
    }
    if (died) {
      play_note(200);
      osDelay(200);
      stop_note();
    }

    if (!g_menu_active) {
      vTaskDelayUntil(&last_wake_time, step_delay);
    } else {
      osDelay(20);
    }
  }
  /* USER CODE END StartGameTask */
}

/* USER CODE BEGIN Header_StartRenderTask */
/**
* @brief Function implementing the RenderTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRenderTask */
void StartRenderTask(void *argument)
{
  /* USER CODE BEGIN StartRenderTask */
  GameState snapshot;
  memset(&snapshot, 0, sizeof(snapshot));

  /* Infinite loop */
  for(;;)
  {
    osMutexAcquire(gameStateMutexHandle, osWaitForever);
    if (g_dirty == 0U) {
      osMutexRelease(gameStateMutexHandle);
      osDelay(10);
      continue;
    }
    g_dirty = 0U;
    memcpy(&snapshot, &g_state, sizeof(snapshot));
    uint8_t menu = g_menu_active;
    uint8_t diff = g_selected_diff;
    osMutexRelease(gameStateMutexHandle);

    if (menu) {
      render_menu(diff);
    } else {
      render_game_full(&snapshot);
    }
  }
  /* USER CODE END StartRenderTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

static uint8_t key_to_command(char key, uint8_t *cmd_out)
{
  switch (key) {
  case '#':
    *cmd_out = CMD_RESTART;
    return 1;
  case '2':
    if (g_menu_active) {
      *cmd_out = CMD_MENU_UP;
    } else {
      *cmd_out = DIR_UP;
    }
    return 1;
  case '4':
    *cmd_out = DIR_LEFT;
    return 1;
  case '6':
    *cmd_out = DIR_RIGHT;
    return 1;
  case '8':
    if (g_menu_active) {
      *cmd_out = CMD_MENU_DOWN;
    } else {
      *cmd_out = DIR_DOWN;
    }
    return 1;
  default:
    return 0;
  }
}

static uint8_t is_opposite(Direction a, Direction b)
{
  return ((a == DIR_UP && b == DIR_DOWN) ||
          (a == DIR_DOWN && b == DIR_UP) ||
          (a == DIR_LEFT && b == DIR_RIGHT) ||
          (a == DIR_RIGHT && b == DIR_LEFT));
}

static uint32_t random32(void)
{
  rng_state = rng_state * 1664525U + 1013904223U;
  return rng_state;
}

static bool cell_occupied(const GameState *state, uint8_t x, uint8_t y)
{
  for (uint16_t i = 0; i < state->length; ++i) {
    if (state->body[i].x == x && state->body[i].y == y) {
      return true;
    }
  }
  return false;
}

static void place_food(GameState *state)
{
  if (state->length >= MAX_SNAKE_LEN) {
    state->alive = false;
    return;
  }
  for (uint16_t attempt = 0; attempt < MAX_SNAKE_LEN; ++attempt) {
    uint8_t x = random32() % BOARD_WIDTH;
    uint8_t y = random32() % BOARD_HEIGHT;
    if (!cell_occupied(state, x, y)) {
      state->food.x = x;
      state->food.y = y;
      return;
    }
  }
  for (uint8_t y = 0; y < BOARD_HEIGHT; ++y) {
    for (uint8_t x = 0; x < BOARD_WIDTH; ++x) {
      if (!cell_occupied(state, x, y)) {
        state->food.x = x;
        state->food.y = y;
        return;
      }
    }
  }
}

static void reset_game(GameState *state)
{
  state->length = 3;
  state->direction = DIR_RIGHT;
  state->next_direction = DIR_RIGHT;
  state->alive = true;
  state->score = 0;
  uint8_t start_x = BOARD_WIDTH / 2;
  uint8_t start_y = BOARD_HEIGHT / 2;
  for (uint16_t i = 0; i < state->length; ++i) {
    state->body[i].x = start_x - i;
    state->body[i].y = start_y;
  }
  rng_state ^= (uint32_t)osKernelGetTickCount();
  rng_state ^= HAL_GetTick();
  place_food(state);
  g_dirty = 1U;
}

static void handle_input(GameState *state, uint8_t cmd)
{
  if (cmd == CMD_RESTART) {
    reset_game(state);
    return;
  }
  if (cmd > DIR_LEFT) {
    return;
  }
  Direction requested = (Direction)cmd;
  if (!is_opposite(state->direction, requested)) {
    state->next_direction = requested;
  }
}

static void game_step(GameState *state, uint8_t *ate_food, uint8_t *died)
{
  *ate_food = 0;
  *died = 0;
  if (!state->alive) {
    return;
  }

  Direction dir = state->next_direction;
  state->direction = dir;

  Cell next = state->body[0];
  switch (dir) {
  case DIR_UP:
    if (next.y == 0) { state->alive = false; *died = 1; return; }
    next.y -= 1;
    break;
  case DIR_DOWN:
    next.y += 1;
    if (next.y >= BOARD_HEIGHT) { state->alive = false; *died = 1; return; }
    break;
  case DIR_LEFT:
    if (next.x == 0) { state->alive = false; *died = 1; return; }
    next.x -= 1;
    break;
  case DIR_RIGHT:
    next.x += 1;
    if (next.x >= BOARD_WIDTH) { state->alive = false; *died = 1; return; }
    break;
  }

  for (uint16_t i = 0; i < state->length; ++i) {
    if (state->body[i].x == next.x && state->body[i].y == next.y) {
      state->alive = false;
      *died = 1;
      return;
    }
  }

  uint16_t new_length = state->length + ((next.x == state->food.x && next.y == state->food.y) ? 1U : 0U);
  if (new_length > MAX_SNAKE_LEN) {
    state->alive = false;
    *died = 1;
    return;
  }

  for (int32_t i = (int32_t)new_length - 1; i > 0; --i) {
    if ((uint16_t)(i - 1) < state->length) {
      state->body[i] = state->body[i - 1];
    }
  }
  state->body[0] = next;
  state->length = new_length;

  if (next.x == state->food.x && next.y == state->food.y) {
    state->score += 1;
    *ate_food = 1;
    place_food(state);
  }
}

static void draw_cell(uint8_t x, uint8_t y, OLED_COLOR color, uint8_t is_food)
{
  uint8_t base_x = x * CELL_SIZE;
  uint8_t base_y = y * CELL_SIZE;

  if (is_food) {
	  oled_DrawSquare(base_x, base_x + CELL_SIZE - 1, base_y, base_y + CELL_SIZE - 1, White);
	  oled_DrawSquare(base_x + 3, base_x + CELL_SIZE - 1 - 3, base_y + 3, base_y + CELL_SIZE - 1 - 3, White);
	  return;
  }

  for (uint8_t dy = 0; dy < CELL_SIZE; ++dy) {
    for (uint8_t dx = 0; dx < CELL_SIZE; ++dx) {
      oled_DrawPixel((uint8_t)(base_x + dx), (uint8_t)(base_y + dy), color);
    }
  }
}

static int tick = 0;

static void render_game_full(const GameState *state)
{
  oled_Fill(Black);

  if (!state->alive) {
    char buf[24];
    oled_SetCursor(2, 8);
    oled_WriteString("Game Over", Font_11x18, White);
    snprintf(buf, sizeof(buf), "Score: %lu", (unsigned long)state->score);
    oled_SetCursor(2, 32);
    oled_WriteString(buf, Font_7x10, White);
    oled_SetCursor(2, 50);
    oled_WriteString("Press # to cont.", Font_7x10, White);
    oled_UpdateScreen();
    return;
  }

  oled_DrawHLine(0, OLED_WIDTH - 1, 0, White);
  oled_DrawHLine(0, OLED_WIDTH - 1, OLED_HEIGHT - 1, White);
  oled_DrawVLine(0, OLED_HEIGHT - 1, 0, White);
  oled_DrawVLine(0, OLED_HEIGHT - 1, OLED_WIDTH - 1, White);

  draw_cell(state->food.x, state->food.y, White, 1);
  for (uint16_t i = 0; i < state->length; ++i) {
    draw_cell(state->body[i].x, state->body[i].y, White, 0);
  }
  oled_UpdateScreen();
}

static void render_menu(uint8_t selected_diff)
{
  oled_Fill(Black);
  oled_SetCursor(6, 6);
  oled_WriteString("Select difficulty", Font_7x10, White);

  oled_DrawHLine(0, OLED_WIDTH, 20, 1);

  const char *labels[DIFF_COUNT] = { "Easy", "Medium", "Hard", "Extreme" };
  for (uint8_t i = 0; i < DIFF_COUNT; ++i) {
    oled_SetCursor(10, 23 + i * 10);
    if (i == selected_diff) {
      oled_WriteString("> ", Font_7x10, White);
    } else {
      oled_WriteString("  ", Font_7x10, White);
    }
    oled_WriteString((char *)labels[i], Font_7x10, White);
  }

  oled_SetCursor(6, 58);
  oled_WriteString("2/8:select  #:start", Font_7x10, White);
  oled_UpdateScreen();
}
/* USER CODE END Application */
