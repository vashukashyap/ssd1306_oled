#include "ssd1306_oled.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "oled_init";                                       // TAG for all the OLED LOGS

static i2c_master_dev_handle_t oled_handle;                                 // I2C handle for OLED
static i2c_master_bus_handle_t oled_master_handle;                          // I2C handle for Master Bus

static active_scroll_view *scroll_views_head = NULL;                        // Head pointer for managing linkedlist for scrolling views
active_scroll_view **scroll_views_pointer_head = &scroll_views_head;        

TaskHandle_t activate_scroll;                                               // Scrolling task handler

/*!
    @brief  Initalised the OLED over the I2C protocol
    @param  i2c_address
            I2C address of The OLED which is the 0x3C (in hex) in most of the Cases
    @param  sda_gpio_pin
            I2C SDA pin number which is connected to the OLED (it is the data pin).
    @param  scl_gpio_pin
            I2C SCL pin number which is connetec to the OLED (it is the clock pin).
    @return None (Void)
    @note   This function Initalised the OLED with appropriate ssd1306 Command and
            Data as mention in the Datasheet. you can change command and desired value
            by modifing the Commands and Data in this section.
*/
void ssd1306_init_oled_i2c(uint8_t i2c_address, uint8_t sda_gpio_pin, uint8_t scl_gpio_pin)
{
    // Creating the I2C Master config structure
    i2c_master_bus_config_t i2c_mst_config = {
        .i2c_port = I2C_PORT,
        .scl_io_num = scl_gpio_pin,
        .sda_io_num = sda_gpio_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = GLITCH_IGNORE_CNT,
        .flags.enable_internal_pullup = ENABLE_INPERRUPT_PULLUP};

    // Adding the Master config to the Master handle
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &oled_master_handle));
    ESP_LOGI(TAG, "Master Bus is created.");

    // Creating the I2C device config structure ( which is the OLED here )
    i2c_device_config_t oled_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = i2c_address,
        .scl_speed_hz = I2C_SCL_SPEED,
    };

    // Adding the slave ( OLED ) config and handler to the master handler.
    ESP_ERROR_CHECK(i2c_master_bus_add_device(oled_master_handle, &oled_cfg, &oled_handle));
    ESP_LOGI(TAG, "Master Bus Device is added.");

    // Various init command buffer for the OLED
    // { start_command (0x00) , paticular_command ,  its value }

    uint8_t init_CLK_DIV_RATIO[3] = {0x00, OLED_SET_CLOCK_DIVIDE_RATIO, 0x00};  // CLOCK DIVISION RATIO buffer set to the 0x00 for faster frequency (0x80 is prefered in most senerio)
    uint8_t init_MUX_RATIO[3] = {0x00, OLED_SET_MULTIPLEX_RATIO, 0x3F};         // MULTIPLEX ( MUX ) RATIO set to the 0x3F ( because Display is 128x64 )
    uint8_t init_OFFSET[3] = {0x00, OLED_SET_DISPLAY_OFFSET, 0x00};             // OFFSET set to the 0x00 ( 0 means no offset )
    uint8_t init_CHARGE_PUMP[3] = {0x00, OLED_CHARGE_PUMP_SETTING, 0x14};       // CHARGE PUMP set to the 0x14 according to the Datasheet ( it use to drive LEDS with sufficient power by enabling 2 capacitor present )
    uint8_t init_AUTO_ADDRESS[3] = {0x00, OLED_SET_MEMORY_ADDRESS_MODE, 0x00};  // MEMORY ADDRESS MODE set to the 0x00 which is HORIZONTAL MODE ( you can check Datasheet and change it if you want )
    uint8_t init_COM_PIN[3] = {0x00, OLED_SET_COM_PIN, 0x12};                   // COM PIN HARDWARE CONFIGURATION set to 0x12 which is ALTERNATIVE COM ( you can check Datasheet and change it if you want )
    uint8_t init_CONTRAST[3] = {0x00, OLED_SET_CONTRAST_CONTROL, 0x7F};         // CONTRAST set to 0x7F ( this value keep oled stable and glow biright. )
    uint8_t init_PRECHARGED[3] = {0x00, OLED_SET_PRECHARGE_PERIOD, 0x77};       // PRE-CHARGE PERIOD set to 0xF1 ( it help the clock of OLED to work )
    uint8_t init_VCOM_DESELECT[3] = {0x00, OLED_SET_VCOM_DESELECT_LEVEL, 0x20}; // VCOM DE-SELECT LEVEL set to 0x20 which is 0.77v * Vcc (RESET)
    uint8_t page_set[] = {0x00, 0xB0 | 0x00, 0x00, 0x10};                       // Page START set to 0 + Column LSB/MSB  set to 0x00 / 0x10 ( make the buffer display properly otherwise it wil get display in segments )

    // Sending commands in proper sequence ( the is no sequence you can send how ever you want but sometimes random sequence create problem )
    // You can check Datasheet ssd1306 for checking various commands

    ssd1306_send_oled_command(OLED_SET_DISPLAY_OFF);
    ssd1306_send_oled_command(OLED_STOP_SCROLLING);
    ssd1306_send_oled_buffer(init_CLK_DIV_RATIO, 3);
    ssd1306_send_oled_buffer(init_MUX_RATIO, 3);
    ssd1306_send_oled_buffer(init_OFFSET, 3);
    ssd1306_send_oled_command(OLED_SET_DISPLAY_START_LINE);
    ssd1306_send_oled_buffer(init_CHARGE_PUMP, 3);
    ssd1306_send_oled_buffer(init_AUTO_ADDRESS, 3);
    ssd1306_send_oled_buffer(page_set, sizeof(page_set));
    ssd1306_send_oled_command(OLED_SET_SEGMENT_REMAP);
    ssd1306_send_oled_command(OLED_SET_COM_OUTPUT_SCAN_REMAPPED);
    ssd1306_send_oled_buffer(init_COM_PIN, 3);
    ssd1306_send_oled_buffer(init_CONTRAST, 3);
    ssd1306_send_oled_buffer(init_PRECHARGED, 3);
    ssd1306_send_oled_buffer(init_VCOM_DESELECT, 3);
    ssd1306_send_oled_command(OLED_ENTIRE_DISPLAY_ON_RAM_CONTENT);
    ssd1306_send_oled_command(OLED_SET_NORMAL_DISPLAY);
    ssd1306_send_oled_command(OLED_SET_DISPLAY_ON);

    ESP_LOGI(TAG, "oled Initialized");
}


