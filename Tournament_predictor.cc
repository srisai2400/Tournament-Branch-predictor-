//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                              //
//   Author: Anusha Puttaswamy, Sri Sai Sumanth				                                                        //
//   Date: 25 Feb 2024 		                                                                                                //
//   Description: Implementation of Tournamment Branch predictor                                                          //
//                                                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "predictor.h"

static unsigned short int local_history_table [1024];                   //local history table
static unsigned short int global_predictor [4096];                      //global predictor
static unsigned short int local_predictor [1024];                       //local predictor
static unsigned short int choice_predictor [4096];                      //choice predictor
static unsigned short int path_history;                                 //path history to store 12 bits of global branch history.  
static unsigned short int lht_10;                                       //10bits bits of local history index to local predictor.
static unsigned short int lp_3;                                    	  //3 bit saturation count for local predictor.
static unsigned short int choice_2;                                     //2 bit saturation count for global predictor.
static unsigned short int gp_2;                                         //2 bit saturation count for global predictor.
static unsigned short int mux_lp, mux_gp, mux_out;                      //mux inputs from local, global predictor and mux output.
static bool prediction;                                           //predition, which is true for taken and false for not taken.
     
    bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
        {
           
	     unsigned int pc_addr = br->instruction_addr;
             unsigned int pc_10bits;
             pc_10bits = (pc_addr >> 2)& 1023;
             pc_10bits = (pc_10bits & 1023);                    //bit masking to get 10 bits [11:2] from the PC address.
             lht_10 = local_history_table[pc_10bits];           //indexing the 10 bits of pc to the local history table 
             lht_10 = (lht_10 & 1023);
             lp_3 = local_predictor[lht_10];                   //indexing the 10 bits of histrory table to the local predictor.
             lp_3 = (lp_3 & 7);                 
             path_history =  (path_history & 4095);          	//indexing the path history to both global and choice predictor. 
             gp_2 = global_predictor[path_history];
             gp_2 = (gp_2 & 3);
             choice_2 = choice_predictor[path_history];
             choice_2 = (choice_2 & 3); 
          
          
          if(br->is_call || !(br->is_conditional) || br-> is_return)
          {
              prediction = true;
          }
         else if(br->is_conditional)
          {                                                                                    //local predictor.
             if((lp_3 == 0) || (lp_3 == 1) || (lp_3 == 2) || (lp_3 == 3))
               {
                   mux_lp = 0;                                                                 //local not taken
               }
             else if((lp_3 == 4) || (lp_3 == 5) || (lp_3 == 6) || (lp_3 == 7)) 
               {
                mux_lp = 1;                                                                    // local taken
               }  
             if((gp_2 == 0) || (gp_2 == 1))                                                  //global predictor
               {                                                                                      
                  mux_gp = 0;                                                                 //global not taken
               }
             else if ((gp_2 == 2) || (gp_2 == 3))
              {                                                                               //global taken                             
                 mux_gp = 1;
              }
                                                                                  
             if((choice_2 == 0) || (choice_2 == 1))                                        //choice predictor
              {
               mux_out = mux_lp;                                                            //choice predictor selects local if MSB bit 
              }                                                                             // saturation count is 0
             else if ((choice_2 == 2) || (choice_2 == 3))
              {                                                                             //else if MSB bit is 1
                mux_out = mux_gp;                                                           //global predictor is selected.
              } 
               
             if(mux_out == 0)
             {                                                                              //if mux out is 1, prediction is taken 
               prediction = false;
             }                                                                              //else prediction is not taken.
             else if (mux_out == 1)
              {
                  prediction = true;
              }
           }
         
                
            return prediction;                                          // true for taken, false for not taken
        }

// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
    
    
    
    
 void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{
		
        
        unsigned int pc_addr = br->instruction_addr;
        unsigned int pc_10bits;
        pc_10bits = (pc_addr >> 2)& 1023;
        pc_10bits = (pc_10bits & 1023);            //masking into 10bits
        
        
          if(br->is_conditional)
          {        
                if(mux_lp == taken)      
                {
                  if(mux_lp == 1)                  //move towards strongly taken
                  {
                    if(lp_3 == 0)
                    lp_3 = 1;
                    else if( lp_3 == 1)
                    lp_3 = 2;
                    else if( lp_3 == 2)
                    lp_3 = 3;
                    else if( lp_3 == 3)
                    lp_3 = 4;
                    else if( lp_3 == 4)
                    lp_3 = 5;
                    else if( lp_3 == 5)
                    lp_3 = 6;
                    else if( lp_3 == 6)
                    lp_3 = 7;
                    else if( lp_3 == 7)
                    lp_3 = 7;  
                  }
                  else if(mux_lp == 0)             ///move towards strongly not taken
                  {
                    if(lp_3 == 0)
                    lp_3 = 0;
                    else if( lp_3 == 1)
                    lp_3 = 0;
                    else if( lp_3 == 2)
                    lp_3 = 1;
                    else if( lp_3 == 3)
                    lp_3 = 2;
                    else if( lp_3 == 4)
                    lp_3 = 3;
                    else if( lp_3 == 5)
                    lp_3 = 4;
                    else if( lp_3 == 6)
                    lp_3 = 5;
                    else if( lp_3 == 7)
                    lp_3 = 6; 
                  }
                }
                
                else if(mux_lp != taken)
                {
                    if(mux_lp == 1)                 //move towards strongly not taken
                    {
                    if(lp_3 == 0)
                    lp_3 = 0;
                    else if( lp_3 == 1)
                    lp_3 = 0;
                    else if( lp_3 == 2)
                    lp_3 = 1;
                    else if( lp_3 == 3)
                    lp_3 = 2;
                    else if( lp_3 == 4)
                    lp_3 = 3;
                    else if( lp_3 == 5)
                    lp_3 = 4;
                    else if( lp_3 == 6)
                    lp_3 = 5;
                    else if( lp_3 == 7)
                    lp_3 = 6;  
                    }
                    else if(mux_lp == 0)           ///move towards strongly taken
                    {
                    if(lp_3 == 0)
                    lp_3 = 1;
                    else if( lp_3 == 1)
                    lp_3 = 2;
                    else if( lp_3 == 2)
                    lp_3 = 3;
                    else if( lp_3 == 3)
                    lp_3 = 4;
                    else if( lp_3 == 4)
                    lp_3 = 5;
                    else if( lp_3 == 5)
                    lp_3 = 6;
                    else if( lp_3 == 6)
                    lp_3 = 7;
                    else if( lp_3 == 7)
                    lp_3 = 7; 
                    }  
                }
                
                if( mux_gp == taken)
                {
                    if(mux_gp == 1)                //move towards strongly taken
                    {
                     if(gp_2 == 0)
                     gp_2 = 1;
                     else if(gp_2 == 1)
                     gp_2 = 2;
                     else if(gp_2 == 2)
                     gp_2 = 3;
                     else if(gp_2 == 3)
                     gp_2 = 3;
                    }
                
                  else if(mux_gp == 0)            //move towards strongly not taken
                  {
                     if(gp_2 == 0)
                     gp_2 = 0;
                     else if(gp_2 == 1)
                     gp_2 = 0;
                     else if(gp_2 == 2)
                     gp_2 = 1;
                     else if(gp_2 == 3)
                     gp_2 = 2;
                  }
                }
                
                else if(mux_gp != taken)
                {
                    if(mux_gp == 1)            //move towards strongly not taken
                    {
                     if(gp_2 == 0)
                     gp_2 = 0;
                     else if(gp_2 == 1)
                     gp_2 = 0;
                     else if(gp_2 == 2)
                     gp_2 = 1;
                     else if(gp_2 == 3)
                     gp_2 = 2;
                    }
                
                  else if(mux_gp == 0)        //move towards strongly taken
                  {
                     if(gp_2 == 0)
                     gp_2 = 1;
                     else if(gp_2 == 1)
                     gp_2 = 2;
                     else if(gp_2 == 2)
                     gp_2 = 3;
                     else if(gp_2 == 3)
                     gp_2 = 3;
                  }
                }
                         
                
                  global_predictor[path_history] = gp_2;
                  local_predictor[lht_10] = lp_3;             

                  
                  
                if(taken == prediction)
                {                            
                    if(( choice_2 == 0 || choice_2 == 1))
                   {
                    if (mux_gp == mux_lp)
                    {   
                    if( choice_2 == 0)
                    choice_2 = 0;

                    else if( choice_2 == 1)
                    choice_2 = 1;
                    }  
                    else if (mux_gp != mux_lp)
                    {    
                    if( choice_2 == 0)
                    choice_2 = 0;

                    else if( choice_2 == 1)
                    choice_2 = 0;
                    }
                   }

                   else if (( choice_2 == 2 || choice_2 == 3))
                   {
                    if (mux_lp == mux_gp)
                    { 
                    if( choice_2 == 3)
                    choice_2 = 3;

                    else if( choice_2 == 2)
                    choice_2 = 2;
                    }  
                    else if (mux_lp != mux_gp)
                    { 
                    if( choice_2 == 3)
                    choice_2 = 3;

                    else if( choice_2 == 2)
                    choice_2 = 3;
                    }
                   }
                  
                  choice_predictor[path_history] = choice_2;
                }
                
                 else if(taken != prediction)                     // prediction mismatches
               { 
                   if(( choice_2 == 0 || choice_2 == 1))
                   {
                    if (mux_gp == mux_lp)
                    {   
                    if( choice_2 == 0)
                    choice_2 = 0;

                    else if( choice_2 == 1)
                    choice_2 = 1;
                    }  
                    else if (mux_gp != mux_lp)
                    {    
                    if( choice_2 == 0)
                    choice_2 = 1;

                    else if( choice_2 == 1)
                    choice_2 = 2;
                    }
                   }

                   else if (( choice_2 == 2 || choice_2 == 3))
                   {
                    if (mux_lp == mux_gp)
                    { 
                    if( choice_2 == 3)
                    choice_2 = 3;

                    else if( choice_2 == 2)
                    choice_2 = 2;
                    }  
                    else if (mux_lp != mux_gp)
                    { 
                    if( choice_2 == 3)
                    choice_2 = 2;

                    else if( choice_2 == 2)
                    choice_2 = 1;
                    }
                   }
                  choice_predictor[path_history] = choice_2; 
                }
                
            }  
            
                  lht_10 = lht_10 << 1;                            // local history table updation 
                  if (taken == 1)                                   //actually taken
                    {
                     lht_10 = (lht_10 | 1);                       // add 1 to lsb
                    }
                  else if (taken == 0)
                    {
                     lht_10 = (lht_10 & (~1));                    // add 0 to lsb
                    }                  
                  local_history_table[pc_10bits] = lht_10;
                  
                  
            
                                                                  // update path history                  
                  path_history = path_history << 1;
                  if (taken == 1)
                    {               
                       path_history = (path_history | 1);
                    }                                           // add 1 to lsb
                  else
                    {
                       path_history = (path_history & (~1)); 
                    }        
              

}