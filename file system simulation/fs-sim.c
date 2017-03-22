
#include <stdio.h>
#include "fs-sim.h"
#include <string.h>
#include <stdlib.h>
#define ALL 2
#define SINGLE 1

static void *create(Directory *present_dir, const char *p,int type);
static void delete_files(File *to_delete, int how_many);
static void delete_subdirectory(Directory *dir_pyramid, int how_many);
static int special_case(const char *arg);
static char helper_type(Directory *present_directory, const char *name);
static void helper_ls(Directory *present_directory);
static void helper_print(Directory *dir, Fs_sim *files);

/*function starts the filesystem by initializing root directory*/
void mkfs(Fs_sim *files){
  if(files){
    /*creates a directory and casts it from void pointer*/
    files-> root_dir= (Directory *) create(NULL, "/", 1);
    /*every malloc operation is assumed to be successful*/
    if (files-> root_dir)
      files-> curr_dir= files-> root_dir;      
  }   
}

/*function destroys file system by deleting everything in it
All dynamically allocated memory in the heap will be freed*/
void rmfs(Fs_sim *files){
  if (files){   
    if(files-> root_dir-> files){
      delete_files(files-> root_dir-> files, ALL);
    }
    /*all existing subdirectories in current directory must be deleted*/
    if (files-> root_dir){
      /*remove contents of this directory branch*/
      delete_subdirectory(files-> root_dir, ALL);      
    }
    /*when the file system is empty, the root directory is freed*/
    free(files-> root_dir-> dir_name);
    free(files-> root_dir);  
  }

  
}

/*helper function to  delete file(s) in a directory*/
static void delete_files(File *to_delete, int how_many){
  /*delete only one file here*/
  if (how_many == 1){
    free(to_delete-> file_name);
    /*setting freed varaible to NULL in order to prevent double free*/
    to_delete-> file_name= NULL;
    free(to_delete);
    to_delete= NULL;
    return;
  }
  /*delete all files here*/
  else{
    if (!to_delete){
      /*base case*/
      return;
    }
    else{
      /*recursively frees every other file pointer in root directory*/
      delete_files(to_delete-> next_file, ALL);
      free(to_delete-> file_name);
      free(to_delete);
    }
  }
}

/*helper function recursively removes subdirectories and their contents (which
may also be be another subdirectory or files) until theres nothing left*/
static void delete_subdirectory(Directory *to_be_deleted, int how_many){
  Directory *branch_holder= NULL;
  /*for rmfs, delete everything in a directory's sub directory*/
  if (how_many > SINGLE){    
      while (to_be_deleted-> sub_dir){
	/*if curr_dir has sub_dir */
	if (to_be_deleted-> sub_dir-> sub_dir){
	  delete_subdirectory(to_be_deleted-> sub_dir, ALL);	
	}
	/*remove branch at the base of dir_tree*/
	else {
	  branch_holder= to_be_deleted-> sub_dir;
	  to_be_deleted-> sub_dir= to_be_deleted-> sub_dir-> next_dir;
	  
	  /*all files in this branch must be deleted*/
	  if (branch_holder-> files){
	    delete_files(branch_holder-> files, ALL);
	  }
	  free(branch_holder-> dir_name);
	  free(branch_holder);
	}
      }
    }
 /*for rm, deletes single directory*/
    if (how_many == SINGLE){     
      
      /*single tree member with child (subdirectories)*/
      if (to_be_deleted){	
	delete_subdirectory(to_be_deleted-> sub_dir, SINGLE);
	delete_files(to_be_deleted-> files,ALL);
	free(to_be_deleted-> dir_name);
	free(to_be_deleted);
      }

      else{
	return;
      }
	
    }
}

