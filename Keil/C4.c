//  Create at 2023-10-2
//  STC89C52RC @ 33.1776MHz(6T)
//  C4(CSGO)
//  TH TL 00100111(39) 11111111(255) 10239 100@33.1776MHz(6T)
//  TH TL 11101010(234) 11100111(231) 60135 1024@33.1776MHz(6T)

#include <reg52.h>
#include <serialfunc.h>

//键盘引脚定义
sbit keypad1 = P0 ^ 0;
sbit keypad2 = P0 ^ 1;
sbit keypad3 = P0 ^ 2;
sbit keypad4 = P1 ^ 0;
sbit keypad5 = P1 ^ 1;
sbit keypad6 = P1 ^ 2;
sbit keypad7 = P1 ^ 3;

//蜂鸣器引脚定义
sbit speaker0 = P1 ^ 4;
sbit led0 = P1 ^ 5;

//调试引脚定义
sbit serialdebugpin = P0 ^ 6;
sbit debugpin = P0 ^ 7;

//1601引脚定义
// sbit lcd0vl = P1 ^ 6;
sbit lcd0backlight = P1 ^ 7;
sbit lcd0rs = P3 ^ 7;
sbit lcd0rw = P3 ^ 6;
sbit lcd0e = P3 ^ 5;

//雷管引脚定义
sbit explodepin = P3 ^ 4;

//主函数变量
unsigned char g_mainfunc = 1, g_c4active = 0;
//按键变量
unsigned char g_keys[12] = {0}, g_keysold[12] = {0};
//调试变量
unsigned char g_serialoutput = 0;
//定时变量
unsigned char g_TH = 0, g_TL = 0, g_lastTH = 0, g_lastTL = 0;
//C4变量
unsigned char g_c4passwordset[7] = {'7', '3', '5', '5', '6', '0', '8'}, g_c4password[7] = {'*', '*', '*', '*', '*', '*', '*'}, g_c4passwordcur = 0, g_c4passwordvalid = 0, g_c4timer = 0;
//延时变量
unsigned char g_delaycount = 0;
//屏幕变量
unsigned char g_charpos = 0;

//延时函数
void delay976Us(unsigned char t){
    g_delaycount = t;
    g_TH = 234;
    g_TL = 231;
    TH0 = g_TH;
    TL0 = g_TL;
    TR0 = 1;
    while(g_delaycount);
}

void delay10Ms(unsigned char t){
    g_delaycount = t;
    g_TH = 39;
    g_TL = 255;
    TH0 = g_TH;
    TL0 = g_TL;
    TR0 = 1;
    while(g_delaycount);
}

void delayBeep(unsigned char t){
    unsigned char t1 = t;
    unsigned char t2 = t;
    for(;t > 0; t--)
        for(;t1 > 0; t1--)
            for(;t2 > 0; t2--);
}

void delayS(unsigned char t){
    unsigned times = 0;
    for(; t > 0; t--){
        delay10Ms(100);
        delay10Ms(100);
        delay10Ms(100);
        delay10Ms(100);
    }
}

//C4哔哔函数
void c4Beep(unsigned char t2, unsigned char th, unsigned char tl){
    unsigned char t;
    //EA = 0;
    speaker0 = 0;
    for(; t2 > 0; t2--)
        for(t = 255; t > 0; t--){
            delayBeep(th);
            speaker0 = ~speaker0;
            delayBeep(tl);
        }
    speaker0 = 1;
    //EA = 1;
}

void c4BeepNew();

//C4叨哔函数
void c4Beeps(unsigned char times, unsigned char c4beeptime, unsigned char beepdelay, unsigned char th, unsigned char tl){
    for(;times > 0; times--){
        c4Beep(c4beeptime, th, tl);
        delay10Ms(beepdelay);
    }
}

//按键循环
/* void detectKeyLoop(){
    keypad1 = 0;
    keypad1 = 1;
    keypad2 = 0;
    keypad2 = 1;
    keypad3 = 0;
    keypad3 = 1;
} */

