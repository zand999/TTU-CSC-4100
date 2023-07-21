#ifndef LEGACY_H
#define LEGACY_H
//extern void k_print(char *string, int string_length, int row, int col);

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



#endif