/*function removes an existing file or subdirectory with arg name*/
int rm(Fs_sim *files, const char arg[]){
  File *file_pointer= NULL, *file_holder= NULL;
  Directory *dir_pointer= NULL, *dir_holder= NULL;
  int result= 1;
  if (files || arg){   
    if (!helper_type(files-> curr_dir, arg) || special_case(arg) ||
	(strchr(arg, '/') && strcmp(arg, "/"))){
      result= 0;
      return result;
    }
    /*pre-existing file will be deleted*/
    if (helper_type(files-> curr_dir, arg) == 'f'){
      file_pointer= files-> curr_dir-> files;
      /*file to be deleted is the head file*/
      if (!strcmp(file_pointer-> file_name, arg)){	
	file_holder= file_pointer;
	/*head points to head-> next_file*/
	files-> curr_dir-> files= file_pointer-> next_file;
	delete_files(file_holder, SINGLE);
	file_holder= NULL;
	return result;
	}
      else{
      /*check other parts of the file list*/
	while (file_pointer && file_pointer-> next_file){
	  /*next file after head file*/
	  if (!strcmp(file_pointer->next_file-> file_name, arg)){
	    file_holder= file_pointer-> next_file;
	    /*head pointer's next file becomes to_be_deleted's next file*/
	    file_pointer-> next_file= file_pointer-> next_file-> next_file;
	     /*delete just ONE file*/
	    delete_files(file_holder, SINGLE);
	    file_holder= NULL;
	    return result;
	  }
	  /*move along the list*/
	  file_pointer= file_pointer-> next_file;
	}
      }
    }
    
    /*pre-existing subdirectory and its contents will be deleted*/
    if (helper_type(files-> curr_dir, arg) == 's'){
      /*points to the head directory*/
      dir_pointer= files-> curr_dir-> sub_dir;
      /*when directory to be deleted is the head directory*/
      if (!strcmp(dir_pointer-> dir_name, arg)){
	/*keep a pointer to the first directory safe*/
	dir_holder= dir_pointer;
	/*head points to head-> next_dir*/
	files-> curr_dir-> sub_dir= dir_pointer-> next_dir;	
	/*directory with sub directories*/
	if (dir_pointer-> sub_dir){
	  delete_subdirectory(dir_holder, ALL);
	}
	/*empty directory or directory with no files*/
	if (!dir_pointer-> sub_dir){
	  delete_subdirectory(dir_holder, SINGLE);
	}
	return result;
      }
      else{
	/*when the directory to be deleted is not in the first directory*/
	while (dir_pointer && dir_pointer-> next_dir){
	  /*next directory after head directory*/
	  if (!strcmp(dir_pointer-> next_dir-> dir_name, arg)){
	    dir_holder= dir_pointer-> next_dir;
	    /*head pointer's next directory becomes to_be_deleted's next_dir*/
	    dir_pointer-> next_dir= dir_pointer-> next_dir-> next_dir;
	    /*recursively delete everything in this directory branch*/
	    
	    if (dir_pointer-> sub_dir){
	      delete_subdirectory(dir_holder, ALL);
	    }
	    if (!dir_pointer-> sub_dir){
	      delete_subdirectory(dir_holder, SINGLE);
	    }
	    return result;
	  }
	  /*move along the list until u find the directory to be removed*/
	  dir_pointer= dir_pointer-> next_dir;
	}
	return result;
      }
    }
  }
  return result;
}

