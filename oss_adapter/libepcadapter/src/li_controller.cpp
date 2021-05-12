/*
 * Copyright (c) 2020 Sprint
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include "li_controller.h"
#include "tcp_listener.h"


uint8_t Controller :: iRefCntr = 0;
Controller * Controller :: controller = NULL;


Controller :: Controller()
{
}


Controller :: ~Controller()
{
}


Controller *
Controller :: getInstance()
{
	if (controller == NULL) {

		controller = new Controller();
		if (NULL == controller) {

			return NULL;
		}
	}

	++iRefCntr;
	return controller;
}


Void
Controller :: startUp(const uint8_t *ddf_ip, uint16_t port, const uint8_t *ddf_local_ip,
		uint8_t *mode)
{
	try {

		listenerObject = new TCPListener;
		listenerObject->init(1, 1, NULL, 100000);
		listenerObject->initDir(ddf_ip, port, ddf_local_ip, mode);
	}
	catch (...) {

		clLog(STANDARD_LOGID, eCLSeverityCritical, LOG_FORMAT"Exception "
			"initializing Application", LOG_VALUE);

		return;
	}
}


Void
Controller :: shutdown()
{
	if (NULL != listenerObject) {

		listenerObject->quit();
		listenerObject->join();

		delete listenerObject;
		listenerObject = NULL;
	}
}

void
Controller :: releaseInstance()
{
	--iRefCntr;

	if ((iRefCntr == 0) && (controller != NULL)) {

		delete controller;
		controller = NULL;
	}
}