//按键检测函数(物理)
void detectKeys(){

    keypad1 = 0;
    if(keypad4 == 0) g_keys[1] = 1;
    else g_keys[1] = 0;
    if(keypad5 == 0) g_keys[4] = 1;
    else g_keys[4] = 0;
    if(keypad6 == 0) g_keys[7] = 1;
    else g_keys[7] = 0;
    if(keypad7 == 0) g_keys[10] = 1;
    else g_keys[10] = 0;
    keypad1 = 1;

    delay976Us(1);

    keypad2 = 0;
    if(keypad4 == 0) g_keys[2] = 1;
    else g_keys[2] = 0;
    if(keypad5 == 0) g_keys[5] = 1;
    else g_keys[5] = 0;
    if(keypad6 == 0) g_keys[8] = 1;
    else g_keys[8] = 0;
    if(keypad7 == 0) g_keys[0] = 1;
    else g_keys[0] = 0;
    keypad2 = 1;

    delay976Us(1);

    keypad3 = 0;
    if(keypad4 == 0) g_keys[3] = 1;
    else g_keys[3] = 0;
    if(keypad5 == 0) g_keys[6] = 1;
    else g_keys[6] = 0;
    if(keypad6 == 0) g_keys[9] = 1;
    else g_keys[9] = 0;
    if(keypad7 == 0) g_keys[11] = 1;
    else g_keys[11] = 0;
    keypad3 = 1;

    delay976Us(1);
}

//初始化扬声器函数
void initSpeaker(){
    c4Beeps(2, 4, 1, 21, 1);
}

//发送命令
void sendC1601CMD(unsigned char inCMD){
    if(inCMD == 1) g_charpos = 0;
    lcd0rs = 0;
    lcd0rw = 0;
    P2 = inCMD;
    lcd0e = 1;
    delay976Us(2);
    lcd0e = 0;
}

//不跳过发送数据
void sendC1601CharU(unsigned char inChar){
    lcd0rs = 1;
    lcd0rw = 0;
    P2 = inChar;
    lcd0e = 1;
    delay976Us(2);
    lcd0e = 0;
}

//反向发送数据
void sendC1601CharR(unsigned char inChar){
    lcd0rs = 1;
    lcd0rw = 0;
    P2 = inChar;
    lcd0e = 1;
    delay976Us(2);
    lcd0e = 0;
    g_charpos--;
    if(g_charpos == 8) sendC1601CMD(135);
}

//发送数据
void sendC1601Char(unsigned char inChar){
    lcd0rs = 1;
    lcd0rw = 0;
    P2 = inChar;
    lcd0e = 1;
    delay976Us(2);
    lcd0e = 0;
    g_charpos++;
    if(g_charpos == 8) sendC1601CMD(192);
}

//反向打印字符串
void printlR(unsigned char *inChar){
    sendC1601CMD(4);
    while(*inChar != '\0'){
        sendC1601CharR(*inChar);
        inChar++;
    }
}

//延时反向打印字符串
void printlRD(unsigned char *inChar, unsigned char inDelay){
    sendC1601CMD(4);
    while(*inChar != '\0'){
        sendC1601CharR(*inChar);
        delay10Ms(inDelay);
        inChar++;
    }
}

//不跳过打印字符串
void printlU(unsigned char *inChar){
    while(*inChar != '\0'){
        sendC1601CharU(*inChar);
        inChar++;
    }
}

//打印字符串
void printl(unsigned char *inChar){
    sendC1601CMD(6);
    while(*inChar != '\0'){
        sendC1601Char(*inChar);
        inChar++;
    }
}

//更新屏幕
void updateScreenPassword(){
    unsigned char passwordpos = g_c4passwordcur;
    sendC1601CMD(6);
    sendC1601CMD(128);
    g_charpos = 0;
    for(; 7 - passwordpos; passwordpos++) printl(" *");
    for(passwordpos = 0; passwordpos < g_c4passwordcur; passwordpos++){
        sendC1601Char(' ');
        sendC1601Char(g_c4password[passwordpos]);
    }
}

