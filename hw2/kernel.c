//definitions
#define QUEUE_SIZE 100
#define MAX_STACK 100
#define STACK_SIZE 1024

//include external functions
extern void k_print(char *string, int string_length, int row, int col);

//include headers
#include "legacy.h"
#define null 0
//TYPE definitions
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

//struct declerations
struct idt_entry{

    uint16_t base_low16;
    uint16_t selector;
    uint8_t always0;
    uint8_t access;
    uint16_t base_hi16;

} __attribute__((packed));
typedef struct idt_entry idt_entry_t;
idt_entry_t idt[256];

struct idtr {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));
typedef struct idtr idtr_t;


struct pcb{
    uint32_t esp;  // stack ptr
    uint32_t processNum;       // process id
};
typedef struct pcb pcb_t;



int queLocation = 0;
struct pcbq{

    uint32_t buffer;
    uint32_t head;
    uint32_t tail;
    pcb_t pcbq[QUEUE_SIZE];

};
typedef struct pcbq pcbq_t;
//variable declerations
const int  MAX_PCB = 20;
int currentPCBnum = 0;
int queueln = 0;
pcb_t *running;
pcbq_t q;
idtr_t idtr;
uint32_t stacks[MAX_STACK][STACK_SIZE];

//NASM-method declerations
extern void go();
extern void lidt(idtr_t var);
extern void dispatch();

//c-method declerations
void k_clearscr();
void init_idt_entry(idt_entry_t *entry, uint32_t base, uint16_t selector, uint8_t access);
void init_idt();
void defaultHandler();
int create_process(uint32_t processNum);
void enqueue(pcbq_t *q, pcb_t *pcb);
pcb_t *dequeue(pcbq_t *q);
void enqueueRunning();
void setCurrentProcess();
uint32_t getCurrentESP();
void setCurrentESP();
uint32_t *allocate_stack();
pcb_t *allocatePCB();


//   \/--This is inefficent I no likey
void countloop(int);
void p1();
void p2();
void p3();
void p4();
void p5();


//main method
int main(){


    
    k_clearscr();
    
    k_print("Running processes", 17, 0, 0);

    init_idt();

    
    q.head = 0;
    q.tail = 0;
    q.buffer = 0;
    char test[1];

    int retval = 2;
    retval = create_process((uint32_t)p1);
    if(retval == -1){
        defaultHandler();
    }
    retval = create_process((uint32_t)p2);
    if(retval == -1){
        defaultHandler();
    }
    retval = create_process((uint32_t)p3);
    if(retval == -1){
        defaultHandler();
    }
    retval = create_process((uint32_t)p4);
    if(retval == -1){
        defaultHandler();
    }
    retval = create_process((uint32_t)p5);
    if(retval == -1){
        defaultHandler();
    }
    go();

    while(1);
    return 0;
}

//function to clear screen
void k_clearscr(){

    //make vars
    int screenln = 80*25; 
    char blankscreen[screenln]; 
    //fillarray
    for (int i = 0; i < screenln; i++ /*i+=2*/){
        blankscreen[i] = ' ';
        //blankscreen[i+1] = 31;
    }

    k_print(blankscreen, screenln,0,0);

    
    return;
}

//Interupter Discriptor Table(IDT) creator--------------------------------
void initIDTEntry(idt_entry_t *entry, uint32_t base, uint16_t selector, uint8_t access){
    
    //dont understand the need for mask
	entry->base_low16 = (uint16_t) base ;
	entry->base_hi16 = (uint16_t) (base >> 16) ;
    entry->always0 = 0x0;
	entry->selector = selector ;
	entry->access = access;
	
}

void init_idt() {

    uint8_t access = 0x8e;
	uint16_t selector = 0x10;
    //Call init_idt_entry() for entries 0 to 31 setting these entries to point to the default handler
    for (int i = 0; i < 32; i++)
    {
        initIDTEntry(&idt[i],(uint32_t)defaultHandler,selector,access);
    }
    //Call init_idt_entry() for entry 32 to point to your dispatcher function
    initIDTEntry(&idt[32],(uint32_t)dispatch,selector,access);

    //Call init_idt_entry() for entries 33 to 255 setting these entries to point to 0
    for (int i = 33; i < 256; i++)
    {
        initIDTEntry(&idt[i],0,selector,access);
    }
    //Declare a variable called idtr, and fill in its structure with the limit and base of the idt array
    idtr.base = (uint32_t)idt;
    idtr.limit = sizeof(idt) -1;
    
    lidt(idtr);

}

//Error handler for IDT--------------------------------------------
void defaultHandler(){

    k_clearscr();
    k_print("ERROR", 5, 0, 0);

    while(0);

}

//process creator---------------------------------------------------
int create_process(uint32_t processEntry){

    if(MAX_PCB <= currentPCBnum){
        return -1;
    }

    uint32_t *stackptr = allocate_stack();

    uint32_t *st = stackptr + STACK_SIZE;

  
    
    st--;
    *st = 0;
    // CS
    st--;
    *st = 16;
    // Address of process
    st--;
    *st = processEntry;
    // EBP
    st--;
    *st = 0;
    // ESP
    st--;
    *st = 0;
    // EDI
    st--;
    *st = 0;
    // ESI
    st--;
    *st = 0;
    // EDX
    st--;
    *st = 0;
    // ECX
    st--;
    *st = 0;
    // EBX
    st--;
    *st = 0;
    // EAX
    st--;
    *st = 0;
    // DS
    st--;
    *st = 8;
    // ES
    st--;
    *st = 8;
    // FS
    st--;
    *st = 8;
    // GS
    st--;
    *st = 8;

    pcb_t *pcb = allocatePCB();
    pcb->esp = (uint32_t)st;
    pcb->processNum = ++currentPCBnum;

    //currentPCBnum++;

    enqueue(&q,pcb);

    return 0;

}

