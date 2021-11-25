/*---------------------------------------------------------
file: <<pi.c>>
  by: Joseph Morelli
 org: COP 4610, Summer 2021
 for: <<receives number of iterations and processes the user
        desires, via the command line, then computes an 
        approximation of pi based on these variables.>>
---------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void computePartialSum(int p2c[], int c2p[]); // function definition
void main(int argc,char* argv[]) {
  if(argc > 3 ) { // checks count if arguments entered is too hi
    printf("Too many arguments supplied.\n");
    exit(1);
  }
  else if (argc < 3){  // checks count if arguments entered is too low
    printf("3 arguments expected.\n");
    exit(1);
  }

  int N = atoi(argv[1]); // initializes var N to 1st argument pass
  int T = atoi(argv[2]); // initializes var T to 2nd argument pass
  int p2c[T][2]; // creates T # of arrays for parent-to-child pipes
  int c2p[T][2]; // creates T # of arrays for child-to-parent pipes

  if(T > N){ // checks if T is greater than N
    printf("N should be greater than T.\n\n");
    exit(1);
  } else if ((N < 1) || (T < 1)){ // check if N and T > 0
    printf("Not valid variable entries. Please try again.");
    exit(1);
  }

  for(int j = 0; j < T; j++){ // iterates for number of processes (T)
    int r = pipe(p2c[j]); // initializes parent-to-child pipe
    if(r < 0){ // checks if pipe was unsuccessfully created
      printf(stderr, "pipe failed\n\n"); // error output
      exit(3);
    }
    r = pipe(c2p[j]); // initializes child-to-parent pipe
    if(r < 0){ // checks if pipe was unsuccessfully created
      printf(stderr, "pipe failed\n\n"); // error output
      exit(3);
    }
    int pid = fork(); // creates fork for each iteration
    if(pid > 0){ // checks if fork is the parent
      close(p2c[j][0]); // close read end of parent-to-child pipe
      write (p2c[j][1], &N, sizeof(N)); // write value for N to pipe
      write (p2c[j][1], &T, sizeof(T)); // write value for T to pipe
      write (p2c[j][1], &j, sizeof(j)); // write value for j to pipe
      close(p2c[j][1]); // close write end of parent-to-child pipe
    }  else if (pid == 0) { // if fork is child
      computePartialSum(p2c[j], c2p[j]); // call computation function
      exit(0);
    } else { // fork failed
      printf("Fork failed. Please try again."); // error output
	  exit(1);
    }
  }

  double accumulator = 3.0; // initialize accumulator to 3.0
  for(int j = 0; j < T; j++){ // iterates for number of processes (T)
    close(c2p[j][1]);  // close write end of parent-to-child pipe
    double partialSum; // initalize partialSum
    read(c2p[j][0], &partialSum, sizeof(partialSum)); // read the partial sums from child-to-parent pipe
    accumulator += partialSum; // increment accumulator by read value
    close(c2p[j][0]); // close read end of child-to-parent pipe 
  } 

  printf("The approximation of pi with N=%d and T=%d processes is %.16f.\n", N, T, accumulator);  // prints N, T, and accumulator values
  
  for(int k = 0; k < T; k++){
    wait(NULL); // wait for all processes to end
  }
  exit(0);
}

void computePartialSum(int p2c[], int c2p[]){
  double k = 1.0; // sign switch for the next operation
  double pi_approx = 0.0; // initialize var for child’s approximation
  int numIterations, numProcess, numPipe; // initializes vars passed by parent-to-child pipe

  close(c2p[0]); // closes read end of child-to-parent pipe
  read(p2c[0], &numIterations, sizeof(numIterations)); // read # of iterations passed from parent-to-child pipe
  read(p2c[0], &numProcess, sizeof(numProcess)); // read # of process passed from parent-to-child pipe
  read(p2c[0], &numPipe, sizeof(numPipe)); // read which pipe these values belong from parent-to-child pipe
  close(p2c[0]); // closes read end of parent-to-child pipe

  double i = numIterations * numPipe / numProcess + 1; // loop start value
  int range = (numIterations/numProcess)*(numPipe+1); // number of iterations 

  for(i; i < range; i += 1){ // iterates i through calculated range
    pi_approx += (k * 4.0) / ((2.0 * i) * (2.0 * i + 1.0) * (2.0 * i + 2.0)); // increments approximation 
    k = -k; // sign switch
  } 
  write (c2p[1], &pi_approx, sizeof(pi_approx)); // write approximation to child-to-parent pipe
}