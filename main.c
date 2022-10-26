#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <winuser.h>
#include <unistd.h>

typedef enum{
  false,
  true,
} bool;

struct VM{
  uint8_t *mem; 
  uint8_t V[16];
  unsigned short I;
  unsigned short PC;
  unsigned short *stack;
  uint8_t SP;
  bool screen[32*64];
};

enum ERR{
  OK,
  SEG_FAULT,
};

enum ERR run(struct VM vm);

struct VM init(){
     struct VM vm = {
    .mem = (uint8_t*)malloc(4096),
    .I = 0,
    .PC = 0,
    .stack = (unsigned short*)malloc(16*2),
  };
     return vm;
}

void destroy(struct VM vm){
  free(vm.mem);
  free(vm.stack);
}


int main(){
  struct VM vm = init();
  //Read Debug
  FILE *f = fopen("ibmlogo.ch8","r");

  fseek(f,0,SEEK_END);
  size_t size = ftell(f);
  fseek(f,0,SEEK_SET);

  uint8_t temp_array[size];

  fread(temp_array,1,size,f);
  fclose(f);
  
  // Debug end

  for(size_t x = 0; x < size; ++x)
    *(vm.mem+x) = temp_array[x];

  int error = (int)run(vm);
  printf("Error level %d\n" , error);
  destroy(vm);
  return 0;
}

//Helper functions

void debugger(struct VM vm){
  /*
  uint8_t *mem; 
  uint8_t V[16];
  unsigned short I;
  unsigned short PC;
  unsigned short *stack;
  uint8_t SP;
  bool screen[32*64];
  */

  puts("Vx: ");
  for(int i = 0; i < 8; ++i)
    printf("V%d:%d ",i,vm.V[i]);
  printf("\n");
  for(int i = 8; i < 16; ++i)
    printf("V%d:%d ",i,vm.V[i]);
  printf("\nI: %d\n",vm.I);
  printf("PC: %d\n",vm.PC);
  printf("SP: %d\n",vm.SP);
}

void print(bool *screen){
  char graph[32*64+32];
  for(int i = 0; i < 32; ++i){
    for(int j = 0; j < 64; ++j){
      if(screen[i*64+j])
        graph[i*65+j] = '@';
      else
        graph[i*65+j] = ' ';
    }
  }
  for(int i = 0; i < 32; ++i){
    screen[i*65+64] = '\n';
  }
  printf("%s",graph);
}


void cls(bool *screen){
  char graph[32*64+32];
  for(int i = 0; i < 32*65; ++i){
    screen[i] = ' ';
  }
  for(int i = 0; i < 32; ++i){
    screen[i*65+64] = '\n';
  }
  printf("%s",graph);
}

enum ERR run(struct VM vm){
  if(vm.PC%2 != 0){
    return SEG_FAULT;
  }

  debugger(vm);
  unsigned short inst = (((unsigned short)vm.mem[vm.PC++])<<8)  |  (((unsigned short)vm.mem[vm.PC++])<<0);

  for(int i = 0; i < 10; ++i)
    printf("| %d |\n", vm.mem[i]); 
  printf("%d %d \n" , (inst&0xFF00)>>8, inst&0x00FF);

  printf("inst %d\n" , inst);

  uint8_t t1 = 0, t2 = 0; 

  switch(inst & 0xF000){
    case 0:
      switch(inst){
        case 0x00E0:
          cls(vm.screen); 
          break; 
        case 0x00EE:
          if(vm.SP == 0)
            return OK;
          else
            vm.PC = vm.stack[--vm.SP];
            vm.PC--;
          break;
      };
      break;
    case 1:
      vm.PC = 0x0FFF & inst;
      vm.PC--;
      break;
    case 2:
      vm.stack[vm.SP++] = vm.PC;
      vm.PC = 0x0FFF & inst;
      vm.PC--;
      break;
    case 3:
      if ((inst & 0x00FF) == vm.V[(inst & 0x0F00)>>8])
        vm.PC++;
      break;
    case 4:
      if ((inst & 0x00FF) != vm.V[(inst & 0x0F00)>>8])
        vm.PC++;
      break;
    case 5:
      if (vm.V[(inst & 0x0F00) >> 8] == vm.V[(inst & 0x00F0) >> 4])
        vm.PC++;
      break;
    case 6:
      vm.V[(inst & 0x0F00) >> 8] = inst&0X00FF;
      break;
    case 7:
      vm.V[(inst & 0x0F00) >> 8] += inst&0X00FF;
      break;
    case 8:
      t1 = (inst&0x0F00) >> 8;
      t2 = (inst&0x00F0) >> 4;
      switch(inst&0x000F){
        case 0 :
          vm.V[t1] = vm.V[t2];
          break;
        case 1 :
          vm.V[t1] = vm.V[t1] | vm.V[t2];
          break;
        case 2 :
          vm.V[t1] = vm.V[t1] & vm.V[t2];
          break;
        case 3 :
          vm.V[t1] = vm.V[t1] ^ vm.V[t2];
          break;
        case 4 :
          if(255 - vm.V[t1] < vm.V[t2]) vm.V[15] = 1;
          else vm.V[15] = 0;
          vm.V[t1] = vm.V[t1] + vm.V[t2];
          break;
        case 5 :
          if(vm.V[t1] > vm.V[t2]) vm.V[15] = 1;
          else vm.V[15] = 0;
          vm.V[t1] = vm.V[t1] - vm.V[t2];
          break;
        case 6 :
          vm.V[15] = vm.V[t1]&1;
          vm.V[t1] >>= 1;
          break;
        case 7 :
          if(vm.V[t2] > vm.V[t1]) vm.V[15] = 1;
          else vm.V[15] = 0;
          vm.V[t1] = vm.V[t2] - vm.V[t1];
          break;
        case 0xE :
          vm.V[15] = vm.V[t1]&128;
          vm.V[t1] <<= 1;
          break;
      };
      break;

    case 9:
      t1 = (inst&0x0F00) >> 8;
      t2 = (inst&0x00F0) >> 4;
      if(vm.V[t1] != vm.V[t2])
        vm.PC++;
      break;
    case 0xA:
      vm.I = inst&0x0FFF;
      break;
    case 0xB:
      vm.PC = vm.V[0] + inst&0x0FFF;
      vm.PC--;
      break;
    case 0xC:
      t1 = (inst&0x0F00) >> 8;
      vm.V[t1] = ((uint8_t)rand()%255) & (inst & 0x00FF);
      break;
    case 0xD:
      t1 = (inst&0x0F00) >> 8;
      t2 = (inst&0x00F0) >> 4;
      //draw(t1,t2,inst&0x000F); //TODO
      break;
    case 0xE:
      switch(inst&0x000F){
        case 0x1:
          break;
        case 0xE:
          break;
      };
      break;


  };

  return OK;
}