/*!
    @brief  Create a buffer (array) for stroing data of display.
    @param  width
            width for the oled buffer ( 1 <= WIDTH <= 128 ).
    @param  page
            it is hte height of the oled buffer, in term of pages. ( 1 <= PAGE <= 8 ), 1 page equal 8 vertical bits or a Byte.
    @return oled_buffer pointer.
    @note   make sure you pass the value within the limit otherwise you will not the get proper display on oled.
*/
oled_buffer *ssd1306_create_viewbox(uint16_t width, uint8_t page)
{
    oled_buffer *new_buffer = (oled_buffer *)malloc(sizeof(oled_buffer));
    if (!new_buffer) {
        ESP_LOGE(TAG, "Memory allocation failed for viewbox.");
        return 0;
    }

    new_buffer->oled_user_buffer = malloc((width * page) + 1);
    if (!new_buffer->oled_user_buffer) {
        ESP_LOGE(TAG, "Memory allocation failed for buffer in viewbox.");
        return 0;
    }

    new_buffer->oled_user_buffer_size = (width * page) + 1;
    new_buffer->page = page;
    new_buffer->width = width;
    new_buffer->col_pos = 1;
    new_buffer->page_pos = 1;
    new_buffer->scroll_type = NO_SCROLL;
    new_buffer->oled_user_buffer[0] = 0x40;
    new_buffer->cursor = 0;
    memset(&new_buffer->oled_user_buffer[1], 0x00, new_buffer->oled_user_buffer_size - 1);
    return new_buffer;
}


/*!
    @brief  Delete the viewbox buffer for free the memory.
    @param  oled_user_buffer
            pointer of the viewbox.
    @return None (Void)
    @note   don't pass the pointer of already deleted buffer, otherwise it will make core panic.
*/
void ssd1306_delete_viewbox(oled_buffer *oled_user_buffer)
{
    free(oled_user_buffer->oled_user_buffer);   // free the allocated memory of the array
    free(oled_user_buffer);                     // free the allocated memory of the struct.
}


