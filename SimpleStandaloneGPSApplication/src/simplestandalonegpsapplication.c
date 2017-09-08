#include "simplestandalonegpsapplication.h"


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
	location_manager_h manager;
} appdata_s;

static void win_delete_request_cb(void *data, Evas_Object *obj,
		void *event_info) {
	ui_app_exit();
}

static void app_get_resource(const char *edj_file_in, char *edj_path_out,
		int edj_path_max) {
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

static Eina_Bool naviframe_pop_cb(void *data, Elm_Object_Item *it) {
	ui_app_exit();
	return EINA_FALSE;
}

/* Callback for whenever gps state is changed */
void standalone_gps_state_changed_cb(location_service_state_e state,
		void *user_data) {

	int ret;
	/* Check if the service is enabled - can only read location data if the service is enabled */
	if (LOCATIONS_SERVICE_ENABLED == state) {

		appdata_s* ad = user_data;
		elm_object_text_set(ad->label,
				"GPS Location service started. Waiting for location.<br/><br/>");

		/* Receive the current information about position, velocity, or location accuracy: */
		double altitude, latitude, longitude;
		time_t timestamp;
		int ret = location_manager_get_position(ad->manager, &altitude, &latitude,
				&longitude, &timestamp);

		if (LOCATIONS_ERROR_NONE == ret) {

			//Process the location information

			/* Sample code to display the data */
			dlog_print(DLOG_INFO, LOG_TAG,
					"location got altitude %f, latitude %f, longitude %f",
					altitude, latitude, longitude);
			char buf[1024];
			char* timebuf;
			timebuf = asctime(localtime(&timestamp));
			snprintf(buf, 1023,
					"GPS Location Acquired.<br/>"
							"Altitude is - %f <br/>"
							"Latitude is - %f <br/>"
							"Longitude is - %f <br/>"
							"Timestamp is - %s <br/><br/>",
					altitude, latitude, longitude,  timebuf);
			elm_object_text_set(ad->label, buf);
		}
	}
}

/* Function which handles all the work of setting up and starting the location manager */
void enter_gps(appdata_s *ad) {

	/*
	 * Variable to hold the return value of functions.
	 * Check the Sensors API document for values returned to corresponding errors.
	 * For all the below functions,
	 * if (LOCATIONS_ERROR_NONE != ret) {
	 * 		//Some Error.
	 * }
	 */
	int ret;

	/* Create a location manager handle */
	ret = location_manager_create(LOCATIONS_METHOD_HYBRID, &(ad->manager));

	/* Register a callback function for location service state changes */
	ret = location_manager_set_service_state_changed_cb(ad->manager,
			standalone_gps_state_changed_cb, ad);

	/*  Start the Location Manager */
	ret = location_manager_start(ad->manager);


}

/* Function which handles all the work of unsetting and stopping the location manager */
void exit_gps(appdata_s *ad) {

	/*
	 * At the end of the application, destroy all used resources, such as the location manager
	 * If you destroy the handle, there is no need to call the location_manager_stop() function to stop the service.
	 * The service is automatically stopped. Also, you do not have to unset previously set callbacks.
	 */
	location_manager_destroy(ad->manager);
	ad->manager = NULL;
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
	elm_object_part_content_set(ad->layout, "elm.swallow.content",
			ad->scroller);

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
	elm_object_text_set(ad->label,
			"<br/> Waiting for Stand alone GPS to start. <br/><br/>");
	evas_object_show(ad->label);
	elm_object_content_set(ad->scroller, ad->label);

	/* Set callback for popping the naviframe when back button is pressed */
	ad->nf_it = elm_naviframe_item_push(ad->naviframe, _("Standalone GPS"),
			NULL, NULL, ad->layout, NULL);
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
		elm_win_wm_rotation_available_rotations_set(ad->win,
				(const int *) (&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request",
			win_delete_request_cb, NULL);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
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
	enter_gps(data);
}

static void app_terminate(void *data) {

	/* Release all resources. */
	exit_gps(data);
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
		dlog_print(DLOG_ERROR, LOG_TAG, "ui_app_main() is failed. err = %d",
				ret);
	}

	return ret;
}
