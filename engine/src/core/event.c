#include "event.h"

b8 event_register(u16 code, void* listener, sys_event event) {
    (void)code;
    (void)listener;
    (void)event;
    return SN_TRUE;
}

b8 event_unregister(u16 code, void* listener, sys_event event) {
    (void)code;
    (void)listener;
    (void)event;
    return SN_TRUE;
}

b8 event_fire(u16 code, void* sender, event_context context) {
    (void)code;
    (void)sender;
    (void)context;
    return SN_TRUE;
}