/*!
    @brief  send one command at a time to the ssd1306 over I2C
    @param  command
            ssd1306 command in hex
    @return None (Void)
    @note   You can find commands in ssd1306 Datasheet
*/
void ssd1306_send_oled_command(uint8_t command)
{
    const uint8_t buffer[2] = {0x00, command};
    ESP_ERROR_CHECK(i2c_master_probe(oled_master_handle, 0x3C, XFER_TIMEOUT_MS));
    ESP_ERROR_CHECK(i2c_master_transmit(oled_handle, buffer, 2, XFER_TIMEOUT_MS));
}



/*!
    @brief  send a buffer to the ssd1306 over I2C
    @param  buffer
            buffer pointer containing your commands
    @param  size
            size of the buffer
    @return None (Void)
    @note   make sure you include the correct hex value for the buffer in starting.
            " 0x00 " for the command buffer.
            " 0x40 " for the data buffer.
*/
void ssd1306_send_oled_buffer(uint8_t *buffer, size_t size)
{
    ESP_ERROR_CHECK(i2c_master_probe(oled_master_handle, 0x3C, XFER_TIMEOUT_MS));
    ESP_ERROR_CHECK(i2c_master_transmit(oled_handle, buffer, size, XFER_TIMEOUT_MS));
}

/*!
    @brief  clear the screen viewbox buffer.
    @param  oled_user_buffer
            oled_buffer pointer created by the viewbox function.
    @param  clear_with
            the value with which you want to clear the buffer.
    @return None (Void)
    @note   pass the value 0 or 1 in the clear_with param for fill with black and white respectively.
*/
void ssd1306_oled_clear_view(oled_buffer *oled_user_buffer, uint8_t clear_with)
{
    memset(&oled_user_buffer->oled_user_buffer[1], clear_with, oled_user_buffer->oled_user_buffer_size - 1);
}



/*!
    @brief  clear the screen oled screen.
    @param  clear_with
            the value with which you want to clear the buffer.
    @return None (Void)
    @note   pass the value 0 or 1 in the clear_with param for fill with black and white respectively.
*/
void ssd1306_oled_clear(uint8_t clear_with)
{
    uint8_t page_set[4] = {0x00, 0xB0 | 0x00, 0x00, 0x10};                  // setting the start and end position of the ssd1306 ram cursor.
    uint8_t init_CURSOR[7] = {0x00, 0x21, 0x0, 0x7F, 0x22, 0x0, 0x07};      // Setting the RAM pointer of ssd1306 to the begining
    ssd1306_send_oled_buffer(page_set, 4);
    ssd1306_send_oled_buffer(init_CURSOR, 7);

    // sending the temprory buffer to clear the display with desired clear_with value. 
    uint8_t *buffer = (uint8_t *)malloc(1025);
    if (!buffer) {
        ESP_LOGE(TAG, "Memory allocation failed for temprory clear buffer.");
        return;
    }

    buffer[0] = 0x40;
    memset(&buffer[1], clear_with, 1024);
    ssd1306_send_oled_buffer(buffer, 1025);
    free(buffer);
}



/*!
    @brief  send the display buffer to the ssd1306 oled.
    @param  oled_user_buffer
             oled_buffer pointer created by the viewbox function.
    @return None (Void)
    @note   this function take the oled_buffer struct, don't pass any array in it.
*/
void ssd1306_send_oled_display_buffer(oled_buffer *oled_user_buffer)
{

    // defining the start and the column of the buffer.

    uint8_t start_col = (uint8_t)(oled_user_buffer->col_pos - 1);
    uint8_t end_col = (uint8_t)((oled_user_buffer->col_pos + oled_user_buffer->width) - 2);

    // defining the start and the page of the buffer.

    uint8_t start_page = (uint8_t)(oled_user_buffer->page_pos - 1);
    uint8_t end_page = (uint8_t)(oled_user_buffer->page_pos + oled_user_buffer->page - 2);

    // keeping the sending buffer parameter in limit.
    end_page = end_page > 0x07 ? 0x07 : end_page;
    end_col = end_col > 0x7f ? 0x7f : end_col;


    uint8_t init_CURSOR[7] = {
        0x00,
        0x21,
        start_col,
        (end_col > 0x7f ? 0x7f : end_col),
        0x22,
        start_page,
        (end_page > 0x07 ? 0x07 : end_page)}; // Setting the RAM pointer of ssd1306 to the begining
    ssd1306_send_oled_buffer(init_CURSOR, 7);

    ESP_ERROR_CHECK(i2c_master_transmit(oled_handle, oled_user_buffer->oled_user_buffer, oled_user_buffer->oled_user_buffer_size, XFER_TIMEOUT_MS));

}



