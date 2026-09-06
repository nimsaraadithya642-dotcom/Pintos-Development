#include "threads/init.h"
#include <console.h>
#include <debug.h>
#include <inttypes.h>
#include <limits.h>
#include <random.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "devices/kbd.h"
#include "devices/input.h"
#include "devices/serial.h"
#include "devices/shutdown.h"
#include "devices/timer.h"
#include "devices/vga.h"
#include "devices/rtc.h"
#include "threads/interrupt.h"
#include "threads/io.h"
#include "threads/loader.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/thread.h"
#ifdef USERPROG
#include "userprog/process.h"
#include "userprog/exception.h"
#include "userprog/gdt.h"
#include "userprog/syscall.h"
#include "userprog/tss.h"
#else
#include "tests/threads/tests.h"
#endif
#ifdef FILESYS
#include "devices/block.h"
#include "devices/ide.h"
#include "filesys/filesys.h"
#include "filesys/fsutil.h"
#endif

/* ============== SHELL IMPLEMENTATION START ================*/
static void shell_run(void);
static void shell_read_line(char* buffer, size_t size);
static void shell_process_command (char *cmd);
static void shell_trim (char *str);
static void shell_cmd_whoami (void);
static void shell_cmd_shutdown (void);
static void shell_cmd_time (void);
static void shell_cmd_ram (void);
static void shell_cmd_thread (void);
static void shell_cmd_priority (void);
static void shell_cmd_exit (void);
static void shell_cmd_help (void);

/* Shell state */
static bool shell_running = false;

/* shell state */
static void shell_run(void){

  char cmd[128];

  /* printing the welcome banner */
  print_adios();
  printf("\nWelcome to CS2042 Interactive shell for ADIOS\n");
  printf("Type 'help' for available commands. \n\n");

  shell_running = true;

  while(shell_running){

    printf("CS2042> "); // display prompt

    shell_read_line(cmd, sizeof(cmd)); // read command form user

    shell_process_command(cmd);  // Process the command

  }

}

