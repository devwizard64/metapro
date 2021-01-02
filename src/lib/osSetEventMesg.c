void lib_osSetEventMesg(void)
{
    struct os_event_t *event = &lib_event_table[a0.i[IX]];
    event->mq  = a1.i[IX];
    event->msg = a2.i[IX];
}