/*function creates a file if arg is not found in curr_dir*/
int touch (Fs_sim *files, const char arg[]){
  /*flag variable to check for presence of special cases*/
  int result= 1;
  File *curr_file= NULL, *file_holder= NULL;
  /*flag variable to exit loops after file insertion*/
  int file_inserted= 0;
  /*if files is not null*/
  if (!files){
    result= 0;
    return result;
  }
 /*temporary file node to move pointer through entire list*/
  curr_file= files-> curr_dir-> files;
    /*if arg contains '/' somewhere but also contains other characters*/
    if (strchr(arg, '/') && strcmp(arg, "/")){
      result= 0;
    }
    /*if arg is empty string*/
    if (!strlen(arg)){
      result= 0;
    }
    /*checks if arg is not found in curr_dir and arg is not a special case
     create a file named arg in curr_dir*/
    if (helper_type(files-> curr_dir,arg) == '\0' && result &&
	!special_case(arg)){
      /*if curr_dir has no files at all,
	arg will be the only file in curr_dir */
      if (!files-> curr_dir-> files){
	/*creates a file and casts it from void pointer*/
	files-> curr_dir-> files= (File *) create(files-> curr_dir, arg, 2);
	file_inserted= 1;
      }      
      else{
	/*curr_file moves through entire list until it reaches the end
	 indicated by NULL*/
	while (curr_file && !file_inserted){
	  /*if arg < curr_file's name insert new file in front of  */
	  if (strcmp(arg,curr_file-> file_name) < 0){
	    file_holder= files-> curr_dir-> files;
	    /*creates a file and casts it from void pointer*/
	    files-> curr_dir-> files= (File *) create(files-> curr_dir, arg, 2);
	    files-> curr_dir-> files-> next_file= file_holder;
	    file_inserted= 1;
	    return result;
	  }
	 
	  /*if curr_file has no next file*/
	  if (!curr_file-> next_file){
	    /*creates a file and casts it from void pointer*/
	    curr_file-> next_file= (File *) create(files-> curr_dir, arg, 2);
	    file_inserted= 1;
	    return result;
	  }
	  /*that means it has a next file*/
	  if (strcmp(arg, curr_file-> file_name) > 0 &&
	      strcmp(arg, curr_file-> next_file-> file_name) < 0){
	      file_holder= curr_file-> next_file;
	      /*creates a file and casts it from void pointer*/
	      curr_file-> next_file= (File *) create(files-> curr_dir, arg, 2);
	      curr_file-> next_file-> next_file= file_holder;
	      file_inserted= 1;
	      return result;
	  }
	  /*move curr_file pointer to next file*/
	  curr_file= curr_file-> next_file;
	}
      }
    }
    return result;
}

/*helper function to check for special cases*/
static int special_case(const char *arg){
 /*if arg is a special case*/
 if (!strcmp(arg,".") || !strcmp(arg,"/") || !strcmp(arg,"..")
     || !strcmp(arg,"")){
    /*do nothing other indicate a special case*/
    return 1;
  }
 return 0;
}


/*function creates a sub directory in present directory*/
int mkdir(Fs_sim *files, const char arg[]){
  int result= 1;
  Directory *present_dir= NULL, *dir_holder= NULL;
  int file_inserted= 0;
  
  if (files && arg){
    /*return 0 if arg is a special case*/
    if (special_case(arg) || (strchr(arg, '/') && strcmp(arg, "/"))){
      result= 0;
      return result;
    }
    /*for directories with no previous sub directories, make a new directory
     node and insert at the beginning*/
    if (!files-> curr_dir-> sub_dir){
      /*creates a directory and casts it from void pointer*/
      files-> curr_dir-> sub_dir= (Directory *)create(files-> curr_dir, arg, 1);
      file_inserted= 1;
    }
    /*if arg is neither an existing file or subdirectory and arg is not
      special case, create a directory in curr_dir in sorted order*/
    if (!helper_type(files-> curr_dir, arg) && !special_case(arg)){
      present_dir= files-> curr_dir-> sub_dir;
      /*if we have not reached the end of the directory list*/
      while (present_dir && !file_inserted){
	/*if arg name comes before current directory's name
	 insert here*/
	  if (strcmp(arg, present_dir-> dir_name) < 0){
	    dir_holder= files-> curr_dir-> sub_dir;
	    /*creates a directory and casts it from void pointer*/
	      files-> curr_dir-> sub_dir= (Directory *)
		create(files-> curr_dir, arg, 1);
	      files-> curr_dir->sub_dir-> next_dir= dir_holder;
	      files-> curr_dir-> sub_dir-> parent_dir= files-> curr_dir;
	      file_inserted= 1;
	      return result;
	    }
	  /*if present directory is the last sub dir in current directory
	   ,name comes after previous directory so insert after curr_dirr*/
	  if (!present_dir-> next_dir){
	    /*creates a directory and casts it from void pointer*/
	    present_dir-> next_dir= (Directory *)
	      create(files-> curr_dir, arg, 1);
	    file_inserted= 1;
	    return result;
	  }
	  /*name must be in sorted order during insertion
	   condition checks both prev and next  directory*/
	  if (strcmp(arg, present_dir-> dir_name) > 0 &&
	      strcmp(arg, present_dir-> next_dir-> dir_name) < 0){
	    dir_holder= present_dir-> next_dir;
	    /*creates a directory and casts it from void pointer*/
	    present_dir-> next_dir= (Directory *)
	      create(files-> curr_dir, arg, 1);
	    present_dir-> parent_dir= files-> curr_dir;
	    present_dir-> next_dir-> next_dir= dir_holder;
	    file_inserted= 1;
	    return result;
	  }
	  present_dir= present_dir-> next_dir;
      }
    }
  }
  /*returns 1 in all other cases besides special case*/
  return result;
}

