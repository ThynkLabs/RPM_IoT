#include "hmi.h"
#include <arduino.h>

xQueueHandle interruptQueue;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

TaskHandle_t task_interrupt_handle;
TaskHandle_t task_oled_handle;
TaskHandle_t task_led_indication_handle;

void interrupt_gpio_init() {
  gpio_pad_select_gpio((gpio_num_t)HMI_SWITCH_A);
  gpio_set_direction((gpio_num_t)HMI_SWITCH_A, GPIO_MODE_INPUT);
  gpio_pulldown_en((gpio_num_t)HMI_SWITCH_A);
  gpio_pullup_dis((gpio_num_t)HMI_SWITCH_A);
  gpio_set_intr_type((gpio_num_t)HMI_SWITCH_A, GPIO_INTR_POSEDGE);
}

void IRAM_ATTR interrupt_gpio_isr_handler(void *args) {
  int pinNumber = (int)args;
  xQueueSendFromISR(interruptQueue, &pinNumber, NULL);
}

void interrupt_install() {
  gpio_install_isr_service(0);
  gpio_isr_handler_add((gpio_num_t)HMI_SWITCH_A, interrupt_gpio_isr_handler, (void *)HMI_SWITCH_A);
}

void start_interrupt_task() {
  xTaskCreate(&interrupt_task, "interrupt_task", INTERRUPT_TASK_STACK_SIZE, NULL, INTERRUPT_TASK_PRIORITY, &task_interrupt_handle);
}

void interrupt_task(void *Params) {
  interruptQueue = xQueueCreate(10, sizeof(int));
  interrupt_gpio_init();
  interrupt_install();
  gpio_num_t pinNumber;
  int count = 0;
  while (1) {
    if (xQueueReceive(interruptQueue, &pinNumber, portMAX_DELAY)) {
      // disable the interrupt
      gpio_isr_handler_remove(pinNumber);

      // wait some time while we check for the button to be released
      while (gpio_get_level(pinNumber) == 1) {
        vTaskDelay(20 / portTICK_PERIOD_MS);
        ESP_LOGI(HMI_LOG_TAG, "Pressed, count : %d", count);
        count++;
        // Break the loop if count is greater than 600(10 Sec) and enter in OTA mode
        // if(count>=600) break;
      }
      if (count >= 5 && count <= 30) {
        // if pressed for 1 time
        change_screen_inc_logic();
        // Change Screen

        printf("Count Loop 3 to 5 : %s\n", &hmi_oled_screens[store_get_current_screen()]);
        count = 0;
      }
      if (count >= 150 && count <= 250) {
        // If pressed for 3 to 5 Seconds
        // set start monitoring
        store_set_monitoring_flag_status(!store_get_monitoring_flag_status());
        count = 0;
      }

      //do some work
      printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level((gpio_num_t)HMI_SWITCH_A));

      // re-enable the interrupt
      gpio_isr_handler_add(pinNumber, interrupt_gpio_isr_handler, (void *)pinNumber);
    }
  }
}

//-------------------------------------------------------OLED-------------------------------


void start_oled_task() {
  xTaskCreate(&oled_task, "oled_task", OLED_HMI_TASK_STACK_SIZE, NULL, OLED_HMI_TASK_PRIORITY, &task_oled_handle);
}

void oled_task(void *Params) {
  while (1) {
    if (hmi_oled_screens[store_get_current_screen()] == "ecg") {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      // Display static text
      display.println("Hello, world!");
      display.display();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}


void change_screen_inc_logic() {
  uint8_t cs = store_get_current_screen();
  if (cs >= OLED_SCREEN_COUNT - 1) {
    store_change_current_screen(1);
  } else {
    store_change_current_screen(cs + 1);
  }
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -LED INDICATION-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

                                                                                                     void
                                                                                                     start_led_indication_task() {
  xTaskCreate(&led_indication_task, "led_indication_task", LED_INDICATION_TASK_STACK_SIZE, NULL, LED_INDICATION_TASK_PRIORITY, &task_led_indication_handle);
}

void led_indication_task(void *Params) {
  pinMode(HMI_INDICATION_LED, OUTPUT);
  while (1) {
    if(store_get_wifi_status() == true && store_get_mqtt_status() == true&& store_get_monitoring_flag_status()==false){
      digitalWrite(HMI_INDICATION_LED, HIGH); 
    }
    else if(store_get_wifi_status() == true && store_get_mqtt_status() == true && store_get_monitoring_flag_status()==true){
      digitalWrite(HMI_INDICATION_LED, HIGH); 
      vTaskDelay(250/ portTICK_PERIOD_MS);               
      digitalWrite(HMI_INDICATION_LED, LOW);   
      vTaskDelay(250/ portTICK_PERIOD_MS);
    }
    else {
      digitalWrite(HMI_INDICATION_LED, HIGH); 
      vTaskDelay(1000/ portTICK_PERIOD_MS);               
      digitalWrite(HMI_INDICATION_LED, LOW);   
      vTaskDelay(1000/ portTICK_PERIOD_MS);               
    }
    vTaskDelay(100/ portTICK_PERIOD_MS);
  }
}
