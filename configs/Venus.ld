ENTRY(_start);

SECTIONS
{
    _start = 0x90000000;
    . = 0x90000000;
    .data : {
        *(.data)
    }
}