/*!
    @brief  place the buffer on the screen of oled at desired location.
    @param  oled_user_buffer
            oled_buffer pointer created by the viewbox function.
    @param  new_col
            it the value of the column in which you want to place viewbox
    @param  new_page
            it the value of the page in which you want to place viewbox
    @return None (Void)
    @note   it is the value of the top left corner of the viewbox, (make sure [new_col + width not exceed the 128] and [new_page + pages not exceed the 8] ).
*/
void ssd1306_place_oled_view(oled_buffer *oled_user_buffer, uint32_t new_col, uint32_t new_page)
{
    oled_user_buffer->col_pos = new_col;
    oled_user_buffer->page_pos = new_page;
}


/*!
    @brief  put a pixel white in the oled buffer.
    @param  oled_user_buffer
            oled_buffer pointer created by the viewbox function.
    @param  draw_x
            x coordinate of the pixel.
    @param  draw_y
            x coordinate of the pixel.
    @param  fill
            '1' for fill with white and '0' for fill with black.
    @return None (Void)
    @note   Here you have to pass the coordinate of the pixel. (this function don't accept value in column and page form).
*/
void ssd1306_draw_oled_apixel(oled_buffer *oled_user_buffer, int32_t draw_x, int32_t draw_y, uint8_t fill)
{

    draw_x = draw_x - 1;
    draw_y = draw_y - 1;
    uint16_t page = draw_y / 8;
    uint16_t col = (page * oled_user_buffer->width) + draw_x;
    uint8_t page_y;
    if(fill) page_y = oled_user_buffer->oled_user_buffer[col + 1] | (0x01 << (((uint8_t)draw_y) % 8));  // if fill is 1.
    else page_y = oled_user_buffer->oled_user_buffer[col + 1] & ~(0x01 << (((uint8_t)draw_y) % 8));     // if fill is 0.
    oled_user_buffer->oled_user_buffer[col + 1] = page_y;

}




/*!
    @brief  add the border to your viewbox oled buffer.
    @param  oled_user_buffer
            oled_buffer pointer created by the viewbox function.
    @param  up
            '1' for border and '0' for no border.
    @param  buttom
            '1' for border and '0' for no border.
    @param  left
            '1' for border and '0' for no border.
    @param  right
            '1' for border and '0' for no border.
    @return None (Void)
    @note   the border width is a bit.
*/
void ssd1306_viewbox_oled_border(oled_buffer *oled_user_buffer, uint8_t top, uint8_t buttom, uint8_t left, uint8_t right)
{

    if (top)
    {
        int32_t i = 0;
        while (i < oled_user_buffer->width)
        {
            uint8_t page_y = oled_user_buffer->oled_user_buffer[i + 1] | (0xFF >> 7);
            oled_user_buffer->oled_user_buffer[i + 1] = page_y;
            i++;
        }
    }

    if (buttom)
    {

        int32_t i = 0;
        while (i <= oled_user_buffer->width)
        {
            uint16_t col = (oled_user_buffer->oled_user_buffer_size - oled_user_buffer->width) + i;
            uint8_t page_y = oled_user_buffer->oled_user_buffer[col] | (0xFF << 7);
            oled_user_buffer->oled_user_buffer[col] = page_y;
            i++;
        }
    }

    if (left)
    {
        int32_t i = 0;
        while (i <= oled_user_buffer->page)
        {
            uint8_t page_y = oled_user_buffer->oled_user_buffer[i * oled_user_buffer->width + 1] | (0xFF);
            oled_user_buffer->oled_user_buffer[i * oled_user_buffer->width + 1] = page_y;
            i++;
        }
    }

    if (right)
    {
        int32_t i = 1;
        while (i <= oled_user_buffer->page)
        {
            uint8_t page_y = oled_user_buffer->oled_user_buffer[i * oled_user_buffer->width] | (0xFF);
            oled_user_buffer->oled_user_buffer[i * oled_user_buffer->width] = page_y;
            i++;
        }
    }
}


