#pragma once 
#include "network/session/session.h"
namespace gb
{

	struct HandleInterface
	{
		virtual void SetReceivedCallBack(Session::session_received_callback_t callback) = 0;
		virtual void SetConnnectCallBack(Session::session_connected_callback_t callback) = 0;
		virtual void SetCloseCallBack(Session::session_closed_callback_t callback) = 0;
	};



}