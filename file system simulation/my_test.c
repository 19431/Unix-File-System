#include <stdio.h>
#include "fs-sim.h"

int main() {
  Fs_sim filesystem;

  mkfs(&filesystem);  
  mkdir (&filesystem,kangaroo);  
  touch(&filesystem,pear);

  mkdir (&filesystem,platypus);

  touch (&filesystem,papaya);

  touch (&filesystem,orange);

  cd (&filesystem,kangaroo);

  touch(&filesystem, kiwi);

 mkdir (&filesystem, papaya);

 mkdir (&filesystem,pear);

  touch (&filesystem,orange);

  cd (&filesystem,"..");

/* % % cd platypus */

/* % % touch apple */

/* % mkdir grape */

/* % mkdir banana */

/* % touch cherry */

/* % % cd grape */

/* % % mkdir blue */

/* % touch green */

/* % mkdir purple */

/* % touch yellow */

/* % touch white */

/* % mkdir red */

% % rm red

% % cd ..

% rm grape

% % cd ..

% rm kangaroo

% % ls
orange
papaya
pear
platypus/

% % ls platypus
apple
banana/
cherry

% % cd platypus

% ls banana


  
  
   /* ls(&filesystem, ""); */
  
  rmfs(&filesystem);
  return 0;
}
