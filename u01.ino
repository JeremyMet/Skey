    // Clignotement d'une LED (mot anglais pour DEL) à 1 Hz par une
    // interruption en provenance du timer 2,
    // pendant que le programme principal fait
    // clignoter une autre LED (période 10s)

const byte Led = 13; // Pour utiliser la LED du module

volatile byte high_pc ;
volatile byte low_pc ;

#define LedToggle digitalWrite (Led, !digitalRead(Led))



typedef struct processus {
  byte id ;
  byte* context_addr ;
} processus ;

const processus NULL_PROCESSUS = { .id = 0, .context_addr = NULL } ; // Id 0 is reserved.


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

proc_queue* new_queue(unsigned int limit_size)
{
  proc_queue* queue = (proc_queue*) malloc(sizeof(proc_queue)) ;
  queue -> limit_size = limit_size ;
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
  unsigned int i = 0 ;
  unsigned int current_size = queue->current_size ;
  proc_node* current_node = queue->HEAD ;
  for(i=0;i<current_size;i++)
  {
    Serial.println(current_node->p.id) ;
    current_node = current_node->next ;
  }
  Serial.println("End of Queue !!") ;
}



void setup () {
  Serial.begin(9600) ;
  pinMode (Led, OUTPUT);


  // Timer clock = I/O clock / 1024
  TCCR1B = (1<<CS02)|(1<<CS00);
  // Clear overflow flag
  TIFR1  = 1<<TOV1;
  // Enable Overflow Interrupt
  TIMSK1 = 1<<TOIE1;

}


byte* save_context()
{

  byte* context_array = (byte*) malloc(sizeof(byte)*32) ;
  asm(

    "st X+, r0 \n"
    "st X+, r1 \n"
    "st X+, r2 \n"
    "st X+, r3 \n"

    "st X+, r4 \n"
    "st X+, r5 \n"
    "st X+, r6 \n"
    "st X+, r7 \n"

    "st X+, r8 \n"
    "st X+, r9 \n"
    "st X+, r10 \n"
    "st X+, r11 \n"

    "st X+, r12 \n"
    "st X+, r13 \n"
    "st X+, r14 \n"
    "st X+, r15 \n"

    "st X+, r16 \n"
    "st X+, r17 \n"
    "st X+, r18 \n"
    "st X+, r19 \n"

    "st X+, r20 \n"
    "st X+, r21 \n"
    "st X+, r22 \n"
    "st X+, r23 \n"

    "st X+, r24 \n"
    "st X+, r25 \n"
  : : "x" (context_array) : "memory");

  return context_array ;

}

void load_context(byte* context_addr)
{
  asm(
    "ld r0, X+ \n"
    "ld r1, X+ \n"
    "ld r2, X+ \n"
    "ld r3, X+ \n"

    "ld r4, X+ \n"
    "ld r5, X+ \n"
    "ld r6, X+ \n"
    "ld r7, X+ \n"

    "ld r8, X+ \n"
    "ld r9, X+ \n"
    "ld r10, X+ \n"
    "ld r11, X+ \n"

    "ld r12, X+ \n"
    "ld r13, X+ \n"
    "ld r14, X+ \n"
    "ld r15, X+ \n"

    "ld r16, X+ \n"
    "ld r17, X+ \n"
    "ld r18, X+ \n"
    "ld r19, X+ \n"

    "ld r20, X+ \n"
    "ld r21, X+ \n"
    "ld r22, X+ \n"
    "ld r23, X+ \n"

    "ld r24, X+ \n"
    "ld r25, X+ \n"

  : : "x" (context_addr) : "memory") ;

  free(context_addr) ;
}

// Routine d'interruption
ISR(TIMER1_OVF_vect)
{
  byte* context_array ; context_array = save_context() ;
  Serial.println("Vous êtes des jambons ! \n") ;
  load_context(context_array) ;
  /*asm(
    "reti \n"
  ) ;*/
  return ;
}

// Exemple 

void function_0()
{
  Serial.println("Vous êtes dans la fonction 0") ;
}

void function_1()
{
  Serial.println("Vous êtes dans la fonction 0") ;
}

void loop () {
  // Mettre ici le programme. Exemple :
  processus my_processus ;
  my_processus.id = 10 ;
  proc_queue* queue = new_queue(10) ;
  add_to_queue(queue, my_processus) ;
  print_queue(queue) ;
  int i = 0 ;
  while(true)
  {
    i++ ;
    Serial.println("coucou");
    Serial.println(i) ;
    delay(1000) ;
  }
}
