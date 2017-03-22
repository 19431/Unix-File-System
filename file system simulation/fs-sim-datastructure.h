/*The designed used follows an hierachial tree (pyramid) structure*/
/*The filesystem is at the top of the pyramid (tree) home to the  */
/*all powerful root directory */

/*A singly linked list data structure stores each file and a 
pointer to the next file in the same directory*/

#if !defined(HEADER2)
#define HEADER2

typedef struct file{
  char *file_name; 
  struct file *next_file;
}File;

/*data structure to keep track of directories. 
A directory can have a sub-directory and next directory */
typedef struct directory{
  char *dir_name;
  struct directory *parent_dir;
  struct directory *sub_dir;
  struct directory *next_dir;
  File *files;
} Directory; 

/*top of tree structure*/
typedef struct fs_sim{
  Directory *root_dir;
  Directory *curr_dir;
}Fs_sim;

#endif
