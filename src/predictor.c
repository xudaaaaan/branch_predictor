//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Dan Xu";
const char *studentID   = "A53271228";
const char *email       = "d7xu@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //

uint32_t *gshare_bht;  //gshare global history table
uint32_t ghistory;     //global history

uint32_t *local_bht;   //local history table
uint32_t *local_pt;    //local prediction table
uint32_t *global_pt;   //global prediction table
uint32_t *cp;          //choice predictor


/*
For choice predictor:
0: Strongly select local predictor
1: Weakly select local predictor
2: Weakly select global predictor
3: Strongly select global predictor
*/


//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  ghistory = 0;


  if(bpType == GSHARE){
    int num_of_entry = 1<<ghistoryBits;
    gshare_bht = (uint32_t*) malloc(sizeof(uint32_t) * num_of_entry);
    for (int i = 0; i < num_of_entry; i++){
      gshare_bht[i] = WN;
    }
  }
  else if (bpType == TOURNAMENT){
    int num_entry_lpt = 1<<lhistoryBits;
    int num_entry_ght = 1<<ghistoryBits;
    int num_entry_lht = 1<<pcIndexBits;

    local_bht = (uint32_t*) malloc(sizeof(uint32_t) * num_entry_lht);
    local_pt = (uint32_t*) malloc(sizeof(uint32_t) * num_entry_lpt);
    global_pt = (uint32_t*) malloc(sizeof(uint32_t) * num_entry_ght);
    cp = (uint32_t*) malloc(sizeof(uint32_t) * num_entry_ght);

    for(int i = 0; i < num_entry_lht; i++){
      local_bht[i] = 0;
    }
    for(int i = 0; i < num_entry_lpt; i++){
      local_pt[i] = WN;
    }
    for(int i = 0; i < num_entry_ght; i++){
      global_pt[i] = WN;
      cp[i] = 2;
    }
  }
  else if (bpType == CUSTOM){ //Gshare + local predictor
    ghistoryBits = 9;
    pcIndexBits = 10;

    int num_entry_ght = 1<<ghistoryBits;
    int num_entry_lht = 1<<pcIndexBits;

    gshare_bht = (uint32_t*) malloc(sizeof(uint32_t) * num_entry_lht);
    global_pt = (uint32_t*) malloc(sizeof(uint32_t) * num_entry_ght);
    cp = (uint32_t*) malloc(sizeof(uint32_t) * num_entry_ght);

    for(int i = 0; i < num_entry_lht; i++){
      gshare_bht[i] = WN;
    }
    for(int i = 0; i < num_entry_ght; i++){
      global_pt[i] = WN;
      cp[i] = 2;
    }    
  }
  else{
    ;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  uint32_t helper;
  uint32_t pc_to_use;       //gshare
  uint32_t ghistory_to_use; //gshare
  uint32_t xor_result;      //gshare

  uint32_t lhistory_to_use; //tournament
  int local_result;         //tournament
  int global_result;        //tournament
  

  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      helper = 0;
      for(int i = 0; i < ghistoryBits; i++){
        helper = helper | 1 << i;
      }
      pc_to_use = pc & helper;
      ghistory_to_use = ghistory & helper;
      xor_result = pc_to_use ^ ghistory_to_use;
      if(gshare_bht[xor_result] >= WT){
        return TAKEN;
      }
      else return NOTTAKEN;
      

    case TOURNAMENT:
      helper = 0;
      for(int i = 0; i < pcIndexBits; i++){
        helper = helper | 1 << i;
      }
      pc_to_use = pc & helper;

      //Local predictor
      helper = 0;
      for(int i = 0; i < lhistoryBits; i++){
        helper = helper | 1 << i;
      }      
      lhistory_to_use = local_bht[pc_to_use] & helper;
      if (local_pt[lhistory_to_use] >= WT) {
        local_result = TAKEN;
      }
      else local_result = NOTTAKEN;
      
      //global predictor
      helper = 0;
      for(int i = 0; i < ghistoryBits; i++){
        helper = helper | 1 << i;
      }            
      ghistory_to_use = ghistory & helper;
      if(global_pt[ghistory_to_use] >= WT){
        global_result = TAKEN;
      }
      else global_result = NOTTAKEN;

      //choice predictor
      if(cp[ghistory_to_use] >= 2){
        return global_result;
      }
      else return local_result;

      
      
    case CUSTOM:
    //gshare 
      helper = 0;
      for(int i = 0; i < pcIndexBits; i++){
        helper = helper | 1 << i;
      }
      pc_to_use = pc & helper;
      ghistory_to_use = ghistory & helper;
      xor_result = pc_to_use ^ ghistory_to_use;
      if(gshare_bht[xor_result] >= WT){
        local_result = TAKEN;
      }
      else local_result = NOTTAKEN;

      //global predictor
      helper = 0;
      for(int i = 0; i < ghistoryBits; i++){
        helper = helper | 1 << i;
      }            
      ghistory_to_use = ghistory & helper;
      if(global_pt[ghistory_to_use] >= WT){
        global_result = TAKEN;
      }
      else global_result = NOTTAKEN;

      //choice predictor
      if(cp[ghistory_to_use] >= 2){
        return global_result;
      }
      else return local_result;

    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  uint32_t helper;
  uint32_t pc_to_use;       //gshare
  uint32_t ghistory_to_use; //gshare
  uint32_t xor_result;      //gshare

  uint32_t lhistory_to_use; //tournament
  int local_correctness;         //tournament
  int global_correctness;        //tournament
  
  switch(bpType){
    case GSHARE:
      helper = 0;
      for(int i = 0; i < ghistoryBits; i++){
        helper = helper | 1 << i;
      }
      pc_to_use = pc & helper;
      ghistory_to_use = ghistory & helper;
      xor_result = pc_to_use ^ ghistory_to_use;    
      if(outcome == TAKEN && gshare_bht[xor_result] < ST){
        gshare_bht[xor_result]++;
      } 
      if (outcome == NOTTAKEN && gshare_bht[xor_result] > SN){
        gshare_bht[xor_result]--;
      }
      ghistory = ghistory << 1 | outcome;
      return;


    case TOURNAMENT:
      helper = 0;
      for(int i = 0; i < pcIndexBits; i++){
        helper = helper | 1 << i;
      }      
      pc_to_use = pc & helper;

      /*local predictor*/
      helper = 0;
      for(int i = 0; i < lhistory_to_use; i++){
        helper = helper | 1 << i;
      }    
      lhistory_to_use = local_bht[pc_to_use] & helper;
      //Correctness of local predictor
      if (local_pt[lhistory_to_use] >= WT){             
        if (outcome == TAKEN) local_correctness = 1;
        else local_correctness = 0;
      }
      else{
        if(outcome == NOTTAKEN) local_correctness = 1;
        else local_correctness = 0;
      }
      //Train the local predictor
      if (outcome == TAKEN && local_pt[lhistory_to_use] < ST){
        local_pt[lhistory_to_use]++;
      }
      if (outcome == NOTTAKEN && local_pt[lhistory_to_use] > SN){
        local_pt[lhistory_to_use]--;
      }
      //Update local history
      local_bht[pc_to_use] = (local_bht[pc_to_use] << 1 | outcome) & helper;
      
      /*global predictor*/
      helper = 0;
      for(int i = 0; i < ghistoryBits; i++){
        helper = helper | 1 << i;
      }    
      ghistory_to_use = ghistory & helper;
      //Correctness of the global predictor
      if(global_pt[ghistory_to_use] >= WT){
        if(outcome == TAKEN) global_correctness = 1;
        else global_correctness = 0;
      }
      else{
        if(outcome == NOTTAKEN) global_correctness = 1;
        else global_correctness = 0;
      }      
      //Train the global predictor
      if (outcome == TAKEN && global_pt[ghistory_to_use] < ST){
        global_pt[ghistory_to_use]++;
      }
      if (outcome == NOTTAKEN && global_pt[ghistory_to_use] > SN){
        global_pt[ghistory_to_use]--;
      }      
      //Train the choice predictor
      if((global_correctness - local_correctness) == 1 && cp[ghistory_to_use] < 3){
        cp[ghistory_to_use]++;
      }
      if((global_correctness - local_correctness) == -1 && cp[ghistory_to_use] > 0){
        cp[ghistory_to_use]--;
      }
      //Update the global history
      ghistory = (ghistory << 1 | outcome) & helper;
      return;

    case CUSTOM:
    /*gshare predictor*/
      helper = 0;
      for(int i = 0; i < pcIndexBits; i++){
        helper = helper | 1 << i;
      }      
      pc_to_use = pc & helper;
      ghistory_to_use = ghistory & helper;
      xor_result = pc_to_use ^ ghistory_to_use;    
    
      if (gshare_bht[xor_result] >= WT){             
        if (outcome == TAKEN) local_correctness = 1;
        else local_correctness = 0;
      }
      else{
        if(outcome == NOTTAKEN) local_correctness = 1;
        else local_correctness = 0;
      }
      //Train the gshare predictor
      if (outcome == TAKEN && gshare_bht[xor_result] < ST){
        gshare_bht[xor_result]++;
      }
      if (outcome == NOTTAKEN && gshare_bht[xor_result] > SN){
        gshare_bht[xor_result]--;
      }
      
      /*global predictor*/
      helper = 0;
      for(int i = 0; i < ghistoryBits; i++){
        helper = helper | 1 << i;
      }      
      ghistory_to_use = ghistory & helper;
      //Correctness of the global predictor
      if(global_pt[ghistory_to_use] >= WT){
        if(outcome == TAKEN) global_correctness = 1;
        else global_correctness = 0;
      }
      else{
        if(outcome == NOTTAKEN) global_correctness = 1;
        else global_correctness = 0;
      }      
      //Train the global predictor
      if (outcome == TAKEN && global_pt[ghistory_to_use] < ST){
        global_pt[ghistory_to_use]++;
      }
      if (outcome == NOTTAKEN && global_pt[ghistory_to_use] > SN){
        global_pt[ghistory_to_use]--;
      }      
      //Train the choice predictor
      if((global_correctness - local_correctness) == 1 && cp[ghistory_to_use] < 3){
        cp[ghistory_to_use]++;
      }
      if((global_correctness - local_correctness) == -1 && cp[ghistory_to_use] > 0){
        cp[ghistory_to_use]--;
      }
      //Update the global history
      ghistory = ghistory << 1 | outcome;
      return;
    default:
      break;

  }


}
