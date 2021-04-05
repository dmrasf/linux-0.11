SETUPLEN = 1				! nr of setup-sectors
BOOTSEG  = 0x07c0			! original address of boot-sector
INITSEG  = 0x9000			! we move boot here - out of the way
SETUPSEG = 0x9020			! setup starts here

! entry 表示程序入口
entry _start
_start:
    ! ah = 03 读取光标位置
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#23
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
    ! es:bp 字符串地址
	mov	bp,#msg1
    mov ax,#BOOTSEG
    mov es,ax
    ! ah=13 显示字符串 al=01 光标停在字符串结尾处
	mov	ax,#0x1301		! write string, move cursor
    ! 显示中断
	int	0x10

    mov ax,#INITSEG
    mov es,ax

! 读取setup代码到0x90200处
load_setup:
	mov	dx,#0x0000		! drive 0, head 0
	mov	cx,#0x0002		! sector 2, track 0
	mov	bx,#0x0200		! address = 512, in INITSEG
	mov	ax,#0x0200+SETUPLEN	! service 2, nr of sectors
    ! bios 读磁盘扇区中断
    ! al:读取的扇区数量
    ! ch:柱面号=0 cl:开始扇区=2
    ! dh:磁头号=0 dl:驱动器=0
    ! es:bx:内存地址 读setup到0x90200 512(10)=200(16)
	int	0x13			! read it
    ! CF=0 成功
	jnc	ok_load_setup		! ok - continue
	mov	dx,#0x0000
	mov	ax,#0x0000		! reset the diskette
	int	0x13
	j	load_setup

ok_load_setup:
	jmpi	0,SETUPSEG

msg1:
    ! 13:\r 移动到行首  10:\n 换行
	.byte 13,10
	.ascii "dmr dmr dmr dmr"
	.byte 13,10,13,10

.org 510
! 引导标志，必须在最后两个字节
boot_flag:
	.word 0xAA55
