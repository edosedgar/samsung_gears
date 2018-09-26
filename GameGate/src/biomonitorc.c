#include "biomonitorc.h"

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	//appdata_h ad = data;
	/* Let window go to hide state. */
	//elm_win_lower(ad->win);
	ui_app_exit();
}

static void
create_base_gui(appdata_h ad)
{
	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Box */
	ad->box = elm_box_add(ad->conform);
	evas_object_size_hint_weight_set(ad->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ad->box);
	elm_object_content_set(ad->conform, ad->box);

	/* Label for Connection */
	ad->l_conn = elm_label_add(ad->box);
	elm_object_text_set(ad->l_conn, "<color=#FFFF20FF><align=center>Server: Inactive</align></color>");
	evas_object_size_hint_weight_set(ad->l_conn, 0.0, 0.0);
	evas_object_size_hint_align_set(ad->l_conn, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_min_set(ad->l_conn, 50, 50);
	evas_object_show(ad->l_conn);
	elm_box_pack_end(ad->box, ad->l_conn);

	/* Label for HRM */
	ad->l_hrm = elm_label_add(ad->box);
	elm_object_text_set(ad->l_hrm, "<align=center>HRM: </align>");
	evas_object_size_hint_weight_set(ad->l_hrm, 0.0, 0.0);
	evas_object_size_hint_align_set(ad->l_hrm, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_min_set(ad->l_hrm, 50, 50);
	evas_object_show(ad->l_hrm);
	elm_box_pack_end(ad->box, ad->l_hrm);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);

	hrm_sensor_h hrm_sh = malloc(sizeof(struct hrm_sensor));
	acc_sensor_h acc_sh = malloc(sizeof(struct acc_sensor));
	conn_info_h conn_ih = malloc(sizeof(struct conn_info));

	int supported = hrm_init(hrm_sh, ad);
	if (supported) {
	    char out[32];
		sprintf(out, "<align=center>HRM : %s<\align>", supported ? "yes" : " not");
		elm_object_text_set(ad->l_hrm, out);
	}
	hrm_create_listener(hrm_sh);

	supported = acc_init(acc_sh, ad);
	acc_create_listener(acc_sh);

	init_connection(HOST_ADDR, conn_ih, ad);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_h ad = data;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
    device_power_release_lock(POWER_LOCK_DISPLAY);
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
	device_power_request_lock(POWER_LOCK_DISPLAY, 0);
}

static void
app_terminate(void *data)
{
	appdata_h ad = (appdata_h)data;
	hrm_sensor_h hrm_sh = (hrm_sensor_h)ad->hrm_sh;
	acc_sensor_h acc_sh = (acc_sensor_h)ad->acc_sh;
	conn_info_h conn_ih = (conn_info_h)ad->conn_ih;

	sensor_listener_stop(hrm_sh->listener);
	sensor_destroy_listener(hrm_sh->listener);

	sensor_listener_stop(acc_sh->listener);
	sensor_destroy_listener(acc_sh->listener);

	connection_destroy(conn_ih->connection);
	close(conn_ih->sockfd);
	/* Release all resources. */

}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	struct appdata ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
