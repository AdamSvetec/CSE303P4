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
  if (file = fopen(name, "r")){
    fclose(file);
    return 1;
  }
  return 0;
}

/*
 * grep_file() - display all lines of filename that contain searchstr
 */
void grep_file(char *searchstr, char *filename) {
  if(searchstr == NULL){
    printf("Error: search string needed\n");
    exit(1);
  }

  if(filename == NULL){
    return;
  }

  FILE * fp;
  fp = fopen(filename, "r");
  if(fp == NULL){
    return;
  }

  char line [2048];
  while(fgets(line, 1024, fp) != NULL){
    if(strstr(line, searchstr) != NULL){
      printf("%s\n", filename);
      fclose(fp);
      return;
    }
  }
  fclose(fp);
}

/*
 * myrgrep() - find files (recursively) with matching pattern
 */
void myrgrep(char *pattern, char *file) {
  if(is_directory(file)){
    DIR * directory;
    directory = opendir(file);
    struct dirent *d;
    char fullpath [1024];
    while((d = readdir(directory)) != NULL){
      if(d->d_name[0] != '.'){
	sprintf(fullpath,"%s/%s",file,d->d_name);
	myrgrep(pattern, fullpath);
      }
    }
    closedir(directory);
  }else{
    grep_file(pattern, file);
  }
}

/*
 * help() - Print a help message.
 */
void help(char *progname) {
    printf("Usage: %s [OPTIONS] [PATTERN] [FILE]...\n", progname);
    printf("List the names of the FILEs (the current directory by default)\n");
    printf("whose contents contain PATTERN.\n");
    printf("The search should be recursive into subdirectories of any FILE\n");
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
    /* TODO: make sure to handle any other arguments specified by the */
    /*       assignment */
    while ((opt = getopt(argc, argv, "h")) != -1) {
      switch(opt) {
      case 'h': help(argv[0]); exit(1); break;
      }
    }

    if(argc != 3){
      help(argv[0]);
    }
    
    char * pattern;
    pattern = argv[1];
    char * filename;
    filename = argv[2];
    
    myrgrep(pattern, filename);
}
