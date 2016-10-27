#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support.h"
#include "dirent.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

//Returns extended version of the file output
char * get_extended_version(char * fullname, char * shortname){
  struct stat f_stat;
  lstat(fullname,&f_stat);
  //Get file type
  char file_type;
  switch (f_stat.st_mode & S_IFMT) {
  case S_IFBLK: file_type = 'b'; break;
  case S_IFCHR: file_type = 'c'; break;
  case S_IFDIR: file_type = 'd'; break;
  case S_IFIFO: file_type = 'p'; break;
  case S_IFLNK: file_type = 'l'; break;
  case S_IFREG: file_type = '-'; break;
  case S_IFSOCK: file_type = 's'; break;
  default: file_type = '?'; break;
  }
  //Get permissions
  char permissions [20];
  char * ptr = permissions;
  *ptr = (f_stat.st_mode & S_IRUSR) ? 'r' : '-'; ptr++;
  *ptr = (f_stat.st_mode & S_IWUSR) ? 'w' : '-'; ptr++;
  *ptr = (f_stat.st_mode & S_IXUSR) ? 'x' : '-'; ptr++;
  *ptr = (f_stat.st_mode & S_IRGRP) ? 'r' : '-'; ptr++;
  *ptr = (f_stat.st_mode & S_IWGRP) ? 'w' : '-'; ptr++;
  *ptr = (f_stat.st_mode & S_IXGRP) ? 'x' : '-'; ptr++;
  *ptr = (f_stat.st_mode & S_IROTH) ? 'r' : '-'; ptr++;
  *ptr = (f_stat.st_mode & S_IWOTH) ? 'w' : '-'; ptr++;
  *ptr = (f_stat.st_mode & S_IXOTH) ? 'x' : '-'; ptr++;
  *ptr = '\0';
  //Number of hard links
  int link_num = f_stat.st_nlink;
  //User
  char * uname = getpwuid(f_stat.st_uid)->pw_name;
  //Group
  char * gname = getgrgid(f_stat.st_gid)->gr_name;
  //Size
  int size = f_stat.st_size;
  //Time/Year
  //File time
  time_t m_time = f_stat.st_mtime;
  struct tm tm = *localtime(&m_time);
  //Local time
  time_t local_time;
  struct tm localtm;
  time(&local_time);
  localtm = *localtime(&local_time);
  char time_or_year [6];
  if(localtm.tm_year == tm.tm_year){
    sprintf(time_or_year, "%02d:%02d", tm.tm_hour, tm.tm_min);
  }else{
    sprintf(time_or_year, "%4d", tm.tm_year + 1900);
  }
  //Month
  char month [4];
  switch(tm.tm_mon){
  case 0: strcpy(month,"Jan"); break;
  case 1: strcpy(month,"Feb"); break;
  case 2: strcpy(month, "Mar"); break;
  case 3: strcpy(month, "Apr"); break;
  case 4: strcpy(month, "May"); break;
  case 5: strcpy(month,"Jun"); break;
  case 6: strcpy(month,"Jul"); break;
  case 7: strcpy(month,"Aug"); break;
  case 8: strcpy(month,"Sep"); break;
  case 9: strcpy(month,"Oct"); break;
  case 10: strcpy(month,"Nov"); break;
  case 11: strcpy(month,"Dec"); break;
  }
  //Symbolic Link
  char sym_link [1024];
  sym_link[0] = '\0';
  if(file_type == 'l'){
    int size = readlink(fullname, sym_link, 1019);
    int i;
    for(i = size - 1; i >= 0; i--){
      sym_link[i+4] = sym_link[i];
    }
    sym_link[0] = ' ';
    sym_link[1] = '-';
    sym_link[2] = '>';
    sym_link[3] = ' ';
    sym_link[size+4] = '\0';
  }

  char * full_line;
  full_line = malloc(1024);
  sprintf(full_line, "%c%s. %4d %s %s %5d %s %2d %5s %s%s", file_type, permissions, link_num, uname, gname, size, month, tm.tm_mday, time_or_year, shortname, sym_link);
  return full_line;
}

//returns 1 if given string refers to a directory
int is_directory(char * name){
  DIR * dir;
  dir = opendir(name);
  if(dir != NULL){
    closedir(dir);
    return 1;
  }
  return 0;
}

//returns 1 if given string refers to a file (and not a dir)
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
	sub_file_list[count] = malloc(2048);
	strcpy(sub_file_list[count], d->d_name);
	count++;
      }
    }
    closedir(directory);
    qsort(sub_file_list, count, sizeof(char*), compare_function);
    int i;
    char short_path [1024];
    char full_path [1024];
    int total_blocks = 0;
    char ** extended_list;
    extended_list = malloc(sub_count*sizeof(char*));
    for(i = 0; i < count; i++){
      strcpy(short_path, sub_file_list[i]);
      free(sub_file_list[i]);
      sprintf(full_path, "%s/%s", name, short_path);
      extended_list[i] = get_extended_version(full_path, short_path);
      
      //Total blocks
      struct stat f_stat;
      stat(full_path,&f_stat);
      total_blocks = total_blocks + ((f_stat.st_blocks+1)/2);
    }
    free(sub_file_list);
    
    printf("total %d\n", total_blocks);
    i = 0;
    for(i; i < count; i++){
      printf("%s\n", extended_list[i]);
      free(extended_list[i]);
    }
    free(extended_list);
  }
}

/*
 * mylsl() - produce the appropriate directory listing(s)
 */
void mylsl(char **roots, int arg_count) {
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

  char * extended_line;
  for(i = 0; i < filelist_ptr; i++){
    extended_line = get_extended_version(filelist[i], filelist[i]);
    printf("%s\n", extended_line);
    free(extended_line);
    free(filelist[i]);
  }
  free(filelist);
  for(i = 0; i < dirlist_ptr; i++){
    print_dir_list(dirlist[i], arg_count != 1);
    free(dirlist[i]);
  }
  free(dirlist);
}

/*
 * help() - Print a help message.
 */
void help(char *progname) {
    printf("Usage: %s [FILE]...\n", progname);
    printf("List information about the FILEs (the current directory by default).\n");
    printf("Use a long listing format\n");
    printf("Behavior should mirror /bin/ls -1 -l\n");
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

    char ** args;
    int file_count = argc - 1;

    args = malloc((file_count)*sizeof(char*));
    int offset = optind;
    for(optind; optind < argc; optind++){
      args[optind - offset] = malloc(strlen(argv[optind]+1));
      strcpy(args[optind-offset], argv[optind]);
      //printf("%s\n", args[optind-offset]);
    }
    
    mylsl(args, file_count);

    int i = 0;
    for(i; i < file_count; i++){
      free(args[i]);
    }
    free(args);
}
