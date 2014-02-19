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

#pragma once

#if defined(__APPLE__) // always first include for jack weaklinking !
#include <weakjack.h>
#endif

#include <jack.h>
#include <ringbuffer.h>

#ifdef aud_JACK_LIBRARY_IMPL
# define JACK_SYM
#else
# define JACK_SYM extern
#endif

/* All loadable Jack sumbols, prototypes from original jack.h */

JACK_SYM jack_transport_state_t (*aud_jack_transport_query) (
        const jack_client_t *client,
        jack_position_t *pos);

JACK_SYM int (*aud_jack_transport_locate) (jack_client_t *client,
                                           jack_nframes_t frame);

JACK_SYM void (*aud_jack_transport_start) (jack_client_t *client);
JACK_SYM void (*aud_jack_transport_stop) (jack_client_t *client);

JACK_SYM void (*aud_jack_ringbuffer_reset) (jack_ringbuffer_t *rb);
JACK_SYM size_t (*aud_jack_ringbuffer_write) (jack_ringbuffer_t *rb,
                                              const char *src, size_t cnt);
JACK_SYM size_t (*aud_jack_ringbuffer_write_space) (const jack_ringbuffer_t *rb);
JACK_SYM void (*aud_jack_ringbuffer_write_advance) (jack_ringbuffer_t *rb,
                                                    size_t cnt);
JACK_SYM size_t (*aud_jack_ringbuffer_read) (jack_ringbuffer_t *rb, char *dest,
                                             size_t cnt);
JACK_SYM jack_ringbuffer_t *(*aud_jack_ringbuffer_create) (size_t sz);
JACK_SYM void (*aud_jack_ringbuffer_free) (jack_ringbuffer_t *rb);
JACK_SYM size_t (*aud_jack_ringbuffer_read_space) (const jack_ringbuffer_t *rb);
JACK_SYM int  (*aud_jack_set_sync_callback) (jack_client_t *client,
     JackSyncCallback sync_callback,
     void *arg);

JACK_SYM void *(*aud_jack_port_get_buffer) (jack_port_t *, jack_nframes_t);

JACK_SYM jack_client_t *(*aud_jack_client_open) (const char *client_name,
                                                 jack_options_t options,
                                                 jack_status_t *status, ...);
JACK_SYM int (*aud_jack_set_process_callback) (jack_client_t *client,
       JackProcessCallback process_callback, void *arg);
JACK_SYM void (*aud_jack_on_shutdown) (jack_client_t *client,
       JackShutdownCallback function, void *arg);
JACK_SYM jack_port_t *(*aud_jack_port_register) (jack_client_t *client,
                                                 const char *port_name,
                                                 const char *port_type,
                                                 unsigned long flags,
                                                 unsigned long buffer_size);
JACK_SYM int (*aud_jack_client_close) (jack_client_t *client);
JACK_SYM jack_nframes_t (*aud_jack_get_sample_rate) (jack_client_t *);
JACK_SYM int (*aud_jack_activate) (jack_client_t *client);
JACK_SYM const char **(*aud_jack_get_ports) (jack_client_t *, 
                                             const char *port_name_pattern,
                                             const char *type_name_pattern, 
                                             unsigned long flags);
JACK_SYM const char *(*aud_jack_port_name) (const jack_port_t *port);
JACK_SYM int (*aud_jack_connect) (jack_client_t *,
                                  const char *source_port,
                                  const char *destination_port);

/* Public API */

void aud_jack_init(void);
void aud_jack_exit(void);
bool aud_jack_supported(void);
