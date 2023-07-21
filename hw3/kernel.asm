BITS 32

GLOBAL k_print
GLOBAL go
GLOBAL dispatch
GLOBAL init_timer_dec
GLOBAL lidt
GLOBAL outportb
GLOBAL init_timer_dev

EXTERN running
EXTERN dequeue
EXTERN enqueue
EXTERN setCurrentProcess
EXTERN getCurrentESP
EXTERN setCurrentESP
EXTERN enqueueRunning

;void k_print(char *string, int string_length, int row, int col);
k_print:
    
    push ebp
    mov ebp, esp
    pushf
    push eax
    push ecx
    push edx
    
    push edi
    push esi
    
    
    
    mov edi, [ebp+16]
    imul edi, 80
    add edi, [ebp+20]
    imul edi, 2
    add edi, 0xB8000
     
    mov ecx, [ebp+12]
    mov esi, [ebp+8]
    
    printloop:
    
    ;if edi is greater than 0xB8F9E
    cmp edi, 0xB8F9E
    jle skipvarset
    mov edi, 0xB8000
    skipvarset:
    
    cmp ecx, 0
    je endloop
    
    movsb
    mov BYTE[edi], 31
    inc edi
    dec ecx
    jmp printloop
    
    endloop:pop esi
    pop edi
    pop edx
    pop ecx
    pop eax
    popf
    pop ebp
    ret
    
dispatch:

    pushad
    push ds
    push es
    push fs
    push gs
    
    push esp
    call setCurrentESP
    add esp, 4

    call enqueueRunning

    call setCurrentProcess

    call getCurrentESP
    mov esp, eax
    
    
    ;mov eax, running
    ;mov [eax], esp
    ;
    ;push eax
    ;call enqueue
    ;pop eax
    ;
    ;call dequeue
    ;
    ;mov running, eax
    ;mov esp, [eax]
    
    pop gs
    pop fs
    pop es
    pop ds
    popad
    
    push eax
    ;send EOI to PIC
    mov al, 0x20
    out 0x20, al
    pop eax
    
    iret
    
go:
    
    ;call dequeue
    ;mov running, eax;
    call setCurrentProcess
    call getCurrentESP
    mov esp, eax;
    pop gs
    pop fs
    pop es
    pop ds
    popad
    iret
    
lidt:
  ;push ebp
  ;mov ebp, esp
  ;pushad

  ;mov eax, [ebp+4]
  ;lidt [eax]
  lidt [esp+4]
  ;popad
  ;pop ebp
  ret 
    
outportb:
    
    push ebp
    mov ebp, esp
    push edx
    push eax
    mov edx, [ebp+8]
    mov eax, [ebp+12]
    out dx, al
    pop eax
    pop edx
    pop ebp
    ret

; void init_timer_dev(int ms)    
init_timer_dev:
    ; 1) Do the normal preamble for assembly functions (set up ebp and save any registers
    ; that will be used). The first arg is time in ms
    ; 2) move the ms argument value into a register (say, edx)
    ; 3) Multiply dx (only the bottom 16 bits will be used) by 1193. 
    ; Why? because the timer cycles at 1193180 times a second, which is 1193 times a ms 
    ; note: The results must fit in 16 bits, so ms can't be more than 54.
    ; So, put your code for steps 1 - 3 HERE:
    push ebp
    mov ebp, esp
    push eax
    push edx
    mov edx, [ebp+8]
    mov eax, 1193
    mul eax
    
    ;// The code for steps 4-6 is given to you for free below...
    ;// 4) Send the command word to the PIT (Programmable Interval Timer) that initializes Counter 0
    ;// (there are three counters, but you will only use counter 0).
    ;// The PIT will be initalized to mode 3, Read or Load LSB first then MSB, and
    ;// Channel (counter) 0 with the following bits: 0b00110110 =
    ;// Counter 0 |Read then load|Mode 3|Binary. So, the instructions will be:
    mov al, 0b00110110
    out 0x43, al
    mov ax, dx
    out 0x40, al
    xchg ah, al
    out 0x40, al
    pop edx
    pop eax
    pop ebp
    ret