uint32_t *allocate_stack(){ 
    return stacks[currentPCBnum];
}
pcb_t *allocatePCB(){
    return &(q.pcbq[q.tail]);
    
    //return pcb_t[currentPCBnum]; 
};


//queue management--------------------------------------------------
void enqueue(pcbq_t *q, pcb_t *pcb){

    if(q->buffer == 100){
        return;
    }

    if(q->buffer == 0){

        q ->pcbq[0] = *(pcb);
        q->tail = 0;
        q->head = 0;
        //q -> tail = 1;
        //q->buffer++;
    }
    
    q->pcbq[q->tail] = *pcb;
    q->buffer++;
    
    q -> tail++;
    q-> tail %= 100;

}
pcb_t *dequeue(pcbq_t *q){

    pcb_t *temp = &(q->pcbq[q->head]);
    q->head++;
    
    if(q->head == 100){
        q->head = 0;
    }
    q->buffer--;
    return temp;

}
void enqueueRunning(){
    enqueue(&q,running);
}
void setCurrentProcess(){
    running = dequeue(&q);
}
uint32_t getCurrentESP(){
    return running -> esp;
}
void setCurrentESP(uint32_t esp){
    running ->esp = esp;
}


//test processes----------------------------------------------------

void p1(){
    //countloop(1);
    /*k_print("hello",5,5,0);
    asm("int $32");*/
    
    for (int i = 0; i <= 500; i++)
    {
        
        char hundred = (i/100) %10;
        char ten = (i/10) % 10;
        char one = i % 10;

        char numString[3] = {hundred,ten,one};
        for (int i = 0; i < 3; i++)
        {
            
                numString[i] += '0';
        }
        k_print("process p1:", 11, 9,0);
        k_print(numString, 3, 9,12);

        if(i == 500){
            i = 0;
        }
        for (int j = 0; j < 20000; j++)
        {
            asm("int $32");
        }
    }
    
}
void p2(){
    //countloop(2);
    /*k_print("hello2",6,6,0);
    asm("int $32");*/
    for (int i = 0; i <= 500; i++)
    {
        
        char hundred = (i/100) %10;
        char ten = (i/10) % 10;
        char one = i % 10;

        char numString[3] = {hundred,ten,one};
        for (int i = 0; i < 3; i++)
        {
            numString[i] += '0';
        }
        k_print("process p2:", 11, 10,0);
        k_print(numString, 3, 10,12);

        if(i == 500){
            i = 0;
        }
        for (int j = 0; j < 20000; j++)
        {
            asm("int $32");
        }
    }
}
void p3(){
    //countloop(3);
    /*k_print("hello3",6,7,0);
    asm("int $32");*/
    for (int i = 0; i <= 500; i++)
    {
        
        char hundred = (i/100) %10;
        char ten = (i/10) % 10;
        char one = i % 10;

        char numString[3] = {hundred,ten,one};
        for (int i = 0; i < 3; i++)
        {
            numString[i] += '0';
        }
        k_print("process p3:", 11,11,0);
        k_print(numString, 3, 11,12);

        if(i == 500){
            i = 0;
        }
        for (int j = 0; j < 20000; j++)
        {
            asm("int $32");
        }
    }
}
void p4(){
    //countloop(4);
    /*k_print("hello4",6,8,0);
    asm("int $32");*/
    for (int i = 0; i <= 500; i++)
    {
        
        char hundred = (i/100) %10;
        char ten = (i/10) % 10;
        char one = i % 10;

        char numString[3] = {hundred,ten,one};
        for (int i = 0; i < 3; i++)
        {
            numString[i] += '0';
        }
        k_print("process p4:", 11, 12,0);
        k_print(numString, 3, 12,12);

        if(i == 500){
            i = 0;
        }
        for (int j = 0; j < 20000; j++)
        {
            asm("int $32");
        }
    }
}
void p5(){
    //countloop(5);
    /*k_print("hello5",6,9,0);
    asm("int $32");*/
    
    
    for (int i = 0; i <= 500; i++)
    {
        
        char hundred = (i/100) %10;
        char ten = (i/10) % 10;
        char one = i % 10;

        char numString[3] = {hundred,ten,one};
        for (int i = 2; i >= 0; i--)
        {
            
            numString[i] += '0';
            
        }
        


        k_print("process p5:", 11, 13,0);
        k_print(numString, 3, 13,12);

        if(i == 500){
            i = 0;
        }
        for (int j = 0; j < 20000; j++)
        {
            asm("int $32");
        }
        
    }
}
//simplification p1-p5 process tasks
/*char numchar[1];
void countloop(int num){
    
    k_print("hello",5,4,6*num);
    int i =0;
    while (1)
    {
        //string = "process p" + ((string)num+48) + ": " + ((char)i+48)
        //k_print("process p"+((char)num+48),10,num+5,0);
        //k_print(":"+((char)i+48),4,num+5,11);
        numchar[1] = num+'0';
        k_print("process p",9,num+6,0);
        k_print(numchar,1,num+6,10);
        numchar[1] = i+'0';
        k_print(":",1,num+6,11+1);
        k_print(numchar,1,num+6,11+2);
        i = ((i+1) % 10);
        asm("int $32");
    }
    
}*/


