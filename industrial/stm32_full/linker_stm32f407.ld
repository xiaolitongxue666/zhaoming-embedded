/**
  * @file    linker_stm32f407.ld
  * @brief   STM32F407 linker script with 8 moduleExport sections.
  *
  * 基于 CubeMX 生成的 STM32F407VGTx 链接脚本扩写. 关键差异:
  *   1. 在 .text 后嵌入 8 个 moduleExport 段 (initcall 7 级 + UNIT_TEST 1 级)
  *   2. KEEP(.moduleExportN) 防 linker 误剪裁
  *   3. moduleExportN_start / moduleExportN_end 符号导出, 给
  *      platform_module_export.c 用
  *
  * 完整 CubeMX 生成的脚本里还有 _Min_Heap_Size / _Min_Stack_Size /
  * .preinit_array / .init_array / .ARM.exidx 等段, 真机移植时把
  * CubeMX 生成版整段抄进来, 在 .text 段后插入 moduleExport1..8 那一段
  * 即可.
  */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    CCMRAM (rw) : ORIGIN = 0x10000000, LENGTH = 64K
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)

        . = ALIGN(4);
        moduleExport1_start = .;
        KEEP(*(moduleExport1))
        moduleExport1_end = .;

        . = ALIGN(4);
        moduleExport2_start = .;
        KEEP(*(moduleExport2))
        moduleExport2_end = .;

        . = ALIGN(4);
        moduleExport3_start = .;
        KEEP(*(moduleExport3))
        moduleExport3_end = .;

        . = ALIGN(4);
        moduleExport4_start = .;
        KEEP(*(moduleExport4))
        moduleExport4_end = .;

        . = ALIGN(4);
        moduleExport5_start = .;
        KEEP(*(moduleExport5))
        moduleExport5_end = .;

        . = ALIGN(4);
        moduleExport6_start = .;
        KEEP(*(moduleExport6))
        moduleExport6_end = .;

        . = ALIGN(4);
        moduleExport7_start = .;
        KEEP(*(moduleExport7))
        moduleExport7_end = .;

        . = ALIGN(4);
        moduleExport8_start = .;
        KEEP(*(moduleExport8))
        moduleExport8_end = .;

        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .data : AT(_etext)
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM
}
