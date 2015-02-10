/** cp318.c
 *     Contians both homework 2.11 and 2.15 and 3.13
 *	   by Jacqueline Lee
 *     version 1 of cp - uses read and write with tunable buffer size
 *
 *     usage: cp318 src dest
 *	      cp318 -R src destDir
 *	      cp318 -i srcFile dest		
 *		where -R is recursive, -i for overwriting files 
 */

#include 	<stdlib.h>
#include    <stdio.h>
#include 	<sys/stat.h>
#include    <unistd.h>
#include    <fcntl.h>
#include 	<string.h>
#include 	<stdbool.h>
#include	<dirent.h>

#define BUFFERSIZE      4096
#define COPYMODE        0644
#define NAMEBUF			500

void oops(char *, char *);
char* makeFileDest(char *,char *);
char* makeDest(char *);
int fileCp(char *, char *, bool);
int cpRecurse(char *,char *, bool);

int main(int ac, char *av[])
{
    int     out_fd;
	char 	dest[NAMEBUF];
	bool 	overwrite = true;		//overwrite flag initialized to true
	struct stat info;				

	
	/* check args 	*/
    if ( ac < 3 ){
        fprintf( stderr, "usage: %s source destination\n", *av);
        exit(1);
    }

	if((strcmp(av[1], "-R") == 0)){ 	/*if option -R used, recurses*/
			
		//dest must be directory	
		//source may be dir or file
		if (stat(av[2], &info) == -1) oops("stat error","");
		if( S_ISREG(info.st_mode)) {   
			//src is a regular file, use fileCp, with file path as dest
			strcpy(dest, makeFileDest(av[2], av[3]));
			fileCp(av[2], dest, overwrite);
		}
		if( S_ISDIR(info.st_mode)){	
		 
			strcpy(dest, makeDest(av[3]));  //makeDest() will test to see if it is a directory
			cpRecurse(av[2], dest, overwrite); //files with same name in dest dir will be overwritten
		}
	}	
	else if( strcmp(av[1], "-i") == 0){ 	/*if -i option is used */
						/*only used if overwriting file in working dir*/
				
		if( strcmp(av[2], av[3]) == 0)  /* if filenames are the same, error */
			oops("File names are the same", dest);
		
		strcpy(dest, makeFileDest(av[2], av[3]) );
		
		char overw[10];
		if( (out_fd=open(dest, O_RDONLY)) ){  /* if file exist(open test for file), prompt user */
		        printf("\noverwrite exisiting file: %s ?", dest);
		        scanf("%s", overw);
			while(strcmp(overw,"y") != 0  && strcmp(overw, "n") != 0){
			      printf("\nmust be y or n:");			         
			      scanf("%s", overw);
			}
		      
			if(strcmp(overw, "n") == 0)  //if user says no exit
			        overwrite = false; //fileCp() will not overwrite
			else			// else, yes, set overwrite to true
				overwrite = true; //fileCp() will overwrite
			fileCp(av[2], dest, overwrite);
			
		}
		else   /* else file to overwrite does not exist and overwrite is true by default */
			fileCp(av[2],dest,overwrite);  //fileCp() will creat if file not found		
	}
	else{ /* -i or -R options not used */

		if( strcmp(av[1], av[2]) == 0)  /* if filenames are the same, error */
			oops("File names are the same", dest);
		
		strcpy(dest, makeFileDest(av[1], av[2]) );	
		fileCp(av[1],dest, overwrite); 	
	}
	return 0;	
}




/**********************************************
*	function fileCP()
*   copies one file to anoether
*   will return 0 if it did not copy because of overwrite setting
*   returns 1 if file copies sussessfully
*/
int fileCp(char *src, char *dest, bool overwrite){

	int in_fd, out_fd, n_chars;
	char buf[BUFFERSIZE];

					/* open files */
	if ( (in_fd=open(src, O_RDONLY)) == -1 )
                oops("Cannot open ", src);
	
	if( (out_fd=open(dest, O_RDONLY)) == -1){ //try to open
	                                          // if does not exists, creat
		if( (out_fd=creat( dest, COPYMODE)) == -1)	
		       	oops( "Cannot creat", dest);
	}
	else{   /* files exist */ 
		
		if(overwrite){
		// O_TRUNC should clear existing file
		// while keeping ownership and permissions (like cp)
		if ( (out_fd=open(dest, O_WRONLY | O_TRUNC)) == -1)
			oops("Cannot Open ", dest);
		}
		else return 0;
	}
				/* copy files */
	while ( (n_chars = read(in_fd , buf, BUFFERSIZE)) > 0 )
        	if ( write( out_fd, buf, n_chars ) != n_chars )
                	oops("Write error to ", dest);
	if ( n_chars == -1 )                                       
		oops("Read error from ", src);  
                                                           
					/* close files	*/
        if ( close(in_fd) == -1 || close(out_fd) == -1 )
                oops("Error closing files","");

	return 1;

}


/*************************************************
*	function cpRecurse()
*    will cp files from src to a destination directory
*    will return number of successful copies
*    overwrites all file in directory that have the samea name in src dir	
*/
int cpRecurse(char *src,char *dest,bool overwrite){

	char  newdest[NAMEBUF], newsrc[NAMEBUF];
	DIR *srcdir, *testdir;
	struct dirent *dp;
	struct stat info;
	int retval = 0;

	if( (srcdir = opendir(src)) == NULL)
		oops("Could not open directory ", src);
	
	char newdir[NAMEBUF];
        sprintf(newdir, "%s%s", dest, src);
	
	if( (testdir=opendir(newdir)) == NULL ){ /* if dest does not exist */
		if ( mkdir( newdir , 0755) == -1)
			oops("Could not make Directory", dest);
	}
	closedir(testdir);

	while( (dp=readdir(srcdir)) != NULL){

		if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
		continue;	 /*skip . .. */
		
		sprintf(newsrc, "%s%s", src, dp->d_name);
		if( stat(newsrc, &info) == -1)
			oops("stat error","");	
		if( S_ISDIR(info.st_mode)){ /*if src contains dir, call cpRecurse on dir */
		      
			cpRecurse(strcat(newsrc, "/"),dest,overwrite);
		}	 
                if(S_ISREG(info.st_mode)){
		  	sprintf(newdest, "%s%s", dest, newsrc);
	  		retval+= ( fileCp(newsrc,  newdest ,overwrite) );  
		}
			
	}//end while
	
	if ( closedir(srcdir) == -1 )
		oops("Could not cose directory ", src);	
	return retval;
}


/***********************************************
*	function oops() handles errors
*/
void oops(char *s1, char *s2)
{
        fprintf(stderr,"Error: %s ", s1);
        perror(s2);
        exit(1);
}


/**********************************************
*	function makeFileDest()
*   constructs the desination filename
*   which open(3) is able to understand
*/
char* makeFileDest(char *f1,char *dest)
{
	struct stat info;
	static char f2[NAMEBUF];
	if(stat(dest, &info) == -1) return  dest;
	if(!S_ISDIR(info.st_mode)) return dest;
	else{ /* is a directory */
		sprintf(f2, "%s/%s", dest, f1);
		return f2;
	}
}

/**********************************************
*	function makeDest()
*   constructs the desination directory
*   used with: -R option
*/
char* makeDest(char *dest){
	struct stat info;
	
	if( stat(dest, &info) == 0){
		if(S_ISDIR(info.st_mode)) return dest;
		else /* is not a directory */
			oops("Not a directory", dest);
	}else oops("stat error", "");
		
}

