#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

//Function prototypes
void read_input();
void parse_input();
void shell();
char* mapKey(char *key);
int find_key(char *stuff);
void checkForDollarSign();
void execute_shell_builtin();
void on_child_exit();
void execute_command();
void execute_outside_shell();

char **args;

int number_of_commands=0;

char *keys[100];

char *values[100];

int key_val_counter=0;





char* mapKey(char *key){

    //printf("%s ",key);
    for(int i=0;i<key_val_counter;i++){
        if(!(strcmp(key,keys[i]))){
            return values[i];
        }
    }
    return "";
}


int find_key(char *stuff){
    for(int i=0;i<key_val_counter;i++){
        if(!(strcmp(stuff,keys[i]))){
            return i;
        }
    }
    return -1;
}



void checkForDollarSign(){
    for(int i=1;i<number_of_commands;i++){
        int cnt=0;
        while(args[i][cnt]!='\0'){
            if(args[i][cnt]=='$'){
                cnt++;
                char *key=NULL;
                int key_counter=0;
                while(args[i][cnt]!='\0'){
                    key=(char *)realloc(key, (key_counter + 2) * sizeof(char));
                    key[key_counter++]=args[i][cnt];
                    cnt++;
                }

                char *sent=malloc(100*sizeof(sent));
                sent=mapKey(key);
                char *token = strtok(sent, " ");
                int arg_index = i;

                // Replace elements of args with tokenized strings
                while (token != NULL) {
                    args[arg_index] = (char *)malloc((strlen(token) + 1) * sizeof(char));
                    strcpy(args[arg_index], token);
                    arg_index++;
                    token = strtok(NULL, " ");
                }

                
            }
            cnt++;
        }

    }
}




void execute_shell_builtin(){
    if (!strcmp(args[0],"cd")) {
        if(number_of_commands>1){
            chdir(args[1]);
        }
    }
    else if(!strcmp(args[0],"echo")){
        
        for(int i=1;i<number_of_commands;i++){

            //find if the sentence contains $ or not

            int cnt=0;

            char *sent=NULL;

            while(args[i][cnt]!='\0'){
            
                if(args[i][cnt]!='$'){
                    //printing any sentence comes after echo unless it's not after dollar sign

                    sent=(char *)realloc(sent, (cnt + 2) * sizeof(char));
                    printf("%c",args[i][cnt]);
                    cnt++;
                }
                else{
                    //Dollar sign is found, then
                    //skip dollar sign
                    cnt++;
                    
                    //Parsing the key to map it to its value
                    int key_counter=0;
                    char *key=NULL;

                    while(args[i][cnt]!='\0'){
                        key=(char *)realloc(key, (key_counter + 2) * sizeof(char));
                        key[key_counter++]=args[i][cnt];
                        cnt++;
                    }
                    
                    if(sent==NULL){
                        sent=(char *)realloc(sent, (cnt + 2) * sizeof(char));
                    }
                    sent=mapKey(key);
                }
               
            }

            printf("%s ",sent);
            
        }
        printf("\n");

        
    }
    else if (!strcmp(args[0],"export")){
        int count1=0;
        //new string to assign a key
        //from the begining until reaching '='
        
        while(args[1][count1]!='\0' && args[1][count1]!='='){
            count1++;
        }
        
        //Allocat the new key by its size
        
        char *new_key = (char *)malloc((count1 + 1) * sizeof(char));

        //assign the key
        for (int i = 0; i < count1; i++) {
            new_key[i] = args[1][i];
        }

        //specify the end of the string
        new_key[count1]='\0';

        //skip '=' character
        count1++;

        //the start of the value
        int count2=count1;
        
        while(args[1][count1]!='\0'){
            count1++;
        }

        //getting the value mapped to the key
        char *new_val = (char *)malloc(100 * sizeof(char));
        

        int i = 0;
        for (; count2<count1; i++) {
            new_val[i] = args[1][count2++];
        }

        
        int last_index=i;
        
        new_val[last_index++]=' ';
        for(int k=2;k<number_of_commands;k++){
            int j=0;
            while(args[k][j]!='\0'){
                new_val[last_index++]=args[k][j];
                j++;
            }
            new_val[last_index++]=' ';
        }
        new_val[last_index-1]='\0';

    

        //checking if the key is already used
        int idx=find_key(new_key);

        //Overwriting in the same key
        if(idx!=-1){
            strcpy(values[idx], new_val);
            return;
        }

        keys[key_val_counter]=new_key;
        values[key_val_counter]=new_val;

        key_val_counter++;

        //remove pointers
    }

}



void on_child_exit(){
    //Once child is terminated, sentence is appended to log file
    const char *filename="log file.txt";
    FILE *fp=fopen(filename,"a");
    if(fp==NULL){
        printf("Error opening file.\n");
    }
    fprintf(fp,"%s\n","Child process terminated");
    fclose(fp);
}


//Handling if commands need to work in the background
void execute_outside_shell(){
    int child_id=fork();
    
    if(child_id<0){
        perror("fork failed");
        exit(1);
    }
    else if(child_id==0){
        execvp(args[0],args);
        printf("Command execution failed\n");
    }
    else{
        signal(SIGCHLD,on_child_exit);
    }
    
}

//Shell blocks until the command completes and,
//if the child creation failed or the command is unknown so it pops an error message
void execute_command(){

    int child_id=fork();
    if(child_id<0){
        perror("fork failed");
        exit(1);
    }
    else if(child_id==0){
        checkForDollarSign();
        execvp(args[0],args);
        printf("Command execution failed\n");
    }
    else{
        waitpid(child_id,NULL,0);
    }
    
}


void parse_input(char *input) {
    
    char *  p    = strtok (input, " ");
    int n_spaces = 0, i;
    /* split string and append tokens to args by reallocating it */
    
    while (p) {
      args = realloc (args, sizeof (char*) * ++n_spaces);
      if (args == NULL)
        exit (-1); 
        args[n_spaces-1] = p;
        p = strtok (NULL, " ");
    }
    args = realloc (args, sizeof (char*) * (n_spaces+1));
    args[n_spaces] = 0;
    
    number_of_commands=n_spaces;

    free(p);
}

void read_input(char *input) {
    printf("Enter input: ");
    fgets(input, 300, stdin); 
    input[strcspn(input, "\n")] = '\0'; 
    for(int i=0; input[i]; i++){ 
        input[i]=tolower(input[i]);
    }
}



void shell() {
    char input[300];
    
    while(1) {
        read_input(input);
        
        if(strcmp(input, "exit") == 0) {
            break;
        }
        

        parse_input(input);

        if(args[0] != NULL) {
            if(strcmp(args[0], "cd") == 0 || strcmp(args[0], "echo") == 0 || strcmp(args[0], "export") == 0) {
                execute_shell_builtin();
            } 
            else if(number_of_commands > 1 && strcmp(args[number_of_commands - 1], "&") == 0){
                //Removing '&' from args
                args[number_of_commands - 1]=NULL;
                number_of_commands--;
                execute_outside_shell();
            }
            else {
                execute_command();
            }
        }
    }
}

  

int main() {

    shell();
}