//初始化屏幕
void initC1601(){
    // lcd0vl = 0;
    lcd0backlight = 0;
    sendC1601CMD(56);
    sendC1601CMD(12);
    sendC1601CMD(6);
    sendC1601CMD(1);
}

//初始化函数
void initHw(){
    //初始化IO接口
    P0 = 248;
    P1 = 255;
    P2 = 0;
    P3 = 23;

    //初始化定时设置
    TMOD = 33;

    //初始化中断
    EA = 1;
    IT0 = 1;
    ET0 = 1;
    // PT0 = 1;

    initSpeaker();  //嘟嘟嘟测试
    initC1601();  //亮晶晶测试

}

//调试模式函数
void debugMode(){
    while(1);
}

//启用串口调试
void enableSerialDebug(){
    g_serialoutput = 1;
    PCON = 128;
    SM0 = 0;
    SM1 = 1;
    REN = 1;
    TH1 = 253;
    TL1 = 253;
    TR1 = 1;
    prints("Serial debug enabled!\n\r");
    printl("Serial Debug On");
    delay10Ms(200);
    ES = 1;
}

//调试功能检测
void detectDebugPin(){
    if(debugpin == 1) debugMode();
    if(serialdebugpin == 1) enableSerialDebug();
}

//初始化软件
void initSys(){
    EX0 = 0;
    if(g_serialoutput) prints("->initSys()");
    sendC1601CMD(1);
    g_c4timer = 100;
    g_c4passwordvalid = 0;
    g_c4passwordcur = 0;
    g_charpos = 0;
    g_c4active = 0;

    g_charpos = 15;
    sendC1601CMD(198);
    printlRD(" * * * * * * * ", 2);
    g_charpos = 15;
    sendC1601CMD(198);
     if(g_serialoutput) prints("initSys()->");
    EX0 = 1;
}

//爆炸
void explode(){
    delay10Ms(100);
    c4Beeps(11, 1, 6, 19, 1);
    explodepin = 0; //激活雷管
    initSys();
}

//密码错误
void wrongPassword(){
    unsigned char t = 3;
    delay10Ms(100);
    if(g_serialoutput) prints("Password mismatch");
    while(t){
        sendC1601CMD(8);
        delay10Ms(15);
        sendC1601CMD(12);
        delay10Ms(15);
        c4Beep(4, 29, 1);
        t--;
    }
    initSys();
}

//检查密码
void checkPassword(){
    while(g_c4passwordcur){
        if(g_c4password[g_c4passwordcur - 1] == g_c4passwordset[g_c4passwordcur - 1]){
            g_c4passwordvalid++;
        }
        g_c4passwordcur--;
    }
    if(g_c4passwordvalid != 7) wrongPassword();
}

//输入密码
void enterPassword(unsigned char inChar){
    g_c4password[g_c4passwordcur] = inChar;
    g_c4passwordcur++;
    // sendC1601CharR(' ');
    // sendC1601CharR(inChar);
    updateScreenPassword();
    if(g_c4passwordcur >= 7) checkPassword();
}

//按键处理函数
void keyPress(unsigned char key){
    switch(key){
    case 0:
        if(g_serialoutput) prints("Key 0 press\n\r");
        enterPassword(48);
        break;
    case 1:
        if(g_serialoutput) prints("Key 1 press\n\r");
        enterPassword(49);
        break;
    case 2:
        if(g_serialoutput) prints("Key 2 press\n\r");
        enterPassword(50);
        break;
    case 3:
        if(g_serialoutput) prints("Key 3 press\n\r");
        enterPassword(51);
        break;
    case 4:
        if(g_serialoutput) prints("Key 4 press\n\r");
        enterPassword(52);
        break;
    case 5:
        if(g_serialoutput) prints("Key 5 press\n\r");
        enterPassword(53);
        break;
    case 6:
        if(g_serialoutput) prints("Key 6 press\n\r");
        enterPassword(54);
        break;
    case 7:
        if(g_serialoutput) prints("Key 7 press\n\r");
        enterPassword(55);
        break;
    case 8:
        if(g_serialoutput) prints("Key 8 press\n\r");
        enterPassword(56);
        break;
    case 9:
        if(g_serialoutput) prints("Key 9 press\n\r");
        enterPassword(57);
        break;
    case 10:
        if(g_serialoutput) prints("Key * press\n\r");
        // enterPassword(48);
        break;
    case 11:
        if(g_serialoutput) prints("Key # press\n\r");
        // enterPassword(48);
        break;
    default:
        break;
    }
}

