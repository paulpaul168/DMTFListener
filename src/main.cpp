#include <Arduino.h>
#include <driver/i2s.h>
#include <driver/dac.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 16
#define SCREEN_ADDRESS 0x3C
#define OLED_SDA 4
#define OLED_SCL 15

// I2S Microphone Configuration
#define I2S_WS 25  // Word Select
#define I2S_SD 34  // Serial Data (input-only pin)
#define I2S_SCK 33 // Serial Clock
#define I2S_PORT I2S_NUM_0

// Audio Processing Configuration
#define SAMPLE_RATE 8000
#define BUFFER_SIZE 1024
#define TARGET_FREQS 8
#define N BUFFER_SIZE

// Display Configuration
#define DISPLAY_UPDATE_INTERVAL 100 // Update every 100ms
#define HISTORY_SIZE 12

// DTMF Frequency Configuration
const float dtmf_freqs[] = {697, 770, 852, 941, 1209, 1336, 1477, 1633};
const char *freq_names[] = {"697Hz", "770Hz", "852Hz", "941Hz", "1209Hz", "1336Hz", "1477Hz", "1633Hz"};
const char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// Global Variables
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int16_t samples[BUFFER_SIZE];
float coeffs[TARGET_FREQS];
float q1[TARGET_FREQS];
float q2[TARGET_FREQS];

char lastKey = ' ';
char digitHistory[HISTORY_SIZE + 1] = "";
int historyLength = 0;
unsigned long lastDebugTime = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long detectionCount = 0;

// Configuration Variables
float detectionThreshold = 1000.0;
bool debugMode = true;

void setupI2S()
{
  // Disable DAC on GPIO25 to avoid conflicts
  dac_output_disable(DAC_CHANNEL_1);

  // Configure I2S
  i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0};

  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD};

  // Install and configure I2S driver
  esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (result != ESP_OK)
  {
    Serial.printf("I2S driver install failed: %d\n", result);
    // Try alternative format
    i2s_config.communication_format = I2S_COMM_FORMAT_I2S;
    result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  }

  if (result == ESP_OK)
  {
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_start(I2S_PORT);
    Serial.println("✅ I2S initialized successfully");
  }
  else
  {
    Serial.printf("❌ I2S initialization failed: %d\n", result);
  }
}

void setupOLED()
{
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("DTMF Listener"));
  display.println(F("Initializing..."));
  display.display();
}

void calculateDTMFCoefficients()
{
  for (int i = 0; i < TARGET_FREQS; i++)
  {
    float normalizedFreq = dtmf_freqs[i] / SAMPLE_RATE;
    coeffs[i] = 2 * cos(2 * PI * normalizedFreq);
    if (debugMode)
    {
      Serial.printf("Freq %s: coeff=%.6f\n", freq_names[i], coeffs[i]);
    }
  }
}

void printInitialInfo()
{
  Serial.println("DTMF Listener v1.0");
  Serial.println("==================");
  Serial.println("Pin Configuration:");
  Serial.println("- Microphone GND -> GND");
  Serial.println("- Microphone VDD -> 3.3V");
  Serial.println("- Microphone WS  -> GPIO 25");
  Serial.println("- Microphone SCK -> GPIO 33");
  Serial.println("- Microphone SD  -> GPIO 34");
  Serial.println("- Microphone L/R -> GND");
  Serial.println("==================");
  Serial.println("Commands:");
  Serial.println("  'd' - Toggle debug mode");
  Serial.println("  's' - Show raw samples");
  Serial.println("  't' - Test microphone");
  Serial.println("  'r' - Reset I2S driver");
  Serial.println("==================");
}

void addToHistory(char key)
{
  if (historyLength < HISTORY_SIZE)
  {
    digitHistory[historyLength] = key;
    historyLength++;
    digitHistory[historyLength] = '\0';
  }
  else
  {
    // Shift history left and add new key
    for (int i = 0; i < HISTORY_SIZE - 1; i++)
    {
      digitHistory[i] = digitHistory[i + 1];
    }
    digitHistory[HISTORY_SIZE - 1] = key;
    digitHistory[HISTORY_SIZE] = '\0';
  }
}

