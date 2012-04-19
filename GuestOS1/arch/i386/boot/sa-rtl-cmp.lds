SECTIONS
{
  . = 0x0;
  _init_kernel = .;

  .head : {
    head.o
  }
  . = ALIGN(4096);

  .text : {
    *(EXCLUDE_FILE(../../../tasks/*.o) .text)
  }

  .data : {
    *(EXCLUDE_FILE(../../../tasks/*.o) .data)
    *(EXCLUDE_FILE(../../../tasks/*.o) .rodata*)
  }
  . = ALIGN(4096);
  _edata = .;
  .bss : {*(EXCLUDE_FILE(../../../tasks/*.o) .bss)}
  . = ALIGN(4096);
  .pagetable : { *(.pagetable)}
  _end = .;
  _end_kernel = .;
   . = ALIGN(4096);
  
  _init_tskcontext = .;

  _start_context00 = .;
  _stack_context00 = .;
  . = . + 0x2000;
  .context00 : {
    ../../../tasks/periodic_task_1.o
  }
  _end_context00 = .;  
  . = ALIGN(4096);
  
  _start_context01 = .;  
  _stack_context01 = .;
  . = . + 0x2000;
  .context01 : {
    ../../../tasks/periodic_task_2.o
  }
  _end_context01 = .;

  _end_tskcontext = .;
  . = ALIGN(4096);
   
   _init_allocspace = .;
}

