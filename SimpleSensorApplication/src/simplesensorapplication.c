#include "simplesensorapplication.h"

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *layout;
	Evas_Object *naviframe;
	Evas_Object *label;
	Evas_Object *scroller;
	Evas_Object *circle_scroller;
	Eext_Circle_Surface *circle_surface;
	Elm_Object_Item *nf_it;
	sensor_listener_h listener;
	sensor_type_e type;
} appdata_s;

static void win_delete_request_cb(void *data, Evas_Object *obj,
		void *event_info) {
	ui_app_exit();
}

static Eina_Bool naviframe_pop_cb(void *data, Elm_Object_Item *it) {
	ui_app_exit();
	return EINA_FALSE;
}

static void app_get_resource(const char *edj_file_in, char *edj_path_out,
		int edj_path_max) {
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

/* Callback for whenever a sensor event is detected (such as value changed). It is called whenever such an event is detected */
void on_sensor_event(sensor_h sensor, sensor_event_s *event, void *data) {
	appdata_s *ad = data;

	// This example uses sensor type, assuming there is only 1 sensor for each type
	sensor_type_e type;
	sensor_get_type(sensor, &type);

	//Check the sensor type of the sensor event
	if(type == (ad->type)){
		dlog_print(DLOG_INFO, LOG_TAG, "sensor event detected");

		char buf[1024];
		char tempbuf[1024];
		snprintf(buf, 1023, "Sensor event detected.<br/>");

		for (int i = 0; i<event->value_count;i++){
			snprintf(tempbuf, 1023, "Sensor value[%d] is - %f<br/>",i,event->values[i]);
			strcat(buf, tempbuf);
		}

		snprintf(tempbuf, 1023, "Sensor timestamp is - %llu<br/>",event->timestamp);
		strcat(buf, tempbuf);

		snprintf(tempbuf, 1023, "Sensor accuracy is - %d<br/>",event->accuracy);
		strcat(buf, tempbuf);

		elm_object_text_set(ad->label, buf);
	}
}

/* Function which handles all the work of setting up sensor listeners */
void enter_sensor(appdata_s *ad) {

	/*
	 * Select the required sensor. Tizen supports the following
	 * sensors, which are defined in enum sensor_type_e in <sensor.h>
	 * Note that not all sensors are available on all devices.
	 */
	ad->type = SENSOR_ACCELEROMETER;

	/*
	 * Variable to hold the return value of functions.
	 * Check the Sensors API document for values returned to corresponding errors.
	 * For all the below functions,
	 * if (SENSOR_ERROR_NONE != ret) {
	 * 		//Some Error.
	 * }
	 */
	int ret;

	/* Check whether a sensor is supported on the current device */
	bool supported;
	ret = sensor_is_supported(ad->type, &supported);
	//Error handling depending on value of ret

	/* If supported is false, sensor is not supported. Return. */
	if (supported == false) {
		dlog_print(DLOG_INFO, LOG_TAG, "sensor is not supported");
		elm_object_text_set(ad->label,
				"Sensor is not available on this device<br/><br/>");
		return;
	}

	/* Get a handle of the default sensor */
	sensor_h sensor;
	ret = sensor_get_default_sensor(ad->type, &sensor);
	//Error handling depending on value of ret

	/*
	 * Create a sensor listener.
	 * This listener is an event listener used to receive sensor data asynchronously.
	 */
	ret = sensor_create_listener(sensor, &(ad->listener));
	//Error handling depending on value of ret

	/* Register a callback if a sensor event is detected (sensor value changed) */
	ret = sensor_listener_set_event_cb((ad->listener), 100, on_sensor_event,
			ad);
	//Error handling depending on value of ret

	/*
	 * Change the sensor to be always-on.
	 * By default sensor data cannot be recieved when the LCD is off and in the power save mode.
	 */
	ret = sensor_listener_set_option((ad->listener), SENSOR_OPTION_ALWAYS_ON);
	//Error handling depending on value of ret

	/*
	 * Starts the sensor server for the given listener.
	 * After this function is called, sensor events will occur and the specific sensor type
	 *  related callback function will be called. An application can read sensor data.
	 */
	ret = sensor_listener_start((ad->listener));
	//Error handling depending on value of ret

	/* Read sensor data */
	sensor_event_s event;
	ret = sensor_listener_read_data((ad->listener), &event);
	//Error handling depending on value of ret

	/* Display the data read. The number of values returned by the sensor is event.value_count */
	if (SENSOR_ERROR_NONE == ret) {

		dlog_print(DLOG_INFO, LOG_TAG, "sensor data read successfully");

		char buf[1024];
		char tempbuf[1024];
		snprintf(buf, 1023, "Sensor data read successfully.<br/>");

		for (int i = 0; i<event.value_count;i++){
			snprintf(tempbuf, 1023, "Sensor value[%d] is - %f<br/>",i,event.values[i]);
			strcat(buf, tempbuf);
		}

		snprintf(tempbuf, 1023, "Sensor timestamp is - %llu<br/>",event.timestamp);
		strcat(buf, tempbuf);

		snprintf(tempbuf, 1023, "Sensor accuracy is - %d<br/>",event.accuracy);
		strcat(buf, tempbuf);

		elm_object_text_set(ad->label, buf);

		// Use sensor information

	}
}


/* Function which handles all the work of unsetting and stopping the sensor listeners */
void exit_sensor(appdata_s *ad) {

	/*
	 * Variable to hold the return value of functions.
	 * Check the Sensors API document for values returned to corresponding errors.
	 * For all the below functions,
	 * if (SENSOR_ERROR_NONE != ret) {
	 * 		//Some Error.
	 * }
	 */
	int ret;

	/* Unset the callback for when a sensor event is detected */
	ret = sensor_listener_unset_event_cb((ad->listener));
	//Error handling depending on value of ret

	/* Stops the sensor server for the given listener. */
	ret = sensor_listener_stop((ad->listener));
	//Error handling depending on value of ret

	/* Destroys the sensor handle and releases all its resources. */
	ret = sensor_destroy_listener((ad->listener));
	//Error handling depending on value of ret
}

static void create_top_view(void *data) {
	char edj_path[PATH_MAX] = { 0, };
	appdata_s *ad = data;

	/* Create Layout */
	app_get_resource(EDJ_FILE, edj_path, (int) PATH_MAX);
	ad->layout = elm_layout_add(ad->naviframe);
	elm_layout_file_set(ad->layout, edj_path, "info_layout");
	evas_object_show(ad->layout);

	/* Create Scroller */
	ad->scroller = elm_scroller_add(ad->layout);
	evas_object_show(ad->scroller);
	elm_object_part_content_set(ad->layout, "elm.swallow.content", ad->scroller);

	/* Create Circle Scroller */
	ad->circle_scroller = eext_circle_object_scroller_add(ad->scroller,
			ad->circle_surface);

	/* Set Scroller Policy */
	eext_circle_object_scroller_policy_set(ad->circle_scroller,
			ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	/* Activate Rotary Event */
	eext_rotary_object_event_activated_set(ad->circle_scroller, EINA_TRUE);

	/* Set the Label for the text */
	ad->label = elm_label_add(ad->scroller);
	elm_label_line_wrap_set(ad->label, ELM_WRAP_WORD);
	elm_object_text_set(ad->label, "Waiting for sensors to start<br/><br/><br/>");
	evas_object_show(ad->label);
	elm_object_content_set(ad->scroller, ad->label);

	/* Set callback for popping the naviframe when back button is pressed */
	ad->nf_it = elm_naviframe_item_push(ad->naviframe, _("Sensors"), NULL, NULL,
			ad->layout, NULL);
	elm_naviframe_item_pop_cb_set(ad->nf_it, naviframe_pop_cb, ad->win);

}

static void create_base_gui(appdata_s *ad) {

	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_conformant_set(ad->win, EINA_TRUE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	/* Setting Rotations */
	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Naviframe */
	ad->naviframe = elm_naviframe_add(ad->conform);
	elm_object_content_set(ad->conform, ad->naviframe);

	/* Eext Circle Surface*/
	ad->circle_surface = eext_circle_surface_naviframe_add(ad->naviframe);

	/* Main View */
	create_top_view(ad);

	/*Register callback for back and more buttons to work */
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_BACK,
			eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_MORE,
			eext_naviframe_more_cb, NULL);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);

}

static bool app_create(void *data) {
	/* Hook to take necessary actions before main event loop starts
	 Initialize UI resources and application's data
	 If this function returns true, the main loop of application starts
	 If this function returns false, the application is terminated */
	appdata_s *ad = data;
	create_base_gui(ad);

	/* Start Sensor */
	enter_sensor(ad);

	return true;
}

static void app_control(app_control_h app_control, void *data) {
	/* Handle the launch request. */
}

static void app_pause(void *data) {
	/* Take necessary actions when application becomes invisible. */
}

static void app_resume(void *data) {
	/* Take necessary actions when application becomes visible. */
}

static void app_terminate(void *data) {
	/* Release all resources. */

	/* Stop Sensor */
	appdata_s *ad = data;
	exit_sensor(ad);
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
			&locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[]) {
	appdata_s ad = { 0, };
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY],
			APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY],
			APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED],
			APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
			APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
			APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
