/*******************************************************************************
 * Copyright 2009-2013 Jörg Müller
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#define aud_JACK_LIBRARY_IMPL

#include "JackLibrary.h"

#ifdef WITH_JACK_DYNLOAD
# include <dlfcn.h>
# include <stdio.h>
#endif

#ifdef WITH_JACK_DYNLOAD
static void *jack_handle = NULL;
#endif

static bool jack_supported = false;

void aud_jack_init(void)
{
#ifdef WITH_JACK_DYNLOAD
	jack_handle = dlopen("libjack.so", RTLD_LAZY);

	if (!jack_handle) {
		return;
	}

# define JACK_SYMBOL(sym) \
	{ \
		char *error; \
		*(void **) (&(AUD_##sym)) = dlsym(jack_handle, #sym); \
		if ((error = dlerror()) != NULL)  { \
			fprintf(stderr, "%s\n", error); \
			return; \
		} \
	} (void)0

	dlerror();    /* Clear any existing error */
#else  // WITH_JACK_DYNLOAD
# define JACK_SYMBOL(sym) aud_##sym = sym
#endif  // WITH_JACK_DYNLOAD

	JACK_SYMBOL(jack_transport_query);
	JACK_SYMBOL(jack_transport_locate);

	JACK_SYMBOL(jack_transport_start);
	JACK_SYMBOL(jack_transport_stop);

	JACK_SYMBOL(jack_ringbuffer_reset);
	JACK_SYMBOL(jack_ringbuffer_write);
	JACK_SYMBOL(jack_ringbuffer_write_space);
	JACK_SYMBOL(jack_ringbuffer_write_advance);
	JACK_SYMBOL(jack_ringbuffer_read);
	JACK_SYMBOL(jack_ringbuffer_create);
	JACK_SYMBOL(jack_ringbuffer_free);
	JACK_SYMBOL(jack_ringbuffer_read_space);
	JACK_SYMBOL(jack_set_sync_callback);

	JACK_SYMBOL(jack_port_get_buffer);

	JACK_SYMBOL(jack_client_open);
	JACK_SYMBOL(jack_set_process_callback);
	JACK_SYMBOL(jack_on_shutdown);
	JACK_SYMBOL(jack_port_register);
	JACK_SYMBOL(jack_client_close);
	JACK_SYMBOL(jack_get_sample_rate);
	JACK_SYMBOL(jack_activate);
	JACK_SYMBOL(jack_get_ports);
	JACK_SYMBOL(jack_port_name);
	JACK_SYMBOL(jack_connect);

	jack_supported = true;

#undef JACK_SYMBOL
}

void aud_jack_exit(void)
{
#ifdef WITH_JACK_DYNLOAD
	if (jack_handle) {
		dlclose(jack_handle);
	}
#endif
	jack_supported = false;
}

bool aud_jack_supported(void)
{
	return jack_supported;
}
