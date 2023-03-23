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
        store_set_interrupt_time_counter(count / 50 > 0 ? count / 50 : 0);
        // Break the loop if count is greater than 600(10 Sec) and enter in OTA mode
        // if(count>=600) break;
      }
      if (count >= 5 && count <= 30) {
        // if pressed for 1 time
        change_screen_inc_logic();
        // Change Screen

        printf("Count Loop 3 to 5 : %s\n", &hmi_oled_screens[store_get_current_screen()]);
        // count = 0;
      }
      if (count >= 150 && count <= 250) {
        // If pressed for 3 to 5 Seconds
        // set start monitoring
        store_set_monitoring_flag_status(!store_get_monitoring_flag_status());
      }
      count = 0;
      store_set_interrupt_time_counter(0);
      //do some work
      printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level((gpio_num_t)HMI_SWITCH_A));

      // re-enable the interrupt
      gpio_isr_handler_add(pinNumber, interrupt_gpio_isr_handler, (void *)pinNumber);
    }
  }
}

//-------------------------------------------------------OLED-------------------------------


void display_interrupt_count() {
  if (store_get_interrupt_time_counter() > 0) {
    display.setCursor(120, 10);
    display.println(" ");
    display.setTextSize(1);
    display.setCursor(120, 53);
    // Display static text
    display.println(store_get_interrupt_time_counter());
  }
}

void oled_ecg_task(void *Params) {
  while (1) {

    if (xSemaphoreTake(i2c_bus_mutex, 2000 / portTICK_PERIOD_MS)) {
      if (store_get_monitoring_flag_status() == true) {
        if (strcmp(hmi_oled_screens[store_get_current_screen()], "ecg") == 0) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0, 0);
          display.println("ECG Sensor Graph");

          // int ecg_value = store_get_ecg();

          // // Map ECG value to OLED display height
          // int display_value = map(ecg_value, 0, 4096, 0, SCREEN_HEIGHT);

          // // Draw graph on OLED display
          // display.drawLine(0, SCREEN_HEIGHT - display_value, 1, SCREEN_HEIGHT - display_value, WHITE);
          // display.display();
          int x = 0;
          for (int i = 0; i < SCREEN_WIDTH; i++) {
            float sensorValue = store_get_ecg();
            float y = map(sensorValue, 0, 4096, 0, SCREEN_HEIGHT);
            ESP_LOGI(SENSOR_TAG, "ECG : %f", y);
            //sensorValue * SCREEN_HEIGHT / 4095.0;
            display.drawLine(x, SCREEN_HEIGHT, x + 1, SCREEN_HEIGHT - y, WHITE);
            display_interrupt_count();
            x++;
            if (x >= SCREEN_WIDTH) {
              x = 0;
              display.clearDisplay();
              display.setCursor(0, 0);
              display.println("ECG Sensor Graph");
            }
            display.display();
          }
        }
      }
      xSemaphoreGive(i2c_bus_mutex);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void start_oled_task() {
  xTaskCreate(&oled_task, "oled_task", OLED_HMI_TASK_STACK_SIZE, NULL, OLED_HMI_TASK_PRIORITY, &task_oled_handle);
}

void oled_task(void *Params) {
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  // xTaskCreate(&oled_ecg_task, "oled_ecg_task", 2048, NULL, 4, NULL);
  while (1) {

    if (xSemaphoreTake(i2c_bus_mutex, 2000 / portTICK_PERIOD_MS)) {
      if (store_get_monitoring_flag_status() == true) {
        if (strcmp(hmi_oled_screens[store_get_current_screen()], "pulse_spo2") == 0) {
          display.clearDisplay();
          display.setTextSize(1.5);
          display.setTextColor(WHITE);
          display.setCursor(0, 10);
          // Display static text
          display.print("Pulse : ");
          display.print(floorf(store_get_pulse() * 100) / 100);
          display.println("/bpm");

          display.println(" ");

          display.print("SPO2 : ");
          display.print(floorf(store_get_spo2() * 100) / 100);
          display.println("%");


          display_interrupt_count();
          display.display();
        }

        else if (strcmp(hmi_oled_screens[store_get_current_screen()], "temperature") == 0) {
          display.clearDisplay();
          display.setTextSize(1.5);
          display.setTextColor(WHITE);
          display.setCursor(0, 10);
          // Display static text
          display.print("Temperature : ");
          display.print(floorf(store_get_temperature() * 100) / 100);
          display.println(" C");

          display.println(" ");

          display.print("Humidity : ");
          display.print(floorf(store_get_humidity() * 100) / 100);
          display.println(" %");

          display_interrupt_count();
          display.display();
        } else if (strcmp(hmi_oled_screens[store_get_current_screen()], "fall_alert") == 0) {
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(0, 10);
          // Display static text
          display.println("FALL Detected!!!");

          display_interrupt_count();
          display.display();
        }

      } else if (store_get_monitoring_flag_status() == false) {
        display.clearDisplay();
        display.setTextColor(WHITE);

        display.setTextSize(2);
        display.setCursor(20, 0);
        // Display static text
        display.println("WELCOME");

        if (store_get_interrupt_time_counter() > 0) {
          display.setTextSize(1);
          display.setCursor(120, 25);
          // Display static text
          display.println(store_get_interrupt_time_counter());
        }
        display.setTextSize(1);
        display.setCursor(0, 45);
        // Display static text
        display.println("Status:");
        display.println(store_get_init_screen_buffer());

        display.display();
      }
      // else {
      //   display.clearDisplay();
      //   display.setTextSize(1);
      //   display.setTextColor(WHITE);
      //   display.setCursor(0, 10);
      //   // Display static text
      //   display.println("Hello, world!");
      //   display.display();
      // }
      xSemaphoreGive(i2c_bus_mutex);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}


void change_screen_inc_logic() {
  uint8_t cs = store_get_current_screen();
  if (cs >= OLED_SCREEN_COUNT - 2) {
    store_change_current_screen(0);
  } 
  else {
    store_change_current_screen(cs + 1);
  }
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -LED INDICATION-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

void start_led_indication_task() {
  xTaskCreate(&led_indication_task, "led_indication_task", LED_INDICATION_TASK_STACK_SIZE, NULL, LED_INDICATION_TASK_PRIORITY, &task_led_indication_handle);
}

void led_indication_task(void *Params) {
  pinMode(HMI_INDICATION_LED, OUTPUT);
  while (1) {
    if (store_get_wifi_status() == true && store_get_mqtt_status() == true && store_get_monitoring_flag_status() == false) {
      digitalWrite(HMI_INDICATION_LED, HIGH);
    } else if (store_get_wifi_status() == true && store_get_mqtt_status() == true && store_get_monitoring_flag_status() == true) {
      digitalWrite(HMI_INDICATION_LED, HIGH);
      vTaskDelay(250 / portTICK_PERIOD_MS);
      digitalWrite(HMI_INDICATION_LED, LOW);
      vTaskDelay(250 / portTICK_PERIOD_MS);
    } else {
      digitalWrite(HMI_INDICATION_LED, HIGH);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      digitalWrite(HMI_INDICATION_LED, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
