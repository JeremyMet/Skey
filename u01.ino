    // Clignotement d'une LED (mot anglais pour DEL) à 1 Hz par une
    // interruption en provenance du timer 2,
    // pendant que le programme principal fait
    // clignoter une autre LED (période 10s)

const byte Led = 13; // Pour utiliser la LED du module

volatile byte high_pc ;
volatile byte low_pc ;

#define LedToggle digitalWrite (Led, !digitalRead(Led))
#define echo(s) Serial.println(s)

byte* context_array = (byte*) malloc(sizeof(byte)*32) ;

void* next_SP = NULL ;
void* PC ;

typedef struct processus {
  byte id ;
  void (*fct_ptr)() ;
  void* proc_SP ;
  void* proc_PC ;
} processus ;

const processus NULL_PROCESSUS = { .id = 0, .fct_ptr = NULL, .proc_SP = NULL } ; // Id 0 is reserved.

typedef struct proc_node {
  processus p ;
  struct proc_node *next ;
} proc_node ;

typedef struct proc_queue {
  proc_node* HEAD ;
  proc_node* TAIL ;
  unsigned int current_size ;
  unsigned int limit_size ;
} proc_queue ;

proc_queue* new_queue(unsigned int limit_size) ;
proc_queue* SCHEDULER_QUEUE = new_queue(8) ;


/////////////

proc_queue* new_queue(unsigned int limit_size)
{
  proc_queue* queue = (proc_queue*) malloc(sizeof(proc_queue)) ;
  queue -> limit_size = limit_size ;
  queue -> current_size = 0 ;
  return queue ;
}

// Add a process "p" to the tail of the "queue".
bool add_to_queue(proc_queue* queue, processus p)
{
  bool ret = false ;
  if (queue->current_size == 0)
  {
    // Node creation
    proc_node* node = malloc(sizeof(proc_node)) ;
    node->p = p ;
    node->next = NULL ;
    // Add it.
    queue->HEAD = node ;
    queue->TAIL = node ;
    queue-> current_size++ ;
    ret = true ;
  }
  else if (queue->current_size < queue->limit_size)
  {
    // Node creation
    proc_node* node = malloc(sizeof(proc_node)) ;
    node->p = p ;
    node->next = NULL ;
    // Add it.
    queue-> TAIL -> next = node ;
    queue-> TAIL = node ;
    queue-> current_size++ ;
    ret = true ;
  }
  return ret ;
}

// Pop a processus "p" from queue "queue" (remove "p" from the head of the "queue").
processus pop_from_queue(proc_queue* queue)
{
  processus ret = NULL_PROCESSUS ;
  if (queue->current_size > 0)
  {
    proc_node* current_node = queue->HEAD ;
    ret = current_node->p ;
    queue->HEAD = current_node->next ;
    queue->current_size-- ;
    free(current_node) ;
    return ret ;
  }
  return ret ;
}

// Print queue (processus) from HEAD to QUEUE.
void print_queue(proc_queue* queue)
{
  unsigned int i ;
  unsigned int current_size = queue->current_size ;
  proc_node* current_node = queue->HEAD ;
  for(i=0;i<current_size;i++)
  {
    Serial.println(current_node->p.id) ;
    current_node = current_node->next ;
  }
  Serial.println("End of Queue !!") ;
}



void setup() {
  Serial.begin(9600) ;
  pinMode (Led, OUTPUT);
  processus my_processus_0 ;
  my_processus_0.id = 1 ;
  my_processus_0.proc_SP = NULL ;
  my_processus_0.fct_ptr = &function_0 ;
  processus my_processus_1 ;
  my_processus_1.id = 2 ;
  my_processus_1.proc_SP = NULL ;
  my_processus_1.fct_ptr = &function_1 ;
  add_to_queue(SCHEDULER_QUEUE, my_processus_0) ;
  add_to_queue(SCHEDULER_QUEUE, my_processus_1) ;
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

}



// Routine d'interruption
ISR(TIMER1_COMPA_vect)
{
  echo("Interrupt") ; echo(int(SP)) ;
  static processus current_process = NULL_PROCESSUS ;
  static void* previous_SP = NULL ;
  cli() ; // Can not be interrupted during Scheduling.
  asm(
    "push r0 \n"
    "push r1 \n"
    "push r2 \n"
    "push r3 \n"
    "push r4 \n"
    "push r5 \n"
    "push r6 \n"
    "push r7 \n"
    "push r8 \n"
    "push r9 \n"
    "push r10 \n"
    "push r11 \n"
    "push r12 \n"
    "push r13 \n"
    "push r14 \n"
    "push r15 \n"
    "push r16 \n"
    "push r17 \n"
    "push r18 \n"
    "push r19 \n"
    "push r20 \n"
    "push r21 \n"
    "push r22 \n"
    "push r23 \n"
    "push r24 \n"
    "push r25 \n"
    "push r26 \n"
    "push r27 \n"
    "push r28 \n"
    "push r29 \n"
    "push r30 \n"
    "push r31 \n");
  // Scheduling Part (Round-Robin)
  // Add current task to the bottom of the SCHEDULER_QUEUE
  if (current_process.id != 0) // NULL_PROCESSUS can not be set on the queue, except for initialization.
  {
    current_process.proc_SP = SP ;
    add_to_queue(SCHEDULER_QUEUE, current_process) ;
  }
  // Pop next processus, Load next context and then execute.
  if (SCHEDULER_QUEUE->current_size > 0)
  {
    current_process = pop_from_queue(SCHEDULER_QUEUE) ;
    previous_SP = SP+32 ;
    //echo(">>> ") ; echo(int(previous_SP)) ;
    if (current_process.proc_SP != NULL)
    {
      SP = current_process.proc_SP ;
      asm(
        "pop r31 \n"
        "pop r30 \n"
        "pop r29 \n"
        "pop r28 \n"
        "pop r27 \n"
        "pop r26 \n"
        "pop r25 \n"
        "pop r24 \n"
        "pop r23 \n"
        "pop r22 \n"
        "pop r21 \n"
        "pop r20 \n"
        "pop r19 \n"
        "pop r18 \n"
        "pop r17 \n"
        "pop r16 \n"
        "pop r15 \n"
        "pop r14 \n"
        "pop r13 \n"
        "pop r12 \n"
        "pop r11 \n"
        "pop r10 \n"
        "pop r9 \n"
        "pop r8 \n"
        "pop r7 \n"
        "pop r6 \n"
        "pop r5 \n"
        "pop r4 \n"
        "pop r3 \n"
        "pop r2 \n"
        "pop r1 \n"
        "pop r0 \n"
      ) ; // and finally execute the function.
    }
    // execute function  <-- here.
    (*current_process.fct_ptr)() ;
    SP = previous_SP ;
  }
  else
  {
    current_process = NULL_PROCESSUS ;
  }
  sei() ; // Interrupt is on again.
  return ;
}

// Exemple

void function_0()
{
  static int a = 0 ;
  a++ ;
  echo(int(PC)) ; 
  Serial.println("Vous êtes dans la fonction 0") ;
  Serial.println(a) ;
}

void function_1()
{
  static int a = 18 ;
  a++ ;
  Serial.println("Vous êtes dans la fonction 1") ;
  Serial.println(a) ;
}

void loop () {
  while(true) {}
}