void handleSerialCommands()
{
  if (!Serial.available())
    return;

  char cmd = Serial.read();
  switch (cmd)
  {
  case 'd':
  case 'D':
    debugMode = !debugMode;
    Serial.printf("Debug mode: %s\n", debugMode ? "ON" : "OFF");
    break;

  case 's':
  case 'S':
    Serial.println("\n======= RAW SAMPLES =======");
    for (int i = 0; i < 20; i++)
    {
      Serial.printf("Sample[%d]: %d\n", i, samples[i]);
    }
    Serial.println("===========================\n");
    break;

  case 't':
  case 'T':
    performMicrophoneTest();
    break;

  case 'r':
  case 'R':
    resetI2SDriver();
    break;
  }
}

void performMicrophoneTest()
{
  Serial.println("\n🎤 MICROPHONE TEST - 10 seconds");
  Serial.println("Make noise into the microphone NOW!");

  unsigned long testStart = millis();
  float maxAudioSeen = 0;
  int maxSampleSeen = 0;
  int totalNonZero = 0;
  int testCount = 0;

  while (millis() - testStart < 10000)
  {
    size_t bytes_read;
    esp_err_t result = i2s_read(I2S_PORT, (char *)samples, BUFFER_SIZE * sizeof(int16_t), &bytes_read, 100);

    if (result == ESP_OK)
    {
      float audioLevel = 0;
      int nonZero = 0;
      int16_t maxSample = 0;

      for (int i = 0; i < BUFFER_SIZE; i++)
      {
        audioLevel += abs(samples[i]);
        if (samples[i] != 0)
          nonZero++;
        if (abs(samples[i]) > abs(maxSample))
          maxSample = samples[i];
      }
      audioLevel /= BUFFER_SIZE;

      if (audioLevel > maxAudioSeen)
        maxAudioSeen = audioLevel;
      if (abs(maxSample) > abs(maxSampleSeen))
        maxSampleSeen = maxSample;
      totalNonZero += nonZero;
      testCount++;

      if (testCount % 20 == 0)
      {
        Serial.printf("Test %ds: Audio=%.1f, Max=%d, NonZero=%d\n",
                      (int)((millis() - testStart) / 1000), audioLevel, maxSample, nonZero);
      }
    }
    delay(50);
  }

  Serial.printf("\n📊 TEST RESULTS:\n");
  Serial.printf("  Max audio level: %.1f\n", maxAudioSeen);
  Serial.printf("  Max sample value: %d\n", maxSampleSeen);
  Serial.printf("  Avg non-zero samples: %d/%d\n", totalNonZero / testCount, BUFFER_SIZE);

  if (maxAudioSeen == 0)
  {
    Serial.println("❌ NO AUDIO DETECTED!");
    Serial.println("Check microphone connections and power");
  }
  else
  {
    Serial.printf("✅ Audio detected! Level: %.1f\n", maxAudioSeen);
  }
}

void resetI2SDriver()
{
  Serial.println("🔄 Resetting I2S driver...");
  i2s_driver_uninstall(I2S_PORT);
  delay(100);
  setupI2S();
}

void updateDisplay(char detectedKey, float max_row, float max_col, float confidence)
{
  if (millis() - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL)
    return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Title and count
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("DTMF Live:");
  display.setCursor(80, 0);
  display.printf(":%lu", detectionCount);

  // Large digit display
  display.setCursor(0, 12);
  display.setTextSize(4);
  if (max_row > detectionThreshold && max_col > detectionThreshold && detectedKey != ' ')
  {
    display.print(detectedKey);
  }
  else
  {
    display.print("-");
  }

  // Signal levels
  display.setTextSize(1);
  display.setCursor(70, 12);
  display.printf("R:%.0f", max_row);
  display.setCursor(70, 22);
  display.printf("C:%.0f", max_col);
  display.setCursor(70, 32);
  display.printf("T:%.0f", detectionThreshold);

  // Status
  display.setCursor(0, 44);
  if (max_row > detectionThreshold && max_col > detectionThreshold)
  {
    display.print("DETECTED!");
  }
  else if (confidence > 50)
  {
    display.print("Signal...");
  }
  else
  {
    display.print("Listening");
  }

  // History
  display.setCursor(0, 54);
  display.print("Last: ");
  for (int i = 0; i < historyLength; i++)
  {
    display.print(digitHistory[i]);
  }

  display.display();
  lastDisplayUpdate = millis();
}