/*!
    @brief  draw circle.
    @param  oled_user_buffer
            oled_buffer pointer created by the viewbox function.
    @param  xc
            x coordinate of the circle center
    @param  yc
            y coordinate of the circle center
    @param  r
            radius of the circle.
    @return None (Void)
    @note   make sure your circle can properly draw in the buffer with the passed coordinate.
*/
void ssd1306_draw_circle(oled_buffer *buf, int xc, int yc, int r)
{
    if (!buf || r <= 0)
        return;

    int x = 0, y = r;
    int d = 3 - 2 * r;

    while (y >= x)
    {
        // Draw all 8 octants
        for (int dx = -1; dx <= 1; dx += 2)
        {
            for (int dy = -1; dy <= 1; dy += 2)
            {
                ssd1306_draw_oled_apixel(buf, xc + dx * x, yc + dy * y, 1);
                ssd1306_draw_oled_apixel(buf, xc + dx * y, yc + dy * x, 1);
            }
        }

        x++;
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
    }
}



/*!
    @brief  draw line.
    @param  oled_user_buffer
            oled_buffer pointer created by the viewbox function.
    @param  x0
            start x coordinate of the line.
    @param  y0
            start y coordinate of the line.
    @param  x1
            end x coordinate of the line.
    @param  y1
            end y coordinate of the line.
    @param  fill
            '1' for fill with white and '0' for fill with black.
    @return None (Void)
    @note   make sure your line can properly draw in the buffer with the passed coordinate.
*/
void ssd1306_draw_oled_line(oled_buffer *oled_user_buffer, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint8_t fill)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1)
    {
        // Set the pixel at (x0, y0)
        if (x0 >= 0 && x0 < (oled_user_buffer->width - 1) && y0 >= 0 && y0 < ((oled_user_buffer->page - 1) * 8))
        {
            uint16_t index = x0 + (y0 / 8) * (oled_user_buffer->width);
            if (fill)
                oled_user_buffer->oled_user_buffer[index + 1] |= (1 << (y0 % 8));
            else
                oled_user_buffer->oled_user_buffer[index + 1] &= ~(1 << (y0 % 8));
        }

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}


/*!
    @brief  stop the scrolling in the ssd1306.
    @return None (Void)
    @note   this stop scrolling which is done by the ssd1306 itself.
*/
void ssd1306_scroll_oled_stop(void)
{
    ssd1306_send_oled_command(0x2E);
}



/*!
    @brief  this init your font array so that you can use it with this library.
    @param  font_array
            pointer of  font array.
    @param  width
            width of the font.
    @param  height
           height of the font.
    @param  offset
            it is use to manage the conversion of the ASCI value and its font position in array.
    @return font_pack struct.
    @note   currently this library don't support custom font. so use the 5x8 font provided with this library.
*/
font_pack *ssd1306_init_oled_font(const uint8_t *font_array, uint8_t width, uint8_t height, uint8_t offset)
{
    if (width < 1)
        return NULL;
    
    font_pack *font_pack_array = (font_pack *)malloc(sizeof(font_pack));
    if(!font_pack_array)
    {
        ESP_LOGE(TAG, "Memory allocation failed for temprory clear buffer.");
        return NULL;
    }

    font_pack_array->font_array = font_array;
    font_pack_array->width = width;
    font_pack_array->height = height;
    font_pack_array->offset = offset;

    return font_pack_array;
}


/*!
    @brief  delete the initalized font pack and free the memory
    @param  font_pack
            font_pack struct pointer.
    @note   after deleting you can't use that font pack pointer variable futher, it will generate error
*/
void ssd1306_delete_oled_font(font_pack *font_pack)
{
    free(font_pack);
}



