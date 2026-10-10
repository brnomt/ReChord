/*
 * fixture_module.c — build fixture for run_tests.sh.
 *
 * Linked to an ELF with arm-none-eabi-gcc and packed via
 * module_builder.py --elf to prove the builder's ELF path (section
 * extraction, e_entry -> entry_offset, NOBITS -> bss). Never executed.
 */
volatile unsigned int rmf_fixture_counter; /* forces a NOBITS (.bss) section */

int rmf_fixture_entry(void);

int rmf_fixture_entry(void)
{
    rmf_fixture_counter = 1u;
    return (int)rmf_fixture_counter;
}
