 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdint.h>
 
 uint32_t arena_size;  //variabilele globale:
 void *arena=NULL;     //dimensiunea arenei si pointerul catre arena

 //INITIALIZE
 void* INITIALIZE (uint32_t size)
         { 	  
	  void *p=calloc(size,sizeof(char));
	  return p;
         }


 //FINALIZE
 void FINALIZE()
	{ 	
	 free(arena);
  	 arena=NULL;
	}


 //DUMP
 void DUMP ()
          { 	  
	  uint32_t i;
	  for( i = 0; i < arena_size; i++)
		if ( i%16 == 0 ) 
			{ printf( "%08X\t", i);
		   	  printf( "%02X", *((unsigned char*)(arena)+i));
 			  if ( i == arena_size-1) printf("\n");
		  	  else 			  printf(" ");
		        }

		else if ( (i+1)%16 == 0) 
			{printf( "%02X\n", *((unsigned char*)(arena)+i));
			}

		else { printf( "%02X", *((unsigned char*)(arena)+i));
		       if ( i == arena_size-1) printf("\n");
		       else 
		       	  {	printf(" ");
		       		if (i>=7 && (i-7)%16 == 0 ) printf(" ");
		       	  }
		     }
	  }

 //ALLOC
 void ALLOC ( uint32_t size, uint32_t *index_bloc0, int *ok)
	{ 
	 void *indice;
  	 if (arena_size >= size+12)
  	    {
	     if ( *index_bloc0 == 0 && *ok == 0 ) //caz 1:nu exista niciun bloc
  	    	{ 
		 indice=arena;
	  	 *((uint32_t*)(indice))=0; //index bloc urmator
	  	 *((uint32_t*)(indice)+1)=0; //index bloc anterior
	  	 *((uint32_t*)(indice)+2)=size;
	  	 *ok=1; 
	  	 printf("%u\n", (*index_bloc0)+12);
		}

  	     else if ( (*index_bloc0)>=(size+12) ) //caz 2:inaintea primului bloc
		{ 
		 indice=arena;
		 *((uint32_t*)(indice))=(*index_bloc0); //index bloc urmator
	  	 *((uint32_t*)(indice)+1)=0; //index bloc anterior
	  	 *((uint32_t*)(indice)+2)=size;
		 *index_bloc0=0;
		 //am modificat pentru blocul curent
		 *((unsigned char*)(indice)+*((uint32_t*)(indice))+4)=(*index_bloc0);
		 printf("%u\n", (*index_bloc0)+12);
		 //am modificat pentru bloc urmator
		}
  	    else  // caz 3:intre blocuri/dupa ultimul bloc
		{
	 	 int index=*index_bloc0; //pornesc cu primul
	 	 void *indice=((unsigned char*)(arena)+*index_bloc0);
	 	 int gasit=0; 
	 	 while ( *(uint32_t*)(indice) != 0 &&  //caut pana la ultimul bloc
		  	 gasit == 0 ) 
	 	
		    { //verific portiunea dintre blocuri
		  	if ( *(uint32_t*)(indice)-
			   ( index+12+
			     *((uint32_t*)(indice)+2)) >= (size+12) 
			   )
		    
			{ gasit=1;
			  int index_bloc_nou=(index+12+ *((uint32_t*)(indice)+2));
			  void *indice_bloc_nou=(unsigned char*)(arena)+index_bloc_nou;

			  //modific bloc curent
			  *(uint32_t*)(indice_bloc_nou)=*(uint32_t*)(indice);//bloc urmator
			  *((uint32_t*)(indice_bloc_nou)+1)=index;//bloc anterior
			  *((uint32_t*)(indice_bloc_nou)+2)=size; //size

			  //modific bloc urmator
			  *(uint32_t*)((unsigned char*)(arena)+
					*(uint32_t*)(indice)+4)=index_bloc_nou;

			 //modific bloc anterior
 			 *(uint32_t*)(indice)=index_bloc_nou;
			 printf("%u\n", index_bloc_nou+12);
			 }

		     //actualizez indice si index daca n-am gasit
		     index=*(uint32_t*)(indice);
		     indice=((unsigned char*)(arena)+index);

		    } //aici se termina while de cautare intre blocuri

		
		 //caz 4: dupa ultimul bloc
		 if (gasit == 0 && *(uint32_t*)(indice) == 0 ) //daca n-am gasit	
	 						       //verific dupa ultimul bloc
		  { 
		    if ( arena_size-(index+12+*((uint32_t*)(indice)+2)) >= (size+12) )
		  	{//modific blocul nou
		       	 int index_bloc_nou=(index+12+ *((uint32_t*)(indice)+2));
			 void *indice_bloc_nou=(unsigned char*)(arena)+index_bloc_nou;
			 gasit=1;
 
			 *(uint32_t*)(indice_bloc_nou)=*(uint32_t*)(indice);//bloc urmator
			 *((uint32_t*)(indice_bloc_nou)+1)=index;//bloc anterior
			 *((uint32_t*)(indice_bloc_nou)+2)=size; //size

			 //modific bloc anterior
 			 *(uint32_t*)(indice)=index_bloc_nou;
			 printf("%u\n", index_bloc_nou+12);
		  	}
		 }
	//daca la final n-am gasit
	if (gasit == 0) printf("0\n"); 

	}}
 else printf("0\n"); //daca size depaseste dimensiunea arenei
 } 




 //FREE
 void FREE ( uint32_t index, uint32_t *index_bloc0,int *ok)

 { //acest index este corespunzator sectiunii de date
   //voi lucra cu cel al sectiunii de gestiune
   index=index-12;
   void *indice=((unsigned char*)(arena)+index);

   //actualizez bloc anterior
   //verific daca exista 
   if (*((uint32_t*)(indice)+1) != 0 || (*((uint32_t*)(indice)+1) == 0 && *index_bloc0 == 0 && 
					 index != *index_bloc0))
		{ void *indice_bloc_anterior=(unsigned char*)(arena)+*((uint32_t*)(indice)+1);
		  *(uint32_t*)(indice_bloc_anterior)=*(uint32_t*)(indice);
		 }
   else 
		{//daca e primul din arena, verific sa existe un bloc dupa el
		 //actualizez indexul blocului0
		 if (*(uint32_t*)(indice) != 0 ){ *index_bloc0=*(uint32_t*)(indice);
						   }
		 //sau daca nu exista bloc urmator
		 else  {*index_bloc0=0;
			*ok=0;
			
			}

		}
  //blocul urmator
  if (*(uint32_t*)(indice) != 0 ) //daca exista
		{ void *indice_bloc_urmator=(unsigned char*)(arena)+*((uint32_t*)(indice));
		  *((uint32_t*)(indice_bloc_urmator)+1)=*((uint32_t*)(indice)+1);}
  
  //setez octeti pe 0
  uint32_t octet_final=*((uint32_t*)(indice)+2)+index+12;
  uint32_t i;
  for(i=index;i<octet_final;i++){*((unsigned char*)(arena)+i)=0;}
  }



 //FILL
 void fill (uint32_t index, uint32_t size, uint32_t value)
 { 
   //lucrez cu indexul blocului de gestiune
   index=index-12;
   void *indice=((unsigned char*)(arena)+index);
   uint32_t numar_octeti=*((uint32_t*)(indice)+2);

   //i pleaca de la primul index al blocului de date
   //merge pana la ultimul
   uint32_t i=index+12;
  
   while ( size > 0 ) //cat timp mai am octeti de setat

	{ while ( i < numar_octeti+12+index && size > 0) { *((unsigned char*)(arena)+i)=value;
				                           i++;  
	 			                           size--;
				                         }
	 index = *((uint32_t*)(indice)); //merg la blocul urmator daca exista
	 if (index != 0)
         	{indice=((unsigned char*)(arena)+index); 
	 	 numar_octeti=*((uint32_t*)(indice)+2); 
		 i=index+12;
		}
	 else break; 	 
   
	}

  }


