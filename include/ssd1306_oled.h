#ifndef SSD1306_OLED_H
#define SSD1306_OLED_H

#include <stdint.h>
#include <driver/i2c_master.h>
#include <string.h>

// I2C Config parameters.

#define I2C_PORT                    0
#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_SCL_SPEED               400000
#define GLITCH_IGNORE_CNT           7
#define ENABLE_INPERRUPT_PULLUP     1
#define XFER_TIMEOUT_MS             (1000 / portTICK_PERIOD_MS)

// SSD1306 oled commands, taken from it datasheet.

#define OLED_SET_CONTRAST_CONTROL               0x81
#define OLED_ENTIRE_DISPLAY_ON_RAM_CONTENT      0xA4
#define OLED_ENTIRE_DISPLAY_LED_ON              0xA5
#define OLED_SET_NORMAL_DISPLAY                 0xA6
#define OLED_SET_INVERSE_DISPLAY                0xA7
#define OLED_SET_DISPLAY_OFF                    0xAE
#define OLED_SET_DISPLAY_ON                     0xAF
#define OLED_RIGHT_HORIZONTAL_SCROLL            0x26
#define OLED_LEFT_HORIZONTAL_SCROLL             0x27
#define OLED_VER_RIGHT_HORIZ_SCROLL             0x29
#define OLED_VER_LEFT_HORIZ_SCROLL              0x2A
#define OLED_STOP_SCROLLING                     0x2E
#define OLED_START_SCROLLING                    0X2F
#define OLED_SET_VERTICAL_SCROLL_AREA           0xA3
#define OLED_SET_LOWER_COL_START_LINE           0x00
#define OLED_SET_HIGHER_COL_START_LINE          0x10
#define OLED_SET_MEMORY_ADDRESS_MODE            0X20
#define OLED_SET_COL_ADDRESS                    0x21
#define OLED_SET_PAGE_ADDRESS                   0x22
#define OLED_SET_PAGE_START_ADDRESS             0xB0
#define OLED_SET_DISPLAY_START_LINE             0X40
#define OLED_SET_SEGMENT_REMAP                  0xA1
#define OLED_SET_MULTIPLEX_RATIO                0xA8
#define OLED_SET_COM_OUPUT_SCAN_NORMAL          0xC0
#define OLED_SET_COM_OUTPUT_SCAN_REMAPPED       0xC8
#define OLED_SET_DISPLAY_OFFSET                 0xD3
#define OLED_SET_COM_PIN                        0XDA
#define OLED_SET_CLOCK_DIVIDE_RATIO             0xD5
#define OLED_SET_PRECHARGE_PERIOD               0xD9
#define OLED_SET_VCOM_DESELECT_LEVEL            0xDB
#define OLED_CHARGE_PUMP_SETTING                0x8D
#define OLED_NO_OPERATION                       0xE3



// enum for identifing different directions
typedef enum
{
    NO_SCROLL,
    HORIZONTAL_RIGHT,
    HORIZONTAL_LEFT,
    VERTICAL_DOWN,
    VERTICAL_UP
} scroll_type;                             


// struct for managing the oled buffer with different properties.
typedef struct
{
    uint8_t *oled_user_buffer;
    uint32_t oled_user_buffer_size;
    uint16_t width;
    uint8_t page;
    uint16_t col_pos;
    uint8_t page_pos;
    scroll_type scroll_type;
    uint8_t cursor;
} oled_buffer;


// struct for managing the fonts.
typedef struct
{
    const uint8_t *font_array;
    uint8_t width;
    uint8_t height;
    uint8_t offset;
} font_pack;


// struct for managing the scrolling of different buffer through the linked list.
struct active_scroll_view;
typedef struct active_scroll_view {
    oled_buffer *view;
    struct active_scroll_view *next_view;
    struct active_scroll_view *prev_view;
} active_scroll_view;





void ssd1306_init_oled_i2c( uint8_t i2c_address, uint8_t sda_gpio_pin, uint8_t scl_gpio_pin );

font_pack *ssd1306_init_oled_font(const uint8_t *font_array, uint8_t width, uint8_t height, uint8_t offset);

void ssd1306_delete_oled_font(font_pack *font_array);

oled_buffer *ssd1306_create_viewbox(uint16_t width, uint8_t page);

void ssd1306_delete_viewbox(oled_buffer *oled_user_buffer);

void ssd1306_viewbox_oled_border(oled_buffer *oled_user_buffer, uint8_t top, uint8_t buttom, uint8_t left, uint8_t right);

void ssd1306_draw_circle(oled_buffer *buf, int xc, int yc, int r);

void ssd1306_send_oled_command(uint8_t command);

void ssd1306_oled_clear_view(oled_buffer * oled_user_buffer, uint8_t clear_with);

void ssd1306_oled_clear(uint8_t clear_with);

void ssd1306_place_oled_view(oled_buffer *oled_user_buffer ,uint32_t new_col, uint32_t new_page);

void ssd1306_send_oled_buffer(uint8_t * buffer, size_t size);

void ssd1306_send_oled_display_buffer(oled_buffer *oled_user_buffer);

void ssd1306_scroll_oled_stop(void);

void ssd1306_scroll_oled_view(oled_buffer *oled_user_buffer, scroll_type scroll);

void ssd1306_scroll_stop_oled_view(oled_buffer *oled_user_buffer);

void ssd1306_draw_oled_apixel(oled_buffer *oled_user_buffer, int32_t draw_x, int32_t draw_y, uint8_t fill);

void ssd1306_draw_oled_line(oled_buffer *oled_user_buffer, int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y, uint8_t fill);

void ssd1306_draw_oled_char(oled_buffer *oled_user_buffer, int cursor,char c, font_pack *font_pack, int offset);

void ssd1306_draw_oled_string(oled_buffer *oled_user_buffer, int cursor, const char *c, font_pack *font_pack, int y_offset);

void scroll_task(void *pvargs);

void ssd1306_shift_oled_buffer(oled_buffer *oled_user_buffer,scroll_type direction,uint32_t steps);

void ssd1306_inset_oled_bitmap(oled_buffer *oled_user_buffer, const uint8_t *bitmap, int16_t size);


#endif 