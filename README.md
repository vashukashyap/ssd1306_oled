# SSD1306 OLED Display Driver for ESP-IDF

A lightweight and customizable **SSD1306 OLED display driver** written in **C for ESP-IDF**, supporting I2C communication.  
Includes features like **multi-view scrolling**, **ASCII text rendering**, **bitmap drawing**, and **buffer manipulation**.

> ⚡ Designed for embedded developers using ESP32 with ESP-IDF.  
> 📦 Modular component structure.  
> 💻 MIT Licensed – free for personal and commercial use.  


## Features

- ✅ Supports **128x64** and **128x32** OLED displays using **I2C**
- ✅ Draw text using **5x8**.
- ✅ Render **bitmaps** (vertical layouts)
- ✅ Multi-directional **software scrolling**
- ✅ Efficient buffer system with **view management**
- ✅ Optimized for **FreeRTOS**
- ✅ Clean, readable **API** with documentation support (Doxygen)  

---

## Getting Started

### 1. 📦 Add to Your ESP-IDF Project

Clone this repository into your project's `components` directory:

```
cd your-project/components
git clone https://github.com/vashukashyap/ssd1306_oled.git 
```

## add it as a Git submodule:
```
git submodule add https://github.com/vashukashyap/ssd1306_oled.git components/ssd1306_oled
```

### Include in Your Project
In your CMakeLists.txt:
``` set(EXTRA_COMPONENT_DIRS components/ssd1306_oled) ```
In your source file:
``` #include "ssd1306_oled.h" ```

## Configuration
Make sure to set the correct I2C pins in your main app. Example:
``` ssd1306_init_desc(&dev, I2C_NUM_0, SDA_GPIO, SCL_GPIO); ```

## Example Usage
More examples can be found in the examples/ directory.
- basic.   
  ![1000013285](https://github.com/user-attachments/assets/874f723d-8018-4c94-a70c-0ce22eb146e3)


## 🙌 Contributions
Feel free to open issues, suggest features, or submit pull requests.
