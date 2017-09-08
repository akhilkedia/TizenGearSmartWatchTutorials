#ifndef __HELLO_ACCESSORY_CONSUMER_H__
#define __HELLO_ACCESSORY_CONSUMER_H__

#include <app.h>
#include <glib.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "HelloAccessoryConsumer"

void     initialize_sap();
gboolean find_peers();
gboolean request_service_connection(void);
gboolean terminate_service_connection(void);
gboolean send_data(char *message);
void     update_ui(char *data);

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.helloaccessoryconsumer"
#endif

#endif
