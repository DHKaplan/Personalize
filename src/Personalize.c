#include "pebble.h"

#define PERSONALIZED_TEXT_INPUT 0
#define DATE_STYLE 1  
#define BT_VIBRATE 2  

Window      *window;

TextLayer   *text_personalized_layer;
TextLayer   *text_dayname_layer;
TextLayer   *text_date_layer;
TextLayer   *text_time_layer;
TextLayer   *text_battery_layer;

Layer       *LineLayer;
Layer       *window_layer;

Layer       *BTLayer;

GFont        fontHelvNewLight20;
GFont		     fontRobotoCondensed21;
GFont        fontMonaco19;
GFont        fontMonaco22;
GFont        fontMonaco24;
GFont        fontMonaco26;
GFont        fontMonaco30;
GFont        fontMonaco37;
GFont        fontRobotoBoldSubset49;

GRect        bluetooth_rect;

static int  batterychargepct;
static int  BatteryVibesDone = 0;
static int  batterycharging = 0;

static char personalized_text[20] = "Enter Data";

GPoint     Linepoint;
static int BTConnected = 1;
static int BTVibesDone = 0;
static char VibOnBTLoss[] = "0"; //From Config Page

static int FirstTime = 0;

static char date_type[]="us  ";
static char date_text[] = "Xxx 00       0000";
static char dayname_text[] = "XXXXXXXXXX";
static char time_text[] = "00:00";
static char seconds_text[] = "00";

static char date_format[]="%b %e, %Y";

GColor TextColorHold;
GColor BGColorHold;

void handle_bluetooth(bool connected) {
     if (connected) {
         BTConnected = 1;     // Connected
         BTVibesDone = 0;

    } else {
         BTConnected = 0;      // Not Connected

         if ((BTVibesDone == 0) && (strcmp(VibOnBTLoss,"0") == 0)) {    
             BTVibesDone = 1;
             vibes_long_pulse();
         }
    }
    layer_mark_dirty(BTLayer);
}

//BT Logo Callback;
void BTLine_update_callback(Layer *BTLayer, GContext* BT1ctx) {

       GPoint BTLinePointStart;
       GPoint BTLinePointEnd;

       graphics_context_set_stroke_color(BT1ctx, TextColorHold);

      if (BTConnected == 0) {
        #ifdef PBL_COLOR
            graphics_context_set_stroke_color(BT1ctx, GColorRed);
            graphics_context_set_fill_color(BT1ctx, GColorWhite);
            graphics_fill_rect(BT1ctx, layer_get_bounds(BTLayer), 0, GCornerNone);
        #else
           graphics_context_set_stroke_color(BT1ctx, GColorBlack);
           graphics_context_set_fill_color(BT1ctx, GColorWhite);
           graphics_fill_rect(BT1ctx, layer_get_bounds(BTLayer), 0, GCornerNone);
        #endif
        

        // "X"" Line 1
        BTLinePointStart.x = 1;
        BTLinePointStart.y = 1;

        BTLinePointEnd.x = 20;
        BTLinePointEnd.y = 20;
        graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);

        // "X"" Line 2
        BTLinePointStart.x = 1;
        BTLinePointStart.y = 20;

        BTLinePointEnd.x = 20;
        BTLinePointEnd.y = 1;
        graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
      }
      else
      {
       //Line 1
       BTLinePointStart.x = 10;
       BTLinePointStart.y = 1;

       BTLinePointEnd.x = 10;
       BTLinePointEnd.y = 20;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);

       //Line 2
       BTLinePointStart.x = 10;
       BTLinePointStart.y = 1;

       BTLinePointEnd.x = 16;
       BTLinePointEnd.y = 5;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);

       //Line 3
       BTLinePointStart.x = 4;
       BTLinePointStart.y = 5;

       BTLinePointEnd.x = 16;
       BTLinePointEnd.y = 16;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);

       //Line 4
       BTLinePointStart.x = 4;
       BTLinePointStart.y = 16;

       BTLinePointEnd.x = 16;
       BTLinePointEnd.y = 5;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);

       //Line 5
       BTLinePointStart.x = 10;
       BTLinePointStart.y = 20;

       BTLinePointEnd.x = 16;
       BTLinePointEnd.y = 16;
       graphics_draw_line(BT1ctx, BTLinePointStart, BTLinePointEnd);
      }
}

