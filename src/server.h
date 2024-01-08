extern "C"
{
    int startserver(int port);
    int stopserver();
    int getmsg(char msg_read[1024]);
}
