#include "elf.h"
#include "heap.h"
#include "terminal.h"
#include <stdint.h>

// -------------------------------------------------------
// helpers — access section headers
// -------------------------------------------------------
static inline Elf32_Shdr* elf_sheader(Elf32_Ehdr* hdr) {
    return (Elf32_Shdr*)((uint32_t)hdr + hdr->e_shoff);
}

static inline Elf32_Shdr* elf_section(Elf32_Ehdr* hdr, int idx) {
    return &elf_sheader(hdr)[idx];
}

static inline char* elf_str_table(Elf32_Ehdr* hdr) {
    if (hdr->e_shstrndx == SHN_UNDEF) return 0;
    return (char*)hdr + elf_section(hdr, hdr->e_shstrndx)->sh_offset;
}

static inline char* elf_lookup_string(Elf32_Ehdr* hdr, int offset) {
    char* strtab = elf_str_table(hdr);
    if (!strtab) return 0;
    return strtab + offset;
}

// -------------------------------------------------------
// check ELF magic number
// -------------------------------------------------------
static int elf_check_file(Elf32_Ehdr* hdr) {
    if (!hdr) { terminal_writestring("ELF: null header\n"); return 0; }
    // terminal_writestring("ELF magic: ");
    // terminal_writehex(hdr->e_ident[EI_MAG0]);
    // terminal_putchar(' ');
    // terminal_writehex(hdr->e_ident[EI_MAG1]);
    // terminal_putchar(' ');
    // terminal_writehex(hdr->e_ident[EI_MAG2]);
    // terminal_putchar(' ');
    // terminal_writehex(hdr->e_ident[EI_MAG3]);
    // terminal_putchar('\n');
    if (hdr->e_ident[EI_MAG0] != ELFMAG0) { terminal_writestring("ELF: bad MAG0\n"); return 0; }
    if (hdr->e_ident[EI_MAG1] != ELFMAG1) { terminal_writestring("ELF: bad MAG1\n"); return 0; }
    if (hdr->e_ident[EI_MAG2] != ELFMAG2) { terminal_writestring("ELF: bad MAG2\n"); return 0; }
    if (hdr->e_ident[EI_MAG3] != ELFMAG3) { terminal_writestring("ELF: bad MAG3\n"); return 0; }
    return 1;
}

// -------------------------------------------------------
// check ELF is supported (32-bit, x86, little-endian)
// -------------------------------------------------------
int elf_check_supported(Elf32_Ehdr* hdr) {
    if (!elf_check_file(hdr)) return 0;
    
    // terminal_writestring("class="); terminal_writehex(hdr->e_ident[EI_CLASS]); terminal_putchar('\n');
    // terminal_writestring("data=");  terminal_writehex(hdr->e_ident[EI_DATA]);  terminal_putchar('\n');
    // terminal_writestring("mach=");  terminal_writehex(hdr->e_machine);         terminal_putchar('\n');
    // terminal_writestring("ver=");   terminal_writehex(hdr->e_ident[EI_VERSION]); terminal_putchar('\n');
    // terminal_writestring("type=");  terminal_writehex(hdr->e_type);            terminal_putchar('\n');

    if (hdr->e_ident[EI_CLASS] != ELFCLASS32)  { terminal_writestring("fail: class\n");   return 0; }
    if (hdr->e_ident[EI_DATA]  != ELFDATA2LSB) { terminal_writestring("fail: data\n");    return 0; }
    if (hdr->e_machine         != EM_386)       { terminal_writestring("fail: machine\n"); return 0; }
    if (hdr->e_ident[EI_VERSION] != EV_CURRENT) { terminal_writestring("fail: version\n"); return 0; }
    if (hdr->e_type != ET_REL && hdr->e_type != ET_EXEC) { terminal_writestring("fail: type\n"); return 0; }
    return 1;
}

// -------------------------------------------------------
// get symbol value — handles external, absolute, local
// -------------------------------------------------------
static int elf_get_symval(Elf32_Ehdr* hdr, int table, uint32_t idx) {
    if (table == SHN_UNDEF || idx == SHN_UNDEF) return 0;

    Elf32_Shdr* symtab = elf_section(hdr, table);
    uint32_t entries = symtab->sh_size / symtab->sh_entsize;

    if (idx >= entries) {
        // terminal_writestring("ELF: symbol index out of range\n");
        return ELF_RELOC_ERR;
    }

    Elf32_Sym* symbol = &((Elf32_Sym*)((uint32_t)hdr +
                           symtab->sh_offset))[idx];

    if (symbol->st_shndx == SHN_UNDEF) {
        // external symbol — not supported yet
        // terminal_writestring("ELF: undefined external symbol\n");
        if (ELF32_ST_BIND(symbol->st_info) & STB_WEAK)
            return 0;
        return ELF_RELOC_ERR;
    } else if (symbol->st_shndx == SHN_ABS) {
        return symbol->st_value;
    } else {
        Elf32_Shdr* target = elf_section(hdr, symbol->st_shndx);
        return (int)hdr + symbol->st_value + target->sh_offset;
    }
}

