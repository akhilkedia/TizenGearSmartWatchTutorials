#include "helloaccessory.h"
#include <glib.h>
#include <sap.h>

#define HELLO_ACCESSORY_PROFILE_ID "/sample/hello"
#define HELLO_ACCESSORY_CHANNELID 104

struct priv {
	sap_agent_h agent;
	sap_socket_h socket;
	sap_peer_agent_h peer_agent;
};

static gboolean agent_created = FALSE;

static struct priv priv_data = { 0 };

void on_peer_agent_updated(sap_peer_agent_h peer_agent,
			   sap_peer_agent_status_e peer_status,
			   sap_peer_agent_found_result_e result,
			   void *user_data)
{
	switch (result) {
	case SAP_PEER_AGENT_FOUND_RESULT_DEVICE_NOT_CONNECTED:
		LOGI("device is not connected");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_FOUND:
		if (peer_status == SAP_PEER_AGENT_STATUS_AVAILABLE) {
			priv_data.peer_agent = peer_agent;
			LOGI("Find Peer Success!!");
			request_service_connection();
		} else {
			LOGI("peer agent removed");
			sap_peer_agent_destroy(peer_agent);
		}
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_SERVICE_NOT_FOUND:
		LOGI("service not found");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_TIMEDOUT:
		LOGI("peer agent find timed out");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_INTERNAL_ERROR:
		LOGI("peer agent find search failed");
		break;
	}
}


static void on_service_connection_terminated(sap_peer_agent_h peer_agent,
					     sap_socket_h socket,
					     sap_service_connection_terminated_reason_e result,
					     void *user_data)
{
	switch (result) {
	case SAP_CONNECTION_TERMINATED_REASON_PEER_DISCONNECTED:
		LOGI("disconnected because peer lost");
		update_ui("Peer Disconnected");
		break;

	case SAP_CONNECTION_TERMINATED_REASON_DEVICE_DETACHED:
		LOGI("disconnected because device is detached");
		update_ui("Disconnected Device Detached");
		break;

	case SAP_CONNECTION_TERMINATED_REASON_UNKNOWN:
		LOGI("disconnected because of unknown reason");
		update_ui("Disconnected Unknown Reason");
		break;
	}

	sap_socket_destroy(priv_data.socket);
	priv_data.socket = NULL;

	LOGI("status:%d", result);
}


static void on_data_recieved(sap_socket_h socket,
			     unsigned short int channel_id,
			     unsigned int payload_length,
			     void *buffer,
			     void *user_data)
{
	LOGI("received data: %s, len:%d", buffer, payload_length);
	update_ui(buffer);
}


static void on_service_connection_created(sap_peer_agent_h peer_agent,
					  sap_socket_h socket,
					  sap_service_connection_result_e result,
					  void *user_data)
{
	switch (result) {
	case SAP_CONNECTION_SUCCESS:
		LOGI("peer agent connection is successful, pa :%u", peer_agent);
		sap_peer_agent_set_service_connection_terminated_cb(priv_data.peer_agent,
								    on_service_connection_terminated,
								    NULL);

		sap_socket_set_data_received_cb(socket, on_data_recieved, peer_agent);
		priv_data.socket = socket;
		update_ui("Connection Established");
		break;

	case SAP_CONNECTION_ALREADY_EXIST:
		LOGI("connection is already exist");
		priv_data.socket = socket;
		update_ui("Connection already exist");
		break;

	case SAP_CONNECTION_FAILURE_DEVICE_UNREACHABLE:
		LOGI("device is not unreachable");
		update_ui("Device Not Reachable");
		break;

	case SAP_CONNECTION_FAILURE_INVALID_PEERAGENT:
		LOGI("invalid peer agent");
		update_ui("Invalid Peer Agent");
		break;

	case SAP_CONNECTION_FAILURE_NETWORK:
		LOGI("network failure");
		update_ui("Network Failure");
		break;

	case SAP_CONNECTION_FAILURE_PEERAGENT_NO_RESPONSE:
		LOGI("peer agent is no response");
		update_ui("PEERAGENT_NO_RESPONSE");
		break;

	case SAP_CONNECTION_FAILURE_PEERAGENT_REJECTED:
		LOGI("peer agent is rejected");
		update_ui("PEERAGENT_REJECTED");
		break;

	case SAP_CONNECTION_FAILURE_UNKNOWN:
		LOGI("unknown error");
		update_ui("UNKNOWN_ERROR");
		break;
	}
}

static gboolean _create_service_connection(gpointer user_data)
{
	struct priv *priv = NULL;
	sap_result_e result = SAP_RESULT_FAILURE;

	priv = (struct priv *)user_data;
	result = sap_agent_request_service_connection(priv->agent,
						      priv->peer_agent,
						      on_service_connection_created,
						      NULL);

	if (result == SAP_RESULT_SUCCESS) {
		LOGI("req service conn call succeeded");
	} else {
		update_ui("Connection Establishment Failed");
		LOGI("req service conn call is failed (%d)", result);
	}

	return FALSE;
}

