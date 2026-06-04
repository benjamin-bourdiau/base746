#include "lvgl.h"
#include <Wire.h>
#include <SPI.h>

const int SPI_CS_PIN = 3;

const uint32_t MCP42010_SPI_SPEED = 2000000;

const uint8_t MCP42010_POT_0 = 0x01;
const uint8_t MCP42010_POT_1 = 0x02;
const uint8_t MCP42010_POT_BOTH = 0x03;

const uint8_t MCP42010_CMD_WRITE = 0x10;
const uint8_t MCP42010_CMD_SHUTDOWN = 0x20;

/*static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);

    int32_t value = lv_slider_get_value(slider);

    lv_label_set_text_fmt(label, "Valeur : %d", (int)value);
}*/

void mcp42010Write(uint8_t command, uint8_t value)
{
  SPI.beginTransaction(SPISettings(MCP42010_SPI_SPEED, MSBFIRST, SPI_MODE0));

  digitalWrite(SPI_CS_PIN, LOW);
  SPI.transfer(command);
  SPI.transfer(value);
  digitalWrite(SPI_CS_PIN, HIGH);

  SPI.endTransaction();
}

void mcp42010SetPot(uint8_t pot, uint8_t value)
{
  mcp42010Write(MCP42010_CMD_WRITE | pot, value);
}

void mcp42010Shutdown(uint8_t pot)
{
  mcp42010Write(MCP42010_CMD_SHUTDOWN | pot, 0x00);
}

void init_MCP42010()
{
  pinMode(SPI_CS_PIN, OUTPUT);
  digitalWrite(SPI_CS_PIN, HIGH);

  SPI.begin();

  mcp42010SetPot(MCP42010_POT_BOTH, 0x80); //volume milieu
}

void tdaWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(0x44);
  Wire.write(reg);
  Wire.write(val);
  Serial.println(Wire.endTransmission());
}

void init_TDA7439() {
  tdaWrite(0x00, 0x03); // IN1
  tdaWrite(0x01, 0x00); // input gain 0 dB
  tdaWrite(0x02, 0x08); // volume -8 dB
  tdaWrite(0x03, 0x07); // bass 0 dB
  tdaWrite(0x04, 0x07); // mid 0 dB
  tdaWrite(0x05, 0x07); // treble 0 dB
  tdaWrite(0x06, 0x00); // attenuation R 0 dB
  tdaWrite(0x07, 0x00); // attenuation L 0 dB
}

