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
    }
    closedir(directory);
    return count;
  }
  return 0;
}

/* Function used by qsort to compare lines */
int compare_function(const void * a, const void * b){
  char * a_cast = *(char**)a;
  char * b_cast = *(char**)b;
  char * a_lower, * b_lower;
  a_lower = malloc(strlen(a_cast)+1);
  b_lower = malloc(strlen(b_cast)+1);
  //lowercase strings and remove any chars that are not alphanume\
  ric
    int lower_ptr = 0, orig_ptr = 0;
  for(orig_ptr; orig_ptr < strlen(a_cast); orig_ptr++){
    if(isalnum(a_cast[orig_ptr])){
      a_lower[lower_ptr] = tolower(a_cast[orig_ptr]);
      lower_ptr++;
    }
  }
  a_lower[lower_ptr] = '\0';
  lower_ptr = 0, orig_ptr = 0;
  for(orig_ptr; orig_ptr < strlen(b_cast); orig_ptr++){
    if(isalnum(b_cast[orig_ptr])){
      b_lower[lower_ptr] = tolower(b_cast[orig_ptr]);
      lower_ptr++;
    }
  }
  b_lower[lower_ptr] = '\0';
  return strcmp(a_lower, b_lower);
}

//Print all subdirectories and child files
void print_dir_list(char * name, int print_dir_name){
  if(print_dir_name){
    printf("\n");
    printf("%s:\n", name);
  }

  if(is_directory(name)){
    DIR * directory;
    directory = opendir(name);
    struct dirent *d;
    int sub_count = get_file_count(name);
    char ** sub_file_list;
    sub_file_list = malloc(sub_count*sizeof(char*));
    int count = 0;
    while((d = readdir(directory)) != NULL){
      sub_file_list[count] = malloc(strlen(d->d_name)+1);
      strcpy(sub_file_list[count], d->d_name);
      count++;
    }
    closedir(directory);
    qsort(sub_file_list, count, sizeof(char*), compare_function);
    int i;
    for(i = 0; i < count; i++){
      printf("%s\n", sub_file_list[i]);
      free(sub_file_list[i]);
    }
    free(sub_file_list);
  }
}

/*
 * mylsa() - produce the appropriate directory listing(s)
 */
void mylsa(char **roots, int arg_count) {
  if(arg_count == 0){
    print_dir_list(".", 0);
    return;
  }

  int filecount = 0, dircount = 0;
  int i;
  for(i = 0; i < arg_count; i++){
    if(is_file(roots[i])){
      filecount++;
    }else if(is_directory(roots[i])){
      dircount++;
    }
  }

  //Split between files and directories
  char ** filelist;
  char ** dirlist;

  filelist = malloc(filecount*sizeof(char*));
  dirlist = malloc(dircount*sizeof(char*));

  int filelist_ptr = 0;
  int dirlist_ptr = 0;

  for(i = 0; i < arg_count; i++){
    if(is_file(roots[i])){
      filelist[filelist_ptr] = malloc(strlen(roots[i])+1);
      strcpy(filelist[filelist_ptr], roots[i]);
      filelist_ptr++;
    }else if(is_directory(roots[i])){
      dirlist[dirlist_ptr] = malloc(strlen(roots[i])+1);
      strcpy(dirlist[dirlist_ptr], roots[i]);
      dirlist_ptr++;
    }
  }

  qsort(filelist, filelist_ptr, sizeof(char*), compare_function);
  qsort(dirlist, dirlist_ptr, sizeof(char*), compare_function);

  for(i = 0; i < filelist_ptr; i++){
    printf("%s\n", filelist[i]);
  }
  for(i = 0; i < dirlist_ptr; i++){
    print_dir_list(dirlist[i], arg_count != 1);
  }
}

/*
 * help() - Print a help message.
 */
void help(char *progname) {
    printf("Usage: %s [FILE]...\n", progname);
    printf("List information about the FILEs (the current directory by default).\n");
    printf("Do not ignore entries starting with .\n");
    printf("Behavior should mirror /bin/ls -1 -a\n");
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
	case 'h': help(argv[0]); exit(1);break;
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

    mylsa(args, file_count);

    int i = 0;
    for(i; i < file_count; i++){
      free(args[i]);
    }
    free(args);
}
