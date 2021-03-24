#pragma once
#ifndef _SCREEN_SYNC_WRAPPER
#define _SCREEN_SYNC_WRAPPER

#include <napi.h>
#include "../screen_sync.h"

namespace ScreenSync {
  Napi::Value init(const Napi::CallbackInfo& info);
  Napi::Value record(const Napi::CallbackInfo& info);
  Napi::Value extract(const Napi::CallbackInfo& info);
  Napi::Value release(const Napi::CallbackInfo& info);
  Napi::Object initMethods(Napi::Env env, Napi::Object exports);
}

#endif