#include "lvgl.h"
#include <Wire.h>
#include <SPI.h>

LV_IMAGE_DECLARE(pot100x100);
LV_IMAGE_DECLARE(sliderKnob40x25);

const int SPI_CS_PIN = 3;
const uint32_t MCP42010_SPI_SPEED = 2000000;
const uint8_t MCP42010_POT_0 = 0x01;
const uint8_t MCP42010_POT_1 = 0x02;
const uint8_t MCP42010_POT_BOTH = 0x03;
const uint8_t MCP42010_CMD_WRITE = 0x10;
const uint8_t MCP42010_CMD_SHUTDOWN = 0x20;

lv_obj_t * sliderTr1;
lv_obj_t * sliderMid1;
lv_obj_t * sliderBass1;
lv_obj_t * volume1;
lv_obj_t * btn_casque1;
lv_obj_t * led_mute1;
lv_obj_t * volume2;
lv_obj_t * btn_casque2;
lv_obj_t * led_mute2;

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
  mcp42010SetPot(MCP42010_POT_BOTH, 128);
  Serial.println("MCP42010 Initialise");
}

void tdaWrite(uint8_t reg, uint8_t val)
{
  Wire.beginTransmission(0x44);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void init_TDA7439()
{
  Wire.begin();
  tdaWrite(0x00, 0x03);
  tdaWrite(0x01, 0x00);
  tdaWrite(0x02, 0x08);
  tdaWrite(0x03, 0x07);
  tdaWrite(0x04, 0x07);
  tdaWrite(0x05, 0x07);
  tdaWrite(0x06, 0x00);
  tdaWrite(0x07, 0x00);
  Serial.println("TDA7439 Initialise");
}

static void arc_rotation_cb(lv_event_t * e)
{
  lv_obj_t * arc = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t * img_knob = (lv_obj_t *)lv_event_get_user_data(e);
  int32_t val = lv_arc_get_value(arc);
  int32_t rotation_relative = (val * 270) / 255;
  int32_t offset = 10;
  int32_t angle_final = rotation_relative + offset;
  
  if (angle_final < 0) angle_final += 360;
  if (angle_final >= 360) angle_final -= 360;
  lv_image_set_rotation(img_knob, angle_final * 10);
}

static void mute_event_cb(lv_event_t * e)
{
  lv_obj_t * target = (lv_obj_t *)lv_event_get_target(e);
  
  if (target == btn_casque1)
  {
    if(lv_obj_has_state(target, LV_STATE_CHECKED))
    {
      mcp42010Shutdown(MCP42010_POT_0);
      lv_led_on(led_mute1);
    }
    else
    {
      mcp42010SetPot(MCP42010_POT_0, lv_arc_get_value(volume1));
      lv_led_off(led_mute1);
    }
  }
  else if (target == btn_casque2)
  {
    if(lv_obj_has_state(target, LV_STATE_CHECKED))
    {
      mcp42010Shutdown(MCP42010_POT_1);
      lv_led_on(led_mute2);
    }
    else
    {
      mcp42010SetPot(MCP42010_POT_1, lv_arc_get_value(volume2));
      lv_led_off(led_mute2);
    }
  }
}

void gestionScreen()
{
  static lv_style_t style_arc_bg, style_arc_indicator, style_arc_knob;
  static lv_style_t style_fader_bg, style_fader_indicator, style_fader_knob;
  static bool styles_inited = false;

  if(!styles_inited)
  {
    lv_style_init(&style_arc_bg);
    lv_style_set_arc_color(&style_arc_bg, lv_color_hex(0x2A1115));
    lv_style_set_arc_width(&style_arc_bg, 15);
    lv_style_set_arc_rounded(&style_arc_bg, false);

    lv_style_init(&style_arc_indicator);
    lv_style_set_arc_color(&style_arc_indicator, lv_color_hex(0x9B111E));
    lv_style_set_arc_width(&style_arc_indicator, 15);
    lv_style_set_arc_rounded(&style_arc_indicator, false);

    lv_style_init(&style_arc_knob);
    lv_style_set_opa(&style_arc_knob, 0);

    lv_style_init(&style_fader_bg);
    lv_style_set_bg_color(&style_fader_bg, lv_color_hex(0x0A0A0A));
    lv_style_set_radius(&style_fader_bg, 0);

    lv_style_init(&style_fader_indicator);
    lv_style_set_bg_color(&style_fader_indicator, lv_color_hex(0x9B111E));
    lv_style_set_radius(&style_fader_indicator, 0);

    lv_style_init(&style_fader_knob);
    lv_style_set_bg_opa(&style_fader_knob, 0);
    lv_style_set_bg_image_src(&style_fader_knob, &sliderKnob40x25);
    lv_style_set_pad_left(&style_fader_knob, 22);
    lv_style_set_pad_right(&style_fader_knob, 22);
    lv_style_set_pad_top(&style_fader_knob, 18);
    lv_style_set_pad_bottom(&style_fader_knob, 18);

    styles_inited = true;
  }

  lv_obj_t * treble = lv_label_create(lv_screen_active());
  lv_label_set_text(treble, "Treble");
  lv_obj_set_style_text_color(treble, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(treble, LV_ALIGN_TOP_LEFT, 40, 20);

  lv_obj_t * mid = lv_label_create(lv_screen_active());
  lv_label_set_text(mid, "Mid");
  lv_obj_set_style_text_color(mid, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(mid, LV_ALIGN_TOP_LEFT, 130, 20);

  lv_obj_t * bass = lv_label_create(lv_screen_active());
  lv_label_set_text(bass, "Bass");
  lv_obj_set_style_text_color(bass, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(bass, LV_ALIGN_TOP_LEFT, 210, 20);

  #define APPLY_FADER_STYLE(obj) \
      lv_obj_add_style(obj, &style_fader_bg, LV_PART_MAIN); \
      lv_obj_add_style(obj, &style_fader_indicator, LV_PART_INDICATOR); \
      lv_obj_add_style(obj, &style_fader_knob, LV_PART_KNOB)

  sliderTr1 = lv_slider_create(lv_screen_active());
  lv_slider_set_range(sliderTr1, 0, 15);
  lv_slider_set_value(sliderTr1, 8, LV_ANIM_OFF);
  lv_obj_set_size(sliderTr1, 8, 180);
  lv_obj_align_to(sliderTr1, treble, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
  APPLY_FADER_STYLE(sliderTr1);

  sliderMid1 = lv_slider_create(lv_screen_active());
  lv_slider_set_range(sliderMid1, 0, 15);
  lv_slider_set_value(sliderMid1, 8, LV_ANIM_OFF);
  lv_obj_set_size(sliderMid1, 8, 180);
  lv_obj_align_to(sliderMid1, mid, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
  APPLY_FADER_STYLE(sliderMid1);

  sliderBass1 = lv_slider_create(lv_screen_active());
  lv_slider_set_range(sliderBass1, 0, 15);
  lv_slider_set_value(sliderBass1, 8, LV_ANIM_OFF);
  lv_obj_set_size(sliderBass1, 8, 180);
  lv_obj_align_to(sliderBass1, bass, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
  APPLY_FADER_STYLE(sliderBass1);

  lv_obj_t * vol_panel = lv_obj_create(lv_screen_active());
  lv_obj_set_size(vol_panel, 150, 270);
  lv_obj_align(vol_panel, LV_ALIGN_RIGHT_MID, -30, 0);
  lv_obj_set_style_bg_color(vol_panel, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_radius(vol_panel, 10, LV_PART_MAIN);
  lv_obj_set_style_border_width(vol_panel, 0, LV_PART_MAIN);
  lv_obj_remove_flag(vol_panel, LV_OBJ_FLAG_SCROLLABLE);

  volume1 = lv_arc_create(vol_panel);
  lv_arc_set_range(volume1, 0, 255);
  lv_arc_set_value(volume1, 128);
  lv_arc_set_bg_angles(volume1, 135, 45);
  lv_obj_set_size(volume1, 125, 125);
  lv_obj_remove_flag(volume1, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(volume1, LV_ALIGN_CENTER, 0, -65);
  lv_obj_add_style(volume1, &style_arc_bg, LV_PART_MAIN);
  lv_obj_add_style(volume1, &style_arc_indicator, LV_PART_INDICATOR);
  lv_obj_add_style(volume1, &style_arc_knob, LV_PART_KNOB);

  btn_casque1 = lv_image_create(volume1);
  lv_image_set_src(btn_casque1, &pot100x100);
  lv_obj_align(btn_casque1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_flag(btn_casque1, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(btn_casque1, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_event_cb(btn_casque1, mute_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(volume1, arc_rotation_cb, LV_EVENT_VALUE_CHANGED, btn_casque1);
  lv_obj_send_event(volume1, LV_EVENT_VALUE_CHANGED, NULL);

  led_mute1 = lv_led_create(vol_panel);
  lv_led_set_color(led_mute1, lv_color_hex(0xFF0000));
  lv_obj_set_size(led_mute1, 16, 16);
  lv_obj_align_to(led_mute1, volume1, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_led_off(led_mute1);

  volume2 = lv_arc_create(vol_panel);
  lv_arc_set_range(volume2, 0, 255);
  lv_arc_set_value(volume2, 128);
  lv_arc_set_bg_angles(volume2, 135, 45);
  lv_obj_set_size(volume2, 125, 125);
  lv_obj_remove_flag(volume2, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(volume2, LV_ALIGN_CENTER, 0, 65);
  lv_obj_add_style(volume2, &style_arc_bg, LV_PART_MAIN);
  lv_obj_add_style(volume2, &style_arc_indicator, LV_PART_INDICATOR);
  lv_obj_add_style(volume2, &style_arc_knob, LV_PART_KNOB);

  btn_casque2 = lv_image_create(volume2);
  lv_image_set_src(btn_casque2, &pot100x100);
  lv_obj_align(btn_casque2, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_flag(btn_casque2, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(btn_casque2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_event_cb(btn_casque2, mute_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(volume2, arc_rotation_cb, LV_EVENT_VALUE_CHANGED, btn_casque2);
  lv_obj_send_event(volume2, LV_EVENT_VALUE_CHANGED, NULL);

  led_mute2 = lv_led_create(vol_panel);
  lv_led_set_color(led_mute2, lv_color_hex(0xFF0000));
  lv_obj_set_size(led_mute2, 16, 16);
  lv_obj_align_to(led_mute2, volume2, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_led_off(led_mute2);

  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x2D2D2D), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);
}

#ifdef ARDUINO
#include "lvglDrivers.h"

void mySetup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Demarrage du systeme");
  init_TDA7439();
  init_MCP42010();
  gestionScreen();
}

void loop()
{
}

void myTask(void *pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  int32_t old_treble1 = -1;
  int32_t old_mid1 = -1;
  int32_t old_bass1 = -1;
  int32_t old_vol1 = -1;
  int32_t old_vol2 = -1;
  
  while (1)
  {
    lv_lock();
    int32_t current_treble1 = lv_slider_get_value(sliderTr1);
    int32_t current_mid1 = lv_slider_get_value(sliderMid1);
    int32_t current_bass1 = lv_slider_get_value(sliderBass1);
    int32_t current_vol1 = lv_arc_get_value(volume1);
    int32_t current_vol2 = lv_arc_get_value(volume2);
    lv_unlock();

    if (current_treble1 != old_treble1)
    {
      tdaWrite(0x05, (uint8_t)current_treble1);
      old_treble1 = current_treble1;
    }
    
    if (current_mid1 != old_mid1)
    {
      tdaWrite(0x04, (uint8_t)current_mid1);
      old_mid1 = current_mid1;
    }
    
    if (current_bass1 != old_bass1)
    {
      tdaWrite(0x03, (uint8_t)current_bass1);
      old_bass1 = current_bass1;
    }
    
    if (current_vol1 != old_vol1 && !lv_obj_has_state(btn_casque1, LV_STATE_CHECKED))
    {
      mcp42010SetPot(MCP42010_POT_0, (uint8_t)current_vol1);
      old_vol1 = current_vol1;
    }
    
    if (current_vol2 != old_vol2 && !lv_obj_has_state(btn_casque2, LV_STATE_CHECKED))
    {
      mcp42010SetPot(MCP42010_POT_1, (uint8_t)current_vol2);
      old_vol2 = current_vol2;
    }

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200));
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
  gestionScreen();
  hal_loop();
  return 0;
}

#endif