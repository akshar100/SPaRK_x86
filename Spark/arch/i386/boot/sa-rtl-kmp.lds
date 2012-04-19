SECTIONS
{
  . = 0x100000;
  _init_kernel = .;

  .head : {
    head.o
  }

  _scheduler_data_ = .;
  . = . + 0x100;
  _scheduler_data_end_ = .;
  . = ALIGN(4096);
  
  .text : {
    *(.text)
  }
  
  .data : {
    *(.data)
    *(.rodata*)
  }
  . = ALIGN(4096);
  
  .data.idt : {
    *(.data.idt)
  }
  
  _edata = .;
  
  .bss : {*(.bss)}
   
  . = ALIGN(4096);
  .pagetable : { *(.pagetable)}
  
  _end = .;
  _end_kernel = .;
  . = ALIGN(4096);
   
   _init_allocspace = .;
}