/*!
    @brief  put a character on the screen from the font_pack
    @param  oled_user_buffer
            oled_buffer type pointer
    @param  cursor
            position of cursor along x axis.
    @param  c
            char alphabet.
    @param  y_offset
            distance fromthe top of your buffer.
    @return None (void)
    @note   you the char you passed is not displaying from the font, try to change the font_pack offset value.
*/
void ssd1306_draw_oled_char(oled_buffer *oled_user_buffer, int cursor, char c, font_pack *font_pack, int y_offset)
{

    const uint8_t *char_bitmap = font_pack->font_array + (c - font_pack->offset) * font_pack->width;

    for (int i = 0; i < font_pack->width; i++)
    {
        uint8_t font_col = char_bitmap[i];

        // Split bits between two pages
        uint8_t upper = font_col << y_offset % 8;
        uint8_t lower = font_col >> (8 - y_offset % 8);

        int page_width = oled_user_buffer->width;
        int page_start = y_offset / 8 * page_width;
        int next_page_start = (y_offset / 8 + 1) * page_width;

        oled_user_buffer->oled_user_buffer[page_start + cursor + i + 1] |= upper;
        oled_user_buffer->oled_user_buffer[next_page_start + cursor + i + 1] |= lower;
    }

    oled_user_buffer->cursor = cursor;
}


/*!
    @brief  put the string on the screen from the font_pack
    @param  oled_user_buffer
            oled_buffer type pointer
    @param  cursor
            position of cursor along x axis.
    @param  string
            string value or the pointe of string value.
    @param  y_offset
            distance fromthe top of your buffer.
    @return None (void)
    @note   if the char you passed is not displaying from the font, try to change the font_pack offset value.
*/
void ssd1306_draw_oled_string(oled_buffer *oled_user_buffer, int cursor, const char *string, font_pack *font_pack, int y_offset)
{
    while (*string)
    {
        ssd1306_draw_oled_char(oled_user_buffer, cursor, *string, font_pack, y_offset);
        cursor += 6; // 5 pixels font + 1 spacing
        string++;
    }
}

/*!
    @brief  flush the oled_buffer with the bitmap.
    @param  oled_user_buffer
            oled_buffer type pointer
    @param  bitmap
            pointer of the bitmap array buffer.
    @param  size
            size of the bitmap array.
    @return None (void)
    @note   don't pass the buffer bigger then your oled_buffer and the bitmap should be in vertical 1 byte mode.
*/
void ssd1306_inset_oled_bitmap(oled_buffer *oled_user_buffer,const uint8_t *bitmap, int16_t size)
{
    memcpy(&oled_user_buffer->oled_user_buffer[1], bitmap, size>oled_user_buffer->oled_user_buffer_size?oled_user_buffer->oled_user_buffer_size:size);    
}


/*!
    @brief  shift the oled buffer.
    @param  oled_user_buffer
            oled_buffer type pointer
    @param  direction
            you may choose from enum values, HORIZONTAL_LEFT, HORIZONTAL_RIGHT, VERTICAL_DOWN, VERTICAL_UP
    @param  steps
            number of step with you want to shift your buffer.
    @return None (void)
    @note   it shift oled buffer by 1 bit therefore 1 step = 1 bit shift.
*/
void ssd1306_shift_oled_buffer(oled_buffer *oled_user_buffer, scroll_type direction, uint32_t steps)
{
    if (direction == HORIZONTAL_LEFT)
    {
        for (int page = 0; page < steps; page++)
        {
            uint8_t *page_start = oled_user_buffer->oled_user_buffer + (page * oled_user_buffer->width);
            uint8_t temp = page_start[1]; // store first byte
            for (int col = 0; col < oled_user_buffer->width; col++)
            {
                page_start[col + 1] = page_start[col + 2];
            }

            // Wrap the first data byte to the end
            page_start[oled_user_buffer->width] = temp;
        }
        ssd1306_send_oled_display_buffer(oled_user_buffer);
    }
    if (direction == HORIZONTAL_RIGHT)
    {
        for (int page = 0; page < steps; page++)
        {
            uint8_t *page_start = oled_user_buffer->oled_user_buffer + (page * oled_user_buffer->width);
            uint8_t temp = page_start[oled_user_buffer->width]; // store first byte
            for (int col = oled_user_buffer->width; col > 1; col--)
            {
                page_start[col] = page_start[col - 1];
            }

            // Wrap the first data byte to the end
            page_start[1] = temp;
        }
        ssd1306_send_oled_display_buffer(oled_user_buffer);
    }
    if (direction == VERTICAL_DOWN)
    {
        int width = oled_user_buffer->width;
        int pages = oled_user_buffer->page - 1;

        for (int col = 1; col <= steps; col++)
        {
            uint8_t carry = (oled_user_buffer->oled_user_buffer[(pages * width) + col] & 0x80) >> 7;

            for (int page = 0; page <= pages; page++)
            {
                uint8_t *byte = &oled_user_buffer->oled_user_buffer[page * width + col];
                uint8_t new_carry = (*byte & 0x80) >> 7; // get MSB before shifting
                *byte = (*byte << 1) | carry;            // shift and insert carry
                carry = new_carry;
            }
        }

        ssd1306_send_oled_display_buffer(oled_user_buffer);
    }
    if (direction == VERTICAL_UP)
    {
        int width = oled_user_buffer->width;
        int pages = oled_user_buffer->page - 1;

        for (int col = 1; col <= steps; col++)
        {
            uint8_t carry = (oled_user_buffer->oled_user_buffer[col] & 0x01) << 7;

            for (int page = pages; page >= 0; page--)
            {
                uint8_t *byte = &oled_user_buffer->oled_user_buffer[page * width + col];
                uint8_t new_carry = (*byte & 0x01) << 7;
                *byte = (*byte >> 1) | carry;
                carry = new_carry;
            }
        }
    }
}


