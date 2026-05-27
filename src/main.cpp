#include "lvgl.h"

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

static void slider_event_cb(lv_event_t * e)
{
    // On récupère le slider qui a déclenché l'événement
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    
    // On récupère le label qu'on a passé en user_data
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);

    // On lit la valeur actuelle du slider
    int32_t value = lv_slider_get_value(slider);

    // On met à jour le texte du label avec la nouvelle valeur
    lv_label_set_text_fmt(label, "Valeur : %d", (int)value);
}

void gestionScreen()
{
  // Initialisations générales
  lv_obj_t * label = lv_label_create(lv_screen_active());

  lv_label_set_text(label, "Valeur : 0");
  lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, -100); 

  lv_obj_t * slider = lv_slider_create(lv_screen_active());
  lv_slider_set_range(slider, 0, 10);
  lv_obj_set_size(slider, 20, 150);
  lv_obj_center(slider);

  lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, label);
  
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x2D2D2D), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
}

#ifdef ARDUINO

#include "lvglDrivers.h"

// à décommenter pour tester la démo
// #include "demos/lv_demos.h"

void mySetup()
{
  // à décommenter pour tester la démo
  // lv_demo_widgets();

  // Initialisations générales
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
