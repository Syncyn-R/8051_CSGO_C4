void sendChar(unsigned char inChar){
    unsigned char lastEA = EA;
    if(EA) EA = 0;
    SBUF = inChar;
    while(!TI);
    TI = 0;
    EA = lastEA;
}

void prints(unsigned char *inChar){
    while(*inChar != '\0'){
        sendChar(*inChar);
        inChar++;
    }
}