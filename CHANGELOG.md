# Changelog

All notable changes to the `SimpleMAX98357A` library will be documented in this file.

## [Unreleased]
- No unreleased changes at this time.

## [1.0.0] - 2025-02-20
### Initial Release
- **Overview**: Initial release of the `SimpleMAX98357A` library, designed for the MAX98357A I2S amplifier on ESP32, supporting MP3 and WAV playback with flexible storage options (SD_MMC, SD via SPI, LittleFS, SPIFFS).
- **Development History**:
  - **Initial Concept**: Created a basic I2S driver for MAX98357A with WAV playback, using raw PCM data streaming (first draft provided on 2025-02-20).
  - **Feature Expansion**: Added volume control (`setVolume`, `volumeUp`, `volumeDown`), playback controls (`play`, `stop`, `pause`, `resume`, `next`, `previous`, `loop`, `repeat`), and status checks (`isPlaying`, `isStorageConnected`) based on user request to mimic DFPlayer Mini functionality.
  - **Storage Flexibility**: Evolved from SD-only to support multiple filesystems via `FS` abstraction. Initially integrated SD_MMC directly in the library, then reverted to user-defined storage initialization in the sketch for greater flexibility (e.g., SD_MMC, SD, LittleFS, SPIFFS).
  - **MP3 Support**: Replaced external `ESP8266Audio` dependency with Espressif’s `libhelix-mp3` for MP3 decoding, ensuring stability and reducing third-party reliance.
  - **Track Indexing**: Expanded from playlist arrays to automatic indexing (0-98 for tracks `/01.mp3` to `/99.mp3` or `.wav`), simplifying user interaction.
  - **Non-Blocking**: Ensured playback is non-blocking via `loop()` function, allowing user code to run concurrently.
  - **Error Handling**: Added `error_t` enum and `getLastError()` for robust debugging.
  - **Equalizer Consideration**: Briefly included basic bass/treble EQ, removed due to complexity and user preference for simplicity.
  - **Examples**: Provided separate examples for SD_MMC, SD_SPI, LittleFS, SPIFFS, and dual-storage use cases in `/examples/`.
- **Features**:
  - Supports up to 99 tracks (`/01.mp3` to `/99.mp3` or `.wav`).
  - Volume control (0-100).
  - Playback control with track indexing.
  - Flexible storage via `FS` (user-initialized in sketch).
  - Error reporting.
- **Limitations**:
  - WAV support limited to 16-bit PCM for simplicity.
  - MP3 assumes 44.1kHz sample rate (common standard); non-standard rates may require adjustments.
- **Dependencies**:
  - `espressif/esp32-libhelix-mp3` for MP3 decoding.
- **Notes**:
  - Designed for ESP32 compatibility, tested with ESP32-S3 SD_MMC setup (pins: CLK=13, CMD=12, DATA1=21, DATA2=10, DATA3=11; I2S: BCLK=27, LRCLK=26, DIN=25).
  - Users configure storage and pins in their sketch; library remains hardware-agnostic beyond I2S.
