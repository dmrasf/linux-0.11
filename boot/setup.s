INITSEG  = 0x9000	! we move boot here - out of the way
SETUPSEG = 0x9020	! this is the current segment

entry start
start:
    mov ax,#SETUPSEG
    mov es,ax

	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#21
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#msg2
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

    ! 获取参数
    mov ax,#INITSEG
    mov ds,ax

    ! 获取扩展内存值
    mov ah,#0x88
    int 0x15
    mov [0],ax

	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#19
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
    mov bp,#msg_mem
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
    mov ah,#0x02
    mov al,dl
    sub al,#6
    mov dl,al
    int 0x10

print_hex:
    mov cx,#4
    mov ah,[1]
    mov al,[0]
    mov dx,ax
print_digit:
    rol dx,#4
    mov ax,#0xe0f
    and al,dl
    add al,#0x30
    cmp al,#0x3a
    jl  outp
    add al,#0x07
outp:
    int 0x10
    loop    print_digit

l1:
	jmp l1

msg2:
	.byte 13,10
	.ascii "Now we are in SETUP"

msg_mem:
	.byte 13,10
	.ascii "MEM SIZE =     KB"

msg_n:
	.byte 13,10
