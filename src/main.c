#include <pebble.h>
    
Window *window; 
TextLayer *clock_layer; 
TextLayer *worktime_layer;     
    
static BitmapLayer *image_layer_fone; 
static GBitmap *image_fone;

int start_utime=0;
int current_utime;
int diff_time=0;
int diff_hour=0;
int diff_min=0;
char worktime_text[] = "00:00";

void update_worktime()
{
    
    current_utime = time(NULL);
    diff_time=0;
    if (start_utime > 0)
    {
        diff_time = current_utime - start_utime;
        diff_hour=diff_time/60/60;
        diff_min=diff_time/60-diff_hour*60;
        snprintf(worktime_text, sizeof(worktime_text), "%0*d:%0*d", 2, diff_hour, 2, diff_min);
        //Куда_писать,размер_записи,форматирование,ширина_первого_аргумента,первый_аргумент
        //,ширина_второго_аргумента,второй_аргумент,
        text_layer_set_text(worktime_layer, worktime_text);
    }
    


    //APP_LOG(APP_LOG_LEVEL_DEBUG, "%ld", current_utime);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "%ld", start_utime);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "%ld", diff_time);

    
}


static void update_clock(struct tm* tick_time, TimeUnits units_changed) 
{
    update_worktime();
    static char time[] = "00:00"; 
    strftime(time, sizeof(time), "%R", tick_time);
    text_layer_set_text(clock_layer, time); 

}

void click_up(ClickRecognizerRef recognizer, void *context)  
{
    if(start_utime = 0)
    {
        start_utime = time(NULL);
        persist_write_int(1, start_utime);
    }


}

void click_select(ClickRecognizerRef recognizer, void *context)  
{
    start_utime = 0;
    persist_write_int(1, start_utime);
}

void click_down(ClickRecognizerRef recognizer, void *context)  
{

}

void WindowsClickConfigProvider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, click_up); 
    window_single_click_subscribe(BUTTON_ID_SELECT, click_select); 
    window_single_click_subscribe(BUTTON_ID_DOWN, click_down); 
    //window_long_click_subscribe(BUTTON_ID_UP, 1000, click_up_long, NULL); 
    //window_long_click_subscribe(BUTTON_ID_DOWN, 1000, click_down_long, NULL); 

}



int main(void) 
{
    window = window_create(); 
    window_set_background_color(window, GColorWhite); 
    window_set_fullscreen(window, true);
    window_stack_push(window, true);
    
    image_layer_fone = bitmap_layer_create(GRect(0, 0, 144, 168));
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(image_layer_fone));
    image_fone = gbitmap_create_with_resource(RESOURCE_ID_FONE_BUTTON);
    bitmap_layer_set_bitmap(image_layer_fone, image_fone);
    
    clock_layer = text_layer_create(GRect(0, -5, 144-24, 168)); 
    text_layer_set_text_color(clock_layer, GColorBlack); 
    text_layer_set_background_color(clock_layer, GColorClear); 
    text_layer_set_font(clock_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_alignment(clock_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(clock_layer));
    
    worktime_layer = text_layer_create(GRect(0, 168/2-20, 144-24, 168)); 
    text_layer_set_text_color(worktime_layer, GColorBlack); 
    text_layer_set_background_color(worktime_layer, GColorClear); 
    text_layer_set_font(worktime_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
    text_layer_set_text_alignment(worktime_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(worktime_layer));
    
    time_t now = time(NULL); 
    struct tm *current_time = localtime(&now); 
    tick_timer_service_subscribe(MINUTE_UNIT, &update_clock); 
    update_clock(current_time, MINUTE_UNIT); 

    if (persist_exists(1))  
    {
        start_utime = persist_read_int(1);
        update_worktime();
    }
    
    window_set_click_config_provider(window, WindowsClickConfigProvider);

    app_event_loop(); 
    
    text_layer_destroy(worktime_layer);
    text_layer_destroy(clock_layer);

    window_destroy(window); 
}