/*!
    @brief  task to do the scrolling.
    @param  pvargs
            takes a void pointer.
    @return None (void)
    @note   don't now call this function externaly, this task function is designed to called by only this library.
*/
void scroll_task(void *pvargs)
{
    active_scroll_view **views_pointer = (active_scroll_view **)pvargs;

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 20 / portTICK_PERIOD_MS; // 50ms between each frame (adjust as needed)
    xLastWakeTime = xTaskGetTickCount();                   // initialize the wake time

    active_scroll_view *current_scroll = *views_pointer;
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        if(*views_pointer == NULL){
            vTaskDelete(NULL);
        }
        if (current_scroll->view->scroll_type == HORIZONTAL_LEFT)
        {
            for (int page = 0; page < current_scroll->view->page; page++)
            {
                uint8_t *page_start = current_scroll->view->oled_user_buffer + (page * current_scroll->view->width);
                uint8_t temp = page_start[1]; // store first byte
                for (int col = 0; col < current_scroll->view->width; col++)
                {
                    page_start[col + 1] = page_start[col + 2];
                }

                // Wrap the first data byte to the end
                page_start[current_scroll->view->width] = temp;
            }
            ssd1306_send_oled_display_buffer(current_scroll->view);
        }
        if (current_scroll->view->scroll_type == HORIZONTAL_RIGHT)
        {
            for (int page = 0; page < current_scroll->view->page; page++)
            {
                uint8_t *page_start = current_scroll->view->oled_user_buffer + (page * current_scroll->view->width);
                uint8_t temp = page_start[current_scroll->view->width]; // store first byte
                for (int col = current_scroll->view->width; col > 1; col--)
                {
                    page_start[col] = page_start[col - 1];
                }

                // Wrap the first data byte to the end
                page_start[1] = temp;
            }
            ssd1306_send_oled_display_buffer(current_scroll->view);
        }
        if (current_scroll->view->scroll_type == VERTICAL_DOWN)
        {
            int width = current_scroll->view->width;
            int pages = current_scroll->view->page - 1;

            for (int col = 1; col <= width; col++)
            {
                uint8_t carry = (current_scroll->view->oled_user_buffer[(pages * width) + col] & 0x80) >> 7;

                for (int page = 0; page <= pages; page++)
                {
                    uint8_t *byte = &current_scroll->view->oled_user_buffer[page * width + col];
                    uint8_t new_carry = (*byte & 0x80) >> 7; // get MSB before shifting
                    *byte = (*byte << 1) | carry;            // shift and insert carry
                    carry = new_carry;
                }
            }

            ssd1306_send_oled_display_buffer(current_scroll->view);
        }
        if (current_scroll->view->scroll_type == VERTICAL_UP)
        {
            int width = current_scroll->view->width;
            int pages = current_scroll->view->page - 1;

            for (int col = 1; col <= width; col++)
            {
                uint8_t carry = (current_scroll->view->oled_user_buffer[col] & 0x01) << 7;

                for (int page = pages; page >= 0; page--)
                {
                    uint8_t *byte = &current_scroll->view->oled_user_buffer[page * width + col];
                    uint8_t new_carry = (*byte & 0x01) << 7;
                    *byte = (*byte >> 1) | carry;
                    carry = new_carry;
                }
            }
            ssd1306_send_oled_display_buffer(current_scroll->view);
        }
        current_scroll = current_scroll->next_view != NULL ? current_scroll->next_view : *views_pointer;
    }
    vTaskDelete(NULL);
}


