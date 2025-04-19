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
    
    font_pack *my_font = ssd1306_init_oled_font(&font_5x7[0][0], 5, 8, 32); // initalizing the font to use in oled_buffer

    oled_buffer *top_left = ssd1306_create_viewbox(63, 4);  // create a small buffer of 63 colums and 4 page which is 63x(4x8) = 2016 bytes.
    oled_buffer *top_right = ssd1306_create_viewbox(63, 4);  // create a small buffer of 63 colums and 4 page which is 63x(4x8) = 2016 bytes.
    oled_buffer *bottom_left = ssd1306_create_viewbox(63, 4);  // create a small buffer of 63 colums and 4 page which is 63x(4x8) = 2016 bytes.
    oled_buffer *bottom_right = ssd1306_create_viewbox(63, 4);  // create a small buffer of 63 colums and 4 page which is 63x(4x8) = 2016 bytes.
    
    ssd1306_place_oled_view(top_left, 1, 1);        // place the buffer at column 1 and page 1 ( there is no starting from 0 )
    ssd1306_place_oled_view(top_right, 64, 1);      // place the buffer at column 64 and page 1 ( there is no starting from 0 )
    ssd1306_place_oled_view(bottom_left, 1, 5);     // place the buffer at column 1 and page 5 ( there is no starting from 0 )
    ssd1306_place_oled_view(bottom_right, 64, 5);   // place the buffer at column 63 and page 5 ( there is no starting from 0 )

    // placing Char at the center of the buffer.

    ssd1306_draw_oled_char(top_left, 31, 'A', my_font, 11);
    ssd1306_draw_oled_char(top_right, 31, 'B', my_font, 11);
    ssd1306_draw_oled_char(bottom_left, 31, 'C', my_font, 11);
    ssd1306_draw_oled_char(bottom_right, 31, 'D', my_font, 11);

    // adding border to the buffer.

    ssd1306_viewbox_oled_border(top_left, 1, 1, 1, 1);
    ssd1306_viewbox_oled_border(top_right, 1, 1, 1, 1);
    ssd1306_viewbox_oled_border(bottom_left, 1, 1, 1, 1);
    ssd1306_viewbox_oled_border(bottom_right, 1, 1, 1, 1);

    // displaying different buffer.

    ssd1306_send_oled_display_buffer(top_left);
    ssd1306_send_oled_display_buffer(top_right);
    ssd1306_send_oled_display_buffer(bottom_left);
    ssd1306_send_oled_display_buffer(bottom_right);

    
   
    ssd1306_delete_viewbox(top_left);           // delete the oled buffer top_left.
    ssd1306_delete_viewbox(top_right);          // delete the oled buffer top_right
    ssd1306_delete_viewbox(bottom_left);        // delete the oled buffer bottom_left
    ssd1306_delete_viewbox(bottom_right);       // delete the oled buffer bottom_right

    ssd1306_delete_oled_font(my_font);          // delete the font buffer.

}