#include "mbed.h"
#include "string.h"

#include "ARQ_FSMevent.h"
#include "ARQ_msg.h"
#include "ARQ_timer.h"
#include "ARQ_LLinterface.h"
#include "ARQ_parameters.h"

//FSM state -------------------------------------------------
#define MAINSTATE_IDLE              0
#define MAINSTATE_TX                1
#define MAINSTATE_ACK               2 //state 추가

//GLOBAL variables (DO NOT TOUCH!) ------------------------------------------
//serial port interface
RawSerial pc(USBTX, USBRX);
//state variables
uint8_t main_state = MAINSTATE_IDLE; //protocol state

//source/destination ID
uint8_t endNode_ID=1;
uint8_t dest_ID=0;

//PDU context/size
uint8_t arqPdu[200];
uint8_t pduSize;

//SDU (input)
uint8_t originalWord[200];
uint8_t wordLen=0;

//ARQ parameters -------------------------------------------------------------
uint8_t seqNum = 0;     //ARQ sequence number
uint8_t retxCnt = 0;    //ARQ retransmission counter
uint8_t arqAck[5];      //ARQ ACK PDU

// 추가한 변수
uint8_t ackSize;        //ARQ ACK 사이즈
uint8_t pduSeqNum;      //pdu 수신시 받은 seqNum

//application event handler : generating SDU from keyboard input
void arqMain_processInputWord(void)
{
    char c = pc.getc();
    if (main_state == MAINSTATE_IDLE &&
        !arqEvent_checkEventFlag(arqEvent_dataToSend))
    {
        if (c == '\n' || c == '\r')
        {
            originalWord[wordLen++] = '\0';
            arqEvent_setEventFlag(arqEvent_dataToSend);
            pc.printf("word is ready! ::: %s\n", originalWord);
        }
        else
        {
            originalWord[wordLen++] = c;
            if (wordLen >= ARQMSG_MAXDATASIZE-1)
            {
                originalWord[wordLen++] = '\0';
                arqEvent_setEventFlag(arqEvent_dataToSend);
                pc.printf("\n max reached! word forced to be ready :::: %s\n", originalWord);
            }
        }
    }
}