void handle_battery(BatteryChargeState charge_state) {
  static char BatteryPctTxt[] = "+100%";

  batterychargepct = charge_state.charge_percent;

  if (charge_state.is_charging) {
    batterycharging = 1;
  } else {
    batterycharging = 0;
  }

  // Reset if Battery > 20% ********************************
  if (batterychargepct > 20) {
     if (BatteryVibesDone == 1) {     //OK Reset to normal
         BatteryVibesDone = 0;
     }
  }

  //
  if (batterychargepct < 30) {
     if (BatteryVibesDone == 0) {            // Do Once
         BatteryVibesDone = 1;
         vibes_long_pulse();
      }
  }

   if (charge_state.is_charging) {
     strcpy(BatteryPctTxt, "Chrg");
  } else {
     snprintf(BatteryPctTxt, 5, "%d%%", charge_state.charge_percent);
  }
   text_layer_set_text(text_battery_layer, BatteryPctTxt);

  layer_mark_dirty(LineLayer);
}

void line_layer_update_callback(Layer *LineLayer, GContext* ctx) {
     graphics_context_set_fill_color(ctx, TextColorHold);
     graphics_fill_rect(ctx, layer_get_bounds(LineLayer), 3, GCornersAll);

     if (batterycharging == 1) {
       #ifdef PBL_COLOR
          graphics_context_set_fill_color(ctx, GColorBlue);
       #else
          graphics_context_set_fill_color(ctx, GColorBlack);
       #endif

       graphics_fill_rect(ctx, GRect(2, 1, 100, 4), 3, GCornersAll);

     } else if (batterychargepct > 20) {
       #ifdef PBL_COLOR
          graphics_context_set_fill_color(ctx, GColorGreen);
       #else
          graphics_context_set_fill_color(ctx, GColorBlack);
       #endif
       graphics_fill_rect(ctx, GRect(2, 1, batterychargepct, 4), 3, GCornersAll);
     } else {
      #ifdef PBL_COLOR
          graphics_context_set_fill_color(ctx, GColorRed);
       #else
          graphics_context_set_fill_color(ctx, GColorBlack);
       #endif
       graphics_fill_rect(ctx, GRect(2, 1, batterychargepct, 4), 3, GCornersAll);
     }
}

void handle_appfocus(bool in_focus){
    if (in_focus) {
        handle_bluetooth(bluetooth_connection_service_peek());
        handle_battery(battery_state_service_peek());
    }
}
void fill_in_personalized_text() {
 
  //Clear out area of largest text size for Bug fix V3.01:
  text_personalized_layer = text_layer_create(GRect(1, 27, 144, 42));
  text_layer_set_background_color(text_personalized_layer, BGColorHold);
  layer_add_child(window_layer, text_layer_get_layer(text_personalized_layer));

  if (strlen(personalized_text) >  12) {       //Shouldn't happen, but error trapping...
      strncpy(personalized_text, "TooMuchData", sizeof(personalized_text));
      text_personalized_layer = text_layer_create(GRect(1, 40, 144, 22));
      text_layer_set_font(text_personalized_layer, fontMonaco19);

    } else if (strlen(personalized_text) ==  0) {
      strncpy(personalized_text, "Enter Text", sizeof(personalized_text));
      text_personalized_layer = text_layer_create(GRect(1, 40, 144, 24));
      text_layer_set_font(text_personalized_layer, fontMonaco19);

    } else if (strlen(personalized_text) ==  11) {
      text_personalized_layer = text_layer_create(GRect(1, 40, 144, 24));
      text_layer_set_font(text_personalized_layer, fontMonaco19);

    } else if (strlen(personalized_text) == 10) {
      text_personalized_layer = text_layer_create(GRect(1,38, 144, 26));
      text_layer_set_font(text_personalized_layer, fontMonaco22);

    } else if (strlen(personalized_text) == 9)  {
      text_personalized_layer = text_layer_create(GRect(1, 36, 144, 28));
      text_layer_set_font(text_personalized_layer, fontMonaco24);

    } else if (strlen(personalized_text) == 8)  {
      text_personalized_layer = text_layer_create(GRect(1, 34, 144, 32));
      text_layer_set_font(text_personalized_layer, fontMonaco26);

    } else if (strlen(personalized_text) ==  7) {
      text_personalized_layer = text_layer_create(GRect(1, 34, 144, 38));
      text_layer_set_font(text_personalized_layer, fontMonaco30);

    } else  {  //6 or less
      text_personalized_layer = text_layer_create(GRect(1, 27, 144, 42));
      text_layer_set_font(text_personalized_layer, fontMonaco37);
  }

  text_layer_set_text_alignment(text_personalized_layer, GTextAlignmentCenter);
  text_layer_set_text(text_personalized_layer, personalized_text);
  text_layer_set_text_color(text_personalized_layer, TextColorHold);
  text_layer_set_background_color(text_personalized_layer, BGColorHold);
  layer_add_child(window_layer, text_layer_get_layer(text_personalized_layer));
}  

