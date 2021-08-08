void lib_osCreateMesgQueue(void)
{
    PTR mq = a0;
    __write_u32(MQ_RECV, NULLPTR);
    __write_u32(MQ_SEND, NULLPTR);
    __write_u32(MQ_COUNT, 0);
    __write_u32(MQ_INDEX, 0);
    __write_u32(MQ_LEN, a2);
    __write_u32(MQ_MSG, a1);
}