//FSM operation implementation ------------------------------------------------
int main(void){

    uint8_t flag_needPrint=1;
    uint8_t prev_state = 0;

    //initialization
    arqEvent_clearAllEventFlag();
    pc.printf("--------- ARQ protocol starts! ---------\n");
    //source & destination ID setting

    //맥북 오류로 인한 첫 시작 부분 엔터로 시작하기
    pc.getc();

    pc.printf(":: ID for this node : ");
    //pc.scanf("%d", &endNode_ID);
    pc.putc(endNode_ID);
    pc.printf(":: ID for the destination : ");
    //pc.scanf("%d", &dest_ID);
    pc.putc(dest_ID);
    pc.getc();

    pc.printf("endnode : %i, dest : %i\n", endNode_ID, dest_ID);

    arqLLI_initLowLayer(endNode_ID);
    pc.attach(&arqMain_processInputWord, Serial::RxIrq);





    while(1){
        //debug message
        if (prev_state != main_state)
        {
            debug_if(DBGMSG_ARQ, "[ARQ] State transition from %i to %i\n", prev_state, main_state);
            prev_state = main_state;
        }


        //FSM should be implemented here! ---->>>>
        switch (main_state){
            case MAINSTATE_IDLE: //IDLE state description
                    
                if (arqEvent_checkEventFlag(arqEvent_dataRcvd)) //if data reception event happens
                {
                
                    //Retrieving data info.
                    uint8_t srcId = arqLLI_getSrcId();
                    uint8_t* dataPtr = arqLLI_getRcvdDataPtr();
                    pduSeqNum = arqMsg_getSeq(dataPtr);
                    uint8_t size = arqLLI_getSize();
                    pc.printf("\n -------------------------------------------------\nRCVD from %i : %s (length:%i, seq:%i)\n-------------------------------------------------\n", srcId, arqMsg_getWord(dataPtr), size, pduSeqNum);

                    //main_state = MAINSTATE_IDLE;
                    // ack를 보내기 위해 TX로 이동 
                    main_state = MAINSTATE_TX;
                    //arqEvent_clearEventFlag(arqEvent_dataRcvd);
                }
                
                else if (arqEvent_checkEventFlag(arqEvent_dataToSend)) //if data needs to be sent (keyboard input)
                {
                    //msg header setting
                    pduSize = arqMsg_encodeData(arqPdu, originalWord, seqNum, wordLen);
                    arqLLI_sendData(arqPdu, pduSize, dest_ID);
                    seqNum++;

                    pc.printf("[MAIN] sending to %i (seq:%i)\n", dest_ID, (seqNum-1)%ARQMSSG_MAX_SEQNUM);

                    //sdu 수신시 data송신 후 ack 수신을 위해 TX로 이동
                    main_state = MAINSTATE_TX;
                    wordLen = 0;
                    arqEvent_clearEventFlag(arqEvent_dataTxDone);
                }
            
                else if (flag_needPrint == 1)
                {
                    pc.printf("Give a word to send : ");
                    flag_needPrint = 0;
                }     

                break;

            case MAINSTATE_TX: //IDLE state description

                // pdu 수신시 ack 전송
                if(arqEvent_checkEventFlag(arqEvent_dataRcvd)){
                    // ack 수신 중 pdu를 수신한 경우 retxCnt가 0이 아니게 된다.
                    if(retxCnt != 0){
                        uint8_t srcId = arqLLI_getSrcId();
                        uint8_t* dataPtr = arqLLI_getRcvdDataPtr();
                        uint8_t size = arqLLI_getSize();
                        pc.printf("RCVD from %i : %s (length:%i, seq:%i)\n", srcId, arqMsg_getWord(dataPtr), size, arqMsg_getSeq(dataPtr));
                        ackSize = arqMsg_encodeAck(arqAck, arqMsg_getSeq(dataPtr));
                        // 받은 데이터를 출력하고 이에 해당하는 ack를 보낸다. ack는 pdu가 다른 node에서 왔을 수 있기 때문에 dest_Id가 아니라 srcid 로 보냄.
                        arqLLI_sendData(arqAck, ackSize, srcId);
                        pc.printf("Send to %i ACK\n",srcId);
                        main_state = MAINSTATE_ACK;
                        //ack 수신과 관련된 TX가 끝난게 아니기 때문에 끝나지 않도록 txdone을 clear한다.
                        arqEvent_clearEventFlag(arqEvent_ackTxDone);
                        arqEvent_clearEventFlag(arqEvent_dataRcvd);
                        arqEvent_clearEventFlag(arqEvent_dataTxDone);
                    }
                    else{
                        //정상 적으로 pdu를 수신하여 ack를 보내는 경우
                        ackSize = arqMsg_encodeAck(arqAck, pduSeqNum);
                        arqLLI_sendData(arqAck, ackSize, dest_ID);
                        pc.printf("Send to %i ACK\n",dest_ID);
                        main_state = MAINSTATE_IDLE;
                        flag_needPrint = 1;
                        arqEvent_clearEventFlag(arqEvent_ackTxDone);
                        arqEvent_clearEventFlag(arqEvent_dataRcvd);
                    }
                }
                // sdu수신시 ack 대기
                else if(arqEvent_checkEventFlag(arqEvent_dataToSend))
                {
                    //timer 시작 후 ack 수신을 위하여 state를 ack로 전환
                    arqTimer_startTimer();
                    main_state = MAINSTATE_ACK;
                    arqEvent_clearEventFlag(arqEvent_dataToSend);
                }
                else if(arqEvent_checkEventFlag(arqEvent_arqTimeout))
                {
                    //timeout 된 경우 다시 타이머를 실행하고 메세지를 다시 보냄
                    arqTimer_startTimer();
                    arqLLI_sendData(arqPdu, pduSize, dest_ID);
                    pc.printf("[retransmit] sending to %i msg : %s\n", dest_ID, originalWord);
                    main_state = MAINSTATE_ACK;
                    arqEvent_clearEventFlag(arqEvent_arqTimeout);
                    arqEvent_clearEventFlag(arqEvent_dataTxDone);
                }

                else if (arqEvent_checkEventFlag(arqEvent_dataTxDone)) //data TX finished
                {
                    main_state = MAINSTATE_IDLE;
                    flag_needPrint = 1;
                    arqEvent_clearEventFlag(arqEvent_dataTxDone);
                }
                break;

            case MAINSTATE_ACK: 
                //ack 수신 중 Pdu 수신시 pdu 처리를 위해 TX로 state 변경
                if(arqEvent_checkEventFlag(arqEvent_dataRcvd)){
                    main_state = MAINSTATE_TX;
                }
                else if(arqEvent_checkEventFlag(arqEvent_ackRcvd)){
                    arqTimer_stopTimer();
                    retxCnt = 0;
                    pc.printf("Receive ACK\n");
                    main_state = MAINSTATE_IDLE;
                    flag_needPrint = 1;
                    arqEvent_clearEventFlag(arqEvent_ackRcvd);
                }

                else if (arqEvent_checkEventFlag(arqEvent_arqTimeout)) {
                    arqTimer_stopTimer();
                    if(retxCnt>=ARQ_MAXRETRANSMISSION){
                        retxCnt = 0;
                        main_state = MAINSTATE_IDLE;
                        pc.printf("Goto IDLE\n");
                        flag_needPrint = 1;
                        arqEvent_clearEventFlag(arqEvent_arqTimeout);
                    }
                    else if(retxCnt<ARQ_MAXRETRANSMISSION){
                        retxCnt++;
                        main_state = MAINSTATE_TX;
                        pc.printf("Time out  ");
                        arqEvent_clearEventFlag(arqEvent_dataToSend);
                    }
                }

                else if(arqEvent_checkEventFlag(arqEvent_ackTxDone)){
                    main_state = MAINSTATE_IDLE;
                    flag_needPrint = 1;
                    arqEvent_clearEventFlag(arqEvent_ackTxDone);
                }
            
                break;

            default :
                break;
                    
        }        

    }    

}
