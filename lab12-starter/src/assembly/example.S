.section .text
.globl draw,Press
.type draw, @function
.type Press, @function
# 字符串数据
.section .rodata
welcome_str:
    .string "Welcome"
loading_str:
    .string "Press"
SW2:
    .string "SW2"

draw:
    
    # 绘制矩形
    li a0, 0      # x坐标
    li a1, 0       # y坐标
    li a2, 80      # 宽度
    li a3, 160       # 高度
    li a4, 0    # 颜色(白色)
    addi sp, sp, -4
    sw ra, 0(sp)
    jal LCD_Fill
    lw ra, 0(sp)
    addi sp, sp, 4

    # 绘制字符串
    li a0, 10      # x坐标
    li a1, 20       # y坐标
    la a2, welcome_str    # 字符串地址
    li a3, 65535        # 颜色
    addi sp, sp, -4
    sw ra, 0(sp)
    jal LCD_ShowString
    lw ra, 0(sp)
    addi sp, sp, 4

    # 绘制字符串
    li a0, 15      # x坐标
    li a1, 80       # y坐标
    la a2, loading_str    # 字符串地址
    li a3, 65535        # 颜色
    addi sp, sp, -4
    sw ra, 0(sp)
    jal LCD_ShowString
    lw ra, 0(sp)
    addi sp, sp, 4


    li a0, 20      # x坐标
    li a1, 100       # y坐标
    la a2, SW2    # 字符串地址
    li a3, 65535        # 颜色
    addi sp, sp, -4
    sw ra, 0(sp)
    jal LCD_ShowString
    lw ra, 0(sp)
    addi sp, sp, 4
    ret

Press:
    mv t1, a0
    li a1, 1
    beq t1, a1, Start
    li a0, 0
    ret
Start:
    li a0, 1
    ret




    