//definitions
#define QUEUE_SIZE 100
#define MAX_STACK 100
#define STACK_SIZE 1024

//include external functions
extern void k_print(char *string, int string_length, int row, int col);

//include headers
#include "legacy.h"
#include <limits.h>
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
    uint32_t priority;
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
pcb_t nullprocess;

//NASM-method declerations
extern void go();
extern void lidt(idtr_t var);
extern void dispatch();
extern void exitProcess();
extern void outportb(uint16_t port, uint8_t value);
extern void init_timer_dev(uint16_t ms);

//c-method declarations
void k_clearscr();
void init_idt_entry(idt_entry_t *entry, uint32_t base, uint16_t selector, uint8_t access);
void init_idt();
void defaultHandler();
int create_process(uint32_t process_entry, uint32_t priority);
void enqueue(pcbq_t *q, pcb_t *pcb);
void enqueue_priority(pcbq_t *q, pcb_t *pcb);
pcb_t *dequeue(pcbq_t *q);
void enqueueRunning();
void setCurrentProcess();
uint32_t getCurrentESP();
void setCurrentESP();
uint32_t *allocate_stack();
pcb_t *allocatePCB();
void setup_PIC();
char* itoa(int val, int base);

//processes
//   \/--This is inefficent I no likey
void countloop(int);
void p1();
void p2();
void p3();
void idle();

//main method
int main(){

    
    
    k_clearscr();
    
    k_print("Running processes", 17, 0, 0);

    init_idt();

    
    q.head = 0;
    q.tail = 0;
    q.buffer = 0;
    char test[1];

    init_timer_dev(50);
    setup_PIC();

    int retval = 2;
    
    //create processes with priority
    retval = create_process((uint32_t)p1,10);
    if(retval == -1){
        defaultHandler();
    }
    retval = create_process((uint32_t)p2,10);
    if(retval == -1){
        defaultHandler();
    }
    retval = create_process((uint32_t)p3,12);
    if(retval == -1){
        defaultHandler();
    }
    retval = create_process((uint32_t)idle,5);
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

    //k_clearscr();
    k_print("ERROR                    ", 25, 0, 0);
    
    /*//Set error error number
    char *numstr;
    numstr = itoa(currentPCBnum,10);
    k_print(numstr, 3 , 0, 6);
    */

    while(0);

}