/*!
    @brief  add the scrolling to your oled buffer.
    @param  oled_user_buffer
            oled_buffer type pointer
    @param  scroll
            you may choose from enum values, HORIZONTAL_LEFT, HORIZONTAL_RIGHT, VERTICAL_DOWN, VERTICAL_UP
    @return None (void)
    @note   you can't pass another scrolling type to a buffer without stopping it first.
*/
void ssd1306_scroll_oled_view(oled_buffer *oled_user_buffer, scroll_type scroll)
{
    if (scroll_views_pointer_head == NULL) {
        ESP_LOGE(TAG, "scroll_views_pointer_head is NULL. Cannot proceed.");
        return;
    }

    if (oled_user_buffer->scroll_type != NO_SCROLL)
    {
        ESP_LOGI(TAG, "Can't apply scroll; already active scroll.");
        return;
    }

    oled_user_buffer->scroll_type = scroll;

    active_scroll_view *scroll_view = malloc(sizeof(active_scroll_view));
    if (!scroll_view) {
        ESP_LOGE(TAG, "Memory allocation failed for scroll_view.");
        return;
    }

    scroll_view->view = oled_user_buffer;

    if (*scroll_views_pointer_head == NULL) {
        scroll_view->prev_view = scroll_view;
        scroll_view->next_view = scroll_view;
        *scroll_views_pointer_head = scroll_view;
    } else {
        active_scroll_view *head = *scroll_views_pointer_head;
        active_scroll_view *last = head->prev_view;

        scroll_view->next_view = head;
        scroll_view->prev_view = last;
        head->prev_view = scroll_view;
        last->next_view = scroll_view;
    }

    if (!activate_scroll) {
        xTaskCreate(scroll_task, "ssd1306_scroll_task", 2048, (void *)scroll_views_pointer_head, 2, &activate_scroll);
    }
}

/*!
    @brief  remove the scrolling from your oled buffer.
    @param  oled_user_buffer
            oled_buffer type pointer
    @return None (void)
*/
void ssd1306_scroll_stop_oled_view(oled_buffer *oled_user_buffer)
{

    if (oled_user_buffer == NULL || oled_user_buffer->scroll_type == NO_SCROLL)
    {
        ESP_LOGW(TAG, "Nothing to stop. View is not scrolling.");
        return;
    }

    if (scroll_views_pointer_head == NULL || *scroll_views_pointer_head == NULL)
    {
        ESP_LOGE(TAG, "scroll_views_pointer_head is NULL. Cannot stop scroll.");
        return;
    }

    active_scroll_view *current = *scroll_views_pointer_head;
    active_scroll_view *start = current;
    do
    {
        if (current->view == oled_user_buffer)
        {
            // Disconnect from linked list
            if (current->next_view == current && current->prev_view == current)
            {
                // Only one in list
                *scroll_views_pointer_head = NULL;
                activate_scroll = NULL;
            }
            else
            {
                current->prev_view->next_view = current->next_view;
                current->next_view->prev_view = current->prev_view;

                if (*scroll_views_pointer_head == current)
                    *scroll_views_pointer_head = current->next_view;
            }

            // Stop scroll in buffer
            oled_user_buffer->scroll_type = NO_SCROLL;

            // Free view
            // free(current);
            ESP_LOGI(TAG, "Scroll stopped and view removed.");
            // eTaskState state = eTaskGetState(activate_scroll);
            // ESP_LOGW(TAG, "Scroll task is not suspended, state: %d", state);
            // vTaskResume(activate_scroll);
            return;
        }
        current = current->next_view;
    } while (current != start);

    ESP_LOGW(TAG, "Scroll view not found for given buffer.");
}