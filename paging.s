.section .text

.global load_page_directory
.type load_page_directory, @function
load_page_directory:
    mov 4(%esp), %eax       # first argument = address of page directory
    mov %eax, %cr3          # CR3 = page directory address
    ret

.global enable_paging
.type enable_paging, @function
enable_paging:
    mov %cr0, %eax
    or $0x80000000, %eax    # set bit 31 = paging enable
    mov %eax, %cr0
    ret