//按键检测函数(虚拟)
void processKeys(){
    unsigned char keyNum = 0;
    if(g_keys[0] && !g_keysold[0]) keyPress(0);
    else if(g_keys[1] && !g_keysold[1]) keyPress(1);
    else if(g_keys[2] && !g_keysold[2]) keyPress(2);
    else if(g_keys[3] && !g_keysold[3]) keyPress(3);
    else if(g_keys[4] && !g_keysold[4]) keyPress(4);
    else if(g_keys[5] && !g_keysold[5]) keyPress(5);
    else if(g_keys[6] && !g_keysold[6]) keyPress(6);
    else if(g_keys[7] && !g_keysold[7]) keyPress(7);
    else if(g_keys[8] && !g_keysold[8]) keyPress(8);
    else if(g_keys[9] && !g_keysold[9]) keyPress(9);
    else if(g_keys[10] && !g_keysold[10]) keyPress(10);
    else if(g_keys[11] && !g_keysold[11]) keyPress(11);
    for(; keyNum < 12; keyNum++) g_keysold[keyNum] = g_keys[keyNum];
}

//激活C4
void activeC4(){
    unsigned lastEX0 = EX0;
    if(EX0) EX0 = 0;
    g_c4timer = 100;
    g_c4active = 1;
    delay10Ms(100);
    if(g_serialoutput) prints("C4 actived\n\r");
    sendC1601CMD(1);
    printl(" * * * * * * * ");
    while(g_c4active){
        led0 = 0;
        c4Beep(3, 20, 1);
        delay10Ms(1);
        led0 = 1;
        processKeys();
        delay10Ms(g_c4timer);
        if(g_c4timer >= 52) g_c4timer -= 2;
        if(g_c4timer < 52 && g_c4timer) g_c4timer--;
        if(!g_c4timer) explode();
    }
    EX0 = lastEX0;
}

//主函数
void main(){
    initHw();
    detectDebugPin();
    initSys();
    while(g_mainfunc){
        if(g_c4passwordvalid == 7) activeC4();
    };
}

//外部中断0函数
void keyInterrupt() interrupt 0{
    unsigned char lastPT0 = PT0, lastTR0 = TR0, lastTH = g_TH, lastTL = g_TL, lastTH0 = TH0, lastTL0 = TL0, lastdelaycount = g_delaycount;
    if(TR0) TR0 = 0;
    if(!PT0) PT0 = 1;
    keypad1 = 1;
    keypad2 = 1;
    keypad3 = 1;
    delay976Us(1);
    detectKeys();
    if(!g_c4active) processKeys();
    keypad1 = 0;
    keypad2 = 0;
    keypad3 = 0;
    PT0 = lastPT0;
    g_delaycount = lastdelaycount;
    g_TH = lastTH;
    g_TL = lastTL;
    TH0 = lastTH0;
    TL0 = lastTL0;
    TR0 = lastTR0;
}

//定时器0中断
void timer0Interrupt() interrupt 1{
    g_delaycount--;
    TH0 = g_TH;
    TL0 = g_TL;
    if(!g_delaycount) TR0 = 0;
}

//串口中断
void serialInterrupt() interrupt 4{
    unsigned char lastEA = EA, recvmsg = SBUF;
    if(EA) EA = 0;
    EA = lastEA;
    RI = 0;
}

//Xiziya_R
//Ver 1.0(Release) @ 2023/10/6
//Ver 1.1(Bug fix) @ 2023/10/8