/*function changes current directory of argument*/
int cd(Fs_sim *files, const char arg[]){
  int result= 1;
  Directory *temp= NULL;
  /*make sure file not null*/
  if (files && arg){
    /*if arg not in curr_dir & arg not a special command*/
    if ((!helper_type(files-> curr_dir, arg) && !special_case(arg))){
      /*scenario represented by 0*/
      result= 0;
    }
    /*if argument is a file*/
    if (helper_type(files-> curr_dir, arg) == 'f'){
      result= 0;
      /*cant really change the directory of a file*/
      return result;
      }
    /*if arg contains "/" and something else, or arg found in curr_dir*/
    if ((strchr(arg,'/') && strcmp(arg, "/"))){
      result= 0;
    }
    /*if arg is "" or "/", change to root directory*/
      if (!strcmp(arg,"/") || !strcmp(arg,"")){
	files-> curr_dir= files-> root_dir;
    }
    /*if arg is ".." and curr_dir not root_dir, change curr_dir to root_dir*/
    if(!strcmp(arg, "..") && files-> curr_dir != files-> root_dir){
      files-> curr_dir= files-> curr_dir-> parent_dir;
    }
    /* "." command lets u stay at current directory */
    if (!strcmp(arg,".")){
      /*nothing needs to change, we are already in current directory*/
    }
    /*if arg is name of an immdediate sub directory of curr_dir,
     get to it and make it the current directory*/
    if (helper_type(files-> curr_dir, arg) == 's'){
      temp= files-> curr_dir-> sub_dir;
      while (temp){
	if (!strcmp(temp-> dir_name, arg) && temp){
	  files-> curr_dir= temp;
	}
	  temp= temp-> next_dir;
      }
    }
  }
  else{
    result= 0;
  }
  return result;
}


/*This function when called when called with no arguments displays
 files and sub directory in current directory, or in the directory
 of its argument*/
int ls(Fs_sim *files, const char arg[]){
  int result= 1;
  int printed= 0;
  int special= 0;
  Directory *subdir_pointer= NULL;
  
  /*checks if parameters are null*/
  if (files && arg){
   /*if arg refers to the name of a file in current directory, print its name*/
   if (helper_type(files-> curr_dir, arg) == 'f' ){
     printed= 1;
     printf("%s\n", arg);
   }
    /*prints everything in current directory of files when arg is "." or ""*/
   if (!strcmp(arg, ".") || !strcmp(arg,"")){
     helper_ls(files-> curr_dir);
     special++;
   }
   /*if curr_dir doesnt contain any file or directory with name arg*/
   if (!helper_type(files-> curr_dir, arg) && !special_case(arg)){
     result= 0;
     return result;
  }
  /*prints everything in the parent directory of files when arg is ".."
   also checks to see that curr_dir has a parent*/
   if (!strcmp(arg,"..") && (files-> curr_dir-> parent_dir)){
     helper_ls(files-> curr_dir-> parent_dir);
   }
  /*prints everthing in the root dir if arg is "/"*/
   if (!strcmp(arg,"/")){
     helper_ls(files-> root_dir);
   }
     /*if arg refers to the name of a sub directory in current directory*/
   if (helper_type(files-> curr_dir, arg) == 's' ){
     subdir_pointer= files-> curr_dir-> sub_dir;
     /*move pointer all the day down the list till particular
      sub_dir is found*/
     while (subdir_pointer && !printed){
       if (!strcmp(subdir_pointer-> dir_name, arg)){
	 /*print content of sub directory*/
	 helper_ls(subdir_pointer);
	 printed=1;
       }
       else{
	  subdir_pointer= subdir_pointer-> next_dir;
       }
     }
   }
  }
  /*if files is a null pointer return 0*/
  else{
    result= 0;
  }
  return result;
}
  

