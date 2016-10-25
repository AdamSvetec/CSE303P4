#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support.h"
#include <dirent.h>


//returns if given string refers to a directory
int is_directory(char * name){
  DIR * dir;
  dir = opendir(name);
  if(dir != NULL){
    closedir(dir);
    return 1;
  }
  return 0;
}

//returns if given string refers to a file (and not a dir)
int is_file(char * name){
  if(is_directory(name)){
    return 0;
  }
  FILE *file;
  if (file = fopen(name, "r"))
    {
      fclose(file);
      return 1;
    }
  return 0;
}

//Gets the file count of a given string
//-if a directory will count files in it (including directories)
//-if a file, will return 1
//-if does not exist, return 0
int get_file_count(char * name){
  if(is_file(name)){
    return 1;
  }else if(is_directory(name)){
    int count = 0;
    DIR * directory;
    directory = opendir(name);
    struct dirent *d;
    while((d = readdir(directory)) != NULL){
      count++;
      //only count file if it isn't hidden or current/parent dir
      if(d->d_name[0] == '.'){
	count--;
      }
    }
    closedir(directory);
    return count;
  }
  return 0;
}

//add all necessary files to the list
void add_to_file_list(char * name, char ** file_list, int * list_ptr){
  if(is_file(name)){
    file_list[*list_ptr] = malloc(strlen(name)+1);
    strcpy(file_list[*list_ptr], name);
    (*list_ptr)++;
  }else if(is_directory(name)){
    int count = 0;
    DIR * directory;
    directory = opendir(name);
    struct dirent *d;
    char fullpath [1024];
    //TODO: need to sort this specific output
    while((d = readdir(directory)) != NULL){
      //only count file if it isn't hidden or current/parent dir
      if(d->d_name[0] != '.'){
	fullpath[0] = '\0';
	strcat(fullpath, name);
	strcat(fullpath, "/");
	strcat(fullpath, d->d_name);
	//if it is a file add as usual
	if(is_file(fullpath)){
	  file_list[*list_ptr] = malloc(strlen(d->d_name)+1);
	  strcpy(file_list[*list_ptr], d->d_name);
	  (*list_ptr)++;
	//if it is a directory add trailing /
	}else if(is_directory(fullpath)){
	  file_list[*list_ptr] = malloc(strlen(d->d_name)+2);
	  strcpy(file_list[*list_ptr], d->d_name);
	  file_list[*list_ptr][strlen(d->d_name)] = '/';
          file_list[*list_ptr][strlen(d->d_name)+1] = '\0';
	  (*list_ptr)++;
	}
      }
    }
    closedir(directory);
  }
}

/* Function used by qsort to compare lines */
int compare_function(const void * a, const void * b){
  //TODO: need to make this case neutral
  return strcmp(*(char**)a, *(char**)b);
}

/*
 * myls() - produce the appropriate directory listing(s)
 */
void myls(char **roots, int arg_count) {
  //Get count of files to be displayed
  int filecount = 0;
  if(arg_count == 0){
    filecount = get_file_count(".");
  }else{
    int i;
    for(i = 0; i < arg_count; i++){
      filecount = filecount + get_file_count(roots[i]);
      if(is_directory(roots[i])){
	filecount = filecount + 2; //add two lines for directory definition
      }
    }
  }

  //TODO: need to do some presorting before collecting names
  
  //Create list of filenames and add all filenames
  char ** file_list; 
  file_list = malloc(sizeof(char*)*filecount);
  int file_list_ptr = 0;
  if(arg_count == 0){
    add_to_file_list(".", file_list, &file_list_ptr);
  }else{
    int i;
    for(i = 0; i < arg_count; i++){
      //Add a header for the folder name
      if(is_directory(roots[i]) && arg_count != 1){
	file_list[file_list_ptr] = malloc(3*sizeof(char));
	strcpy(file_list[file_list_ptr], " ");
	file_list_ptr++;
	file_list[file_list_ptr] = malloc(strlen(roots[i])+2);
	strcpy(file_list[file_list_ptr], roots[i]);
	file_list[file_list_ptr][strlen(roots[i])] = ':';
	file_list[file_list_ptr][strlen(roots[i])+1] = '\0';
	file_list_ptr++;
	}
      add_to_file_list(roots[i], file_list, &file_list_ptr);
    }
  }

  //Sort filenames
  //qsort(file_list, file_list_ptr, sizeof(char*), compare_function);
  
  int i;
  for(i = 0; i < file_list_ptr; i++){
    printf("%s\n", file_list[i]);
  }
}

/*
 * help() - Print a help message.
 */
void help(char *progname) {
    printf("Usage: %s [FILE]...\n", progname);
    printf("List information about the FILEs (the current directory by default).\n");
    printf("Behavior should mirror /bin/ls -1\n");
}

/*
 * main() - The main routine parses arguments and dispatches to the
 *          task-specific code.
 */
int main(int argc, char **argv) {
    /* for getopt */
    long opt;
    
    /* run a student name check */
    check_student(argv[0]);

    
    /* parse the command-line options.  For this program, we only support  */
    /* the parameterless 'h' option, for getting help on program usage. */
    /* TODO: make sure to handle any other arguments specified by the assignment */
    while ((opt = getopt(argc, argv, "h")) != -1) {
      switch(opt) {
      case 'h': help(argv[0]); exit(1); break;
      }
    }

    char ** args;
    int file_count = argc - 1;

    args = malloc((file_count)*sizeof(char*));
    int offset = optind;
    for(optind; optind < argc; optind++){
      args[optind - offset] = malloc(strlen(argv[optind]+1));
      strcpy(args[optind-offset], argv[optind]);
      //printf("%s\n", args[optind-offset]);
    }
    
    myls(args, file_count);

    int i = 0;
    for(i; i < file_count; i++){
      free(args[i]);
    }
    free(args);
}
