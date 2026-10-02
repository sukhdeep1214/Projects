#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "driver/i2s_std.h"

#define SD_CS          5
#define I2S_BCLK      27
#define I2S_LRC       26
#define I2S_DOUT      25

File audioFile;
i2s_chan_handle_t tx_chan = NULL;
#define BUFFER_SIZE 512
uint8_t audioBuffer[BUFFER_SIZE];

void initHardwareI2S() {
    // Clean up if channel already exists (for clean re-looping)
    if (tx_chan != NULL) {
        i2s_channel_disable(tx_chan);
        i2s_del_channel(tx_chan);
    }

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, &tx_chan, NULL);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(24000), // FIXED: Matched to your 24000 Hz file
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO), // FIXED: Matched to your 1-channel file
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)I2S_BCLK,
            .ws   = (gpio_num_t)I2S_LRC,
            .dout = (gpio_num_t)I2S_DOUT,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false }
        }
    };

    i2s_channel_init_std_mode(tx_chan, &std_cfg);
    i2s_channel_enable(tx_chan);
}

void setup() {
    Serial.begin(115200);
    
    if(!SD.begin(SD_CS)){
        Serial.println("SD Card Mount Failed!");
        while(true);
    }
    Serial.println("SD Card Mounted successfully!");

    initHardwareI2S();

    audioFile = SD.open("/test.wav", FILE_READ);
    if(!audioFile){
        Serial.println("Failed to find test.wav!");
        while(true);
    }
    
    audioFile.seek(44); 
    Serial.println("Streaming crystal clear audio...");
}

void loop() {
    // FIXED: Changed buffer threshold check to handle the end of the file smoothly
    if (audioFile.available() > 0) { 
        int bytesToRead = audioFile.available();
        if (bytesToRead > BUFFER_SIZE) {
            bytesToRead = BUFFER_SIZE;
        }

        int bytesRead = audioFile.read(audioBuffer, bytesToRead);
        size_t bytesWritten = 0;
        
        i2s_channel_write(tx_chan, audioBuffer, bytesRead, &bytesWritten, portMAX_DELAY);
    } else {
        Serial.println("Track ended. Re-centering audio track...");
        audioFile.seek(44); // Force reset pointer back to audio start
        delay(1000); 
    }
}
