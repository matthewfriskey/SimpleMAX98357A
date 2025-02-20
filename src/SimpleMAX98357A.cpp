#include "SimpleMAX98357A.h"

SimpleMAX98357A::SimpleMAX98357A() 
    : _fs(nullptr), _playing(false), _paused(false), _volume(50), 
      _repeat(false), _loop(false), _currentIndex(0), _lastError(NO_ERROR), 
      _mp3Decoder(nullptr), _mp3Offset(0) {}

bool SimpleMAX98357A::begin(FS &fs, int bclkPin, int lrclkPin, int dinPin) {
    _fs = &fs;
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = bclkPin,
        .ws_io_num = lrclkPin,
        .data_out_num = dinPin,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };
    if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK ||
        i2s_set_pin(I2S_NUM_0, &pin_config) != ESP_OK) {
        _lastError = INIT_FAILED;
        return false;
    }
    return true;
}

void SimpleMAX98357A::setVolume(uint8_t volume) {
    if (volume > 100) volume = 100;
    _volume = volume;
}

void SimpleMAX98357A::volumeUp() { setVolume(_volume + 10); }
void SimpleMAX98357A::volumeDown() { setVolume(_volume - 10); }
uint8_t SimpleMAX98357A::getVolume() { return _volume; }

void SimpleMAX98357A::play(const char* filename) {
    stop();
    _file = _fs->open(filename);
    if (!_file) {
        _lastError = FILE_ERROR;
        return;
    }
    if (strstr(filename, ".mp3") || strstr(filename, ".MP3")) {
        initMP3();
    } else if (strstr(filename, ".wav") || strstr(filename, ".WAV")) {
        if (!parseWavHeader()) {
            _lastError = UNSUPPORTED_FORMAT;
            stop();
            return;
        }
        i2s_set_sample_rates(I2S_NUM_0, _wavHeader.sample_rate);
    } else {
        _lastError = UNSUPPORTED_FORMAT;
        stop();
        return;
    }
    _playing = true;
    _paused = false;
}

void SimpleMAX98357A::play(int index) {
    if (index >= 0 && index < MAX_TRACKS) {
        _currentIndex = index;
        char filename[16];
        snprintf(filename, sizeof(filename), "/%02d.mp3", index + 1);
        if (!_fs->exists(filename)) {
            snprintf(filename, sizeof(filename), "/%02d.wav", index + 1);
        }
        play(filename);
    }
}

void SimpleMAX98357A::next() { 
    if (_currentIndex < MAX_TRACKS - 1) play(_currentIndex + 1); 
}

void SimpleMAX98357A::previous() { 
    if (_currentIndex > 0) play(_currentIndex - 1); 
}

void SimpleMAX98357A::loop(bool enable) { _loop = enable; }
void SimpleMAX98357A::repeat(bool enable) { _repeat = enable; }
void SimpleMAX98357A::pause() { _paused = true; }
void SimpleMAX98357A::resume() { _paused = false; }

void SimpleMAX98357A::stop() {
    _playing = false;
    if (_file) _file.close();
    if (_mp3Decoder) { MP3FreeDecoder(_mp3Decoder); _mp3Decoder = nullptr; }
}

bool SimpleMAX98357A::isPlaying() { return _playing && !_paused; }

bool SimpleMAX98357A::isStorageConnected() {
    File test = _fs->open("/test.txt", "w");
    bool connected = test;
    if (test) test.close();
    return connected;
}

SimpleMAX98357A::error_t SimpleMAX98357A::getLastError() { return _lastError; }

void SimpleMAX98357A::loop() {
    if (_playing && !_paused) {
        if (_mp3Decoder) processMP3();
        else if (_file) processWAV();
    }
}

bool SimpleMAX98357A::parseWavHeader() {
    char chunk[4];
    _file.readBytes(chunk, 4);
    if (strncmp(chunk, "RIFF", 4) != 0) return false;
    _file.seek(20);
    _file.readBytes(chunk, 4);
    if (strncmp(chunk, "fmt ", 4) != 0) return false;
    _file.seek(22);
    _wavHeader.num_channels = _file.read() | (_file.read() << 8);
    _wavHeader.sample_rate = _file.read() | (_file.read() << 8) | (_file.read() << 16) | (_file.read() << 24);
    _file.seek(34);
    _wavHeader.bits_per_sample = _file.read() | (_file.read() << 8);
    _file.seek(44);
    return _wavHeader.bits_per_sample == 16;
}

void SimpleMAX98357A::scaleBuffer(uint8_t* buffer, size_t size) {
    if (_volume == 100) return;
    float scale = _volume / 100.0;
    int16_t* samples = (int16_t*)buffer;
    for (size_t i = 0; i < size / 2; i++) samples[i] = (int16_t)(samples[i] * scale);
}

void SimpleMAX98357A::initMP3() {
    if (_mp3Decoder) MP3FreeDecoder(_mp3Decoder);
    _mp3Decoder = MP3InitDecoder();
    _mp3Offset = 0;
}

void SimpleMAX98357A::processMP3() {
    if (_mp3Offset < BUFFER_SIZE) {
        size_t bytes_read = _file.read(_mp3Buffer + _mp3Offset, BUFFER_SIZE - _mp3Offset);
        _mp3Offset += bytes_read;
        if (bytes_read == 0) {
            if (_repeat) { _file.seek(0); _mp3Offset = 0; }
            else if (_loop) next();
            else stop();
            return;
        }
    }

    int16_t pcm[1152];
    int bytes_left = _mp3Offset;
    int err = MP3Decode(_mp3Decoder, _mp3Buffer, &bytes_left, pcm, 0);
    if (err == ERR_MP3_NONE) {
        scaleBuffer((uint8_t*)pcm, 1152 * 2);
        size_t written;
        i2s_write(I2S_NUM_0, pcm, 1152 * 2, &written, portMAX_DELAY);
        memmove(_mp3Buffer, _mp3Buffer + (_mp3Offset - bytes_left), bytes_left);
        _mp3Offset = bytes_left;
        i2s_set_sample_rates(I2S_NUM_0, 44100);
    }
}

void SimpleMAX98357A::processWAV() {
    size_t bytes_read = _file.read(_buffer, BUFFER_SIZE);
    if (bytes_read > 0) {
        scaleBuffer(_buffer, bytes_read);
        size_t written;
        i2s_write(I2S_NUM_0, _buffer, bytes_read, &written, portMAX_DELAY);
    } else {
        if (_repeat) _file.seek(44);
        else if (_loop) next();
        else stop();
    }
}