void printDebugInfo(float audioLevel, float max_row, float max_col, float magnitudes[])
{
  if (!debugMode || (millis() - lastDebugTime < 1000 && audioLevel <= 1000))
    return;

  Serial.println("========== DEBUG INFO ==========");
  Serial.printf("Audio Level: %.1f\n", audioLevel);
  Serial.printf("Row max: %.1f, Col max: %.1f\n", max_row, max_col);
  Serial.printf("Threshold: %.0f\n", detectionThreshold);

  Serial.println("Frequency Magnitudes:");
  for (int i = 0; i < TARGET_FREQS; i++)
  {
    Serial.printf("  %s: %.1f\n", freq_names[i], magnitudes[i]);
  }

  if (audioLevel == 0)
  {
    Serial.println("⚠️  NO AUDIO - Check microphone connections");
  }

  lastDebugTime = millis();
}

void setup()
{
  Serial.begin(115200);
  printInitialInfo();

  setupOLED();
  setupI2S();
  calculateDTMFCoefficients();

  // Update display to ready status
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("DTMF Listener"));
  display.println(F("Ready"));
  display.println();
  display.println(F("Waiting for"));
  display.println(F("DTMF tones..."));
  display.display();

  Serial.println("✅ DTMF Listener initialized!");
}

void loop()
{
  handleSerialCommands();

  // Read audio samples
  size_t bytes_read;
  esp_err_t result = i2s_read(I2S_PORT, (char *)samples, BUFFER_SIZE * sizeof(int16_t), &bytes_read, portMAX_DELAY);

  if (result != ESP_OK)
  {
    Serial.printf("I2S read error: %d\n", result);
    return;
  }

  // Calculate audio level
  float audioLevel = 0;
  for (int i = 0; i < BUFFER_SIZE; i++)
  {
    audioLevel += abs(samples[i]);
  }
  audioLevel /= BUFFER_SIZE;

  // Goertzel algorithm for DTMF detection
  memset(q1, 0, sizeof(q1));
  memset(q2, 0, sizeof(q2));

  for (int i = 0; i < N; i++)
  {
    float sample = samples[i];
    for (int f = 0; f < TARGET_FREQS; f++)
    {
      float q0 = coeffs[f] * q1[f] - q2[f] + sample;
      q2[f] = q1[f];
      q1[f] = q0;
    }
  }

  // Calculate magnitudes
  float magnitudes[TARGET_FREQS];
  for (int f = 0; f < TARGET_FREQS; f++)
  {
    magnitudes[f] = sqrt(q1[f] * q1[f] + q2[f] * q2[f] - q1[f] * q2[f] * coeffs[f]);
  }

  // Find maximum magnitudes for row and column frequencies
  int row = -1, col = -1;
  float max_row = 0, max_col = 0;

  for (int i = 0; i < 4; i++)
  {
    if (magnitudes[i] > max_row)
    {
      max_row = magnitudes[i];
      row = i;
    }
  }

  for (int i = 4; i < 8; i++)
  {
    if (magnitudes[i] > max_col)
    {
      max_col = magnitudes[i];
      col = i - 4;
    }
  }

  // Determine detected key
  char detectedKey = ' ';
  float confidence = 0;

  if (row >= 0 && col >= 0)
  {
    detectedKey = keys[row][col];
    confidence = (max_row + max_col) / 2;
  }

  // Process detection
  if (max_row > detectionThreshold && max_col > detectionThreshold)
  {
    if (detectedKey != lastKey)
    {
      lastKey = detectedKey;
      detectionCount++;
      addToHistory(detectedKey);

      Serial.printf("\n🎯 DTMF DETECTED: %c (#%lu)\n", detectedKey, detectionCount);
      Serial.printf("Row: %s (%.1f), Col: %s (%.1f)\n",
                    freq_names[row], max_row, freq_names[col + 4], max_col);
    }
  }
  else
  {
    // Reset detection if signal drops
    if (lastKey != ' ' && max_row < detectionThreshold / 2 && max_col < detectionThreshold / 2)
    {
      lastKey = ' ';
    }
  }

  printDebugInfo(audioLevel, max_row, max_col, magnitudes);
  updateDisplay(detectedKey, max_row, max_col, confidence);

  delay(10);
}
