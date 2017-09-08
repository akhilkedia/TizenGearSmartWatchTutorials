#ifndef __simplesensorapplication_H__
#define __simplesensorapplication_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <sensor.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "simplesensorapplication"

#if !defined(PACKAGE)
#define PACKAGE "org.example.simplesensorapplication"
#endif

#define EDJ_FILE "edje/simplesensorapplication.edj"


#endif /* __simplesensorapplication_H__ */