gboolean request_service_connection(void)
{
	g_idle_add(_create_service_connection, &priv_data);

	LOGI("request_service_connection call over");
	return TRUE;
}

static gboolean _terminate_service_connection(gpointer user_data)
{
	struct priv *priv = NULL;
	sap_result_e result = SAP_RESULT_FAILURE;

	priv = (struct priv *)user_data;

	if (priv->socket)
		result = sap_peer_agent_terminate_service_connection(priv->peer_agent);
	else {
		update_ui("No service Connection");
		return FALSE;
	}
	LOGI("Result %d", result);

	if (result == SAP_RESULT_SUCCESS) {
		update_ui("Connection Terminated");
		LOGI("req service conn call succeeded");
	} else {
		update_ui("Connection Termination Failed");
		LOGI("req service conn call is failed (%d)", result);
	}

	return FALSE;
}

gboolean terminate_service_connection(void)
{
	g_idle_add(_terminate_service_connection, &priv_data);
	return TRUE;
}

static gboolean _find_peer_agent(gpointer user_data)
{
	struct priv *priv = NULL;
	sap_result_e result = SAP_RESULT_FAILURE;

	priv = (struct priv *)user_data;

	result = sap_agent_find_peer_agent(priv->agent, on_peer_agent_updated, NULL);

	if (result == SAP_RESULT_SUCCESS) {
		LOGI("find peer call succeeded");
	} else {
		LOGI("findsap_peer_agent_s is failed (%d)", result);
	}

	LOGE("find peer call is over");

	return FALSE;
}

gboolean find_peers()
{
	g_idle_add(_find_peer_agent, &priv_data);
	LOGI("find peer called");
	return TRUE;

}

gboolean send_data(char *message)
{
	int result;
	if (priv_data.socket) {
		LOGI("Sending data ");
		result = sap_socket_send_data(priv_data.socket, HELLO_ACCESSORY_CHANNELID, strlen(message), message);
	} else {
		update_ui("No service Connection");
		return FALSE;
	}
	return TRUE;

}

static void on_agent_initialized(sap_agent_h agent,
				 sap_agent_initialized_result_e result,
				 void *user_data)
{
	switch (result) {
	case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:
		LOGI("agent is initialized");

		priv_data.agent = agent;
		agent_created = TRUE;
		break;

	case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
		LOGI("duplicate registration");
		break;

	case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
		LOGI("invalid arguments");
		break;

	case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
		LOGI("internal sap error");
		break;

	default:
		LOGI("unknown status (%d)", result);
		break;
	}

	LOGI("agent initialized callback is over");

}

static void on_device_status_changed(sap_device_status_e status, sap_transport_type_e transport_type,
				     void *user_data)
{
	switch (transport_type) {
	case SAP_TRANSPORT_TYPE_BT:
		LOGI("connectivity type(%d): bt", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_BLE:
		LOGI("connectivity type(%d): ble", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_TCP:
		LOGI("connectivity type(%d): tcp/ip", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_USB:
		LOGI("connectivity type(%d): usb", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_MOBILE:
		LOGI("connectivity type(%d): mobile", transport_type);
		break;

	default:
		LOGI("unknown connectivity type (%d)", transport_type);
		break;
	}

	switch (status) {
	case SAP_DEVICE_STATUS_DETACHED:

		if (priv_data.peer_agent) {
			sap_socket_destroy(priv_data.socket);
			priv_data.socket = NULL;
			sap_peer_agent_destroy(priv_data.peer_agent);
			priv_data.peer_agent = NULL;
		}
		break;

	case SAP_DEVICE_STATUS_ATTACHED:
		if (agent_created) {
			g_idle_add(_find_peer_agent, &priv_data);
		}
		break;

	default:
		LOGI("unknown status (%d)", status);
		break;
	}

}

gboolean agent_initialize()
{
	int result = 0;

	do {
		result = sap_agent_initialize(priv_data.agent, HELLO_ACCESSORY_PROFILE_ID, SAP_AGENT_ROLE_CONSUMER,
					      on_agent_initialized, NULL);
		LOGI("SAP >>> getRegisteredServiceAgent() >>> %d", result);
	} while (result != SAP_RESULT_SUCCESS);

	return TRUE;
}

void initialize_sap()
{
	sap_agent_h agent = NULL;

	sap_agent_create(&agent);

	if (agent == NULL)
		LOGI("ERROR in creating agent");
	else
		LOGE("SUCCESSFULLY create sap agent");

	priv_data.agent = agent;

	sap_set_device_status_changed_cb(on_device_status_changed, NULL);

	agent_initialize();
}
