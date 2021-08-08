void lib_osSetEventMesg(void)
{
    struct os_event *event = &lib_event_table[a0];
    event->mq  = a1;
    event->msg = a2;
}
