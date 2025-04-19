#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include "esp_err.h"
#include <esp_mac.h>
#include "ssd1306_oled.h"
#include <string.h>
#include <ssd1306_fonts.h>
#include <esp_timer.h>



void app_main(void)
{

    ssd1306_init_oled_i2c(0x3c, 21, 22);    // init the i2c pin and address.
    ssd1306_oled_clear(0);                  // clear the oled Graphic Ram and screen.
    
    oled_buffer *text_box = ssd1306_create_viewbox(30, 1);  // create a small buffer of 30 colums and 1 page which is 30x8 = 240 bytes.
    
    font_pack *my_font = ssd1306_init_oled_font(&font_5x7[0][0], 5, 8, 32); // initalizing the font to use in oled_buffer
    
    while(1)
    {
        //generating the random values
        int rand_1_to_8 = (rand() % 8) + 1;     // random vlaue between 1 to 8
        int rand_1_to_98 = (rand() % 98) + 1;   // random vlaue between 1 to 98 , since we don't want our buffer to display outside the screen therefore we have subtracted 30 columns form 128 columns.
        

        ssd1306_place_oled_view(text_box, rand_1_to_98, rand_1_to_8);   // setting the buffer position on the screen in terms of columns and pages.

        ssd1306_draw_oled_string(text_box, 0, "HELLO!", my_font, 1);    // writing the string in the buffer.
        ssd1306_send_oled_display_buffer(text_box);                     // sending the buffer to the oled

        vTaskDelay(2000/portTICK_PERIOD_MS);                            // delay for 2 seconds.
        
        ssd1306_oled_clear_view(text_box, 0);                           // clearing all the buffer values to 0x00
        ssd1306_send_oled_display_buffer(text_box);                     // sending the buffer to the oled
    }

    /*  
        since above is an infinite loop so these function are not going to call, 
        they are here just to tell these function exist to help you to clear your memory
        and delete the buffer and font if you dont want to use them further.
    */

    ssd1306_delete_viewbox(text_box);           // delete the oled buffer.
    ssd1306_delete_oled_font(my_font);          // delete the font buffer.

}