void gestionScreen()
{
  lv_obj_t * treble = lv_label_create(lv_screen_active());
  lv_label_set_text(treble, "Treble");
  lv_obj_set_style_text_color(treble, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(treble, LV_ALIGN_TOP_LEFT, 17, 5);

  lv_obj_t * mid = lv_label_create(lv_screen_active());
  lv_label_set_text(mid, "Mid");
  lv_obj_set_style_text_color(mid, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(mid, LV_ALIGN_TOP_LEFT, 127, 5);

  lv_obj_t * bass = lv_label_create(lv_screen_active());
  lv_label_set_text(bass, "Bass");
  lv_obj_set_style_text_color(bass, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(bass, LV_ALIGN_TOP_MID, 0, 5);

  lv_obj_t * sliderTr1 = lv_slider_create(lv_screen_active());
  lv_slider_set_range(sliderTr1, 0, 10);
  lv_obj_set_size(sliderTr1, 20, 100);
  lv_obj_align(sliderTr1, LV_ALIGN_TOP_LEFT, 30, 25);

  lv_obj_t * sliderMid1 = lv_slider_create(lv_screen_active());
  lv_slider_set_range(sliderMid1, 0, 10);
  lv_obj_set_size(sliderMid1, 20, 100);
  lv_obj_align(sliderMid1, LV_ALIGN_TOP_LEFT, 130, 25);

  lv_obj_t * sliderBass1 = lv_slider_create(lv_screen_active());
  lv_slider_set_range(sliderBass1, 0, 10);
  lv_obj_set_size(sliderBass1, 20, 100);
  lv_obj_align(sliderBass1, LV_ALIGN_TOP_MID, 0, 25);

  lv_obj_t * sliderTr2 = lv_slider_create(lv_screen_active());
  lv_slider_set_range(sliderTr2, 0, 10);
  lv_obj_set_size(sliderTr2, 20, 100);
  lv_obj_align(sliderTr2, LV_ALIGN_BOTTOM_LEFT, 30, -15);

  lv_obj_t * sliderMid2 = lv_slider_create(lv_screen_active());
  lv_slider_set_range(sliderMid2, 0, 10);
  lv_obj_set_size(sliderMid2, 20, 100);
  lv_obj_align(sliderMid2, LV_ALIGN_BOTTOM_LEFT, 130, -15);

  lv_obj_t * sliderBass2 = lv_slider_create(lv_screen_active());
  lv_slider_set_range(sliderBass2, 0, 10);
  lv_obj_set_size(sliderBass2, 20, 100);
  lv_obj_align(sliderBass2, LV_ALIGN_BOTTOM_MID, 0, -15);

  lv_obj_t * vol_panel = lv_obj_create(lv_screen_active());
  lv_obj_set_size(vol_panel, 150, 250); 
  lv_obj_align(vol_panel, LV_ALIGN_RIGHT_MID, -30, 0);
  
  lv_obj_set_style_bg_color(vol_panel, lv_color_hex(0x000000), LV_PART_MAIN); 
  lv_obj_set_style_radius(vol_panel, 10, LV_PART_MAIN);
  lv_obj_set_style_border_width(vol_panel, 0, LV_PART_MAIN);
  lv_obj_set_scrollbar_mode(vol_panel, LV_SCROLLBAR_MODE_OFF); 

  /*lv_obj_t * vol = lv_label_create(vol_panel);
  lv_label_set_text(vol, "Volume");
  lv_obj_set_style_text_color(vol, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(vol, LV_ALIGN_TOP_MID, 0, 0); */

  lv_obj_t * volume1 = lv_arc_create(vol_panel);
  lv_arc_set_range(volume1, 0, 10);
  lv_arc_set_bg_angles(volume1, 135, 45);
  lv_obj_set_size(volume1, 110, 110);
  lv_obj_remove_flag(volume1, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(volume1, LV_ALIGN_CENTER, 0, -55); 

  lv_obj_t * volume2 = lv_arc_create(vol_panel);
  lv_arc_set_range(volume2, 0, 10);
  lv_arc_set_bg_angles(volume2, 135, 45);
  lv_obj_set_size(volume2, 110, 110);
  lv_obj_remove_flag(volume2, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(volume2, LV_ALIGN_CENTER, 0, 65); 

  /*lv_obj_add_event_cb(sliderTr1, slider_event_cb, LV_EVENT_VALUE_CHANGED, label);
  lv_obj_add_event_cb(sliderMid1, slider_event_cb, LV_EVENT_VALUE_CHANGED, label);
  lv_obj_add_event_cb(sliderBass1, slider_event_cb, LV_EVENT_VALUE_CHANGED, label);
  lv_obj_add_event_cb(sliderTr2, slider_event_cb, LV_EVENT_VALUE_CHANGED, label);
  lv_obj_add_event_cb(sliderMid2, slider_event_cb, LV_EVENT_VALUE_CHANGED, label);
  lv_obj_add_event_cb(sliderBass2, slider_event_cb, LV_EVENT_VALUE_CHANGED, label);*/
  
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x2D2D2D), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
}

#ifdef ARDUINO

#include "lvglDrivers.h"

void mySetup()
{
  init_TDA7439();
  init_MCP42010();

  mcp42010Shutdown(MCP42010_POT_BOTH);

  gestionScreen();
}

void loop()
{
  // Inactif (pour mise en veille du processeur)
}

void myTask(void *pvParameters)
{
  // Init
  TickType_t xLastWakeTime;
  // Lecture du nombre de ticks quand la tâche débute
  xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    // Loop

    // Endort la tâche pendant le temps restant par rapport au réveil,
    // ici 200ms, donc la tâche s'effectue toutes les 200ms
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200)); // toutes les 200 ms
  }
}

#else

#include "lvgl.h"
#include "app_hal.h"
#include <cstdio>

int main(void)
{
  printf("LVGL Simulator\n");
  fflush(stdout);

  lv_init();
  hal_setup();

  testLvgl();

  hal_loop();
  return 0;
}

#endif