//process creator---------------------------------------------------
int create_process(uint32_t process_entry, uint32_t priority){

    if(MAX_PCB <= currentPCBnum){

        return -1;
    }

    uint32_t *stackptr = allocate_stack();

    uint32_t *st = stackptr + STACK_SIZE;

    //set return method
    st--;
    *st = (int)go;
    //EFLAGS
    st--;
    *st = 0x0200;
    // CS
    st--;
    *st = 16;
    // Address of process
    st--;
    *st = process_entry;
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
    pcb->priority = priority;

    //currentPCBnum++;

    enqueue_priority(&q,pcb);

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

    //check is queue is full
    if(q->buffer == 100){
        return;
    }

    //check if queue is empty
    if(q->buffer == 0){

        q ->pcbq[0] = *(pcb);
        q->tail = 0;
        q->head = 0;
        //q -> tail = 1;
        //q->buffer++;
    }
    //setting pcb queue tail to new pcb
    q->pcbq[q->tail] = *pcb;
    //increasing queue size
    q->buffer++;
    //setting new tail location
    q -> tail++;
    q-> tail %= 100;

}
void enqueue_priority(pcbq_t *q, pcb_t *pcb){

    if(q->buffer == 100){
        return;
    }

    if(q->buffer == 0){

        q->pcbq[0] = *(pcb);
        q->tail = 0;
        q->head = 0;
    }else{
    //calculate new queue location and insert
        
        pcb_t temp_pcb,temp_pcb2;
        temp_pcb2 = *pcb;

        //loop through queue
        for(int i =  q->head; i <= q-> tail || q-> tail < q -> head ; (i++)%100 ){
            
            if(i == (q -> tail)){

                //set process to tail of list
                q->pcbq[q->tail] = temp_pcb2;
                
                break;

            }else if( q->pcbq[i].priority  < pcb -> priority && q->pcbq[i].processNum != temp_pcb2.processNum){

                //move process down the queue
                temp_pcb = q->pcbq[i];
                q->pcbq[i] = temp_pcb2;
                temp_pcb2 = temp_pcb;

            }

        }

    }

    //increasing queue size
    q->buffer++;
    //setting new tail location
    q -> tail++;
    q-> tail %= 100;

}
pcb_t *dequeue(pcbq_t *q){

    pcb_t *temp = &(q->pcbq[q->head]);
    //pcb_t temp = (q->pcbq[q->head]);
    //q->pcbq[q->head] = nullprocess ; attempt at clearing queue values
    q->head++;
    
    if(q->head == 100){
        q->head = 0;
    }
    q->buffer--;
    return temp;

}
void enqueueRunning(){
    enqueue_priority(&q,running);
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
int const loopval = 2147483647 / 100 +4;

void p1(){
    
    
    int i = 0;
    char numString[1];
    char one;
    k_print("process p1:", 11, 11,0);
    k_print("process p1 Running...", 21,10,0);
    for (int j = 0; j <= loopval; j++)
    {
        
        
        one = i % 6;
            
        numString[0] = one + '0';
        
        //only print every 3 times to give screen catch up time
        if(j%3==0)k_print(numString, 1, 11,12);

        if(i == 6){
            i = 1;
        }
        i++;
       
    }
    k_print(numString, 1, 11,12);
    k_print("process p1:DONE       ", 21,10,0);
    //exitProcess();
    
}
void p2(){
    
    int i = 0;
    char numString[1];
    char one;
    k_print("process p2:", 11, 13,0);
    k_print("process p1 Running...", 21,12,0);
    for (int j = 0; j <=  loopval; j++)
    {
        
        one = i % 6;

        numString[0] = one + '0';
        
        //only print every 3 times to give screen catch up time
        if(j%3==0) k_print(numString, 1, 13,12);

        if(i == 6){
            i = 1;
        }
        i++;
    }
    k_print(numString, 1, 13,12);
    k_print("process p1:DONE       ", 21,12,0);
    //exitProcess();
}
void p3(){
    
    int i = 0;
    char numString[1];
    char one;
    k_print("process p3:", 11,15,0);
    k_print("process p1 Running...", 21,14,0);
    for (int j = 0; j <= loopval; j++)
    {
        
        one = i % 6;
 
        numString[0] = one + '0';
        
        //only print every 3 times to give screen catch up time
        if(j%3==0) k_print(numString, 1, 15,12);


        if(i == 6){
            i = 1;
        }
        i++;
    }
    k_print(numString, 1, 15,12);
    k_print("process p1:DONE       ", 21,14,0);

    //exitProcess();

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

void idle(){

    k_print("Process Idle running...", 23, 24,0);

    while(1){

        k_print("/", 1, 24,24);
        for(int i = 0; i < 100000000;i ++){}
        k_print("\\", 1, 24,24);
        for(int i = 0; i < 100000000;i ++){}

    }

}

void setup_PIC() {
    // set up cascading mode:
    outportb(0x20, 0x11); // start 8259 master initialization
    outportb(0xA0, 0x11); // start 8259 slave initialization
    outportb(0x21, 0x20); // set master base interrupt vector (idt 32-38)
    outportb(0xA1, 0x28); // set slave base interrupt vector (idt 39-45)
    // Tell the master that he has a slave:
    outportb(0x21, 0x04); // set cascade ...
    outportb(0xA1, 0x02); // on IRQ2
    // Enabled 8086 mode:
    outportb(0x21, 0x01); // finish 8259 initialization
    outportb(0xA1, 0x01);
    // Reset the IRQ masks
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
    // Now, enable the clock IRQ only 
    outportb(0x21, 0xfe); // Turn on the clock IRQ
    outportb(0xA1, 0xff); // Turn off all others
}

//convert int to char array
char* itoa(int val, int base){

    static char buf[32] = {0};

    int i = 30;

    for(; val && i ; --i, val /= base)

        buf[i] = "0123456789abcdef"[val % base];

    return &buf[i+1];

}
