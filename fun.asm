%macro assign 2         ;Calling order    1, 2
       mov eax, [%2]
       mov [%1], eax
%endmacro

;   c = a+b    IMPORTANT  Calling order c,a,b
%macro AddAndAssign 3
    mov eax, [%2]
    add eax, [%3]
    mov [%1], eax
%endmacro

;   c = a-b     IMPORTANT   Calling order   c,a,b
%macro SubAndAssign 3
    mov eax, [%2]
    sub eax, [%3]
    mov [%1], eax
%endmacro

;x += a                 Calling order [x], [a]
%macro Increase 2
    push ecx
    mov eax, [%1]
    mov ecx, [%2]
    add eax, ecx
    mov [%1], eax
    pop ecx
%endmacro

; Compares [x] val with [a] val    Calling [x], [y]  in practise Calling should make no sense as it's a comparison
; but I'll keep it this way for the code to be more redable when deciding conditions after
%macro Cmpvarvar 2
push ecx
    mov eax, [%1]
    mov ecx, [%2]
    cmp eax, ecx
pop ecx
%endmacro


section .data
    centreX dd 0     
    centreY dd 0     
    width dd 0
    height dd 0
    Col dd 1        



    dltA dd 0      
    dltB dd 12      

    sy dd 0        
    sx dd 0        
    d dd 0          
    xpx dd 0       
    xmx dd 0        
    ypy dd 0        
    ymy dd 0        
    xpy dd 0        
    xmy dd 0       
    ypx dd 0        
    ymx dd 0        

section .text


global DrawCircle_a, MoveTo_a, SetColor_a, PutPixel
extern SetPixel_ref
;        imgInfo* DrawCircle(imgInfo* pImg, int radius)
DrawCircle_a:
        push ebp             ; Prolog
        mov ebp,esp

        push edx
        push ebx
        push eax

        mov edx, [ebp+8]        ; Adress of image_header to eax
        add edx, 8              ; Shifting to pointer
        sub edx, 4
        assign height, edx      ; Assign width of bmp file

        sub edx, 4
        assign width, edx

        mov ebx, [ebp+12]
        mov [sy], ebx

        assign dltA, sy      ;dltA = radius

        mov eax, [dltA]
        neg eax                ;dltA = -radius

        imul ebx, eax, 4     ;dltA = -radius*2
        imul eax, eax, 2     ;d= -radius * 4         Calculating part of "d" here in order to save operations
        add eax, 5           ;dltA = 5 -radius*2
        imul eax, eax, 4     ; dltA = (-2*radius +5)*4
        mov [dltA], eax      ; dltA assignment

        add ebx, 5         ;d= 5 - radius*4
        mov [d], ebx

        jmp loop



SetColor_a:    ;                       imgInfo* SetColor(imgInfo* pImg, int col)
        push ebp             ; Prolog
        mov ebp,esp

        mov edx, [ebp+12]   
        mov [Col], edx      

	    pop edx             ;Clearing and return
	    mov esp, ebp
	    pop ebp
	    ret


MoveTo_a:               ;                       imgInfo* MoveTo(imgInfo* pImg, int x, int y)
        push ebp                ;Prolog
        mov ebp,esp

        mov edx, [ebp+12]       ; Assigning value int x to edx
        mov ecx, [ebp+16]       ; Assigning value int y to ecx

        mov [centreX], edx      ; Assigning value x to variable centreX
        mov [centreY], ecx      ; Assigning value y to variable centreY

        pop edx                 ; Clearing and return
        pop ecx
        mov esp,ebp
        pop ebp
        ret

loop:
        ; Add part

    ;WARUNEK WYCHODZENIA Z PĘTLY
        Cmpvarvar sx, sy
        jg finish

    AddAndAssign xpx, centreX, sx    
    AddAndAssign ypy, centreY, sy    
    AddAndAssign xpy, centreX, sy   
    AddAndAssign ypx, centreY, sx    

        ; Minus part
    SubAndAssign xmx, centreX, sx  
    SubAndAssign ymy, centreY, sy  
    SubAndAssign xmy, centreX, sy   
    SubAndAssign ymx, centreY, sx   

        ; Drawing
    ;PutPixel xmx, ymy   ;PutPixel(cX-x, cY-y)
    mov esi, [xmx]
    mov edx, [ymy]
    call PutPixel
   ;PutPixel xmx, ypy   ;PutPixel(cX-x, cY+y)
    mov esi, [xmx]
    mov edx, [ypy]
    call PutPixel
   ;PutPixel xpx, ymy   ;PutPixel(cX+x, cY-y)
    mov esi, [xpx]
    mov edx, [ymy]
    call PutPixel
    ;PutPixel xpx, ypy   ;PutPixel(cX+x, cY+y)
    mov esi, [xpx]
    mov edx, [ypy]
    call PutPixel

   ;PutPixel xmy, ymx   ;PutPixel(cX-y, cY-x)
    mov esi, [xmy]
    mov edx, [ymx]
    call PutPixel
   ;PutPixel xmy, ypx   ;PutPixel(cX-y, cY+x)
    mov esi, [xmy]
    mov edx, [ypx]
    call PutPixel
   ;PutPixel xpy, ymx   ;PutPixel(cX+y, cY-x)
    mov esi, [xpy]
    mov edx, [ymx]
    call PutPixel
   ;PutPixel xpy, ypx   ;PutPixel(cX+y, cY+x)
    mov esi, [xpy]
    mov edx, [ypx]
    call PutPixel


    cmp DWORD[d],0     
    jg greater          

    
    Increase d, dltB
    inc DWORD[sx]
    add DWORD[dltA], 8
    add DWORD[dltB], 8

    jmp loop


greater:
        Increase d, dltA        ; d += dltA
        dec DWORD[sy]                  ; y--
        inc DWORD[sx]                  ; x++
        add DWORD[dltA], 16    ; dltA += 16          add DWORD[dltA], 16
        add DWORD[dltB], 8     ; dltB += 8

        jmp loop


PutPixel:
	mov edi, DWORD[ebp+8]
	
	
	

	;unsigned char *pPix = pImg->pImg + (((pImg->width + 31) >> 5) << 2) * y + (x >> 3);
	mov eax, DWORD[edi]
	add eax, 31
	shr eax, 5
	shl eax, 2
	imul eax, esi
	mov ecx, edx
	shr ecx, 3
	add eax, ecx
	add eax, DWORD[edi+8]

	;unsigned char mask = 0x80 >> (x & 0x07);
	mov ecx, edx
	mov edi, 0x80
	and ecx, 0x07
	shr edi, cl		
	mov ecx, edi

	mov edi, DWORD[ebp+8]

    ;Checking color value and painting proper pixel
	cmp DWORD[Col], 1
	je BlackPixel
	or ecx, DWORD[eax]	;*pPix |= mask;
	mov DWORD[eax], ecx
	jmp Return

BlackPixel:
	not ecx			;*pPix &= ~mask;
	and ecx, DWORD[eax]
	mov DWORD[eax], ecx

Return:
	xor eax, eax		;return 0
	ret


finish:
 ; Clearing and return

    mov DWORD[sy],0
    mov DWORD[sx],0
    
    mov DWORD[dltB], 12
    mov DWORD[d],0

    xor esi, esi
    xor edx, edx
    xor ebx, ebx
    xor eax, eax
    xor ecx, ecx
    xor edi, edi

	pop edi
	pop ecx
	pop ebx
	pop eax
	pop esi
    pop edx

	mov esp, ebp
	pop ebp
    ret
