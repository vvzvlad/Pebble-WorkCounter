#include <pebble.h>
Window *window; //Создаем главное окно

TextLayer *worktime_layer; //Создаем слой с временем работы   
time_t start_utime = 0; //Переменная  времени старта(ВС)
time_t current_utime = 0; //Переменная текущего времени(ТВ)
time_t diff_time = 0; //Разница в секундах между текущим и начальным
char worktime_text[] = "00:00"; //Разница в виде текста

void update_worktime(struct tm* tick_time, TimeUnits units_changed) //Функция вызываемая каждую секунду
{
    current_utime = time(NULL); //Получаем ТВ
    
    if (start_utime != 0) //Если время старта не О, значит счетчик запущен
        {
            diff_time = (current_utime - start_utime); //Вычисляем разницу между ВС и ТВ
        }
    
    strftime(worktime_text, sizeof(worktime_text), "%R", localtime(&diff_time)); //Форматируем время как HH:MM
    text_layer_set_text(worktime_layer, worktime_text); //Выводим форматированное в слой
}



void click_up(ClickRecognizerRef recognizer, void *context)  //Кнопка старта
{
    if(start_utime == 0) //Только если счетчик сейчас остановлен...
        {
            start_utime = time(NULL); //Делаем текущее время временем старта
            diff_time = 0; //Сбрасываем счетчик времени
            persist_write_int(1, start_utime); //Записываем время старта в постоянное хранилище
            update_worktime(NULL, SECOND_UNIT); //Обновляем текст на экране
            vibes_short_pulse(); //Вибрируем

        }
}

void click_down(ClickRecognizerRef recognizer, void *context)  //Кнопка остановки
{
    start_utime = 0; //Обнуляем время
    persist_write_int(1, start_utime); //Записываем время старта в постоянное хранилище
    vibes_short_pulse(); //Вибрируем
}


void window_click_provider(void *context) //Функция подписок на кнопки
{
    window_single_click_subscribe(BUTTON_ID_UP, click_up); //Подписываемся на событие от кнопки UP
    window_single_click_subscribe(BUTTON_ID_DOWN, click_down); //Подписываемся на событие от кнопки SELECT
}

void persist_read() //Читаем из памяти сохраненные значения
{
    if (persist_exists(1)) //Если значение с номером 1 есть в памяти...
    {
        start_utime = persist_read_int(1); //То записываем его в переменную
    }
}



int main(void) 
{
    persist_read();

    window = window_create(); //Создаем окно
    
    worktime_layer = text_layer_create(GRect(0, 45, 144, 168)); //Создаем текстовый слой
    text_layer_set_text_color(worktime_layer, GColorBlack);  //Устанавливаем цвет текста
    text_layer_set_background_color(worktime_layer, GColorClear); //Устанавливаем цвет фона
    text_layer_set_font(worktime_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS)); //Устанавливаем шрифт
    text_layer_set_text_alignment(worktime_layer, GTextAlignmentCenter); //Устанавливаем выравнивание
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(worktime_layer)); //Подключаем к окну
    
    window_set_click_config_provider(window, window_click_provider); //Определяем функцию подписок на кнопки
    
    tick_timer_service_subscribe(SECOND_UNIT, &update_worktime); //Вызывать update_worktime каждую секунду
    update_worktime(NULL, SECOND_UNIT); //Вызываем не дожидаясь обновления
    
    window_stack_push(window, true); //Открываем основное окно
    
    app_event_loop(); //Основной цикл программы
    
    text_layer_destroy(worktime_layer); //Уничтожаем слой
    window_destroy(window); //Уничтожаем окно
}