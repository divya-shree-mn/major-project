#include <stdio.h>

int main() {

    int x = 20;
    
    for (int i=0; i<5; i++) {
      x = x + 2;      
      for(int j=0; j<2; j++) {
        x = x - 5;
        for(int k=0; k<2; k++) {
        	x = x + 4;
      	}
      }      
    }
    
    return 0;
}
