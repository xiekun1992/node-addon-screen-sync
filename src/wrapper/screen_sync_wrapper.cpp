#include "screen_sync_wrapper.h"

namespace ScreenSync {
  int width, height, fps, size, interval;
  BYTE* screenData = NULL;
  BYTE* encodedData = NULL;
  BYTE* decodedData = NULL;
  screen_sync::ScreenCap screenCap;

  Napi::Value init(const Napi::CallbackInfo& info) {
    width = screenCap.sw;
    height = screenCap.sh;
    fps = 25;
    interval = 1000 / fps;
    size = sizeof(uint8_t) * 3 * width * height;
    screenData = (BYTE*)malloc(size);

    Napi::Object option = Napi::Object::New(info.Env());
    option.Set("width", width);
    option.Set("height", height);
    return option;
    // Napi::Env env = info.Env();
    // Napi::Object options = info[0].As<Napi::Object>();
    // if (options.IsObject()) {
    //   width = Napi::Number(env, options.Get("width")).Int32Value();
    //   height = Napi::Number(env, options.Get("height")).Int32Value();
    //   fps = Napi::Number(env, options.Get("fps")).Int32Value();

    //   interval = 1000 / fps;
    //   size = sizeof(uint8_t) * 3 * width * height;
    //   screenData = (BYTE*)malloc(size);
    // } else {
    //   Napi::TypeError::New(env, "options should be type of Object").ThrowAsJavaScriptException();
    //   return Napi::Boolean::New(env, false);
    // }
    // return Napi::Boolean::New(env, true);
  }
  Napi::Value record(const Napi::CallbackInfo& info) {
    screen_sync::Encoder encoder(width, height, fps);
    // screen_sync::ScreenCap screenCap;
    screenCap.desktopshot(screenData);
    encoder.encodeRgb24(screenData);
    encoder.flush();
    
    if (encodedData) {
      free(encodedData);
    }
    encodedData = (BYTE*)malloc(encoder.head->size);
    if (encodedData) {
      memcpy(encodedData, encoder.head->buf, encoder.head->size);
    }

    return Napi::Buffer<uint8_t>::New(info.Env(), encodedData, encoder.head->size);
    // return Napi::Buffer<uint8_t>::New(info.Env(), screenData, size);
  }
  Napi::Value release(const Napi::CallbackInfo& info) {
    free(screenData);
    free(encodedData);
    free(decodedData);
    return Napi::Boolean::New(info.Env(), true);
  }
  Napi::Value extract(const Napi::CallbackInfo& info) {
    Napi::Uint8Array buffer = info[0].As<Napi::Uint8Array>();
    screen_sync::NODE node = {0};
    node.size = buffer.ByteLength();
    node.buf = buffer.Data();

    screen_sync::Decoder decoder(width, height);
    decoder.decodeRgb24(&node);
	  decoder.flush();

    if (decodedData) {
      free(decodedData);
    }
    decodedData = (BYTE*)malloc(decoder.head->size);
    if (decodedData) {
      memcpy(decodedData, decoder.head->buf, decoder.head->size);
    }

    return Napi::Buffer<uint8_t>::New(info.Env(), decodedData, decoder.head->size);
  }
  Napi::Object initMethods(Napi::Env env, Napi::Object exports) {
    exports.Set("init", Napi::Function::New(env, ScreenSync::init));
    exports.Set("record", Napi::Function::New(env, ScreenSync::record));
    exports.Set("extract", Napi::Function::New(env, ScreenSync::extract));
    exports.Set("release", Napi::Function::New(env, ScreenSync::release));
    return exports;
  }
}