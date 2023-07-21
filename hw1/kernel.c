void k_clearscr();
extern void k_print(char *string, int string_length, int row, int col);
void print_border(int start_row, int start_col, int end_row, int end_col);

int main(){

    


    //k_print("Whoo I Am Done.", 15, 1 , 1);
    k_clearscr();
    print_border(0,0,24,79);
    k_print("OS 4100", 7, 12, ((79 - 7)/2));

    while(1){
        
    }

    return 0;
}

void k_clearscr(){

    //make vars
    int screenln = 80*25; 
    char blankscreen[screenln]; 
    //fillarray
    for (int i = 0; i < screenln; i++ /*i+=2*/){
        blankscreen[i] = ' ';
        //blankscreen[i+1] = 31;
    }

    k_print(blankscreen, screenln,0,0);

    
    return;
}


void print_border(int start_row, int start_col, int end_row, int end_col){

    int height = end_row - start_row;
    int length = end_col - start_col;

    if(height < 3 || length < 3){
        return;
    }

    char line[length];

    //char edgechar[1] = {'|'};
    
    //create line
    //set background
    /*for (int i = 1; i < 80; i++ i+=2)
    {
        line[i] = '\0';
    }*/

    //add corners
    
    line[0] = '+';

    line[length] = '+';

    //fill with dash
    for (int i = 1; i < length; i+=1)
    {
        line[i] = '-';
    }
    
    
    //print first row
    k_print(line,length+1,start_row,start_col);

    //print edges

    for(int i = 1; i < height ; i++){
        k_print("|",1,start_row + i,start_col);
        k_print("|",1,start_row + i,end_col);
    }

    //print last row
    k_print(line,length+1,end_row,start_col);

    return;
}