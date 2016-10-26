#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support.h"
#include "dirent.h"

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

/* Function used by qsort to compare lines */
int compare_function(const void * a, const void * b){
  char * a_cast = *(char**)a;
  char * b_cast = *(char**)b;
  char * a_lower, * b_lower;
  a_lower = malloc(strlen(a_cast)+1);
  b_lower = malloc(strlen(b_cast)+1);
  //lowercase strings and remove any chars that are not alphanumeric
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
      //only count file if it isn't hidden or current/parent dir
      if(d->d_name[0] != '.'){
	sub_file_list[count] = malloc(strlen(d->d_name)+1);
	strcpy(sub_file_list[count], d->d_name);
	count++;
      }
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

/*Recurse through directory and print all names and recurse through any subdirectories*/
void recursive_print(char * name){
  print_dir_list(name, 1);
  DIR * directory;
  directory = opendir(name);
  struct dirent *d;
  int sub_count = get_file_count(name);
  char ** sub_file_list;
  sub_file_list = malloc(sub_count*sizeof(char*));
  int count = 0;
  while((d = readdir(directory)) != NULL){
    //only count file if it isn't hidden or current/parent dir
    if(d->d_name[0] != '.'){
      sub_file_list[count] = malloc(strlen(d->d_name)+1);
      strcpy(sub_file_list[count], d->d_name);
      count++;
    }
  }
  closedir(directory);
  qsort(sub_file_list, count, sizeof(char*), compare_function);
  int i;
  char full_dir [1024];
  //append current directory to new to check if it is a directory
  for(i = 0; i < count; i++){
    sprintf(full_dir, "%s/%s", name, sub_file_list[i]);
    if(is_directory(full_dir)){
      recursive_print(full_dir);
    }
    free(sub_file_list[i]);
  }
  free(sub_file_list);
}

/*
 * mylsr() - produce the appropriate directory listing(s)
 */
void mylsr(char *root) {
  if(root == NULL){
    char * cur = malloc(2);
    cur[0] = '.';
    cur[1] = '\0';
    recursive_print(cur);
    free(cur);
  }else if(is_file(root)){
    printf("%s\n", root);
  }else{
    recursive_print(root);
  }
}

/*
 * help() - Print a help message.
 */
void help(char *progname) {
    printf("Usage: %s [FILE]...\n", progname);
    printf("List information about the FILEs (the current directory by default).\n");
    printf("List subdirectories recursively\n");
    printf("Behavior should mirror /bin/ls -1 -r\n");
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
          case 'h': help(argv[0]); break;
        }
    }

    char * arg = NULL;
    if(argc > 1){
      arg = argv[1];
    }
    mylsr(arg);
}