/* Read a line from the keyboard */
static void shell_read_line(char* buffer, size_t size){

      int c;
    size_t i = 0;
    
    while (i < size - 1) {
        c = input_getc ();
        
        if (c == '\r' || c == '\n') {
            /* Enter key - finish input */
            printf("\n");
            break;
        } else if (c == '\b' || c == 127) {
            /* Backspace or Delete */
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (c >= 32 && c < 127) {
            /* Printable character */
            buffer[i++] = c;
            printf("%c", c);
        }
        /* Ignore other control characters */
    }
    buffer[i] = '\0';

}

/* Remove leading and trailing whitespace from a string */
static void shell_trim(char* str) {

  char* start = str;
  char* end;

  /* Skip leading whitespace */
  while(*start == ' ' || *start == '\t') start++;

  if(*start == '\0') {
    str[0] = '\0';
    return;
  }

  /* Move to end of the string */
  end = start + strlen (start) -1;

  /* Skip trailing whitespace */
  while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
      end--;

      /* Copy trimmed string back */
      memmove (str, start, end - start + 1);
      str[ end - start + 1 ] = '\0';

}

/* Process and execute a command */
static void shell_process_command(char* cmd){

    char *save_ptr;
    char *command;
    char *arg;
    
    /* Trim whitespace */
    shell_trim (cmd);
    
    /* Ignore empty commands */
    if (strlen (cmd) == 0)
        return;
    
    /* Parse command and optional argument */
    command = strtok_r (cmd, " ", &save_ptr);
    arg = strtok_r (NULL, "", &save_ptr);
    
    /* Execute the command */
    if (!strcmp (command, "whoami")) {
        shell_cmd_whoami ();
    } else if (!strcmp (command, "shutdown")) {
        shell_cmd_shutdown ();
    } else if (!strcmp (command, "time")) {
        shell_cmd_time ();
    } else if (!strcmp (command, "ram")) {
        shell_cmd_ram ();
    } else if (!strcmp (command, "thread")) {
        shell_cmd_thread ();
    } else if (!strcmp (command, "priority")) {
        shell_cmd_priority ();
    } else if (!strcmp (command, "exit")) {
        shell_cmd_exit ();
    } else if (!strcmp (command, "help")) {
        shell_cmd_help ();
    } else {
        printf ("Unknown command: '%s'\n", command);
        printf ("Type 'help' for available commands.\n");
    }

}

/* Whoami : Display your name and index number */
static void shell_cmd_whoami(void){

  printf("Name: Adithya Nimsara\n");
  printf("Index Number : 240479N\n");
}

static void shell_cmd_shutdown(void){

  printf("\n⚡⚡⚡Power off.............\n");
  printf("Shutting down ......\n");
  printf("ADIOS 👋👋👋 !!!!\n");
  shutdown_power_off();
}

static void shell_cmd_time(void){

  time_t t = rtc_get_time();
  printf("Seconds since Unix epoch %" PRIu32 "\n", t);
}

/* ram - Display available RAM information */
static void
shell_cmd_ram (void)
{
    uint32_t ram_kb = init_ram_pages * PGSIZE / 1024;
    printf ("Total RAM: %" PRIu32 " KB (%" PRIu32 " pages)\n", ram_kb, init_ram_pages);
}


/* thread - Display thread statistics */
static void
shell_cmd_thread (void)
{
    thread_print_stats ();
}

/* priority - Display current thread priority */
static void
shell_cmd_priority (void)
{
    struct thread *cur = thread_current ();
    printf ("Current thread priority: %d\n", cur->priority);
}

/* exit - Exit the interactive shell */
static void
shell_cmd_exit (void)
{
    printf ("Exiting shell. Goodbye!\n");
    shell_running = false;
}

/* help - Display available commands */
static void
shell_cmd_help (void)
{
    printf ("+--------------------------------------------------+\n");
    printf ("|              Available commands:                 |\n");
    printf ("+--------------------------------------------------+\n");
    printf ("|  whoami    - Display your name and index number  |\n");
    printf ("|  shutdown  - Shutdown Pintos OS and exit QEMU    |\n");
    printf ("|  time      - Display seconds since Unix epoch    |\n");
    printf ("|  ram       - Display available RAM               |\n");
    printf ("|  thread    - Display thread statistics           |\n");
    printf ("|  priority  - Display current thread priority     |\n");
    printf ("|  exit      - Exit the interactive shell          |\n");
    printf ("|  help      - Display this help message           |\n");
    printf ("+--------------------------------------------------+\n");
}

/* ==================== SHELL IMPLEMENTATION END ==================== */


/* Page directory with kernel mappings only. */
uint32_t *init_page_dir;

#ifdef FILESYS
/* -f: Format the file system? */
static bool format_filesys;

/* -filesys, -scratch, -swap: Names of block devices to use,
   overriding the defaults. */
static const char *filesys_bdev_name;
static const char *scratch_bdev_name;
#ifdef VM
static const char *swap_bdev_name;
#endif
#endif /* FILESYS */

/* -ul: Maximum number of pages to put into palloc's user pool. */
static size_t user_page_limit = SIZE_MAX;

static void bss_init (void);
static void paging_init (void);

static char **read_command_line (void);
static char **parse_options (char **argv);
static void run_actions (char **argv);
static void usage (void);

#ifdef FILESYS
static void locate_block_devices (void);
static void locate_block_device (enum block_type, const char *name);
#endif

int pintos_init (void) NO_RETURN;

/* Pintos main entry point. */
int
pintos_init (void)
{
  char **argv;

  /* Clear BSS. */  
  bss_init ();

  /* Break command line into arguments and parse options. */
  argv = read_command_line ();
  argv = parse_options (argv);

  /* Initialize ourselves as a thread so we can use locks,
     then enable console locking. */
  thread_init ();
  console_init ();  

  /* Greet user. */
  //print_adios();
  printf ("booting with %'"PRIu32" kB RAM...\n",
          init_ram_pages * PGSIZE / 1024);

  /* Initialize memory system. */
  palloc_init (user_page_limit);
  malloc_init ();
  paging_init ();

  /* Segmentation. */
#ifdef USERPROG
  tss_init ();
  gdt_init ();
#endif

  /* Initialize interrupt handlers. */
  intr_init ();
  timer_init ();
  kbd_init ();
  input_init ();
#ifdef USERPROG
  exception_init ();
  syscall_init ();
#endif

  /* Start thread scheduler and enable interrupts. */
  thread_start ();
  serial_init_queue ();
  timer_calibrate ();

#ifdef FILESYS
  /* Initialize file system. */
  ide_init ();
  locate_block_devices ();
  filesys_init (format_filesys);
#endif

  printf ("Boot complete.\n");
  
  if (*argv != NULL) {
    /* Run actions specified on kernel command line. */
    
    run_actions (argv);
  } else {
    // TODO: no command line passed to kernel. Run interactively 
    shell_run();
  }

  /* Finish up. */
  shutdown ();
  thread_exit ();
}

/* Clear the "BSS", a segment that should be initialized to
   zeros.  It isn't actually stored on disk or zeroed by the
   kernel loader, so we have to zero it ourselves.

   The start and end of the BSS segment is recorded by the
   linker as _start_bss and _end_bss.  See kernel.lds. */
static void
bss_init (void) 
{
  extern char _start_bss, _end_bss;
  memset (&_start_bss, 0, &_end_bss - &_start_bss);
}

/* Populates the base page directory and page table with the
   kernel virtual mapping, and then sets up the CPU to use the
   new page directory.  Points init_page_dir to the page
   directory it creates. */
static void
paging_init (void)
{
  uint32_t *pd, *pt;
  size_t page;
  extern char _start, _end_kernel_text;

  pd = init_page_dir = palloc_get_page (PAL_ASSERT | PAL_ZERO);
  pt = NULL;
  for (page = 0; page < init_ram_pages; page++)
    {
      uintptr_t paddr = page * PGSIZE;
      char *vaddr = ptov (paddr);
      size_t pde_idx = pd_no (vaddr);
      size_t pte_idx = pt_no (vaddr);
      bool in_kernel_text = &_start <= vaddr && vaddr < &_end_kernel_text;

      if (pd[pde_idx] == 0)
        {
          pt = palloc_get_page (PAL_ASSERT | PAL_ZERO);
          pd[pde_idx] = pde_create (pt);
        }

      pt[pte_idx] = pte_create_kernel (vaddr, !in_kernel_text);
    }

  /* Store the physical address of the page directory into CR3
     aka PDBR (page directory base register).  This activates our
     new page tables immediately.  See [IA32-v2a] "MOV--Move
     to/from Control Registers" and [IA32-v3a] 3.7.5 "Base Address
     of the Page Directory". */
  asm volatile ("movl %0, %%cr3" : : "r" (vtop (init_page_dir)));
}

/* Breaks the kernel command line into words and returns them as
   an argv-like array. */
static char **
read_command_line (void) 
{
  static char *argv[LOADER_ARGS_LEN / 2 + 1];
  char *p, *end;
  int argc;
  int i;

  argc = *(uint32_t *) ptov (LOADER_ARG_CNT);
  p = ptov (LOADER_ARGS);
  end = p + LOADER_ARGS_LEN;
  for (i = 0; i < argc; i++) 
    {
      if (p >= end)
        PANIC ("command line arguments overflow");

      argv[i] = p;
      p += strnlen (p, end - p) + 1;
    }
  argv[argc] = NULL;

  /* Print kernel command line. */
  printf ("Kernel command line:");
  for (i = 0; i < argc; i++)
    if (strchr (argv[i], ' ') == NULL)
      printf (" %s", argv[i]);
    else
      printf (" '%s'", argv[i]);
  printf ("\n");

  return argv;
}

/* Parses options in ARGV[]
   and returns the first non-option argument. */
static char **
parse_options (char **argv) 
{
  for (; *argv != NULL && **argv == '-'; argv++)
    {
      char *save_ptr;
      char *name = strtok_r (*argv, "=", &save_ptr);
      char *value = strtok_r (NULL, "", &save_ptr);
      
      if (!strcmp (name, "-h"))
        usage ();
      else if (!strcmp (name, "-q"))
        shutdown_configure (SHUTDOWN_POWER_OFF);
      else if (!strcmp (name, "-r"))
        shutdown_configure (SHUTDOWN_REBOOT);
#ifdef FILESYS
      else if (!strcmp (name, "-f"))
        format_filesys = true;
      else if (!strcmp (name, "-filesys"))
        filesys_bdev_name = value;
      else if (!strcmp (name, "-scratch"))
        scratch_bdev_name = value;
#ifdef VM
      else if (!strcmp (name, "-swap"))
        swap_bdev_name = value;
#endif
#endif
      else if (!strcmp (name, "-rs"))
        random_init (atoi (value));
      else if (!strcmp (name, "-mlfqs"))
        thread_mlfqs = true;
#ifdef USERPROG
      else if (!strcmp (name, "-ul"))
        user_page_limit = atoi (value);
#endif
      else
        PANIC ("unknown option `%s' (use -h for help)", name);
    }

  /* Initialize the random number generator based on the system
     time.  This has no effect if an "-rs" option was specified.

     When running under Bochs, this is not enough by itself to
     get a good seed value, because the pintos script sets the
     initial time to a predictable value, not to the local time,
     for reproducibility.  To fix this, give the "-r" option to
     the pintos script to request real-time execution. */
  random_init (rtc_get_time ());
  
  return argv;
}

/* Runs the task specified in ARGV[1]. */
static void
run_task (char **argv)
{
  const char *task = argv[1];
  
  printf ("Executing '%s':\n", task);
#ifdef USERPROG
  process_wait (process_execute (task));
#else
  run_test (task);
#endif
  printf ("Execution of '%s' complete.\n", task);
}

/* Executes all of the actions specified in ARGV[]
   up to the null pointer sentinel. */
static void
run_actions (char **argv) 
{
  /* An action. */
  struct action 
    {
      char *name;                       /* Action name. */
      int argc;                         /* # of args, including action name. */
      void (*function) (char **argv);   /* Function to execute action. */
    };

  /* Table of supported actions. */
  static const struct action actions[] = 
    {
      {"run", 2, run_task},
#ifdef FILESYS
      {"ls", 1, fsutil_ls},
      {"cat", 2, fsutil_cat},
      {"rm", 2, fsutil_rm},
      {"extract", 1, fsutil_extract},
      {"append", 2, fsutil_append},
#endif
      {NULL, 0, NULL},
    };

  while (*argv != NULL)
    {
      const struct action *a;
      int i;

      /* Find action name. */
      for (a = actions; ; a++)
        if (a->name == NULL)
          PANIC ("unknown action `%s' (use -h for help)", *argv);
        else if (!strcmp (*argv, a->name))
          break;

      /* Check for required arguments. */
      for (i = 1; i < a->argc; i++)
        if (argv[i] == NULL)
          PANIC ("action `%s' requires %d argument(s)", *argv, a->argc - 1);

      /* Invoke action and advance. */
      a->function (argv);
      argv += a->argc;
    }
  
}

/* Prints a kernel command line help message and powers off the
   machine. */
static void
usage (void)
{
  printf ("\nCommand line syntax: [OPTION...] [ACTION...]\n"
          "Options must precede actions.\n"
          "Actions are executed in the order specified.\n"
          "\nAvailable actions:\n"
#ifdef USERPROG
          "  run 'PROG [ARG...]' Run PROG and wait for it to complete.\n"
#else
          "  run TEST           Run TEST.\n"
#endif
#ifdef FILESYS
          "  ls                 List files in the root directory.\n"
          "  cat FILE           Print FILE to the console.\n"
          "  rm FILE            Delete FILE.\n"
          "Use these actions indirectly via `pintos' -g and -p options:\n"
          "  extract            Untar from scratch device into file system.\n"
          "  append FILE        Append FILE to tar file on scratch device.\n"
#endif
          "\nOptions:\n"
          "  -h                 Print this help message and power off.\n"
          "  -q                 Power off VM after actions or on panic.\n"
          "  -r                 Reboot after actions.\n"
#ifdef FILESYS
          "  -f                 Format file system device during startup.\n"
          "  -filesys=BDEV      Use BDEV for file system instead of default.\n"
          "  -scratch=BDEV      Use BDEV for scratch instead of default.\n"
#ifdef VM
          "  -swap=BDEV         Use BDEV for swap instead of default.\n"
#endif
#endif
          "  -rs=SEED           Set random number seed to SEED.\n"
          "  -mlfqs             Use multi-level feedback queue scheduler.\n"
#ifdef USERPROG
          "  -ul=COUNT          Limit user memory to COUNT pages.\n"
#endif
          );
  shutdown_power_off ();
}

#ifdef FILESYS
/* Figure out what block devices to cast in the various Pintos roles. */
static void
locate_block_devices (void)
{
  locate_block_device (BLOCK_FILESYS, filesys_bdev_name);
  locate_block_device (BLOCK_SCRATCH, scratch_bdev_name);
#ifdef VM
  locate_block_device (BLOCK_SWAP, swap_bdev_name);
#endif
}

/* Figures out what block device to use for the given ROLE: the
   block device with the given NAME, if NAME is non-null,
   otherwise the first block device in probe order of type
   ROLE. */
static void
locate_block_device (enum block_type role, const char *name)
{
  struct block *block = NULL;

  if (name != NULL)
    {
      block = block_get_by_name (name);
      if (block == NULL)
        PANIC ("No such block device \"%s\"", name);
    }
  else
    {
      for (block = block_first (); block != NULL; block = block_next (block))
        if (block_type (block) == role)
          break;
    }

  if (block != NULL)
    {
      printf ("%s: using %s\n", block_type_name (role), block_name (block));
      block_set_role (role, block);
    }
}
#endif

void print_adios() {
    printf("\n\n");
    printf("_________________________________________\n\n");
    printf("    █████  ██████   ██  ██████  ███████ \n");
    printf("   ██   ██ ██    ██ ██ ██    ██ ██      \n");
    printf("   ███████ ██    ██ ██ ██    ██ ███████ \n");
    printf("   ██   ██ ██    ██ ██ ██    ██      ██ \n");
    printf("   ██   ██ ██████   ██  ██████  ███████  V 1.0 \n");
    printf("_________________________________________\n\n");
    printf("\n");
};