//FUNCTIE PARSARE COMENZI
void parse_command(char* cmd)
{   
    static uint32_t index_bloc0; 
    //indexul primului bloc
    static int ok=0; 
    //ok e 0 daca nu exista niciun bloc, 1 daca exista macar un bloc
    
    
    const char* delims = " \n";

    char* cmd_name = strtok(cmd, delims);
    if (!cmd_name) {
        goto invalid_command;
    }

    //INITIALIZE
    if (strcmp(cmd_name, "INITIALIZE") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);

        arena_size=size; //nr octeti arena	
	{arena=INITIALIZE(size);
 	 index_bloc0=0; //initializez cu 0 indicele de start
	}
    
    //FINALIZE
    } else if (strcmp(cmd_name, "FINALIZE") == 0) {
        					
	{FINALIZE();
	 index_bloc0=0;
	 ok=0;
	}

    //DUMP
    } else if (strcmp(cmd_name, "DUMP") == 0) {
        					
	{ DUMP();

	}

    //ALLOC
    } else if (strcmp(cmd_name, "ALLOC") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        					
	{ uint32_t dimensiune=size; //cati octeti se aloca
	  ALLOC(dimensiune,&index_bloc0,&ok); 

	}

    //FREE
    } else if (strcmp(cmd_name, "FREE") == 0) {
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        					
	{
	FREE(index,&index_bloc0,&ok);}


    //FILL
    } else if (strcmp(cmd_name, "FILL") == 0) {
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        char* value_str = strtok(NULL, delims);
        if (!value_str) {
            goto invalid_command;
        }
        uint32_t value = atoi(value_str);
        
	fill(index,size,value);

    } else if (strcmp(cmd_name, "ALLOCALIGNED") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        //uint32_t size = atoi(size_str);
        char* align_str = strtok(NULL, delims);
        if (!align_str) {
            goto invalid_command;
        }
        //uint32_t align = atoi(align_str);
        // TODO - ALLOCALIGNED

    } else if (strcmp(cmd_name, "REALLOC") == 0) {
        printf("Found cmd REALLOC\n");
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        //uint32_t index = atoi(index_str);
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        //uint32_t size = atoi(size_str);
        // TODO - REALLOC

    } else {
        goto invalid_command;
    }

    return;

invalid_command:
    printf("Invalid command: %s\n", cmd);
    exit(1);
}

int main(void)
{
    ssize_t read;
    char* line = NULL;
    size_t len;

    /* parse input line by line */
    while ((read = getline(&line, &len, stdin)) != -1) {
        /* print every command to stdout */
        printf("%s", line);

        parse_command(line);
    }

    free(line);

    return 0;
}
