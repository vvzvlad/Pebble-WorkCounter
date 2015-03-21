#include <pebble.h>
Window *window; 
Window *window_settings;
TextLayer *worktime_layer;     
TextLayer *status_layer;    
TextLayer *amount_layer;  
TextLayer *rate_layer;  

ActionBarLayer *action_bar;
ActionBarLayer *action_bar_settings;

static GBitmap *reset_button;
static GBitmap *start_button;
static GBitmap *pause_button;
static GBitmap *up_button;
static GBitmap *down_button;

int start_utime = 0;
int current_utime = 0;
int diff_time = 0;
int diff_hour = 0;
int diff_min = 0;
int diff_time_storage = 0;
int amount;
int rate = 404;

int status; //1=start, 2=pause, 3=stop
char worktime_text[] = "00:00";
char amount_text[] = "Заработано:\n 10000.45 руб.";
char rate_text[] = "Rate: \n1000\n rub/hour";


void update_worktime()
{
    diff_time=0;
    current_utime = time(NULL);
    
    if (start_utime > 0)
        {
            diff_time = (current_utime - start_utime)+diff_time_storage;
        }
    else
        {
            diff_time = diff_time_storage;
        }
    
    amount = (diff_time*(rate*1000/60/60))/10;
    int div = 60;
    diff_hour = diff_time/div/60;
    diff_min = diff_time/div-diff_hour*60;

    
    snprintf(worktime_text, sizeof(worktime_text), "%0*d:%0*d", 2, diff_hour, 2, diff_min);
    //Куда_писать,размер_записи,форматирование,ширина_первого_аргумента,первый_аргумент
    //,ширина_второго_аргумента,второй_аргумент,
    text_layer_set_text(worktime_layer, worktime_text);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "%ld", current_utime);
    
    switch(status)
        {
            case 1:
            text_layer_set_text(status_layer, "Started");
            break;

            case 2:
            text_layer_set_text(status_layer, "Paused");
            break;

            case 3:
            text_layer_set_text(status_layer, "Stopped");
            break;
        }
    snprintf(amount_text, sizeof(amount_text), "Earned: \n %d.%-2d rub.", amount/100, amount-((amount/100)*100));
    text_layer_set_text(amount_layer, amount_text);

}


static void update_clock(struct tm* tick_time, TimeUnits units_changed) 
{
    update_worktime();
}

void click_up(ClickRecognizerRef recognizer, void *context)  //start
{
    if(start_utime == 0)
        {
            status = 1;
            start_utime = time(NULL);
            persist_write_int(1, start_utime);
            persist_write_int(3, status);
            vibes_short_pulse();
            update_worktime();
        }
}

void click_select_long(ClickRecognizerRef recognizer, void *context)  //reset
{
    start_utime = 0;
    diff_time_storage = 0;
    status = 3;
    persist_write_int(1, start_utime);
    persist_write_int(2, diff_time_storage);
    persist_write_int(3, status);
    update_worktime();
    vibes_long_pulse();
}

void click_select_multi(ClickRecognizerRef recognizer, void *context)  //reset
{
    window_stack_push(window_settings, true);  
    vibes_long_pulse();
}

void click_up_long(ClickRecognizerRef recognizer, void *context)  //reset
{
    diff_time_storage = diff_time_storage+30*60;
    persist_write_int(2, diff_time_storage);
    update_worktime();
}

void click_down_long(ClickRecognizerRef recognizer, void *context)  //reset
{
    if(diff_time_storage > 30*60)
        {
            diff_time_storage = diff_time_storage-30*60;
            persist_write_int(2, diff_time_storage);
            update_worktime();
        }
}

void click_down(ClickRecognizerRef recognizer, void *context)  //pause
{
    if(start_utime != 0)
        {
            status = 2;
            update_worktime();
            diff_time_storage = diff_time;
            start_utime = 0;
            persist_write_int(1, start_utime);
            persist_write_int(2, diff_time_storage);
            persist_write_int(3, status);
            vibes_short_pulse();
        }
}

void click_up_settings(ClickRecognizerRef recognizer, void *context)  //reset
{
    rate++;
    persist_write_int(4, rate);
    snprintf(rate_text, sizeof(rate_text), "Rate:\n%d\nrub/hour", rate);
    text_layer_set_text(rate_layer, rate_text);
}

void click_down_settings(ClickRecognizerRef recognizer, void *context)  //reset
{
    rate--;
    persist_write_int(4, rate);
    snprintf(rate_text, sizeof(rate_text), "Rate:\n%d\nrub/hour", rate);
    text_layer_set_text(rate_layer, rate_text);
}