//       ******************** Main Loop **************************************************************

void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  char time_format[] = "%I:%M";

  strftime(seconds_text, sizeof(seconds_text), "%S", tick_time);
   
  if (clock_is_24h_style()) {
       strcpy(time_format,"%R");
     } else {
       strcpy(time_format,"%I:%M");
     }  
  
  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
       memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }
  
  if((strcmp(seconds_text,"00") == 0) || (FirstTime == 0)) {
     // Set day & date
     strftime(dayname_text, sizeof(dayname_text), "%A", tick_time);
     strftime(date_text,    sizeof(date_text), date_format, tick_time);
    
     text_layer_set_text(text_dayname_layer, dayname_text);
     text_layer_set_text(text_date_layer, date_text);
  }
  
  // Only update the day name & date when it's changed.
  if (units_changed & DAY_UNIT) {
     text_layer_set_text(text_dayname_layer, dayname_text);
     text_layer_set_text(text_date_layer, date_text);
  } 

     //Always set time  *****************************************************
  static char timeit[]="00:00:00";
  strftime(timeit, sizeof(timeit), "%I:%M:%S", tick_time);
  
  
  if((strcmp(seconds_text,"00") == 0) || (FirstTime == 0)) {
     text_layer_set_text(text_time_layer, time_text); 
  }  
  
  FirstTime = 1; 
}

//Receive Input from Config html page:
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "In Inbox received callback");
  
  char BTVibeConfig[] = "0";
    ;
  FirstTime = 0;
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    
    // Which key was received?
    switch(t->key) {
    case 0:      
      strcpy(personalized_text, (t->value->cstring));
      fill_in_personalized_text();
      break;
      
    case 1:
      strcpy(date_type, t->value->cstring); 
      
      if (strcmp(date_type, "us") == 0) {
         strcpy(date_format, "%b %e, %Y");
      } else {
         strcpy(date_format, "%e %b %Y");
      }
      text_layer_set_text(text_date_layer, date_text);
      break;
      
      case 2:
      strcpy(BTVibeConfig, t->value->cstring); 
      if (strcmp(BTVibeConfig, "0") == 0) {
         strcpy(VibOnBTLoss,"0");
      } else {
         strcpy(VibOnBTLoss,"1");
      }
      break;

    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  //APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  //APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void handle_deinit(void) {
  persist_write_string(PERSONALIZED_TEXT_INPUT, personalized_text);
  //APP_LOG(APP_LOG_LEVEL_WARNING, "In DeInit - Date Type: %s", date_type);

  persist_write_string(DATE_STYLE, date_type);
  
  persist_write_string(BT_VIBRATE, VibOnBTLoss);

  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  app_focus_service_unsubscribe();

  text_layer_destroy(text_time_layer);
  text_layer_destroy(text_date_layer);
  text_layer_destroy(text_personalized_layer);
  text_layer_destroy(text_dayname_layer);
  text_layer_destroy(text_battery_layer);

  layer_destroy(LineLayer);
  layer_destroy(BTLayer);

  fonts_unload_custom_font(fontHelvNewLight20);
  fonts_unload_custom_font(fontMonaco19);
  fonts_unload_custom_font(fontMonaco22);
  fonts_unload_custom_font(fontMonaco24);
  fonts_unload_custom_font(fontMonaco26);
  fonts_unload_custom_font(fontMonaco30);
  fonts_unload_custom_font(fontMonaco37);

  window_destroy(window);
}

