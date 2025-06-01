
/**
 * Private configuration file for the SSD1306 library.
 */

#ifndef SSD1306_CONF_H__
#define SSD1306_CONF_H__

// Choose a bus
#define SSD1306_USE_I2C
//#define SSD1306_USE_SPI

// Mirror the screen if needed
// #define SSD1306_MIRROR_VERT
// #define SSD1306_MIRROR_HORIZ

// Set inverse color if needed
// # define SSD1306_INVERSE_COLOR

// Include only needed fonts
//#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
//#define SSD1306_INCLUDE_FONT_11x18
//#define SSD1306_INCLUDE_FONT_16x26
//#define SSD1306_INCLUDE_FONT_16x24

// Some OLEDs don't display anything in first two columns.
// In this case change the following macro to 130.
// The default value is 128.
// #define SSD1306_WIDTH           130

// The height can be changed as well if necessary.
// It can be 32, 64 or 128. The default value is 64.
// #define SSD1306_HEIGHT          32

#endif /* __SSD1306_CONF_H__ */
