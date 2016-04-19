#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
  
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

void
handle_signal()
{
  if (proc != 0)
  {
    if ((proc->tf->cs & 3) == 0)
      return;
    if(!proc->handling_signal)
    {

      if (proc->signalHandler != (sig_handler)-1)
      {
        struct cstackframe* sig_frame = pop(&(proc->pending_signals));
        if (sig_frame != 0)
        {
          proc->handling_signal = 1;

          proc->backup_tf.edi = proc->tf->edi;
          proc->backup_tf.esi = proc->tf->esi;
          proc->backup_tf.ebp = proc->tf->ebp;
          proc->backup_tf.oesp = proc->tf->oesp;
          proc->backup_tf.ebx = proc->tf->ebx;
          proc->backup_tf.edx = proc->tf->edx;
          proc->backup_tf.ecx = proc->tf->ecx;
          proc->backup_tf.eax = proc->tf->eax;
          proc->backup_tf.gs = proc->tf->gs;
          proc->backup_tf.fs = proc->tf->fs;
          proc->backup_tf.es = proc->tf->es;
          proc->backup_tf.ds = proc->tf->ds;
          proc->backup_tf.trapno = proc->tf->trapno;
          proc->backup_tf.err = proc->tf->err;
          proc->backup_tf.eip = proc->tf->eip;
          proc->backup_tf.cs = proc->tf->cs;
          proc->backup_tf.eflags = proc->tf->eflags;
          proc->backup_tf.esp = proc->tf->esp;
          proc->backup_tf.ss = proc->tf->ss;
          
       
          proc->tf->esp -= 8;
          unsigned char buf[] = {0xB8, 0x18, 0x00, 0x00, 0x00, 0xCD, 0x40, 0xC3}; //opcode for calling sigret system call
          memmove((uint*)proc->tf->esp, buf, 8 * sizeof(char));

          uint return_address = (uint)proc->tf->esp;          

          proc->tf->esp -= 4;
          *((uint*)(proc->tf->esp)) = sig_frame->value;

          proc->tf->esp -= 4;
          *((uint*)(proc->tf->esp)) = sig_frame->sender_pid;
          
          proc->tf->esp -= 4;
          *((uint*)(proc->tf->esp)) = return_address;
          

          proc->tf->eip = (uint)proc->signalHandler;
        
        }
      }
    }
  }      
}


//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpu->id == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpu->id, tf->cs, tf->eip);
    lapiceoi();
    break;
   
  //PAGEBREAK: 13
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpu->id, tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip, 
            rcr2());
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running 
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();
}
