#include "simplettsapplication.h"

/* Struct to hold various application data variable */
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
	tts_h tts;
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

/* Callback for whenever TTS changes state */
void state_changed_cb(tts_h tts, tts_state_e previous, tts_state_e current,
		void* user_data) {

	/* Current state is READY and previous state is CREATED implies TTS is prepared */

	if (TTS_STATE_READY == current && TTS_STATE_CREATED == previous) {

		appdata_s *ad = (appdata_s *) user_data;
		int ret;

		const char* text =
				"This is a simple text-to-speech Application. "
				"The text-to-speech features include synthesizing text "
				"into sound data as utterances and playing them. "
				"It is also possible to pause and stop playing. "; // Text to read
		const char* language = "en_US"; // Language
		int voice_type = TTS_VOICE_TYPE_FEMALE; // Voice type
		int speed = TTS_SPEED_AUTO; // Read speed
		int utt_id; // Utterance ID for the requested text

		/*
		 * Add the text to speak to TTS
		 * Needs the current state to be Ready, Playing or Paused
		 */
		ret = tts_add_text(ad->tts, text, language, voice_type, speed, &utt_id);
		//Error handling depending on value of ret

		/*
		 * Start playing TTS (speaking)
		 * Needs the current state to be Ready or Paused
		 */
		ret = tts_play(ad->tts);
		//Error handling depending on value of ret

		elm_object_text_set(ad->label, "Started playing Text-to-Speech");
		dlog_print(DLOG_INFO, LOG_TAG, "Added text and started playing");
	}
}


/* Utterance completed callback for the TTS. It is invoked when text synthesized by the TTS engine finishes playing */

void utterance_completed_cb(tts_h tts, int utt_id, void* user_data) {

	appdata_s *ad = (appdata_s *) user_data;
	//for setting the text to display
	elm_object_text_set(ad->label, "Text-To-Speech completed successfully");

}


/* Function which handles all the work of setting up and starting TTS */

void enter_tts(appdata_s *ad) {

	/*
	 * Variable to hold the return value of functions.
	 * Check the TTS API document for values returned to corresponding errors.
	 * For all the below functions,
	 * if (TTS_ERROR_NONE != ret) {
	 * 		//Some Error.
	 * }
	 */
	int ret;

	/*
	 * Create a TTS handle. The TTS handle is used for other TTS functions as a parameter
	 * TTS state changes to TTS_STATE_CREATED
	 *
	 * Note - TTS is not thread-safe and depends on the ecore main loop.
	 * Therefore, you must have the ecore main loop. Do not use TTS in a thread.
	 */
	ret = tts_create(&(ad->tts));
	//Error handling depending on value of ret

	/* set the state change callback for the TTS. It is invoked when the TTS state changes */
	ret = tts_set_state_changed_cb(ad->tts, state_changed_cb, ad);
	//Error handling depending on value of ret


	/* set the utterance completed callback for the TTS. It is invoked when text synthesized by the TTS engine finishes playing */
	ret = tts_set_utterance_completed_cb(ad->tts, utterance_completed_cb, ad);
	//Error handling depending on value of ret

	/*
	 * After you create the TTS handle, connect to the background TTS daemon.
	 * The tts_prepare() function is asynchronous.
	 * The state of the TTS is changes to TTS_STATE_READY when tts_prepare() finishes.
	 */
	ret = tts_prepare(ad->tts);
	//Error handling depending on value of ret
}


/* Function which handles all the work of unsetting and stopping TTS */

void exit_tts(appdata_s *ad) {

	/*
	 * Variable to hold the return value of functions.
	 * Check the TTS API document for values returned to corresponding errors.
	 * For all the below functions,
	 * if (TTS_ERROR_NONE != ret) {
	 * 		//Some Error.
	 * }
	 */
	int ret;

	/*
	 * To stop the playback, use the tts_stop() function.
	 * All the texts in the queue are removed.
	 * The state is changed to TTS_STATE_READY.
	 */
	ret = tts_stop(ad->tts);
	//Error handling depending on value of ret

	/* The tts_unprepare() function is used for disconnection from the TTS daemon */
	ret = tts_unprepare(ad->tts);
	//Error handling depending on value of ret

	/* Unset the callback for utterance completed */
	ret = tts_unset_utterance_completed_cb(ad->tts);
	//Error handling depending on value of ret

	/* Unset the callback for state changed */
	ret = tts_unset_state_changed_cb(ad->tts);
	//Error handling depending on value of ret

	/*
	 * When TTS library is no longer needed, destroy the TTS handle using the tts_destroy()
	 *
	 * Do not use the tts_destroy() function within the callback function,
	 * or the tts_destroy() function fails and returns TTS_ERROR_OPERATION_FAILED.
	 */
	ret = tts_destroy(ad->tts);
	//Error handling depending on value of ret

	dlog_print(DLOG_INFO, LOG_TAG, "exit_tts completed");
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
	elm_object_text_set(ad->label, "Waiting for TTS state changed callback");
	evas_object_show(ad->label);
	elm_object_content_set(ad->scroller, ad->label);

	/* Set callback for popping the naviframe when back button is pressed */
	ad->nf_it = elm_naviframe_item_push(ad->naviframe, _("TTS"), NULL, NULL,
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

	/* Start TTS */
	enter_tts(ad);

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

	/* Stop TTS */
	appdata_s *ad = data;
	exit_tts(ad);
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
