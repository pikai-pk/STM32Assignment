/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
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
#include "usart.h"
#include "bsp_uart_fifo.h"
#include "bsp_es8266.h"
#include "mqtt_client.h"
#include "mqtt_config.h"
#include "game_protocol.h"
#include "buzzer.h"
#include "keys.h"
#include "led.h"
#include "score_display.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  int8_t move;
} NetMessage_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static uint8_t g_mqtt_ready = 0U;
static char g_local_ip[32] = "unknown";
static uint32_t g_last_win_tick = 0U;
/* USER CODE END Variables */
/* Definitions for TaskNet */
osThreadId_t TaskNetHandle;
const osThreadAttr_t TaskNet_attributes = {
  .name = "TaskNet",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for TaskButton */
osThreadId_t TaskButtonHandle;
const osThreadAttr_t TaskButton_attributes = {
  .name = "TaskButton",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskBuzzer */
osThreadId_t TaskBuzzerHandle;
const osThreadAttr_t TaskBuzzer_attributes = {
  .name = "TaskBuzzer",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskStatus */
osThreadId_t TaskStatusHandle;
const osThreadAttr_t TaskStatus_attributes = {
  .name = "TaskStatus",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for TaskDisplay */
osThreadId_t TaskDisplayHandle;
const osThreadAttr_t TaskDisplay_attributes = {
  .name = "TaskDisplay",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for netQueue */
osMessageQueueId_t netQueueHandle;
const osMessageQueueAttr_t netQueue_attributes = {
  .name = "netQueue"
};
/* Definitions for buzzerQueue */
osMessageQueueId_t buzzerQueueHandle;
const osMessageQueueAttr_t buzzerQueue_attributes = {
  .name = "buzzerQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void Net_PublishStatusRepeated(uint8_t times);
static int8_t Button_ReadMoveDirection(void);
static void Buzzer_HandleEvent(BuzzerEvent_t event);
static void Net_HandleReceivePacket(void);
static void Net_HandleOneMqttPacket(uint8_t *data, uint16_t len);
static const char *Button_MoveToText(int8_t move);

/* USER CODE END FunctionPrototypes */

void StartTaskNet(void *argument);
void StartTaskButton(void *argument);
void StartTaskBuzzer(void *argument);
void StartTaskStatus(void *argument);
void StartTaskDisplay(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

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

  /* Create the queue(s) */
  /* creation of netQueue */
  netQueueHandle = osMessageQueueNew (16, sizeof(NetMessage_t), &netQueue_attributes);

  /* creation of buzzerQueue */
  buzzerQueueHandle = osMessageQueueNew (8, sizeof(BuzzerEvent_t), &buzzerQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TaskNet */
  TaskNetHandle = osThreadNew(StartTaskNet, NULL, &TaskNet_attributes);

  /* creation of TaskButton */
  TaskButtonHandle = osThreadNew(StartTaskButton, NULL, &TaskButton_attributes);

  /* creation of TaskBuzzer */
  TaskBuzzerHandle = osThreadNew(StartTaskBuzzer, NULL, &TaskBuzzer_attributes);

  /* creation of TaskStatus */
  TaskStatusHandle = osThreadNew(StartTaskStatus, NULL, &TaskStatus_attributes);

  /* creation of TaskDisplay */
  TaskDisplayHandle = osThreadNew(StartTaskDisplay, NULL, &TaskDisplay_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartTaskNet */
/**
  * @brief  Function implementing the TaskNet thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTaskNet */
__weak void StartTaskNet(void *argument)
{
  /* USER CODE BEGIN StartTaskNet */
  /* 网络任务：初始化 USART6 FIFO，连接 ESP8266、Wi-Fi 和 MQTT，并订阅 RESULT 结果主题。 */
  UART_FIFO_Init(&uart6_fifo, &huart6);
  UART_Start_IT(&uart6_fifo);
  Buzzer_Init();
  ScoreDisplay_Init();
  Led_Init();

  printf("\r\n========== PongCtrl Start ==========\r\n");
  printf("Player ID: %d\r\n", PLAYER_ID);
  printf("WiFi SSID: %s\r\n", WIFI_SSID);
  printf("MQTT Broker: %s:%d\r\n", MQTT_BROKER_HOST, MQTT_BROKER_PORT);
  printf("MQTT ClientId: %s\r\n", MQTT_CLIENT_ID);
  printf("STATUS Topic: %s\r\n", TOPIC_STATUS);
  printf("INPUT Topic: %s\r\n", TOPIC_INPUT);
  printf("RESULT Topic: %s\r\n", TOPIC_RESULT);
  printf("====================================\r\n");

  for (;;)
  {
    g_mqtt_ready = 0U;
    ScoreDisplay_Reset();
    Led_AllOff();

    printf("[NET] ESP8266 init start...\r\n");
    if (ESP_Init() != 0U)
    {
      printf("[NET] ESP8266 init failed, retry after 3s.\r\n");
      osDelay(3000U);
      continue;
    }
    printf("[NET] ESP8266 init OK, WiFi connected.\r\n");

    printf("[NET] Reading ESP8266 IP...\r\n");
    if (ESP_GetLocalIP(g_local_ip, sizeof(g_local_ip)) == 0U)
    {
      GameProtocol_SetDeviceIp(g_local_ip);
      printf("[NET] ESP8266 IP: %s\r\n", g_local_ip);
    }
    else
    {
      GameProtocol_SetDeviceIp("unknown");
      printf("[NET] ESP8266 IP read failed, STATUS will use unknown.\r\n");
    }

    printf("[NET] TCP connect to MQTT Broker %s:%d...\r\n", MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    if (ESP_ConnectMQTTServer() != 0U)
    {
      printf("[NET] TCP connect failed, retry after 3s.\r\n");
      osDelay(3000U);
      continue;
    }
    printf("[NET] TCP connected, ESP8266 entered transparent mode.\r\n");

    printf("[MQTT] CONNECT start, ClientId=%s...\r\n", MQTT_CLIENT_ID);
    if (MQTT_Connect() != 0U)
    {
      printf("[MQTT] CONNECT failed, retry after 3s.\r\n");
      osDelay(3000U);
      continue;
    }
    printf("[MQTT] CONNECT OK.\r\n");

    printf("[MQTT] SUBSCRIBE %s\r\n", TOPIC_RESULT);
    MQTT_Subscribe(TOPIC_RESULT);
    printf("[MQTT] SUBSCRIBE sent.\r\n");

    printf("[MQTT] Publish STATUS start.\r\n");
    Net_PublishStatusRepeated(3U);
    printf("[MQTT] Publish STATUS done, board is online.\r\n");
    g_mqtt_ready = 1U;
    Led_AllOn();

    for (;;)
    {
      NetMessage_t msg;

      if (osMessageQueueGet(netQueueHandle, &msg, NULL, 0U) == osOK)
      {
        printf("[INPUT] move=%d (%s), publish to %s\r\n",
               msg.move,
               Button_MoveToText(msg.move),
               TOPIC_INPUT);
        GameProtocol_PublishInput(msg.move);
      }

      MQTT_KeepAlive();
      Net_HandleReceivePacket();
      osDelay(10U);
    }
  }
  /* USER CODE END StartTaskNet */
}

/* USER CODE BEGIN Header_StartTaskButton */
/**
* @brief Function implementing the TaskButton thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskButton */
void StartTaskButton(void *argument)
{
  /* USER CODE BEGIN StartTaskButton */
  /* 按键任务：检测 SW1(PE1) 与 SW4(PE4)，每次按下发送一次移动指令。 */
  uint8_t button_ready = 1U;

  for(;;)
  {
    int8_t current_move = Button_ReadMoveDirection();
    uint8_t should_send = 0U;

    if (current_move == 0)
    {
      button_ready = 1U;
    }

    if ((current_move != 0) && (button_ready != 0U))
    {
      should_send = 1U;
      button_ready = 0U;
    }

    if (should_send != 0U)
    {
      NetMessage_t msg;

      msg.move = current_move;

      if (g_mqtt_ready != 0U)
      {
        if (osMessageQueuePut(netQueueHandle, &msg, 0U, 0U) == osOK)
        {
          printf("[BUTTON] step queued: %d (%s)\r\n",
                 current_move,
                 Button_MoveToText(current_move));
        }
        else
        {
          printf("[BUTTON] netQueue full, drop move=%d\r\n", current_move);
        }
      }
    }

    osDelay(1U);
  }
  /* USER CODE END StartTaskButton */
}

/* USER CODE BEGIN Header_StartTaskBuzzer */
/**
* @brief Function implementing the TaskBuzzer thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskBuzzer */
void StartTaskBuzzer(void *argument)
{
  /* USER CODE BEGIN StartTaskBuzzer */
  /* 蜂鸣器任务：等待 RESULT 解析后的事件，播放胜利或失败提示音。 */
  for(;;)
  {
    BuzzerEvent_t event;

    if (osMessageQueueGet(buzzerQueueHandle, &event, NULL, osWaitForever) == osOK)
    {
      printf("[BUZZER] event=%d\r\n", event);
      Buzzer_HandleEvent(event);
    }
  }
  /* USER CODE END StartTaskBuzzer */
}

/* USER CODE BEGIN Header_StartTaskStatus */
/**
* @brief Function implementing the TaskStatus thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskStatus */
void StartTaskStatus(void *argument)
{
  /* USER CODE BEGIN StartTaskStatus */
  uint8_t status_tick = 0U;

  for(;;)
  {
    if ((g_mqtt_ready != 0U) && (status_tick == 0U))
    {
      printf("[STATUS] publish online status, ip=%s\r\n", g_local_ip);
      GameProtocol_PublishStatus();
    }

    status_tick++;
    if (status_tick >= 6U)
    {
      status_tick = 0U;
    }

    osDelay(500U);
  }
  /* USER CODE END StartTaskStatus */
}

/* USER CODE BEGIN Header_StartTaskDisplay */
/**
* @brief Function implementing the TaskDisplay thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskDisplay */
void StartTaskDisplay(void *argument)
{
  /* USER CODE BEGIN StartTaskDisplay */
  /* 数码管扫描任务：每 1ms 刷新一位，持续显示当前分数 0000~9999。 */
  for(;;)
  {
    ScoreDisplay_Tick();
    osDelay(1U);
  }
  /* USER CODE END StartTaskDisplay */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* 重复发布 STATUS，提高 UE5 或 MQTTX 收到上线消息的概率。 */
static void Net_PublishStatusRepeated(uint8_t times)
{
  uint8_t i;

  for (i = 0U; i < times; ++i)
  {
    GameProtocol_PublishStatus();
    osDelay(300U);
  }
}

/* 读取当前按键方向：SW1 与 SW4 同时按下或都未按下时不发送移动。 */
static int8_t Button_ReadMoveDirection(void)
{
  uint8_t up_pressed = sw1_is_pressed();
  uint8_t down_pressed = sw4_is_pressed();

  if ((up_pressed != 0U) && (down_pressed == 0U))
  {
    return 1;
  }

  if ((down_pressed != 0U) && (up_pressed == 0U))
  {
    return -1;
  }

  return 0;
}

/* 连续多次读取到相同方向才认为按键稳定，用于按键消抖。 */
#if 0
static int8_t Button_ReadStableDirection(void)
{
  static int8_t stable_move = 0;
  static int8_t last_sample = 0;
  static uint8_t same_count = 0U;
  int8_t sample = Button_ReadMoveDirection();

  if (sample == last_sample)
  {
    if (same_count < 3U)
    {
      same_count++;
    }
  }
  else
  {
    same_count = 0U;
    last_sample = sample;
  }

  if (same_count >= 3U)
  {
    stable_move = sample;
  }

  return stable_move;
}
#endif

/* 根据蜂鸣器事件播放对应旋律。 */
static void Buzzer_HandleEvent(BuzzerEvent_t event)
{
  if (event == BUZZER_EVENT_WIN)
  {
    Buzzer_PlayWinMusic();
  }
  else if (event == BUZZER_EVENT_LOSE)
  {
    Buzzer_PlayLoseMusic();
  }
  else if (event == BUZZER_EVENT_RESET_SCORE)
  {
  }
}

/* 从 ESP8266 透传数据中读取 MQTT 包，解析 RESULT 并投递给蜂鸣器任务。 */
static void Net_HandleReceivePacket(void)
{
  uint8_t rx_buf[256];
  uint16_t rx_len;
  uint16_t i;

  rx_len = ESP_RecvRaw(rx_buf, sizeof(rx_buf));
  if (rx_len == 0U)
  {
    return;
  }

  for (i = 0U; i < rx_len; ++i)
  {
    if ((rx_buf[i] & 0xF0U) == 0x30U)
    {
      Net_HandleOneMqttPacket(&rx_buf[i], (uint16_t)(rx_len - i));
    }
  }
}

/* 处理一个可能的 MQTT PUBLISH 包。串口一次读取中可能粘连多个包，因此由上层循环逐个尝试解析。 */
static void Net_HandleOneMqttPacket(uint8_t *data, uint16_t len)
{
  BuzzerEvent_t event;

  event = GameProtocol_ParseMqttPacket(data, len);
  if (event != BUZZER_EVENT_NONE)
  {
    if (GameProtocol_IsExitMqttPacket(data, len) != 0U)
    {
      g_mqtt_ready = 0U;
      Led_AllOff();
      printf("[LED] game exit/offline, LED1-LED8 off.\r\n");
    }

    if (event == BUZZER_EVENT_WIN)
    {
      uint32_t now_tick = osKernelGetTickCount();

      if ((g_last_win_tick == 0U) || ((now_tick - g_last_win_tick) > 1000U))
      {
        g_last_win_tick = now_tick;
        ScoreDisplay_AddWin();
        printf("[SCORE] win +1, score=%u\r\n", ScoreDisplay_GetScore());
      }
      else
      {
        printf("[SCORE] duplicate win ignored, score=%u\r\n", ScoreDisplay_GetScore());
      }
    }
    else if (event == BUZZER_EVENT_RESET_SCORE)
    {
      ScoreDisplay_Reset();
      g_last_win_tick = 0U;
      printf("[SCORE] reset to 0000.\r\n");
    }

    printf("[MQTT] RESULT received, buzzer event=%d\r\n", event);
    osMessageQueuePut(buzzerQueueHandle, &event, 0U, 0U);
  }
}

/* 将 move 数值转换成文字，便于串口调试助手直接观察方向。 */
static const char *Button_MoveToText(int8_t move)
{
  if (move < 0)
  {
    return "UP";
  }

  if (move > 0)
  {
    return "DOWN";
  }

  return "STOP";
}

/* USER CODE END Application */

