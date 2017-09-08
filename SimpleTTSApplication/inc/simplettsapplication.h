#ifndef __simplettsapplication_H__
#define __simplettsapplication_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <tts.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "simplettsapplication"

#if !defined(PACKAGE)
#define PACKAGE "org.example.simplettsapplication"
#endif

#define EDJ_FILE "edje/simplettsapplication.edj"

#endif
