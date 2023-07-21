BITS 32

GLOBAL k_print
GLOBAL go
GLOBAL dispatch
GLOBAL init_timer_dec
GLOBAL lidt

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
    