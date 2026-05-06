/**
  * @file    startup_stm32f407xx.s
  * @brief   Cortex-M4 boot assembly skeleton.
  *
  * 真机移植时把 CubeMX 生成的 startup_stm32f407xx.s 整段抄进来即可,
  * Reset_Handler 里调用 SystemInit + main 这一段不变. 此处展示骨架.
  *
  * CPU 上电硬件做的事:
  *   1. 从 0x08000000 读 4 字节到 SP
  *   2. 从 0x08000004 读 4 字节到 PC
  *   3. 跳到 PC 指向的地址执行 (即 Reset_Handler)
  */

    .syntax unified
    .cpu cortex-m4
    .fpu softvfp
    .thumb

.global g_pfnVectors
.global Default_Handler

.word _sdata
.word _edata
.word _sbss
.word _ebss

    .section .text.Reset_Handler
    .weak  Reset_Handler
    .type  Reset_Handler, %function
Reset_Handler:
    ldr   sp, =_estack

    /* 1. copy .data section from Flash to SRAM */
    ldr   r0, =_sdata
    ldr   r1, =_edata
    ldr   r2, =_etext
    movs  r3, #0
copy_data:
    cmp   r0, r1
    bge   copy_done
    ldr   r4, [r2, r3]
    str   r4, [r0, r3]
    adds  r3, #4
    b     copy_data
copy_done:

    /* 2. clear .bss section */
    ldr   r0, =_sbss
    ldr   r1, =_ebss
    movs  r2, #0
zero_bss:
    cmp   r0, r1
    bge   zero_done
    str   r2, [r0]
    adds  r0, #4
    b     zero_bss
zero_done:

    /* 3. call SystemInit then jump to main */
    bl    SystemInit
    bl    main
    b     .

    .size  Reset_Handler, .-Reset_Handler

    .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
    b     Infinite_Loop
    .size  Default_Handler, .-Default_Handler

    .section  .isr_vector,"a",%progbits
    .type  g_pfnVectors, %object
    .size  g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word Default_Handler        /* NMI */
    .word Default_Handler        /* HardFault */
    .word Default_Handler        /* MemManage */
    .word Default_Handler        /* BusFault */
    .word Default_Handler        /* UsageFault */
    .word 0
    .word 0
    .word 0
    .word 0
    .word Default_Handler        /* SVCall */
    .word Default_Handler        /* DebugMon */
    .word 0
    .word Default_Handler        /* PendSV */
    .word Default_Handler        /* SysTick */
    /* 80+ STM32F407 peripheral interrupts. CubeMX 生成版整段抄进来. */
