#ifndef __simplestandalonegpsapplication_H__
#define __simplestandalonegpsapplication_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <locations.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "simplestandalonegpsapplication"

#if !defined(PACKAGE)
#define PACKAGE "org.example.simplestandalonegpsapplication"
#endif

#define EDJ_FILE "edje/simplestandalonegpsapplication.edj"
#define GRP_MAIN "main"


#endif /* __simplestandalonegpsapplication_H__ */