//********************************** Handle Init **************************
void handle_init(void) {
  //Ideas Brass BG, Black Text
  //      Black BG, Yellow Text (ARRL?)
  //      DarkGreen BG, PastelYellow Text * * *
  //      ArmyGreen BG, PastelYellow Text

  GColor BGCOLOR   = COLOR_FALLBACK(GColorMidnightGreen, GColorBlack);
  BGColorHold = BGCOLOR;

  GColor TEXTCOLOR = COLOR_FALLBACK(GColorWhite, GColorWhite);
  TextColorHold = TEXTCOLOR;

  window = window_create();
  window_set_background_color(window, BGCOLOR);
  window_stack_push(window, true /* Animated */);

  fontHelvNewLight20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELV_NEW_LIGHT_20));
  fontMonaco19 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_19));
  fontMonaco22 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_22));
  fontMonaco24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_24));
  fontMonaco26 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_26));
  fontMonaco30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_30));
  fontMonaco37 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_37));

  window_layer = window_get_root_layer(window);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Dayname
  text_dayname_layer = text_layer_create(GRect(1, 65, 144, 168-65));
  text_layer_set_text_alignment(text_dayname_layer, GTextAlignmentCenter);
  text_layer_set_font(text_dayname_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_color(text_dayname_layer, TEXTCOLOR);
  text_layer_set_background_color(text_dayname_layer, BGCOLOR);
  layer_add_child(window_layer, text_layer_get_layer(text_dayname_layer));

  // Date
  text_date_layer = text_layer_create(GRect(8, 88, 144-8, 168-88));
  text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_color(text_date_layer, TEXTCOLOR);
  text_layer_set_background_color(text_date_layer, BGCOLOR);
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  // Persistant value Text:
  if(persist_exists(PERSONALIZED_TEXT_INPUT)) {
     persist_read_string(PERSONALIZED_TEXT_INPUT, personalized_text, sizeof(personalized_text));
  } else {
    strncpy(personalized_text, "Enter Text", sizeof(personalized_text)); //Default
  }

  fill_in_personalized_text();

  //Persistent Value Date Format:
  if (persist_exists(DATE_STYLE)) {
     persist_read_string(DATE_STYLE, date_type, sizeof(date_type));  
     //APP_LOG(APP_LOG_LEVEL_WARNING, "In Init: Persistent Exists - Date Type: %s", date_type);
  }  else {
     //APP_LOG(APP_LOG_LEVEL_WARNING, "In Init: No Persistent-Setting us default");
     strcpy(date_type, "us"); //Default
  } 
    
  if (strcmp(date_type, "us") == 0) {
       //APP_LOG(APP_LOG_LEVEL_WARNING, "In Init: Setting Date Type: %s", date_type);
         strcpy(date_format, "%b %e, %Y");
  } else {
         //APP_LOG(APP_LOG_LEVEL_WARNING, "In Init: Setting Date Type: %s", date_type);
         strcpy(date_format, "%e %b %Y");
  }
  
  //Persistent Value VibOnBTLoss
  if(persist_exists(BT_VIBRATE)) {
     persist_read_string(BT_VIBRATE, VibOnBTLoss, sizeof(VibOnBTLoss));  
  }  else {
     strcpy(VibOnBTLoss, "0"); // Default
  } 
  
  // Time of Day
  text_time_layer = text_layer_create(GRect(1, 116, 144, 168-116));
  text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_color(text_time_layer, TEXTCOLOR);
  text_layer_set_background_color(text_time_layer, BGCOLOR);
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  // Line
  GRect line_frame = GRect(22, 118, 104, 6);
  LineLayer = layer_create(line_frame);
  layer_set_update_proc(LineLayer, line_layer_update_callback);
  layer_add_child(window_layer, LineLayer);

  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

  //Bluetooth Logo Setup area
  GRect BTArea = GRect(1, 5, 20, 20);
  BTLayer = layer_create(BTArea);

  layer_add_child(window_layer, BTLayer);

  layer_set_update_proc(BTLayer, BTLine_update_callback);

  bluetooth_connection_service_subscribe(&handle_bluetooth);
  handle_bluetooth(bluetooth_connection_service_peek());


  // Battery Text Setup
  text_battery_layer = text_layer_create(GRect(85,2,55,30));
  text_layer_set_text_color(text_battery_layer, TEXTCOLOR);
  text_layer_set_background_color(text_battery_layer, BGCOLOR);
  text_layer_set_font(text_battery_layer, fontHelvNewLight20);
  text_layer_set_text_alignment(text_battery_layer, GTextAlignmentRight);

  battery_state_service_subscribe(&handle_battery);

  handle_battery(battery_state_service_peek());

  layer_add_child(window_layer, text_layer_get_layer(text_battery_layer));

  //app focus service subscribe
  app_focus_service_subscribe(&handle_appfocus);
}

int main(void) {
   handle_init();

   app_event_loop();

   handle_deinit();
}