/*function prints path to present directory*/
void pwd(Fs_sim *files){
  /*if its the root directory*/
  if (!strcmp(files-> curr_dir-> dir_name, "/")){
    /*base case is root directory*/
    printf("/");
  }
  /*if path is not the root directory*/
  else{
    /*call helper function*/
    helper_print(files-> curr_dir, files);
  }
  printf("\n");
}

/*helper method that recursively prints each directory's name*/
static void helper_print(Directory *dir, Fs_sim *files ){
  /*stop recursive call if its the root directory*/
  if (strcmp(dir-> dir_name, "/")){
    helper_print(dir-> parent_dir, files);
    printf("/%s", dir-> dir_name);
  }
}

/*helper function to print files or (and) sub directories in a directory*/
static void helper_ls(Directory *present_directory){
  File *file_pointer=  present_directory-> files;
  Directory *dir_pointer=  present_directory-> sub_dir;
  /*if current directory has no files or sub directories*/
  if (!dir_pointer && !file_pointer){
    return;
  }
    /*if current directory has files and sub directories*/
   while (file_pointer && dir_pointer){
      if ((strcmp(file_pointer-> file_name,dir_pointer-> dir_name)) < 0){
	printf("%s\n", file_pointer-> file_name);
	file_pointer= file_pointer-> next_file;
      }
    /*since file and sub dir can't have the same name in same directory
     strcmp() cant be 0. therefore it can only be + or - */
      else{
	printf("%s/\n", dir_pointer-> dir_name);
	dir_pointer= dir_pointer-> next_dir;
      }
    }
  /*if current directory has just files only*/
  if (file_pointer && !dir_pointer){
    while (file_pointer){
      printf("%s\n", file_pointer-> file_name);
      file_pointer= file_pointer-> next_file;
    }
    return;
  }
  /*if current directory has just sub directories*/
  if (dir_pointer && !file_pointer){
    while (dir_pointer){
      printf("%s/\n", dir_pointer-> dir_name);
      dir_pointer= dir_pointer-> next_dir;
    }
  }
}
  
/*helper function to determine if name is a file or sub directory in
the current directory. returns 's' for sub directory, 'f' for file
and '\0' if that name is not found there */
static char helper_type(Directory *present_directory, const char *name){
  char type= '\0';
  Directory *dir= NULL;
  File *file= NULL;

  if (present_directory){
    dir= present_directory-> sub_dir;
    file= present_directory-> files;
    while (dir){
      if (!(strcmp(dir-> dir_name, name))){
	type= 's';
      }
      dir= dir-> next_dir;
    }
    while (file){
      if (!(strcmp(file-> file_name, name))){
	type= 'f';
      }
      file= file-> next_file;
    }
  }
  return type;
}

/*helper method to create either file or directory as needed
  returns a void pointer to be casted by calling function*/
static void *create(Directory *present_dir, const char *p, int type){
  void *made= NULL;
  /*creates a directory*/
  if (type== 1){
    Directory *new_directory= malloc(sizeof(Directory));
    /*+1 for null character*/
    new_directory-> dir_name= malloc(sizeof(char) * (strlen(p) + 1));
    strcpy((new_directory-> dir_name), p);
    /*sets every other field to null*/
    new_directory-> sub_dir= NULL;
    new_directory-> next_dir= NULL;
    new_directory-> parent_dir= present_dir;
    new_directory-> files= NULL;
    /*sets pointer to address of the new directory*/
    made= new_directory;
  }  
  /*creates a file*/
  if (type== 2){
  File *new_file= malloc(sizeof(File));
      /*+1 for null character*/
      new_file-> file_name= malloc(sizeof(char) * (strlen(p) + 1));
      strcpy((new_file-> file_name), p);
      new_file-> next_file= NULL;
      /*sets pointer to address of the new file*/
      made= new_file;    
  }
  /*returns void pointer to be converted accoringly by caller*/
  return made;
}