void window_click_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) click_up); 
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) click_down); 
    window_long_click_subscribe(BUTTON_ID_SELECT, 1000, (ClickHandler) click_select_long, NULL);
    window_long_click_subscribe(BUTTON_ID_UP, 1000, (ClickHandler) click_up_long, NULL);
    window_long_click_subscribe(BUTTON_ID_DOWN, 1000, (ClickHandler) click_down_long, NULL);
    window_multi_click_subscribe(BUTTON_ID_SELECT, 3, 3, 500, true, click_select_multi);

}

void window_settings_click_provider(void *context)
{
    window_single_repeating_click_subscribe(BUTTON_ID_UP, 30, (ClickHandler) click_up_settings); 
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 30, (ClickHandler) click_down_settings); 
}

void init_window()
{
    window_set_background_color(window, GColorWhite); 

    status_layer = text_layer_create(GRect(0, -5, 144-ACTION_BAR_WIDTH, 168)); 
    text_layer_set_text_color(status_layer, GColorBlack); 
    text_layer_set_background_color(status_layer, GColorClear); 
    text_layer_set_font(status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(status_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(status_layer));
    
    amount_layer = text_layer_create(GRect(0, 100, 144-ACTION_BAR_WIDTH, 168)); 
    text_layer_set_text_color(amount_layer, GColorBlack); 
    text_layer_set_background_color(amount_layer, GColorClear); 
    text_layer_set_font(amount_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(amount_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(amount_layer));
    
    worktime_layer = text_layer_create(GRect(0, 45, 144-ACTION_BAR_WIDTH, 168)); 
    text_layer_set_text_color(worktime_layer, GColorBlack); 
    text_layer_set_background_color(worktime_layer, GColorClear); 
    text_layer_set_font(worktime_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
    text_layer_set_text_alignment(worktime_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(worktime_layer));
    
    window_set_click_config_provider(window, window_click_provider);

    reset_button = gbitmap_create_with_resource(RESOURCE_ID_RESET_BUTTON);
    start_button = gbitmap_create_with_resource(RESOURCE_ID_START_BUTTON);
    pause_button = gbitmap_create_with_resource(RESOURCE_ID_PAUSE_BUTTON);
    
    action_bar = action_bar_layer_create();
    action_bar_layer_add_to_window(action_bar, window);
    action_bar_layer_set_click_config_provider(action_bar, window_click_provider);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, start_button);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, pause_button);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, reset_button);
}

void init_window_settings()
{
    window_set_background_color(window_settings, GColorWhite);

    rate_layer = text_layer_create(GRect(0, 20, 144-ACTION_BAR_WIDTH, 168)); 
    text_layer_set_text_color(rate_layer, GColorBlack); 
    text_layer_set_background_color(rate_layer, GColorClear); 
    text_layer_set_font(rate_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(rate_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window_settings), text_layer_get_layer(rate_layer));
    
    window_set_click_config_provider(window_settings, window_settings_click_provider);

    up_button = gbitmap_create_with_resource(RESOURCE_ID_UP_BUTTON);
    down_button = gbitmap_create_with_resource(RESOURCE_ID_DOWN_BUTTON);
    
    action_bar_settings = action_bar_layer_create();
    action_bar_layer_add_to_window(action_bar_settings, window_settings);
    action_bar_layer_set_click_config_provider(action_bar_settings, window_settings_click_provider);
    action_bar_layer_set_icon(action_bar_settings, BUTTON_ID_UP, up_button);
    action_bar_layer_set_icon(action_bar_settings, BUTTON_ID_DOWN, down_button);
    action_bar_layer_set_icon(action_bar_settings, BUTTON_ID_SELECT, reset_button);
    
    snprintf(rate_text, sizeof(rate_text), "Rate:\n%d\nrub/hour", rate);
    text_layer_set_text(rate_layer, rate_text);
}

void persist_read()
{
    if (persist_exists(1))  
        {
            start_utime = persist_read_int(1); 
        }
    
    if (persist_exists(2))  
        {
            diff_time_storage = persist_read_int(2);
        }
    
    if (persist_exists(3))  
        {
            status = persist_read_int(3);
        }
    
    if (persist_exists(4))  
        {
            rate = persist_read_int(4);
        }    
    else 
        {
            persist_write_int(4, rate);
        }
}

int main(void) 
{
    persist_read();
    
    window = window_create(); 
    init_window();
    
    window_settings = window_create();
    init_window_settings();
    
    
    time_t now = time(NULL); 
    struct tm *current_time = localtime(&now); 
    tick_timer_service_subscribe(SECOND_UNIT, &update_clock); 
    update_clock(current_time, SECOND_UNIT); 

    update_worktime();

    window_stack_push(window, true);
    
    app_event_loop(); 
    
    text_layer_destroy(worktime_layer);
    text_layer_destroy(status_layer);
    text_layer_destroy(amount_layer);
    text_layer_destroy(rate_layer);

    window_destroy(window); 
    window_destroy(window_settings);
}