// -------------------------------------------------------
// stage 1 — allocate SHT_NOBITS sections (BSS)
// -------------------------------------------------------
static int elf_load_stage1(Elf32_Ehdr* hdr) {
    Elf32_Shdr* shdr = elf_sheader(hdr);

    for (uint32_t i = 0; i < hdr->e_shnum; i++) {
        Elf32_Shdr* section = &shdr[i];

        if (section->sh_type == SHT_NOBITS) {
            if (!section->sh_size) continue;
            if (section->sh_flags & SHF_ALLOC) {
                // allocate and zero memory for BSS
                void* mem = kmalloc(section->sh_size);
                if (!mem) {
                    // terminal_writestring("ELF: BSS alloc failed\n");
                    return ELF_RELOC_ERR;
                }
                // zero it
                uint8_t* p = (uint8_t*)mem;
                for (uint32_t j = 0; j < section->sh_size; j++)
                    p[j] = 0;

                // point section offset to our allocation
                section->sh_offset = (uint32_t)mem - (uint32_t)hdr;
            }
        }
    }
    return 0;
}

// -------------------------------------------------------
// apply one relocation entry
// -------------------------------------------------------
static int elf_do_reloc(Elf32_Ehdr* hdr, Elf32_Rel* rel,
                        Elf32_Shdr* reltab) {
    Elf32_Shdr* target = elf_section(hdr, reltab->sh_info);

    uint32_t addr = (uint32_t)hdr + target->sh_offset;
    int* ref = (int*)(addr + rel->r_offset);

    int symval = 0;
    if (ELF32_R_SYM(rel->r_info) != SHN_UNDEF) {
        symval = elf_get_symval(hdr, reltab->sh_link,
                                ELF32_R_SYM(rel->r_info));
        if (symval == ELF_RELOC_ERR) return ELF_RELOC_ERR;
    }

    switch (ELF32_R_TYPE(rel->r_info)) {
        case R_386_NONE:
            break;
        case R_386_32:
            *ref = symval + *ref;
            break;
        case R_386_PC32:
            *ref = symval + *ref - (int)ref;
            break;
        default:
            terminal_writestring("ELF: unsupported reloc type\n");
            return ELF_RELOC_ERR;
    }
    return symval;
}

// -------------------------------------------------------
// stage 2 — process all relocation sections
// -------------------------------------------------------
static int elf_load_stage2(Elf32_Ehdr* hdr) {
    Elf32_Shdr* shdr = elf_sheader(hdr);

    for (uint32_t i = 0; i < hdr->e_shnum; i++) {
        Elf32_Shdr* section = &shdr[i];

        if (section->sh_type == SHT_REL) {
            uint32_t entries = section->sh_size / section->sh_entsize;
            for (uint32_t j = 0; j < entries; j++) {
                Elf32_Rel* reltab = &((Elf32_Rel*)((uint32_t)hdr +
                                       section->sh_offset))[j];
                int result = elf_do_reloc(hdr, reltab, section);
                if (result == ELF_RELOC_ERR) {
                    // terminal_writestring("ELF: relocation failed\n");
                    return ELF_RELOC_ERR;
                }
            }
        }
    }
    return 0;
}

// -------------------------------------------------------
// load relocatable ELF
// -------------------------------------------------------
static void* elf_load_rel(Elf32_Ehdr* hdr) {
    if (elf_load_stage1(hdr) == ELF_RELOC_ERR) return 0;
    if (elf_load_stage2(hdr) == ELF_RELOC_ERR) return 0;

    // for ET_REL, look up module_main by name
    void* entry = elf_lookup_symbol_by_name(hdr, "module_main");
    if (!entry) {
        // terminal_writestring("ELF: module_main not found\n");
        return 0;
    }
    return entry;
}

// -------------------------------------------------------
// main entry point — load an ELF file from memory
// returns entry point address or NULL on failure
// -------------------------------------------------------
void* elf_load_file(void* file) {
    // terminal_writestring("loading ELF at ");
    // terminal_writehex((uint32_t)file);
    // terminal_writestring("\n");
    Elf32_Ehdr* hdr = (Elf32_Ehdr*)file;
    if (!elf_check_supported(hdr)) {
        // terminal_writestring("ELF: cannot load file\n");
        return 0;
    }
    switch (hdr->e_type) {
        case ET_REL:
            return elf_load_rel(hdr);
        case ET_EXEC:
            terminal_writestring("ELF: ET_EXEC not yet implemented\n");
            return 0;
    }
    return 0;
}

// look up a symbol by name in the ELF file
void* elf_lookup_symbol_by_name(Elf32_Ehdr* hdr, const char* name) {
    Elf32_Shdr* shdr = elf_sheader(hdr);

    for (uint32_t i = 0; i < hdr->e_shnum; i++) {
        Elf32_Shdr* section = &shdr[i];
        if (section->sh_type != SHT_SYMTAB) continue;

        uint32_t entries = section->sh_size / section->sh_entsize;
        Elf32_Sym* symtab = (Elf32_Sym*)((uint32_t)hdr + section->sh_offset);
        Elf32_Shdr* strtab_shdr = elf_section(hdr, section->sh_link);
        char* strtab = (char*)((uint32_t)hdr + strtab_shdr->sh_offset);

        for (uint32_t j = 0; j < entries; j++) {
            char* sym_name = strtab + symtab[j].st_name;
            // simple strcmp
            int match = 1;
            for (int k = 0; ; k++) {
                if (sym_name[k] != name[k]) { match = 0; break; }
                if (name[k] == '\0') break;
            }
            if (match && symtab[j].st_shndx != SHN_UNDEF) {
                Elf32_Shdr* sym_section = elf_section(hdr, symtab[j].st_shndx);
                return (void*)((uint32_t)hdr + sym_section->sh_offset
                               + symtab[j].st_value);
            }
        }
    }
    return 0;
}