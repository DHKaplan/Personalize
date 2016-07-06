#include "pebble.h"

Window      *window;

TextLayer   *text_personalized_layer;
TextLayer   *text_personalize_bg_layer;
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

GPoint     Linepoint;
static int BTConnected = 1;
static int BTVibesDone = 0;
 
static int  PersistBGColor        = 0;
static int  PersistTextColor      = 0;
static char PersistDateFormat[]   = "0";   // 0 = US, 1 = Intl
static int  PersistBTLoss         = 0;     // 0 = No Vib, 1 = Vib
static int  PersistLow_Batt       = 0;     // 0 = No Vib, 1 = Vib
static char PersistPersonalized_Text[13] = "Enter Text"; 

static int FirstTime = 0;


static char date_text[] = "Xxx 00       0000";
static char dayname_text[] = "XXXXXXXXXX";
static char time_text[] = "00:00";
static char seconds_text[] = "00";

static char date_format[]="%b %e, %Y";

GColor GTextColorHold;
GColor GBGColorHold;
GColor ColorHold;

void handle_bluetooth(bool connected) {
     if (connected) {
         BTConnected = 1;     // Connected
         BTVibesDone = 0;

    } else {
         BTConnected = 0;      // Not Connected

         if ((BTVibesDone == 0) && (PersistBTLoss == 1)) {    
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

       graphics_context_set_stroke_color(BT1ctx, GTextColorHold);
       graphics_context_set_fill_color(BT1ctx, GBGColorHold); 
        
  
    if (BTConnected == 0) {
            graphics_context_set_fill_color(BT1ctx, GColorWhite);
      
        #ifdef PBL_COLOR
            graphics_context_set_stroke_color(BT1ctx, GColorRed);
            graphics_fill_rect(BT1ctx, layer_get_bounds(BTLayer), 0, GCornerNone);
        #else
           graphics_context_set_stroke_color(BT1ctx, GColorBlack);
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
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery Charging");
  } else {
    batterycharging = 0;
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery *NOT* Charging");
  }

  // Reset if Battery > 20% ********************************
  if ((batterychargepct > 20) && (batterycharging == 0)) {
      BatteryVibesDone = 0;

      text_layer_set_background_color(text_battery_layer, GBGColorHold);
      text_layer_set_text_color(text_battery_layer, GTextColorHold);       
  }
  
  if ((batterychargepct < 30) && (batterycharging == 0)) {
     if (BatteryVibesDone == 0) {            // Do Once
         BatteryVibesDone = 1;
         #ifdef PBL_COLOR
           text_layer_set_text_color(text_battery_layer, GColorRed);
         #else
           text_layer_set_text_color(text_battery_layer, GColorBlack);
         #endif
       
         text_layer_set_background_color(text_battery_layer, GColorWhite);
       
         APP_LOG(APP_LOG_LEVEL_WARNING, "Battery Vibes Sent");
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
     graphics_context_set_fill_color(ctx, GColorWhite);
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
  
  //Battery % Markers
      #ifdef PBL_COLOR
        graphics_context_set_fill_color(ctx, GColorBlack);
      #else
        if(batterycharging == 1) {
            graphics_context_set_fill_color(ctx, GColorBlack);
        } else {
            graphics_context_set_fill_color(ctx, GColorWhite);
        }
      #endif
  
      graphics_fill_rect(ctx, GRect(89, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(ctx, GRect(79, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(ctx, GRect(69, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(ctx, GRect(59, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(ctx, GRect(49, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(ctx, GRect(39, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(ctx, GRect(29, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(ctx, GRect(19, 1, 3, 4), 3, GCornerNone);
      graphics_fill_rect(ctx, GRect(9,  1, 3, 4), 3, GCornerNone);

}

void handle_appfocus(bool in_focus){
    if (in_focus) {
        handle_bluetooth(bluetooth_connection_service_peek());
        handle_battery(battery_state_service_peek());
    }
}
void fill_in_personalized_text() {
  //int intCharCount = 0;
  
  //APP_LOG(APP_LOG_LEVEL_ERROR, "In fill_in_personalized_text");
  
  //Clear out area of largest text size for Bug fix V3.01/4.0:
  #ifdef PBL_PLATFORM_CHALK
      text_personalize_bg_layer = text_layer_create(GRect(1, 25, 180, 44));
  #else
      text_personalize_bg_layer = text_layer_create(GRect(1, 25, 144, 44));
  #endif
  text_layer_set_background_color(text_personalize_bg_layer, GBGColorHold);
  
  layer_add_child(window_layer, text_layer_get_layer(text_personalize_bg_layer));
  
  //Process Personalized Text
  //intCharCount = strlen(PersistPersonalized_Text);
    
  if (strlen(PersistPersonalized_Text) >  11) {       //Shouldn't happen, but error trapping...
      strncpy(PersistPersonalized_Text, "11 Char Max", sizeof(PersistPersonalized_Text));
      APP_LOG(APP_LOG_LEVEL_ERROR, "    In fill_in_personalized_text, Chars > 12, %s", PersistPersonalized_Text);
      #ifdef PBL_PLATFORM_CHALK
          text_personalized_layer = text_layer_create(GRect(1, 40, 180, 22));
      #else
          text_personalized_layer = text_layer_create(GRect(1, 40, 144, 22)); 
      #endif
        
      text_layer_set_font(text_personalized_layer, fontMonaco19);

    } else if (strlen(PersistPersonalized_Text) ==  0)    { //Shouldn't happen, but error trapping...
      strncpy(PersistPersonalized_Text, "Enter Text", sizeof(PersistPersonalized_Text));
      APP_LOG(APP_LOG_LEVEL_ERROR, "    In fill_in_personalized_text, Chars = 0 , %s", PersistPersonalized_Text);
      #ifdef PBL_PLATFORM_CHALK
          text_personalized_layer = text_layer_create(GRect(1, 40, 180, 24));
      #else
          text_personalized_layer = text_layer_create(GRect(1, 40, 144, 24));
      #endif 
        
      text_layer_set_font(text_personalized_layer, fontMonaco19);

    } else if (strlen(PersistPersonalized_Text) ==  11) {
      APP_LOG(APP_LOG_LEVEL_INFO, "    In fill_in_personalized_text, Chars = 11, %s", PersistPersonalized_Text);
      #ifdef PBL_PLATFORM_CHALK
          text_personalized_layer = text_layer_create(GRect(1, 40, 180, 29));
      #else
          text_personalized_layer = text_layer_create(GRect(1, 40, 144, 29));
      #endif
        
      text_layer_set_font(text_personalized_layer, fontMonaco19);

    } else if (strlen(PersistPersonalized_Text) == 10) {
      APP_LOG(APP_LOG_LEVEL_INFO, "    In fill_in_personalized_text, Chars = 10, %s", PersistPersonalized_Text);    
      #ifdef PBL_PLATFORM_CHALK
          text_personalized_layer = text_layer_create(GRect(1, 38, 180, 31));
      #else
          text_personalized_layer = text_layer_create(GRect(1, 38, 144, 31));
      #endif

      text_layer_set_font(text_personalized_layer, fontMonaco22);

    } else if (strlen(PersistPersonalized_Text) == 9)  {
      APP_LOG(APP_LOG_LEVEL_INFO, "    In fill_in_personalized_text, Chars = 9, %s", PersistPersonalized_Text);    
      #ifdef PBL_PLATFORM_CHALK
          text_personalized_layer = text_layer_create(GRect(1, 36, 180, 33));
      #else
          text_personalized_layer = text_layer_create(GRect(1, 36, 144, 33));
      #endif
        
      text_layer_set_font(text_personalized_layer, fontMonaco24);
    

    } else if (strlen(PersistPersonalized_Text) == 8)  {
      APP_LOG(APP_LOG_LEVEL_INFO, "    In fill_in_personalized_text, Chars = 8, %s", PersistPersonalized_Text);          
      #ifdef PBL_PLATFORM_CHALK
          text_personalized_layer = text_layer_create(GRect(1, 34, 180, 35));      
      #else
          text_personalized_layer = text_layer_create(GRect(1, 34, 144, 35));      
      #endif

      text_layer_set_font(text_personalized_layer, fontMonaco26);

    } else if (strlen(PersistPersonalized_Text) ==  7) {
      APP_LOG(APP_LOG_LEVEL_INFO, "    In fill_in_personalized_text, Chars = 7, %s", PersistPersonalized_Text);    
      #ifdef PBL_PLATFORM_CHALK
          text_personalized_layer = text_layer_create(GRect(1, 32, 180, 35));
      #else
          text_personalized_layer = text_layer_create(GRect(1, 32, 144, 35));
      #endif
    
      text_layer_set_font(text_personalized_layer, fontMonaco30);

    } else  {  //6 or less
      APP_LOG(APP_LOG_LEVEL_INFO, "    In fill_in_personalized_text, Chars <= 6, %s", PersistPersonalized_Text);        
      #ifdef PBL_PLATFORM_CHALK
          text_personalized_layer = text_layer_create(GRect(1, 25, 180, 44));
      #else
          text_personalized_layer = text_layer_create(GRect(1, 25, 144, 44));
      #endif
    
      text_layer_set_font(text_personalized_layer, fontMonaco37);
  }


  text_layer_set_background_color(text_personalized_layer, GBGColorHold);
  text_layer_set_text_color(text_personalized_layer, GTextColorHold);
  text_layer_set_text_alignment(text_personalized_layer, GTextAlignmentCenter);
  text_layer_set_text(text_personalized_layer, PersistPersonalized_Text);
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

//Receive Input from Config html page: * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "In Inbox received callback * * * * * * *");
    
  FirstTime = 0;
  
  #ifdef PBL_COLOR
          Tuple *BG_Color = dict_find(iterator, MESSAGE_KEY_BG_COLOR_KEY);     
            
         if (BG_Color) { // Config Exists
             PersistBGColor = BG_Color->value->int32;
             APP_LOG(APP_LOG_LEVEL_INFO,      "    Added Config Local BG Color: %d", PersistBGColor);   
         } else { //Check for Persist
               if(persist_exists(MESSAGE_KEY_BG_COLOR_KEY)) {
                  PersistBGColor = persist_read_int(MESSAGE_KEY_BG_COLOR_KEY);
                  APP_LOG(APP_LOG_LEVEL_INFO, "    Added Persistant Local BG Color = %d", PersistBGColor);
               }  else {   // Set Default
                  PersistBGColor = 255; 
                  APP_LOG(APP_LOG_LEVEL_INFO, "    Added Default Local BG Color %d", PersistBGColor);
               }
            }
         
         ColorHold = GColorFromHEX(PersistBGColor);
         persist_write_int(MESSAGE_KEY_BG_COLOR_KEY,   PersistBGColor);
        
       #else
         ColorHold = GColorBlack;
       #endif
  
        GBGColorHold = ColorHold;
   
        text_layer_set_background_color(text_battery_layer, ColorHold);
        text_layer_set_background_color(text_time_layer, ColorHold);
        text_layer_set_background_color(text_dayname_layer, ColorHold);
        text_layer_set_background_color(text_date_layer, ColorHold);
        text_layer_set_background_color(text_personalized_layer, ColorHold);
        window_set_background_color(window, ColorHold);
  
  //*****
  
    #ifdef PBL_COLOR
        Tuple *Text_Color =  dict_find(iterator, MESSAGE_KEY_TEXT_COLOR_KEY);
 
         if (Text_Color) { // Config Exists
             PersistTextColor = Text_Color->value->int32;
             ColorHold = GColorFromHEX(PersistTextColor);
             persist_write_int(MESSAGE_KEY_TEXT_COLOR_KEY, Text_Color->value->int32);
        
             APP_LOG(APP_LOG_LEVEL_INFO,    "    Added Config Local Text Color: %d", PersistTextColor);   
         } else { //Check for Persist
               if(persist_exists(MESSAGE_KEY_TEXT_COLOR_KEY)) {
                  PersistTextColor = persist_read_int(MESSAGE_KEY_TEXT_COLOR_KEY);
                  ColorHold = GColorFromHEX(PersistTextColor);
                  APP_LOG(APP_LOG_LEVEL_INFO, "    Added Persistant Local Text Color = %d", PersistTextColor);
               }  else {   // Set Default
                  PersistTextColor = 16777215; 
                    ColorHold = GColorFromHEX(PersistTextColor);

                  APP_LOG(APP_LOG_LEVEL_INFO, "    Added Default Local Text %d", PersistTextColor);
               }
            }
         
         persist_write_int(MESSAGE_KEY_TEXT_COLOR_KEY, PersistTextColor);
       #else
         ColorHold = GColorWhite;
       #endif
  
        GTextColorHold = ColorHold;
     
        text_layer_set_text_color(text_battery_layer, ColorHold);
        text_layer_set_text_color(text_time_layer, ColorHold);
        text_layer_set_text_color(text_dayname_layer, ColorHold);
        text_layer_set_text_color(text_date_layer, ColorHold);
        text_layer_set_text_color(text_personalized_layer, ColorHold);
  
  // For all items * * * * *
  
      //Personalized Text 
      Tuple *Personalized_Text = dict_find(iterator, MESSAGE_KEY_PERSONALIZED_TEXT_KEY);
      
      if (Personalized_Text) {
          strcpy(PersistPersonalized_Text, (Personalized_Text->value->cstring));
          APP_LOG(APP_LOG_LEVEL_INFO,    "    Added Config Personalized Text: %s",PersistPersonalized_Text);   
         } else { //Check for Persist
               if(persist_exists(MESSAGE_KEY_PERSONALIZED_TEXT_KEY)) {
                  persist_read_string(MESSAGE_KEY_PERSONALIZED_TEXT_KEY, PersistPersonalized_Text, sizeof(PersistPersonalized_Text));
                  APP_LOG(APP_LOG_LEVEL_INFO, "    Added PersistantPersonalized Text: %s", PersistPersonalized_Text);
               }  else {   // Set Default
                  strcpy(PersistPersonalized_Text, "Enter Text");
                  APP_LOG(APP_LOG_LEVEL_INFO, "    Added Default Personalized Text: %s", PersistPersonalized_Text);
               }
         }        
  
      fill_in_personalized_text();   
      
      //Date Format  
      Tuple *Date_Type = dict_find(iterator, MESSAGE_KEY_DATE_FORMAT_KEY);
      
      if(Date_Type) {  
          strcpy(PersistDateFormat, (Date_Type->value->cstring));
          APP_LOG(APP_LOG_LEVEL_INFO,    "    Added Config Date Format: %s",PersistDateFormat);   
      } else { //Check for Persist
            if(persist_exists(MESSAGE_KEY_DATE_FORMAT_KEY)) {
               persist_read_string(MESSAGE_KEY_DATE_FORMAT_KEY, PersistDateFormat, sizeof(PersistDateFormat));
               APP_LOG(APP_LOG_LEVEL_INFO, "    Added Persistant Date Format: %s", PersistDateFormat);
            }  else {   // Set Default
               strcpy(PersistPersonalized_Text, "Enter Text");
               APP_LOG(APP_LOG_LEVEL_INFO, "    Added Default Date Format: %s", PersistDateFormat);
             }
      }
  
      if (strcmp(PersistDateFormat, "0") == 0) {     // US
         strcpy(date_format, "%b %e %Y");
      }  else  { 
         strcpy(date_format, "%e %b %Y");
      }      
  
     //Vibrate on BT Loss
     Tuple *BTVib = dict_find(iterator, MESSAGE_KEY_BT_VIBRATE_KEY);  
      
      if(BTVib) {
        PersistBTLoss = BTVib->value->int32;
          APP_LOG(APP_LOG_LEVEL_INFO,      "    Added Config Vib on BT Loss: %d", PersistBTLoss);   
      } else { //Check for Persist
            if(persist_exists(MESSAGE_KEY_BT_VIBRATE_KEY)) {
               PersistBTLoss = persist_read_int(MESSAGE_KEY_BT_VIBRATE_KEY);
               APP_LOG(APP_LOG_LEVEL_INFO, "    Added Persistant Vib on BT Loss: %d", PersistBTLoss);
            }  else {   // Set Default
               PersistBTLoss = 0;  // Default NO Vibrate
               APP_LOG(APP_LOG_LEVEL_INFO, "    Added Default Vib on BT Loss: %d", PersistBTLoss);
             }
      }
      
     
     //Vibrate on Low Batt
     Tuple *LowBatt = dict_find(iterator, MESSAGE_KEY_LOW_BATTERY_KEY);  
      
     if(LowBatt) {
        PersistLow_Batt = LowBatt->value->int32;
        APP_LOG(APP_LOG_LEVEL_INFO,      "    Added Config Vib on Low Batt: %d", PersistLow_Batt);   
      } else { //Check for Persist
            if(persist_exists(MESSAGE_KEY_LOW_BATTERY_KEY)) {
               PersistLow_Batt = persist_read_int(MESSAGE_KEY_LOW_BATTERY_KEY);
               APP_LOG(APP_LOG_LEVEL_INFO, "    Added Persistant Vib on Low Batt: %d", PersistLow_Batt);
            }  else {   // Set Default
               PersistLow_Batt = 0;  // Default NO Vibrate
               APP_LOG(APP_LOG_LEVEL_INFO, "    Added Default Vib on Low Batt: %d", PersistLow_Batt);
             }
      } 
    
  fill_in_personalized_text();
  
  FirstTime = 0;
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void handle_deinit(void) {
  persist_write_string(MESSAGE_KEY_PERSONALIZED_TEXT_KEY, PersistPersonalized_Text);
  APP_LOG(APP_LOG_LEVEL_ERROR, "Writing Persistant Date format %s", PersistDateFormat);
  persist_write_string(MESSAGE_KEY_DATE_FORMAT_KEY,       PersistDateFormat);
  APP_LOG(APP_LOG_LEVEL_ERROR, "Writing Persistant BTVib %d", PersistBTLoss);
  
  persist_write_int(MESSAGE_KEY_BT_VIBRATE_KEY,           PersistBTLoss);
  APP_LOG(APP_LOG_LEVEL_ERROR, "Writing Persistant Low Batt Vib %d", PersistLow_Batt);

  persist_write_int(MESSAGE_KEY_LOW_BATTERY_KEY,          PersistLow_Batt);
  
  persist_write_int(MESSAGE_KEY_BG_COLOR_KEY,             PersistBGColor);
  
  persist_write_int(MESSAGE_KEY_TEXT_COLOR_KEY,           PersistTextColor);

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
  APP_LOG(APP_LOG_LEVEL_ERROR, "In Init... * * * * * * *");
  
  //Set Default Colors
  if(persist_exists(MESSAGE_KEY_BG_COLOR_KEY)) {
     PersistBGColor = persist_read_int(MESSAGE_KEY_BG_COLOR_KEY); 
     GBGColorHold = GColorFromHEX(PersistBGColor);
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set BGColor to Persistant %d", PersistBGColor);    
  }  else {
     #ifdef PBL_COLOR
        APP_LOG(APP_LOG_LEVEL_INFO, "    Set PBL_COLOR BGColor to Default GColorDukeBlue");
        GBGColorHold = GColorDukeBlue;
     #else
        APP_LOG(APP_LOG_LEVEL_INFO, "    Set Non PBL_COLOR BGColor to Default GColorBlack");
        GBGColorHold = GColorBlack;
     #endif    
  } 
  #ifdef PBL_COLOR
  if(persist_exists(MESSAGE_KEY_TEXT_COLOR_KEY)) {
     PersistTextColor = persist_read_int(MESSAGE_KEY_TEXT_COLOR_KEY);  
     GTextColorHold = GColorFromHEX(PersistTextColor);
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set TextColor to Persistant %d", PersistTextColor);    
  }  else {
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set PBL_Color TextColor to Default White");        
     GTextColorHold = GColorWhite;  
  } 
  #else
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set Non PBL_COLOR TextColor to Default White");        
     GTextColorHold = GColorWhite;  
  #endif
  
  window = window_create();

  window_set_background_color(window, GBGColorHold);

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
  app_message_open(128, 128);

  // Dayname
  #ifdef PBL_PLATFORM_CHALK
      text_dayname_layer = text_layer_create(GRect(1, 64, 180, 180-64));
  #else  //Aplite or Basalt
      text_dayname_layer = text_layer_create(GRect(1, 64, 144, 168-64));
  #endif
  
  // Dayname
  text_layer_set_background_color(text_dayname_layer, GBGColorHold);
  text_layer_set_text_color(text_dayname_layer, GTextColorHold);
  text_layer_set_text_alignment(text_dayname_layer, GTextAlignmentCenter);
  text_layer_set_font(text_dayname_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(text_dayname_layer));

  // Date
  #ifdef PBL_PLATFORM_CHALK
      text_date_layer = text_layer_create(GRect(1, 88, 180, 180-88));
  #else
      text_date_layer = text_layer_create(GRect(1, 88, 144, 168-88));    
  #endif

  //text_layer_set_background_color(text_battery_layer, GBGColorHold);
  text_layer_set_background_color(text_date_layer, GBGColorHold);
  text_layer_set_text_color(text_date_layer, GTextColorHold);
  text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  //Persistant Value Personalized Text
  if(persist_exists(MESSAGE_KEY_PERSONALIZED_TEXT_KEY)) {
     persist_read_string(MESSAGE_KEY_PERSONALIZED_TEXT_KEY, PersistPersonalized_Text, sizeof(PersistPersonalized_Text));
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set Personalized Text to Persistant %s", PersistPersonalized_Text);    
  } else {
    strncpy(PersistPersonalized_Text, "Enter Text", sizeof(PersistPersonalized_Text)); //Default
    APP_LOG(APP_LOG_LEVEL_INFO,  "    Set Personalized Text to Default - Enter Text");    
  }

  //Persistent Value Date Format:
  if (persist_exists(MESSAGE_KEY_DATE_FORMAT_KEY)) {
     persist_read_string(MESSAGE_KEY_DATE_FORMAT_KEY  , PersistDateFormat, sizeof(PersistDateFormat));  
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set Date Format to Persistant: %s, 0 = US, 1 = Int'l", PersistDateFormat);
  }  else {
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set Date Format to Default 0 = US");
     strcpy(PersistDateFormat, "0"); //Default
  } 
  
  if (strcmp(PersistDateFormat, "0") == 0) {     // US
     strcpy(date_format, "%b %e %Y");
  } else {  
     strcpy(date_format, "%e %b %Y");
  }
     
  //Persistent Value Vib On BTLoss
  if(persist_exists(MESSAGE_KEY_BT_VIBRATE_KEY)) {
     PersistBTLoss = persist_read_int(MESSAGE_KEY_BT_VIBRATE_KEY);  
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set BT Vibrate To Persistant %d (0 = NO Vib, 1 = Vib", PersistBTLoss);
  }  else {
     PersistBTLoss = 0; // Default
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set BT Vibrate To 0 Default - No Vibrate");

  } 

  //Persistent Value Vib on Low Batt
  if(persist_exists(MESSAGE_KEY_LOW_BATTERY_KEY)) {
     PersistLow_Batt = persist_read_int(MESSAGE_KEY_LOW_BATTERY_KEY);  
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set Low Batt Vibrate To Persistant %d (0 = NO Vib, 1 = Vib", PersistLow_Batt);
  }  else {
     PersistLow_Batt = 0; // Default
     APP_LOG(APP_LOG_LEVEL_INFO, "    Set Low Batt Vibrate To 0 Default - No Vibrate");

  } 
  // Time of Day
  #ifdef PBL_PLATFORM_CHALK
      text_time_layer = text_layer_create(GRect(1, 116, 180, 180-116));
  #else //Aplite or Basalt
      text_time_layer = text_layer_create(GRect(1, 116, 144, 168-116));
  #endif
    
  text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
    #ifdef PBL_PLATFORM_CHALK
        text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    #else //Aplite or Basalt
        text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    #endif  
      
  text_layer_set_text_color(text_time_layer, GTextColorHold);
  text_layer_set_background_color(text_time_layer, GBGColorHold);
  
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  fill_in_personalized_text();
  

  // Line
  #ifdef PBL_PLATFORM_CHALK
      GRect line_frame = GRect(38, 118, 104, 6);
  #else // Aplite or Basalt
      GRect line_frame = GRect(22, 118, 104, 6);
  #endif
    
  LineLayer = layer_create(line_frame);
  layer_set_update_proc(LineLayer, line_layer_update_callback);
  layer_add_child(window_layer, LineLayer);
  
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

  //Bluetooth Logo Setup area
  #ifdef PBL_PLATFORM_CHALK
      GRect BTArea = GRect(55, 5, 20, 20);
  #else
      GRect BTArea = GRect(1, 5, 20, 20);
  #endif 
    
  BTLayer = layer_create(BTArea);

  layer_add_child(window_layer, BTLayer);

  layer_set_update_proc(BTLayer, BTLine_update_callback);

  bluetooth_connection_service_subscribe(&handle_bluetooth);
  handle_bluetooth(bluetooth_connection_service_peek());

  // Battery Text Setup
  #ifdef PBL_PLATFORM_CHALK
      text_battery_layer = text_layer_create(GRect(80,2,55,26));
  #else
      text_battery_layer = text_layer_create(GRect(85,2,55,26));
  #endif 
    
  text_layer_set_text_color(text_battery_layer, GTextColorHold);
  text_layer_set_background_color(text_battery_layer, GBGColorHold);
  text_layer_set_font(text_battery_layer, fontHelvNewLight20);
  text_layer_set_text_alignment(text_battery_layer, GTextAlignmentRight);

  battery_state_service_subscribe(&handle_battery);

  handle_battery(battery_state_service_peek());

  layer_add_child(window_layer, text_layer_get_layer(text_battery_layer));

  window_set_background_color(window, GBGColorHold);

    
  //app focus service subscribe
  app_focus_service_subscribe(&handle_appfocus);
  
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_tick(current_time, SECOND_UNIT);
}

int main(void) {
   handle_init();

   app_event_loop();

   handle_deinit();
}
